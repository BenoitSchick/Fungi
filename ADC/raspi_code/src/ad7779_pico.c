/*******************************************************************************
 *   @file   ad7779_pico.c
 *   @brief  Implementation of AD7779 Driver for Rasberry Pi Pico.
 *   @author Benoit Schick (benoit.schick@hefr.ch)
 *
 *   This implementation is based on the ADI no-OS driver:
 *   no-OS/drivers/adc/ad7779/ad7779.c
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "no-OS/drivers/adc/ad7779/ad7779.h"
#include "no_os_util.h"
#include "no_os_error.h"
#include "no_os_alloc.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"

#define CS_PIN 7 
#define SCK_PIN 4
#define MOSI_PIN 5
#define MISO_PIN 6

static spi_inst_t *spi = spi0;

/**
 * SPI internal register write to device.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The register data.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad7779_spi_int_reg_write(ad7779_dev *dev,
				 uint8_t reg_addr,
				 uint8_t reg_data) 
{

	uint8_t buf[3];
	uint8_t buf_size = 2;
	int32_t ret;
	
    	// Construct message (set ~W bit low, MB bit low)
	buf[0] = 0x00 | (reg_addr & 0x7F);
	buf[1] = reg_data;


	if (dev->spi_crc_en == AD7779_ENABLE) {
		buf[2] = ad7779_compute_crc8(&buf[0], 2);
		buf_size = 3;
	}

	// Write to register
	gpio_put(CS_PIN, 0);
	spi_write_blocking(spi, buf, 2);
	gpio_put(CS_PIN, 1);

	
	/* ret = no_os_spi_write_and_read(dev->spi_desc, buf, buf_size); */
	/* dev->cached_reg_val[reg_addr] = reg_data; */

	return ret;
}

/**
 * SPI internal register read from device.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The register data.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad7779_spi_int_reg_read(ad7779_dev *dev,
				uint8_t reg_addr,
				uint8_t *reg_data)
{
	uint8_t buf[3];
	uint8_t buf_size = 2;
	uint8_t crc;
	int32_t ret;

	buf[0] = 0x80 | (reg_addr & 0x7F);
	buf[1] = 0x00;
	buf[2] = 0x00;
	if (dev->spi_crc_en == AD7779_ENABLE)
		buf_size = 3;

	// Read from register
	gpio_put(CS_PIN, 0);
	spi_write_blocking(spi, &buf, buf_size);
	spi_read_blocking(spi, 0, buf, 1);
	gpio_put(CS_PIN, 1);

	/* ret = no_os_spi_write_and_read(dev->spi_desc, buf, buf_size); */
	/* *reg_data = buf[1]; */

	if (dev->spi_crc_en == AD7779_ENABLE) {
		buf[0] = 0x80 | (reg_addr & 0x7F);
		crc = ad7779_compute_crc8(&buf[0], 2);
		if (crc != buf[2]) {
			printf("%s: CRC Error.\n", __func__);
			ret = -1;
		}
	}
	return ret;
}


