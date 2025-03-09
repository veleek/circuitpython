// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2022 Scott Shawcroft for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/usb/core/Device.h"

#include "tusb_config.h"

#include "lib/tinyusb/src/host/hcd.h"
#include "lib/tinyusb/src/host/usbh.h"
#include "py/runtime.h"
#include "shared/runtime/interrupt_char.h"
#include "shared-bindings/usb/core/__init__.h"
#include "shared-bindings/usb/util/__init__.h"
#include "shared-module/usb/utf16le.h"
#include "supervisor/shared/tick.h"
#include "supervisor/usb.h"

// Track what device numbers are mounted. We can't use tuh_ready() because it is
// true before enumeration completes and TinyUSB drivers are started.
static size_t _mounted_devices = 0;

void tuh_mount_cb(uint8_t dev_addr) {
    _mounted_devices |= 1 << dev_addr;
}

void tuh_umount_cb(uint8_t dev_addr) {
    _mounted_devices &= ~(1 << dev_addr);
}

static xfer_result_t _xfer_result;
static size_t _actual_len;
bool common_hal_usb_core_device_construct(usb_core_device_obj_t *self, uint8_t device_address) {
    if (!tuh_inited()) {
        mp_raise_RuntimeError(MP_ERROR_TEXT("No usb host port initialized"));
    }

    if (device_address == 0 || device_address > CFG_TUH_DEVICE_MAX + CFG_TUH_HUB) {
        return false;
    }
    if ((_mounted_devices & (1 << device_address)) == 0) {
        return false;
    }
    self->device_address = device_address;
    self->first_langid = 0;
    _xfer_result = 0xff;
    return true;
}

void common_hal_usb_core_device_deinit(usb_core_device_obj_t *self) {
    // TODO: Close all of the endpoints we've opened. Some drivers store state
    // for each open endpoint. If we don't close them, then we'll leak memory.
    // Waiting for TinyUSB to add this.
}

uint16_t common_hal_usb_core_device_get_idVendor(usb_core_device_obj_t *self) {
    uint16_t vid;
    uint16_t pid;
    tuh_vid_pid_get(self->device_address, &vid, &pid);
    return vid;
}

uint16_t common_hal_usb_core_device_get_idProduct(usb_core_device_obj_t *self) {
    uint16_t vid;
    uint16_t pid;
    tuh_vid_pid_get(self->device_address, &vid, &pid);
    return pid;
}

static void _transfer_done_cb(tuh_xfer_t *xfer) {
    // Store the result so we stop waiting for the transfer.
    _xfer_result = xfer->result;
    // The passed in xfer is not the original one we passed in, so we need to
    // copy any info out that we want (like actual_len.)
    _actual_len = xfer->actual_len;
}

static bool _wait_for_callback(void) {
    while (!mp_hal_is_interrupted() &&
           _xfer_result == 0xff) {
        // The background tasks include TinyUSB which will call the function
        // we provided above. In other words, the callback isn't in an interrupt.
        RUN_BACKGROUND_TASKS;
    }
    xfer_result_t result = _xfer_result;
    _xfer_result = 0xff;
    return result == XFER_RESULT_SUCCESS;
}

static mp_obj_t _get_string(const uint16_t *temp_buf) {
    size_t utf16_len = ((temp_buf[0] & 0xff) - 2) / sizeof(uint16_t);
    if (utf16_len == 0) {
        return mp_const_none;
    }
    return utf16le_to_string(temp_buf + 1, utf16_len);
}

static void _get_langid(usb_core_device_obj_t *self) {
    if (self->first_langid != 0) {
        return;
    }
    // Two control bytes and one uint16_t language code.
    uint16_t temp_buf[2];
    if (!tuh_descriptor_get_string(self->device_address, 0, 0, temp_buf, sizeof(temp_buf), _transfer_done_cb, 0) ||
        !_wait_for_callback()) {
        return;
    }
    self->first_langid = temp_buf[1];
}

mp_obj_t common_hal_usb_core_device_get_serial_number(usb_core_device_obj_t *self) {
    uint16_t temp_buf[127];
    _get_langid(self);
    if (!tuh_descriptor_get_serial_string(self->device_address, self->first_langid, temp_buf, sizeof(temp_buf), _transfer_done_cb, 0) ||
        !_wait_for_callback()) {
        return mp_const_none;
    }
    return _get_string(temp_buf);
}

mp_obj_t common_hal_usb_core_device_get_product(usb_core_device_obj_t *self) {
    uint16_t temp_buf[127];
    _get_langid(self);
    if (!tuh_descriptor_get_product_string(self->device_address, self->first_langid, temp_buf, sizeof(temp_buf), _transfer_done_cb, 0) ||
        !_wait_for_callback()) {
        return mp_const_none;
    }
    return _get_string(temp_buf);
}

mp_obj_t common_hal_usb_core_device_get_manufacturer(usb_core_device_obj_t *self) {
    uint16_t temp_buf[127];
    _get_langid(self);
    if (!tuh_descriptor_get_manufacturer_string(self->device_address, self->first_langid, temp_buf, sizeof(temp_buf), _transfer_done_cb, 0) ||
        !_wait_for_callback()) {
        return mp_const_none;
    }
    return _get_string(temp_buf);
}


mp_int_t common_hal_usb_core_device_get_bus(usb_core_device_obj_t *self) {
    hcd_devtree_info_t devtree;
    hcd_devtree_get_info(self->device_address, &devtree);
    return devtree.rhport;
}

mp_obj_t common_hal_usb_core_device_get_port_numbers(usb_core_device_obj_t *self) {
    hcd_devtree_info_t devtree;
    hcd_devtree_get_info(self->device_address, &devtree);
    if (devtree.hub_addr == 0) {
        return mp_const_none;
    }
    // USB allows for 5 hubs deep chaining. So we're at most 5 ports deep.
    mp_obj_t ports[5];
    size_t port_count = 0;
    while (devtree.hub_addr != 0 && port_count < MP_ARRAY_SIZE(ports)) {
        // Reverse the order of the ports so most downstream comes last.
        ports[MP_ARRAY_SIZE(ports) - 1 - port_count] = MP_OBJ_NEW_SMALL_INT(devtree.hub_port);
        port_count++;
        hcd_devtree_get_info(devtree.hub_addr, &devtree);
    }
    return mp_obj_new_tuple(port_count, ports + (MP_ARRAY_SIZE(ports) - port_count));
}

mp_int_t common_hal_usb_core_device_get_speed(usb_core_device_obj_t *self) {
    hcd_devtree_info_t devtree;
    hcd_devtree_get_info(self->device_address, &devtree);
    switch (devtree.speed) {
        case TUSB_SPEED_HIGH:
            return PYUSB_SPEED_HIGH;
        case TUSB_SPEED_FULL:
            return PYUSB_SPEED_FULL;
        case TUSB_SPEED_LOW:
            return PYUSB_SPEED_LOW;
        default:
            return 0;
    }
}

void common_hal_usb_core_device_set_configuration(usb_core_device_obj_t *self, mp_int_t configuration) {
    // We assume that the config index is one less than the value.
    uint8_t config_index = configuration - 1;
    // Get the configuration descriptor and cache it. We'll use it later to open
    // endpoints.

    // Get only the config descriptor first.
    tusb_desc_configuration_t desc;
    if (!tuh_descriptor_get_configuration(self->device_address, config_index, &desc, sizeof(desc), _transfer_done_cb, 0) ||
        !_wait_for_callback()) {
        return;
    }

    // Get the config descriptor plus interfaces and endpoints.
    self->configuration_descriptor = m_realloc(self->configuration_descriptor, desc.wTotalLength);
    if (!tuh_descriptor_get_configuration(self->device_address, config_index, self->configuration_descriptor, desc.wTotalLength, _transfer_done_cb, 0) ||
        !_wait_for_callback()) {
        return;
    }
    tuh_configuration_set(self->device_address, configuration, _transfer_done_cb, 0);
    _wait_for_callback();
}

static size_t _xfer(tuh_xfer_t *xfer, mp_int_t timeout) {
    _xfer_result = 0xff;
    xfer->complete_cb = _transfer_done_cb;
    if (!tuh_edpt_xfer(xfer)) {
        mp_raise_usb_core_USBError(NULL);
        return 0;
    }
    uint32_t start_time = supervisor_ticks_ms32();
    while ((timeout == 0 || supervisor_ticks_ms32() - start_time < (uint32_t)timeout) &&
           !mp_hal_is_interrupted() &&
           _xfer_result == 0xff) {
        // The background tasks include TinyUSB which will call the function
        // we provided above. In other words, the callback isn't in an interrupt.
        RUN_BACKGROUND_TASKS;
    }
    if (mp_hal_is_interrupted()) {
        tuh_edpt_abort_xfer(xfer->daddr, xfer->ep_addr);
        return 0;
    }
    xfer_result_t result = _xfer_result;
    _xfer_result = 0xff;
    if (result == XFER_RESULT_STALLED) {
        mp_raise_usb_core_USBError(MP_ERROR_TEXT("Pipe error"));
    }
    if (result == 0xff) {
        tuh_edpt_abort_xfer(xfer->daddr, xfer->ep_addr);
        mp_raise_usb_core_USBTimeoutError();
    }
    if (result == XFER_RESULT_SUCCESS) {
        return _actual_len;
    }

    return 0;
}

static bool _open_endpoint(usb_core_device_obj_t *self, mp_int_t endpoint) {
    bool endpoint_open = false;
    size_t open_size = sizeof(self->open_endpoints);
    size_t first_free = open_size;
    for (size_t i = 0; i < open_size; i++) {
        if (self->open_endpoints[i] == endpoint) {
            endpoint_open = true;
        } else if (first_free == open_size && self->open_endpoints[i] == 0) {
            first_free = i;
        }
    }
    if (endpoint_open) {
        return true;
    }

    if (self->configuration_descriptor == NULL) {
        mp_raise_usb_core_USBError(MP_ERROR_TEXT("No configuration set"));
        return false;
    }

    tusb_desc_configuration_t *desc_cfg = (tusb_desc_configuration_t *)self->configuration_descriptor;

    uint32_t total_length = tu_le16toh(desc_cfg->wTotalLength);
    uint8_t const *desc_end = ((uint8_t const *)desc_cfg) + total_length;
    uint8_t const *p_desc = tu_desc_next(desc_cfg);

    // parse each interfaces
    while (p_desc < desc_end) {
        if (TUSB_DESC_ENDPOINT == tu_desc_type(p_desc)) {
            tusb_desc_endpoint_t const *desc_ep = (tusb_desc_endpoint_t const *)p_desc;
            if (desc_ep->bEndpointAddress == endpoint) {
                break;
            }
        }

        p_desc = tu_desc_next(p_desc);
    }
    if (p_desc >= desc_end) {
        return false;
    }
    tusb_desc_endpoint_t const *desc_ep = (tusb_desc_endpoint_t const *)p_desc;

    bool open = tuh_edpt_open(self->device_address, desc_ep);
    if (open) {
        self->open_endpoints[first_free] = endpoint;
    }
    return open;
}

mp_int_t common_hal_usb_core_device_write(usb_core_device_obj_t *self, mp_int_t endpoint, const uint8_t *buffer, mp_int_t len, mp_int_t timeout) {
    if (!_open_endpoint(self, endpoint)) {
        mp_raise_usb_core_USBError(NULL);
        return 0;
    }
    tuh_xfer_t xfer;
    xfer.daddr = self->device_address;
    xfer.ep_addr = endpoint;
    xfer.buffer = (uint8_t *)buffer;
    xfer.buflen = len;
    return _xfer(&xfer, timeout);
}

mp_int_t common_hal_usb_core_device_read(usb_core_device_obj_t *self, mp_int_t endpoint, uint8_t *buffer, mp_int_t len, mp_int_t timeout) {
    if (!_open_endpoint(self, endpoint)) {
        mp_raise_usb_core_USBError(NULL);
        return 0;
    }
    tuh_xfer_t xfer;
    xfer.daddr = self->device_address;
    xfer.ep_addr = endpoint;
    xfer.buffer = buffer;
    xfer.buflen = len;
    return _xfer(&xfer, timeout);
}

mp_int_t common_hal_usb_core_device_ctrl_transfer(usb_core_device_obj_t *self,
    mp_int_t bmRequestType, mp_int_t bRequest,
    mp_int_t wValue, mp_int_t wIndex,
    uint8_t *buffer, mp_int_t len, mp_int_t timeout) {
    // Timeout is in ms.

    tusb_control_request_t request = {
        .bmRequestType = bmRequestType,
        .bRequest = bRequest,
        .wValue = wValue,
        .wIndex = wIndex,
        .wLength = len
    };
    tuh_xfer_t xfer = {
        .daddr = self->device_address,
        .ep_addr = 0,
        .setup = &request,
        .buffer = buffer,
        .complete_cb = _transfer_done_cb,
    };

    _xfer_result = 0xff;

    if (!tuh_control_xfer(&xfer)) {
        mp_raise_usb_core_USBError(NULL);
        return 0;
    }
    uint32_t start_time = supervisor_ticks_ms32();
    while ((timeout == 0 || supervisor_ticks_ms32() - start_time < (uint32_t)timeout) &&
           !mp_hal_is_interrupted() &&
           _xfer_result == 0xff) {
        // The background tasks include TinyUSB which will call the function
        // we provided above. In other words, the callback isn't in an interrupt.
        RUN_BACKGROUND_TASKS;
    }
    if (mp_hal_is_interrupted()) {
        tuh_edpt_abort_xfer(xfer.daddr, xfer.ep_addr);
        return 0;
    }
    xfer_result_t result = _xfer_result;
    _xfer_result = 0xff;
    if (result == XFER_RESULT_STALLED) {
        mp_raise_usb_core_USBError(MP_ERROR_TEXT("Pipe error"));
    }
    if (result == 0xff) {
        tuh_edpt_abort_xfer(xfer.daddr, xfer.ep_addr);
        mp_raise_usb_core_USBTimeoutError();
    }
    if (result == XFER_RESULT_SUCCESS) {
        return len;
    }

    return 0;
}

bool common_hal_usb_core_device_is_kernel_driver_active(usb_core_device_obj_t *self, mp_int_t interface) {
    #if CIRCUITPY_USB_KEYBOARD_WORKFLOW
    if (usb_keyboard_in_use(self->device_address, interface)) {
        return true;
    }
    #endif
    return false;
}

void common_hal_usb_core_device_detach_kernel_driver(usb_core_device_obj_t *self, mp_int_t interface) {
    #if CIRCUITPY_USB_KEYBOARD_WORKFLOW
    usb_keyboard_detach(self->device_address, interface);
    #endif
}

void common_hal_usb_core_device_attach_kernel_driver(usb_core_device_obj_t *self, mp_int_t interface) {
    #if CIRCUITPY_USB_KEYBOARD_WORKFLOW
    usb_keyboard_attach(self->device_address, interface);
    #endif
}
