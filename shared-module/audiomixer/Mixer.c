// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2018 Scott Shawcroft for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/audiomixer/Mixer.h"
#include "shared-bindings/audiomixer/MixerVoice.h"

#include <stdint.h>

#include "py/runtime.h"
#include "shared-module/audiocore/__init__.h"
#include "shared-bindings/audiocore/__init__.h"

#if defined(__arm__) && __arm__
#include "cmsis_compiler.h"
#endif

void common_hal_audiomixer_mixer_construct(audiomixer_mixer_obj_t *self,
    uint8_t voice_count,
    uint32_t buffer_size,
    uint8_t bits_per_sample,
    bool samples_signed,
    uint8_t channel_count,
    uint32_t sample_rate) {
    self->len = buffer_size / 2 / sizeof(uint32_t) * sizeof(uint32_t);

    self->first_buffer = m_malloc(self->len);
    if (self->first_buffer == NULL) {
        common_hal_audiomixer_mixer_deinit(self);
        m_malloc_fail(self->len);
    }

    self->second_buffer = m_malloc(self->len);
    if (self->second_buffer == NULL) {
        common_hal_audiomixer_mixer_deinit(self);
        m_malloc_fail(self->len);
    }

    self->base.bits_per_sample = bits_per_sample;
    self->base.samples_signed = samples_signed;
    self->base.channel_count = channel_count;
    self->base.sample_rate = sample_rate;
    self->base.single_buffer = false;
    self->voice_count = voice_count;
    self->base.max_buffer_length = buffer_size;
}

void common_hal_audiomixer_mixer_deinit(audiomixer_mixer_obj_t *self) {
    audiosample_mark_deinit(&self->base);
    self->first_buffer = NULL;
    self->second_buffer = NULL;
}

bool common_hal_audiomixer_mixer_get_playing(audiomixer_mixer_obj_t *self) {
    for (uint8_t v = 0; v < self->voice_count; v++) {
        if (common_hal_audiomixer_mixervoice_get_playing(MP_OBJ_TO_PTR(self->voice[v]))) {
            return true;
        }
    }
    return false;
}

void audiomixer_mixer_reset_buffer(audiomixer_mixer_obj_t *self,
    bool single_channel_output,
    uint8_t channel) {
    for (uint8_t i = 0; i < self->voice_count; i++) {
        common_hal_audiomixer_mixervoice_stop(self->voice[i]);
    }
}

__attribute__((always_inline))
static inline uint32_t add16signed(uint32_t a, uint32_t b) {
    #if (defined(__ARM_ARCH_7EM__) && (__ARM_ARCH_7EM__ == 1))
    return __QADD16(a, b);
    #else
    uint32_t result = 0;
    for (int8_t i = 0; i < 2; i++) {
        int16_t ai = a >> (sizeof(int16_t) * 8 * i);
        int16_t bi = b >> (sizeof(int16_t) * 8 * i);
        int32_t intermediate = (int32_t)ai + bi;
        if (intermediate > SHRT_MAX) {
            intermediate = SHRT_MAX;
        } else if (intermediate < SHRT_MIN) {
            intermediate = SHRT_MIN;
        }
        result |= (((uint32_t)intermediate) & 0xffff) << (sizeof(int16_t) * 8 * i);
    }
    return result;
    #endif
}

__attribute__((always_inline))
static inline uint32_t mult16signed(uint32_t val, int32_t mul) {
    #if (defined(__ARM_ARCH_7EM__) && (__ARM_ARCH_7EM__ == 1))
    mul <<= 16;
    int32_t hi, lo;
    enum { bits = 16 }; // saturate to 16 bits
    enum { shift = 15 }; // shift is done automatically
    asm volatile ("smulwb %0, %1, %2" : "=r" (lo) : "r" (mul), "r" (val));
    asm volatile ("smulwt %0, %1, %2" : "=r" (hi) : "r" (mul), "r" (val));
    asm volatile ("ssat %0, %1, %2, asr %3" : "=r" (lo) : "I" (bits), "r" (lo), "I" (shift));
    asm volatile ("ssat %0, %1, %2, asr %3" : "=r" (hi) : "I" (bits), "r" (hi), "I" (shift));
    asm volatile ("pkhbt %0, %1, %2, lsl #16" : "=r" (val) : "r" (lo), "r" (hi)); // pack
    return val;
    #else
    uint32_t result = 0;
    float mod_mul = (float)mul / (float)((1 << 15) - 1);
    for (int8_t i = 0; i < 2; i++) {
        int16_t ai = (val >> (sizeof(uint16_t) * 8 * i));
        int32_t intermediate = (int32_t)(ai * mod_mul);
        if (intermediate > SHRT_MAX) {
            intermediate = SHRT_MAX;
        } else if (intermediate < SHRT_MIN) {
            intermediate = SHRT_MIN;
        }
        intermediate &= 0x0000FFFF;
        result |= (((uint32_t)intermediate)) << (sizeof(int16_t) * 8 * i);
    }
    return result;
    #endif
}

static inline uint32_t tounsigned8(uint32_t val) {
    #if (defined(__ARM_ARCH_7EM__) && (__ARM_ARCH_7EM__ == 1))
    return __UADD8(val, 0x80808080);
    #else
    return val ^ 0x80808080;
    #endif
}

static inline uint32_t tounsigned16(uint32_t val) {
    #if (defined(__ARM_ARCH_7EM__) && (__ARM_ARCH_7EM__ == 1))
    return __UADD16(val, 0x80008000);
    #else
    return val ^ 0x80008000;
    #endif
}

static inline uint32_t tosigned16(uint32_t val) {
    #if (defined(__ARM_ARCH_7EM__) && (__ARM_ARCH_7EM__ == 1))
    return __UADD16(val, 0x80008000);
    #else
    return val ^ 0x80008000;
    #endif
}

static inline uint32_t unpack8(uint16_t val) {
    return ((val & 0xff00) << 16) | ((val & 0x00ff) << 8);
}

static inline uint32_t pack8(uint32_t val) {
    return ((val & 0xff000000) >> 16) | ((val & 0xff00) >> 8);
}

static void mix_down_one_voice(audiomixer_mixer_obj_t *self,
    audiomixer_mixervoice_obj_t *voice, bool voices_active,
    uint32_t *word_buffer, uint32_t length) {
    while (length != 0) {
        if (voice->buffer_length == 0) {
            if (!voice->more_data) {
                if (voice->loop) {
                    audiosample_reset_buffer(voice->sample, false, 0);
                } else {
                    voice->sample = NULL;
                    break;
                }
            }
            if (voice->sample) {
                // Load another buffer
                audioio_get_buffer_result_t result = audiosample_get_buffer(voice->sample, false, 0, (uint8_t **)&voice->remaining_buffer, &voice->buffer_length);
                // Track length in terms of words.
                voice->buffer_length /= sizeof(uint32_t);
                voice->more_data = result == GET_BUFFER_MORE_DATA;
            }
        }

        uint32_t *src = voice->remaining_buffer;

        #if CIRCUITPY_SYNTHIO
        uint32_t n = MIN(MIN(voice->buffer_length, length), SYNTHIO_MAX_DUR * self->base.channel_count);

        // Get the current level from the BlockInput. These may change at run time so you need to do bounds checking if required.
        shared_bindings_synthio_lfo_tick(self->base.sample_rate, n / self->base.channel_count);
        uint16_t level = (uint16_t)(synthio_block_slot_get_limited(&voice->level, MICROPY_FLOAT_CONST(0.0), MICROPY_FLOAT_CONST(1.0)) * (1 << 15));
        #else
        uint32_t n = MIN(voice->buffer_length, length);
        uint16_t level = voice->level;
        #endif

        // First active voice gets copied over verbatim.
        if (!voices_active) {
            if (MP_LIKELY(self->base.bits_per_sample == 16)) {
                if (MP_LIKELY(self->base.samples_signed)) {
                    for (uint32_t i = 0; i < n; i++) {
                        uint32_t v = src[i];
                        word_buffer[i] = mult16signed(v, level);
                    }
                } else {
                    for (uint32_t i = 0; i < n; i++) {
                        uint32_t v = src[i];
                        v = tosigned16(v);
                        word_buffer[i] = mult16signed(v, level);
                    }
                }
            } else {
                uint16_t *hword_buffer = (uint16_t *)word_buffer;
                uint16_t *hsrc = (uint16_t *)src;
                for (uint32_t i = 0; i < n * 2; i++) {
                    uint32_t word = unpack8(hsrc[i]);
                    if (MP_LIKELY(!self->base.samples_signed)) {
                        word = tosigned16(word);
                    }
                    word = mult16signed(word, level);
                    hword_buffer[i] = pack8(word);
                }
            }
        } else {
            if (MP_LIKELY(self->base.bits_per_sample == 16)) {
                if (MP_LIKELY(self->base.samples_signed)) {
                    for (uint32_t i = 0; i < n; i++) {
                        uint32_t word = src[i];
                        word_buffer[i] = add16signed(mult16signed(word, level), word_buffer[i]);
                    }
                } else {
                    for (uint32_t i = 0; i < n; i++) {
                        uint32_t word = src[i];
                        word = tosigned16(word);
                        word_buffer[i] = add16signed(mult16signed(word, level), word_buffer[i]);
                    }
                }
            } else {
                uint16_t *hword_buffer = (uint16_t *)word_buffer;
                uint16_t *hsrc = (uint16_t *)src;
                for (uint32_t i = 0; i < n * 2; i++) {
                    uint32_t word = unpack8(hsrc[i]);
                    if (MP_LIKELY(!self->base.samples_signed)) {
                        word = tosigned16(word);
                    }
                    word = mult16signed(word, level);
                    word = add16signed(word, unpack8(hword_buffer[i]));
                    hword_buffer[i] = pack8(word);
                }
            }
        }
        length -= n;
        word_buffer += n;
        voice->remaining_buffer += n;
        voice->buffer_length -= n;
    }

    if (length && !voices_active) {
        for (uint32_t i = 0; i < length; i++) {
            word_buffer[i] = 0;
        }
    }
}

audioio_get_buffer_result_t audiomixer_mixer_get_buffer(audiomixer_mixer_obj_t *self,
    bool single_channel_output,
    uint8_t channel,
    uint8_t **buffer,
    uint32_t *buffer_length) {
    if (!single_channel_output) {
        channel = 0;
    }

    uint32_t channel_read_count = self->left_read_count;
    if (channel == 1) {
        channel_read_count = self->right_read_count;
    }
    *buffer_length = self->len;

    bool need_more_data = self->read_count == channel_read_count;
    if (need_more_data) {
        uint32_t *word_buffer;
        if (self->use_first_buffer) {
            *buffer = (uint8_t *)self->first_buffer;
            word_buffer = self->first_buffer;
        } else {
            *buffer = (uint8_t *)self->second_buffer;
            word_buffer = self->second_buffer;
        }
        self->use_first_buffer = !self->use_first_buffer;
        bool voices_active = false;
        uint32_t length = self->len / sizeof(uint32_t);

        for (int32_t v = 0; v < self->voice_count; v++) {
            audiomixer_mixervoice_obj_t *voice = MP_OBJ_TO_PTR(self->voice[v]);
            if (voice->sample) {
                mix_down_one_voice(self, voice, voices_active, word_buffer, length);
                voices_active = true;
            }
        }

        if (!voices_active) {
            for (uint32_t i = 0; i < length; i++) {
                word_buffer[i] = 0;
            }
        }

        if (!self->base.samples_signed) {
            if (self->base.bits_per_sample == 16) {
                for (uint32_t i = 0; i < length; i++) {
                    word_buffer[i] = tounsigned16(word_buffer[i]);
                }
            } else {
                for (uint32_t i = 0; i < length; i++) {
                    word_buffer[i] = tounsigned8(word_buffer[i]);
                }
            }
        }

        self->read_count += 1;
    } else if (!self->use_first_buffer) {
        *buffer = (uint8_t *)self->first_buffer;
    } else {
        *buffer = (uint8_t *)self->second_buffer;
    }


    if (channel == 0) {
        self->left_read_count += 1;
    } else if (channel == 1) {
        self->right_read_count += 1;
        *buffer = *buffer + self->base.bits_per_sample / 8;
    }
    return GET_BUFFER_MORE_DATA;
}
