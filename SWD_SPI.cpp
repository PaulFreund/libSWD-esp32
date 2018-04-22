/*
 * Serial Wire Debug Open Library.
 * ESP32 SPI driver 
 *
 * Copyright (C) 2018, Paul Freund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the Tomasz Boleslaw CEDRO nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.*
 *
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010-2014;
 *
 */

#define DIRECTION_GPIO GPIO_NUM_4
#define DIRECTION_MISO 0
#define DIRECTION_MOSI 1

#define direction_set(gpio, direction) gpio_set_level(gpio, direction)
//#define direction_set(gpio, direction)

extern "C" {
    #include <driver/spi_master.h>
    #include <libswd.h>
}

#include <mutex>

//#define spi_printf(...) ets_printf(__VA_ARGS__)
#define spi_printf(...) 

std::mutex mutexTransmit;
inline void transmitSPI(spi_device_handle_t devSPI, spi_transaction_t* transSPI) {
    std::lock_guard<std::mutex> lock(mutexTransmit);
    
    if(ESP_OK != spi_device_transmit(devSPI, transSPI)) {
        spi_printf(" - FAIL\n");
    }
}

extern int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst) {
    spi_printf("[MOSI08][%02d] -> ", bits);

    spi_device_handle_t* devSPI = (spi_device_handle_t*)libswdctx->driver->device;

    spi_transaction_t transSPI;
    memset(&transSPI, 0, sizeof(spi_transaction_t));

    transSPI.flags = SPI_TRANS_USE_TXDATA;
    transSPI.cmd = 0; // If want to set, change command_bits
    transSPI.addr = 0; // If want to set, change address_bits
    transSPI.length = bits; // Bits
    transSPI.rxlength = 0;
    transSPI.tx_data[0] = *data;

    direction_set(DIRECTION_GPIO, DIRECTION_MOSI);
    transmitSPI(*devSPI, &transSPI);
    direction_set(DIRECTION_GPIO, DIRECTION_MISO);

    spi_printf("%02x", transSPI.tx_data[0]);
    
    spi_printf(";\n");
    return LIBSWD_OK;
}

extern int libswd_drv_mosi_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst) {
    spi_printf("[MOSI32][%02d] -> ", bits);

    spi_device_handle_t* devSPI = (spi_device_handle_t*)libswdctx->driver->device;

    spi_transaction_t transSPI;
    memset(&transSPI, 0, sizeof(spi_transaction_t));

    transSPI.flags = SPI_TRANS_USE_TXDATA;
    transSPI.cmd = 0; // If want to set, change command_bits
    transSPI.addr = 0; // If want to set, change address_bits
    transSPI.length = bits; // Bits
    transSPI.rxlength = 0;

    *((int*)(&transSPI.tx_data)) = (*data);

    direction_set(DIRECTION_GPIO, DIRECTION_MOSI);
    transmitSPI(*devSPI, &transSPI);
    direction_set(DIRECTION_GPIO, DIRECTION_MISO);

    spi_printf("%02x %02x %02x %02x", transSPI.tx_data[0], transSPI.tx_data[1], transSPI.tx_data[2], transSPI.tx_data[3]);
    
    spi_printf(";\n");
    return LIBSWD_OK;
}

extern int libswd_drv_miso_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst) {
    spi_printf("[MISO08][%02d] <- ", bits);

    spi_device_handle_t* devSPI = (spi_device_handle_t*)libswdctx->driver->device;

    spi_transaction_t transSPI;
    memset(&transSPI, 0, sizeof(spi_transaction_t));

    transSPI.flags = SPI_TRANS_USE_RXDATA;
    transSPI.cmd = 0; // If want to set, change command_bits
    transSPI.addr = 0; // If want to set, change address_bits
    transSPI.length = 0; // Bits
    transSPI.rxlength = bits; // Bits expected

    transmitSPI(*devSPI, &transSPI);

    spi_printf("%02x", transSPI.rx_data[0]);

    (*data) = transSPI.rx_data[0];

    spi_printf(";\n");
    return LIBSWD_OK;
}

extern int libswd_drv_miso_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst) {
    spi_printf("[MISO32][%02d] <- ", bits);

    spi_device_handle_t* devSPI = (spi_device_handle_t*)libswdctx->driver->device;

    spi_transaction_t transSPI;
    memset(&transSPI, 0, sizeof(spi_transaction_t));

    transSPI.flags = SPI_TRANS_USE_RXDATA;
    transSPI.cmd = 0; // If want to set, change command_bits
    transSPI.addr = 0; // If want to set, change address_bits
    transSPI.length = 0; // Bits
    transSPI.rxlength = bits; // Bits expected

    transmitSPI(*devSPI, &transSPI);

    spi_printf("%02x %02x %02x %02x", transSPI.rx_data[0], transSPI.rx_data[1], transSPI.rx_data[2], transSPI.rx_data[3]);

    (*data) = *((int*)(&transSPI.rx_data));

    spi_printf(";\n");
    return LIBSWD_OK;
}

extern int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int clks) {
    spi_printf("[MOSITN][%02d]", clks);

    if(clks == 0) { return LIBSWD_OK; }

    spi_device_handle_t* devSPI = (spi_device_handle_t*)libswdctx->driver->device;

    spi_transaction_t transSPI;
    memset(&transSPI, 0, sizeof(spi_transaction_t));

    transSPI.flags = SPI_TRANS_USE_TXDATA;
    transSPI.cmd = 0; // If want to set, change command_bits
    transSPI.addr = 0; // If want to set, change address_bits
    transSPI.length = clks; // Bits
    transSPI.rxlength = 0;

    direction_set(DIRECTION_GPIO, DIRECTION_MOSI);
    transmitSPI(*devSPI, &transSPI);
    direction_set(DIRECTION_GPIO, DIRECTION_MISO);

    spi_printf(";\n");
    return LIBSWD_OK;
}

extern int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int clks) {
    spi_printf("[MISOTN][%02d]", clks);

    if(clks == 0) { return LIBSWD_OK; }
    spi_device_handle_t* devSPI = (spi_device_handle_t*)libswdctx->driver->device;

    spi_transaction_t transSPI;
    memset(&transSPI, 0, sizeof(spi_transaction_t));

    transSPI.flags = SPI_TRANS_USE_TXDATA;
    transSPI.cmd = 0; // If want to set, change command_bits
    transSPI.addr = 0; // If want to set, change address_bits
    transSPI.length = clks;
    transSPI.rxlength = 0;

    direction_set(DIRECTION_GPIO, DIRECTION_MOSI);
    transmitSPI(*devSPI, &transSPI);
    direction_set(DIRECTION_GPIO, DIRECTION_MISO);

    spi_printf(";\n");

    return LIBSWD_OK;
}

#include "esp_log.h"
extern int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...) {
    // va_list args;
    // va_start (args, msg);

    // char buffer[256];
    // int length = 0;
    // length = vsnprintf (buffer, 255, msg, args);
    // buffer[length] = '\0';
    // ESP_LOGE("SWD_SPI", "%s", buffer);
    // va_end (args);
    return LIBSWD_OK;
}

extern int libswd_log_level_inherit(libswd_ctx_t *libswdctx, int loglevel) {
    return LIBSWD_OK;
}