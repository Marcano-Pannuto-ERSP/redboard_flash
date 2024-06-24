// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText 2023 Kristin Ebuengan
// SPDX-FileCopyrightText 2023 Melody Gill
// SPDX-FileCopyrightText 2023 Gabriel Marcano

/* 
* This is an edited file of main.c from https://github.com/gemarcano/redboard_lora_example
* which this repo was forked from
*
* Tests reading time from the RTC to reading/writing/erasing from the flash chip.
*/

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include <spi.h>
#include <uart.h>
#include <flash.h>
#include <am1815.h>
#include <syscalls.h>

/*
* Inializing the MCU and UART are the only things kept from the original file
*/
static struct uart *uart;
static struct spi_bus *spi_bus;
static struct spi_device *flash_spi;
static struct flash flash;
static struct am1815 rtc;

int main(void)
{
	// Prepare MCU by init-ing clock, cache, and power level operation
	am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
	am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
	am_hal_cachectrl_enable();
	am_bsp_low_power_init();
	am_hal_sysctrl_fpu_enable();
	am_hal_sysctrl_fpu_stacking_enable(true);

	// Init UART, registers with SDK printf
	uart = uart_get_instance(UART_INST0);
	syscalls_uart_init(uart);

	am_hal_interrupt_master_enable();

	// Print the banner.
	printf("Hello World!\r\n\r\n");

	// Initialize spi and select the CS channel
	spi_bus = spi_bus_get_instance(SPI_BUS_0);
	spi_bus_enable(spi_bus);
	flash_spi = spi_device_get_instance(spi_bus, SPI_CS_2, 2000000u);

	// Initialize flash
	flash_init(&flash, flash_spi);

	// Test flash functions
	int size = 15;
	char buffer[size];		// this is 4x bigger than necessary
	// initialize buffer to all -1 (shouldn't be necessary to do this)
	for (int i = 0; i < size; i++) {
		buffer[i] = -1;
	}

	// print the data before write
	flash_read_data(&flash, 0x04, buffer, size);
	char* buf = buffer;
	printf("before write:\r\n");
	for (int i = 0; i < size; i++) {
		am_util_stdio_printf("%02X ", (int) buf[i]);
		am_util_delay_ms(10);
	}
	printf("\r\n");

	// print the flash ID to make sure the CS is connected correctly
	printf("flash ID: %02X\r\n", flash_read_id(&flash));

	// print the flash ID to make sure the CS is connected coorrectly
	printf("flash ID: %02X\r\n", flash_read_id(&flash));

	// write the RTC time to the flash chip
	// print the seconds from the RTC to make sure the time is correct
	uint8_t tmp[8] = {0, 1, 2, 3, 5, 6, 7, 8};
	flash_page_program(&flash, 0x05, tmp, 8);
	flash_wait_busy(&flash);

	// print flash data after write
	flash_read_data(&flash, 0x04, buffer, size);
	buf = buffer;
	printf("after write:\r\n");
	for (int i = 0; i < size; i++) {
		printf("%02X ", (int) buf[i]);
		am_util_delay_ms(10);
	}
	printf("\r\n");

	// erase data
	flash_sector_erase(&flash, 0x05);
	flash_wait_busy(&flash);

	// print flash data after write
	flash_read_data(&flash, 0x04, buffer, size);
	buf = buffer;
	printf("after erase:\r\n");
	for (int i = 0; i < size; i++) {
		printf("%02X ", (int) buf[i]);
		am_util_delay_ms(10);
	}
	printf("\r\n");
	printf("done\r\n");
}
