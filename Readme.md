# LibSWD for ESP32 IDF

The basis for this component comes from https://github.com/cederom/LibSWD with an addition of a driver layer for the ESP32 IDF SPI master that is able to work in three wire mode. License is BSDv3, same as the libSWD library. 

Place the content of this repository in a folder libswd inside of the components folder of your project.

# Example code

The following example code is able to read out the ID code of a CPU and read 4 bytes from memory at a specific address. It works with three GPIO pins as intended because of the 3wire mode of the SPI master. With an unoptimized cable this code was able to read values with up to 40mHz closk speed from an Atmel SAM C20, after that edges began to deteriorate.

## Neccessary includes
```c
#include <driver/spi_master.h>
#include <libswd.h>
#include <rom/ets_sys.h> // For ets_printf
```

## Set up libSWD and the SPI device
```c
spi_bus_config_t pinsSPI;
pinsSPI.mosi_io_num     = GPIO_NUM_13; // SWD I/O
pinsSPI.miso_io_num     = GPIO_NUM_12; // not connected
pinsSPI.sclk_io_num     = GPIO_NUM_14; // SWD CLK
pinsSPI.quadwp_io_num   = -1;
pinsSPI.quadhd_io_num   = -1;
pinsSPI.max_transfer_sz = 0;

ets_printf("[spi_bus_initialize]");
if(ESP_OK != spi_bus_initialize(HSPI_HOST, &pinsSPI, 0)) { // No DMA
    ets_printf("[spi_bus_initialize] fail");
    return; // Warning, this example does not close handles correctly
}

spi_device_interface_config_t confSPI;
confSPI.command_bits        = 0;
confSPI.address_bits        = 0;
confSPI.dummy_bits          = 0;
confSPI.mode                = 0;
confSPI.duty_cycle_pos      = 0;
confSPI.cs_ena_pretrans     = 0;
confSPI.cs_ena_posttrans    = 0;
confSPI.clock_speed_hz      = 10000;
confSPI.spics_io_num        = -1;
confSPI.flags               = SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_BIT_LSBFIRST;
confSPI.queue_size          = 24;
confSPI.pre_cb              = nullptr;
confSPI.post_cb             = nullptr;

spi_device_handle_t deviceSPI
ets_printf("[spi_bus_add_device]");
if(ESP_OK != spi_bus_add_device(HSPI_HOST, &confSPI, &deviceSPI)) {
    ets_printf("[spi_bus_add_device] fail");
    return; // Warning, this example does not close handles correctly
}

ets_printf("[libswd_init]");
libswd_ctx_t* libswdctx = libswd_init();
if(libswdctx == nullptr) {
    ets_printf("[libswd_init] returned empty context");
    return; // Warning, this example does not close handles correctly
}

libswd_log_level_set(libswdctx, LIBSWD_LOGLEVEL_DEBUG);
libswdctx->driver->device = &deviceSPI;
```

## Request ID code
```c
int idcode = 0;
int* idcode_ptr = &idcode;
ets_printf("[libswd_dap_detect]");
auto dap_res = libswd_dap_detect(libswdctx, LIBSWD_OPERATION_EXECUTE, &idcode_ptr);
if(LIBSWD_OK != dap_res) {
    ets_printf("[libswd_dap_detect] failed with code %d\n", dap_res);
    return; // Warning, this example does not close handles correctly
}

char buff[128];
sprintf(buff, "Detected IDCODE: 0x%08X\n", *idcode_ptr);
ets_printf("%s", buff);
```

## Read four bytes from memory address
```c
uint32_t address = 0x20001000; // Example address
ets_printf("[libswd_memap_init]");
auto memmap_res = libswd_memap_init(libswdctx, LIBSWD_OPERATION_EXECUTE);
if(LIBSWD_OK != memmap_res) {
    ets_printf("[libswd_memap_init] failed");
    return; // Warning, this example does not close handles correctly
}

const uint16_t buffCnt = 4;
uint8_t buff[buffCnt] = {0};    
int read_res = libswd_memap_read_char(libswdctx, LIBSWD_OPERATION_EXECUTE, address, buffCnt, (char*)&buff);
if(read_res < LIBSWD_OK) {
    ets_printf("[libswd_memap_read_char] FAILED");
    return;
}

char stringBuff[128];
sprintf(stringBuff, "MEMAP read at %08X: %02X %02X %02X %02X", address, buff[0], buff[1], buff[2], buff[3]);
ets_printf("%s", stringBuff);
```
