// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2022 Scott Shawcroft for Adafruit Industries
//
// SPDX-License-Identifier: MIT

// This file uses method signatures and comments derived from the PyUSB code
// that has the below BSD-3 license.
/* Copyright 2009-2017 Wander Lairson Costa
 * Copyright 2009-2021 PyUSB contributors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "py/objproperty.h"
#include "shared-bindings/usb/core/Device.h"
#include "py/runtime.h"

//| class Device:
//|     def __init__(self) -> None:
//|         """User code cannot create Device objects. Instead, get them from
//|         `usb.core.find`.
//|         """
//|         ...
//|

//|     def __del__(self) -> None:
//|         """Closes any resources used for this device."""
//|         ...
//|
static mp_obj_t usb_core_device_deinit(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_usb_core_device_deinit(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_deinit_obj, usb_core_device_deinit);

//|     idVendor: int
//|     """The USB vendor ID of the device"""
static mp_obj_t usb_core_device_obj_get_idVendor(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_core_device_get_idVendor(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_get_idVendor_obj, usb_core_device_obj_get_idVendor);

MP_PROPERTY_GETTER(usb_core_device_idVendor_obj,
    (mp_obj_t)&usb_core_device_get_idVendor_obj);

//|     idProduct: int
//|     """The USB product ID of the device"""
static mp_obj_t usb_core_device_obj_get_idProduct(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_core_device_get_idProduct(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_get_idProduct_obj, usb_core_device_obj_get_idProduct);

MP_PROPERTY_GETTER(usb_core_device_idProduct_obj,
    (mp_obj_t)&usb_core_device_get_idProduct_obj);

//|     serial_number: str
//|     """The USB device's serial number string."""
static mp_obj_t usb_core_device_obj_get_serial_number(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return common_hal_usb_core_device_get_serial_number(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_get_serial_number_obj, usb_core_device_obj_get_serial_number);

MP_PROPERTY_GETTER(usb_core_device_serial_number_obj,
    (mp_obj_t)&usb_core_device_get_serial_number_obj);

//|     product: str
//|     """The USB device's product string."""
static mp_obj_t usb_core_device_obj_get_product(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return common_hal_usb_core_device_get_product(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_get_product_obj, usb_core_device_obj_get_product);

MP_PROPERTY_GETTER(usb_core_device_product_obj,
    (mp_obj_t)&usb_core_device_get_product_obj);

//|     manufacturer: str
//|     """The USB device's manufacturer string."""
//|
static mp_obj_t usb_core_device_obj_get_manufacturer(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return common_hal_usb_core_device_get_manufacturer(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_get_manufacturer_obj, usb_core_device_obj_get_manufacturer);

MP_PROPERTY_GETTER(usb_core_device_manufacturer_obj,
    (mp_obj_t)&usb_core_device_get_manufacturer_obj);

//|     bus: int
//|     """The bus number of the root hub this device is connected to."""
//|
static mp_obj_t usb_core_device_obj_get_bus(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_core_device_get_bus(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_get_bus_obj, usb_core_device_obj_get_bus);

MP_PROPERTY_GETTER(usb_core_device_bus_obj,
    (mp_obj_t)&usb_core_device_get_bus_obj);

//|     port_numbers: tuple[int] | None
//|     """The port topology of the devices location. None when connected to the
//|        root port (aka bus)."""
//|
static mp_obj_t usb_core_device_obj_get_port_numbers(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return common_hal_usb_core_device_get_port_numbers(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_get_port_numbers_obj, usb_core_device_obj_get_port_numbers);

MP_PROPERTY_GETTER(usb_core_device_port_numbers_obj,
    (mp_obj_t)&usb_core_device_get_port_numbers_obj);


//|     speed: int
//|     """The speed of the device. One of `usb.util.SPEED_LOW`, `usb.util.SPEED_FULL`, `usb.util.SPEED_HIGH` or 0 for unknown."""
//|
static mp_obj_t usb_core_device_obj_get_speed(mp_obj_t self_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_core_device_get_speed(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_core_device_get_speed_obj, usb_core_device_obj_get_speed);

MP_PROPERTY_GETTER(usb_core_device_speed_obj,
    (mp_obj_t)&usb_core_device_get_speed_obj);

//|     def set_configuration(self, configuration: int = 1) -> None:
//|         """Set the active configuration.
//|
//|         The configuration parameter is the bConfigurationValue field of the
//|         configuration you want to set as active. If you call this method
//|         without parameter, it will use the first configuration found.  As a
//|         device hardly ever has more than one configuration, calling the method
//|         without arguments is enough to get the device ready.
//|         """
//|         ...
//|
static mp_obj_t usb_core_device_set_configuration(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_configuration };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_configuration, MP_ARG_INT, {.u_int = 1} },
    };
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    common_hal_usb_core_device_set_configuration(self, args[ARG_configuration].u_int);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(usb_core_device_set_configuration_obj, 1, usb_core_device_set_configuration);

//|     def write(self, endpoint: int, data: ReadableBuffer, timeout: Optional[int] = None) -> int:
//|         """Write data to a specific endpoint on the device.
//|
//|         :param int endpoint: the bEndpointAddress you want to communicate with.
//|         :param ReadableBuffer data: the data to send
//|         :param int timeout: Time to wait specified in milliseconds. (Different from most CircuitPython!)
//|         :returns: the number of bytes written
//|         """
//|         ...
//|
static mp_obj_t usb_core_device_write(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_endpoint, ARG_data, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_endpoint, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_data, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_timeout, MP_ARG_INT, {.u_int = 0} },
    };
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_data].u_obj, &bufinfo, MP_BUFFER_READ);

    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_core_device_write(self, args[ARG_endpoint].u_int, bufinfo.buf, bufinfo.len, args[ARG_timeout].u_int));
}
MP_DEFINE_CONST_FUN_OBJ_KW(usb_core_device_write_obj, 2, usb_core_device_write);


//|     def read(
//|         self, endpoint: int, size_or_buffer: array.array, timeout: Optional[int] = None
//|     ) -> int:
//|         """Read data from the endpoint.
//|
//|         :param int endpoint: the bEndpointAddress you want to communicate with.
//|         :param array.array size_or_buffer: the array to read data into. PyUSB also allows size but CircuitPython only support array to force deliberate memory use.
//|         :param int timeout: Time to wait specified in milliseconds. (Different from most CircuitPython!)
//|         :returns: the number of bytes read
//|         """
//|         ...
//|
static mp_obj_t usb_core_device_read(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_endpoint, ARG_size_or_buffer, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_endpoint, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_size_or_buffer, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_timeout, MP_ARG_INT, {.u_int = 0} },
    };
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_size_or_buffer].u_obj, &bufinfo, MP_BUFFER_WRITE);

    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_core_device_read(self, args[ARG_endpoint].u_int, bufinfo.buf, bufinfo.len, args[ARG_timeout].u_int));
}
MP_DEFINE_CONST_FUN_OBJ_KW(usb_core_device_read_obj, 2, usb_core_device_read);

//|     def ctrl_transfer(
//|         self,
//|         bmRequestType: int,
//|         bRequest: int,
//|         wValue: int = 0,
//|         wIndex: int = 0,
//|         data_or_wLength: Optional[array.array] = None,
//|         timeout: Optional[int] = None,
//|     ) -> int:
//|         """Do a control transfer on the endpoint 0. The parameters bmRequestType,
//|         bRequest, wValue and wIndex are the same of the USB Standard Control
//|         Request format.
//|
//|         Control requests may or may not have a data payload to write/read.
//|         In cases which it has, the direction bit of the bmRequestType
//|         field is used to infer the desired request direction.
//|
//|         For host to device requests (OUT), data_or_wLength parameter is
//|         the data payload to send, and it must be a sequence type convertible
//|         to an array object. In this case, the return value is the number
//|         of bytes written in the data payload.
//|
//|         For device to host requests (IN), data_or_wLength is an array
//|         object which the data will be read to, and the return value is the
//|         number of bytes read.
//|         """
//|         ...
//|
static mp_obj_t usb_core_device_ctrl_transfer(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_bmRequestType, ARG_bRequest, ARG_wValue, ARG_wIndex, ARG_data_or_wLength, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_bmRequestType, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_bRequest, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_wValue, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_wIndex, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_data_or_wLength, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_timeout, MP_ARG_INT, {.u_int = 0} },
    };
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    // check request type
    if (args[ARG_data_or_wLength].u_obj == mp_const_none) {
        bufinfo.len = 0;
        bufinfo.buf = NULL;
    } else if ((args[ARG_bmRequestType].u_int & 0x80) != 0) {
        mp_get_buffer_raise(args[ARG_data_or_wLength].u_obj, &bufinfo, MP_BUFFER_WRITE);
    } else {
        mp_get_buffer_raise(args[ARG_data_or_wLength].u_obj, &bufinfo, MP_BUFFER_READ);
    }

    mp_int_t result = common_hal_usb_core_device_ctrl_transfer(self,
        args[ARG_bmRequestType].u_int, args[ARG_bRequest].u_int,
        args[ARG_wValue].u_int, args[ARG_wIndex].u_int,
        bufinfo.buf, bufinfo.len, args[ARG_timeout].u_int);

    return MP_OBJ_NEW_SMALL_INT(result);
}
MP_DEFINE_CONST_FUN_OBJ_KW(usb_core_device_ctrl_transfer_obj, 2, usb_core_device_ctrl_transfer);

//|     def is_kernel_driver_active(self, interface: int) -> bool:
//|         """Determine if CircuitPython is using the interface. If it is, the
//|         object will be unable to perform I/O.
//|
//|         :param int interface: the device interface number to check
//|         """
//|         ...
//|
static mp_obj_t usb_core_device_is_kernel_driver_active(mp_obj_t self_in, mp_obj_t interface_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t interface = mp_obj_get_int(interface_in);
    bool active = common_hal_usb_core_device_is_kernel_driver_active(self, interface);
    return mp_obj_new_bool(active);
}
MP_DEFINE_CONST_FUN_OBJ_2(usb_core_device_is_kernel_driver_active_obj, usb_core_device_is_kernel_driver_active);

//|     def detach_kernel_driver(self, interface: int) -> None:
//|         """Stop CircuitPython from using the interface. If successful, you
//|         will then be able to perform I/O. CircuitPython will automatically
//|         re-start using the interface on reload.
//|
//|         :param int interface: the device interface number to stop CircuitPython on
//|         """
//|         ...
//|
static mp_obj_t usb_core_device_detach_kernel_driver(mp_obj_t self_in, mp_obj_t interface_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t interface = mp_obj_get_int(interface_in);
    common_hal_usb_core_device_detach_kernel_driver(self, interface);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(usb_core_device_detach_kernel_driver_obj, usb_core_device_detach_kernel_driver);

//|     def attach_kernel_driver(self, interface: int) -> None:
//|         """Allow CircuitPython to use the interface if it wants to.
//|
//|         :param int interface: the device interface number to allow CircuitPython to use
//|         """
//|         ...
//|
//|
static mp_obj_t usb_core_device_attach_kernel_driver(mp_obj_t self_in, mp_obj_t interface_in) {
    usb_core_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t interface = mp_obj_get_int(interface_in);
    common_hal_usb_core_device_attach_kernel_driver(self, interface);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(usb_core_device_attach_kernel_driver_obj, usb_core_device_attach_kernel_driver);


static const mp_rom_map_elem_t usb_core_device_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),          MP_ROM_PTR(&usb_core_device_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit),           MP_ROM_PTR(&usb_core_device_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_idVendor),         MP_ROM_PTR(&usb_core_device_idVendor_obj) },
    { MP_ROM_QSTR(MP_QSTR_idProduct),        MP_ROM_PTR(&usb_core_device_idProduct_obj) },
    { MP_ROM_QSTR(MP_QSTR_serial_number),    MP_ROM_PTR(&usb_core_device_serial_number_obj) },
    { MP_ROM_QSTR(MP_QSTR_product),          MP_ROM_PTR(&usb_core_device_product_obj) },
    { MP_ROM_QSTR(MP_QSTR_manufacturer),     MP_ROM_PTR(&usb_core_device_manufacturer_obj) },
    { MP_ROM_QSTR(MP_QSTR_bus),              MP_ROM_PTR(&usb_core_device_bus_obj) },
    { MP_ROM_QSTR(MP_QSTR_port_numbers),     MP_ROM_PTR(&usb_core_device_port_numbers_obj) },
    { MP_ROM_QSTR(MP_QSTR_speed),            MP_ROM_PTR(&usb_core_device_speed_obj) },

    { MP_ROM_QSTR(MP_QSTR_set_configuration), MP_ROM_PTR(&usb_core_device_set_configuration_obj) },
    { MP_ROM_QSTR(MP_QSTR_write),            MP_ROM_PTR(&usb_core_device_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_read),             MP_ROM_PTR(&usb_core_device_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_ctrl_transfer),    MP_ROM_PTR(&usb_core_device_ctrl_transfer_obj) },

    { MP_ROM_QSTR(MP_QSTR_is_kernel_driver_active), MP_ROM_PTR(&usb_core_device_is_kernel_driver_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_detach_kernel_driver),    MP_ROM_PTR(&usb_core_device_detach_kernel_driver_obj) },
    { MP_ROM_QSTR(MP_QSTR_attach_kernel_driver),    MP_ROM_PTR(&usb_core_device_attach_kernel_driver_obj) },
};

static MP_DEFINE_CONST_DICT(usb_core_device_locals_dict, usb_core_device_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    usb_core_device_type,
    MP_QSTR_Device,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    locals_dict, &usb_core_device_locals_dict
    );
