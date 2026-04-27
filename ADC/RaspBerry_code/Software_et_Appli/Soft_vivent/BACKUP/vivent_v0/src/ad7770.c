/***************************************************************************//**
 *   @file   ad7770.c
 *   @brief  Implementation of ad7770 Driver.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2016(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//#include "platform_drivers.h"
#include "../include/ad7770.h"
#include "../include/bcm2835.h"

/******************************************************************************/
/*************************** Constants Definitions ****************************/
/******************************************************************************/
const uint8_t pin_mode_options[16][4] = {
/*	GAIN_1	GAIN_2	GAIN_4	GAIN_8 */
	{0x03,	0xFF,	0x07,	0xFF},	// DEC_RATE_128, HIGH_RES, EXT_REF
	{0x0A,	0xFF,	0xFF,	0xFF},	// DEC_RATE_128, HIGH_RES, INT_REF
	{0x0D,	0xFF,	0xFF,	0xFF},	// DEC_RATE_128, LOW_PWR, EXT_REF
	{0x0E,	0xFF,	0xFF,	0xFF},	// DEC_RATE_128, LOW_PWR, INT_REF
	{0x02,	0x04,	0x06,	0xFF},	// DEC_RATE_256, HIGH_RES, EXT_REF
	{0x09,	0xFF,	0xFF,	0xFF},	// DEC_RATE_256, HIGH_RES, INT_REF
	{0x0C,	0xFF,	0xFF,	0xFF},	// DEC_RATE_256, LOW_PWR, EXT_REF
	{0x0F,	0xFF,	0xFF,	0xFF},	// DEC_RATE_256, LOW_PWR, INT_REF
	{0x01,	0xFF,	0x05,	0xFF},	// DEC_RATE_512, HIGH_RES, EXT_REF
	{0x08,	0xFF,	0xFF,	0xFF},	// DEC_RATE_512, HIGH_RES, INT_REF
	{0x08,	0xFF,	0xFF,	0xFF},	// DEC_RATE_512, LOW_PWR, EXT_REF
	{0xFF,	0xFF,	0xFF,	0xFF},	// DEC_RATE_512, LOW_PWR, INT_REF
	{0x00,	0xFF,	0xFF,	0xFF},	// DEC_RATE_1024, HIGH_RES, EXT_REF
	{0xFF,	0xFF,	0xFF,	0xFF},	// DEC_RATE_1024, HIGH_RES, INT_REF
	{0xFF,	0xFF,	0xFF,	0xFF},	// DEC_RATE_1024, LOW_PWR, EXT_REF
	{0xFF,	0xFF,	0xFF,	0xFF},	// DEC_RATE_1024, LOW_PWR, INT_REF	
};

const uint8_t reset_reg_val[ad7770_REG_SRC_UPDATE + 1] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x24, 0x09, 0x80, 0x20, 0x00, 0x00, 0x00, 0x00, 0x38, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x00, 0x00,
	0X00, 0X80, 0X00, 0X00, 0X00};

/******************************************************************************/
/************************** Functions Implementation **************************/
/******************************************************************************/
/**
 * Compute CRC8 checksum.
 * @param data - The data buffer.
 * @param data_size - The size of the data buffer.
 * @return CRC8 checksum.
 */
uint8_t ad7770_compute_crc8(uint8_t *data,
							uint8_t data_size)
{
	uint8_t i;
	uint8_t crc = 0;
	while (data_size) {
		for (i = 0x80; i != 0; i >>= 1) {
			if (((crc & 0x80) != 0) != ((*data & i) != 0)) {
				crc <<= 1;
				crc ^= ad7770_CRC8_POLY;
			} else
				crc <<= 1;
		}
		data++;
		data_size--;
	}

	return crc;
}

/**
 * SPI internal register read from device.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The register data.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_spi_int_reg_read(ad7770_dev *dev,
								uint8_t reg_addr,
								uint8_t *reg_data)
{
	uint8_t buf[3];
	uint8_t buf_size = 2;
	uint8_t crc, i;
	int32_t ret;

	buf[0] = 0x80 | (reg_addr & 0x7F);
	buf[1] = 0x00;
	buf[2] = 0x00;
	if (dev->spi_crc_en == ad7770_ENABLE)
		buf_size = 3;
	//ret = spi_write_and_read(&dev->spi_dev, buf, buf_size);
	/*ret = SUCCESS;
	bcm2835_spi_transfern(buf, buf_size);

	*reg_data = buf[1];
	if (dev->spi_crc_en == ad7770_ENABLE) {
		buf[0] = 0x80 | (reg_addr & 0x7F);
		crc = ad7770_compute_crc8(&buf[0], 2);
		if (crc != buf[2]) {
			printf("%s: CRC Error.\n", __func__);
			ret = FAILURE;
		}
	}
	return ret;*/
	
	ret = SUCCESS;
	for(i = 0; i < 4; ){
		bcm2835_spi_transfern(buf, buf_size);
		*reg_data = buf[1];
		if (dev->spi_crc_en == ad7770_ENABLE) {
			buf[0] = 0x80 | (reg_addr & 0x7F);
			crc = ad7770_compute_crc8(&buf[0], 2);
			if (crc != buf[2]) i++;		// CRC error
			else return ret;	// No CRC error, return SUCCESS	
		}
		else return ret;	// No CRC check, return SUCCESS
	}
	
	ret = FAILURE; // after 4 tries, CRC error during reg_read, return FAILURE
	printf("%s: reg 0x%02X -> CRC Error.\n", __func__, reg_addr);
	return ret;
}

/**
 * SPI internal register write to device.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The register data.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_spi_int_reg_write(ad7770_dev *dev,
								 uint8_t reg_addr,
								 uint8_t reg_data)
{
	uint8_t buf[3];
	uint8_t buf_size = 2;
	uint8_t i, reg_CRC;
	int32_t ret;

	buf[0] = 0x00 | (reg_addr & 0x7F);
	buf[1] = reg_data;
	if (dev->spi_crc_en == ad7770_ENABLE) {
		buf[2] = ad7770_compute_crc8(&buf[0], 2);
		buf_size = 3;
		//ret = spi_write_and_read(&dev->spi_dev, buf, buf_size);
		// DON'T read back CRC test if going in SAR or sigma-delta via SPI mode
		if(((reg_addr == ad7770_REG_GENERAL_USER_CONFIG_2) && ((reg_data & ad7770_SAR_DIAG_MODE_EN) != 0)) ||
			((reg_addr == ad7770_REG_GENERAL_USER_CONFIG_3) && ((reg_data & ad7770_SPI_SLAVE_MODE_EN) != 0))){
				bcm2835_spi_transfern(buf, buf_size);
				dev->cached_reg_val[reg_addr] = reg_data;
				return SUCCESS;
			}
		
		// DON'T read back CRC test if in SAR or sigma-delta via SPI mode
		if(((dev->cached_reg_val[ad7770_REG_GENERAL_USER_CONFIG_2] & ad7770_SAR_DIAG_MODE_EN) != 0) ||
			((dev->cached_reg_val[ad7770_REG_GENERAL_USER_CONFIG_3] & ad7770_SPI_SLAVE_MODE_EN) != 0)){
				bcm2835_spi_transfern(buf, buf_size);
				dev->cached_reg_val[reg_addr] = reg_data;
				return SUCCESS;
			}
			
		for(i = 0; i < 4; ){
			bcm2835_spi_transfern(buf, buf_size);
			ret = ad7770_spi_int_reg_read_mask(dev, ad7770_REG_GEN_ERR_REG_1, ad7770_SPI_CRC_ERR, &reg_CRC);
			if(ret == SUCCESS){
				if(reg_CRC == 0x00){	
					dev->cached_reg_val[reg_addr] = reg_data; // No CRC error during reg_write, return SUCCESS
					return ret;
				}
				else i++;
			}
			else return ret;	// CRC error during reg_read, return FAILURE
		}
	}
	else{
		bcm2835_spi_transfern(buf, buf_size); // No CRC check
		return SUCCESS;
	}
	ret = FAILURE;	// after 4 tries, CRC error during reg_write, return FAILURE
	printf("%s: reg 0x%02X with value 0x%02X-> CRC Error.\n", __func__, reg_addr, reg_data);
	return ret;
}

/**
 * SPI internal register read from device using a mask.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param mask - The mask.
 * @param data - The register data.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_spi_int_reg_read_mask(ad7770_dev *dev,
									 uint8_t reg_addr,
									 uint8_t mask,
									 uint8_t *data)
{
	uint8_t reg_data;
	int32_t ret;

	ret = ad7770_spi_int_reg_read(dev, reg_addr, &reg_data);
	*data = (reg_data & mask);

	return ret;
}

/**
 * SPI internal register write to device using a mask.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param mask - The mask.
 * @param data - The register data.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_spi_int_reg_write_mask(ad7770_dev *dev,
									  uint8_t reg_addr,
									  uint8_t mask,
									  uint8_t data)
{
	uint8_t reg_data;
	int32_t ret;

	reg_data = dev->cached_reg_val[reg_addr];
	reg_data &= ~mask;
	reg_data |= data;
	ret = ad7770_spi_int_reg_write(dev, reg_addr, reg_data);

	return ret;
}

/**
 * SPI SAR conversion code read.
 * @param dev - The device structure.
 * @param mux_next_conv - The SAR mux input configuration for the next
 *						  conversion.
 * 						  Accepted values: ad7770_AUXAINP_AUXAINN
 *										   ad7770_DVBE_AVSSX
 *										   ad7770_REF1P_REF1N
 *										   ad7770_REF2P_REF2N
 *										   ad7770_REF_OUT_AVSSX
 *										   ad7770_VCM_AVSSX
 *										   ad7770_AREG1CAP_AVSSX_ATT
 *										   ad7770_AREG2CAP_AVSSX_ATT
 *										   ad7770_DREGCAP_DGND_ATT
 *										   ad7770_AVDD1A_AVSSX_ATT
 *										   ad7770_AVDD1B_AVSSX_ATT
 *										   ad7770_AVDD2A_AVSSX_ATT
 *										   ad7770_AVDD2B_AVSSX_ATT
 *										   ad7770_IOVDD_DGND_ATT
 *										   ad7770_AVDD4_AVSSX
 *										   ad7770_DGND_AVSS1A_ATT
 *										   ad7770_DGND_AVSS1B_ATT
 *										   ad7770_DGND_AVSSX_ATT
 *										   ad7770_AVDD4_AVSSX_ATT
 *										   ad7770_REF1P_AVSSX
 *										   ad7770_REF2P_AVSSX
 *										   ad7770_AVSSX_AVDD4_ATT
 * @param sar_code - SAR conversion code.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_spi_sar_read_code(ad7770_dev *dev,
								 ad7770_sar_mux mux_next_conv,
								 uint16_t *sar_code)
{
	uint8_t buf[3];
	uint8_t buf_size = 2;
	uint8_t crc;
	int32_t ret;

	buf[0] = 0x00 | (ad7770_REG_GLOBAL_MUX_CONFIG & 0x7F);
	buf[1] = ad7770_GLOBAL_MUX_CTRL(mux_next_conv);
	if (dev->spi_crc_en == ad7770_ENABLE) {
		buf[2] = ad7770_compute_crc8(&buf[0], 2);
		buf_size = 3;
	}
	//ret = spi_write_and_read(&dev->spi_dev, buf, buf_size);
	ret = SUCCESS;
	bcm2835_spi_transfern(buf, buf_size);
	dev->cached_reg_val[ad7770_REG_GLOBAL_MUX_CONFIG] =
									ad7770_GLOBAL_MUX_CTRL(mux_next_conv);
	buf[0] = buf[0] & 0x0F;
	*sar_code = (buf[0] << 8) | buf[1];
	if (dev->spi_crc_en == ad7770_ENABLE) {
		crc = ad7770_compute_crc8(&buf[0], 2);
		if (crc != buf[2]) {
			printf("%s: CRC Error.\n", __func__);
			ret = FAILURE;
		}
	}

	return ret;
}

/**
 * Set SPI operation mode.
 * @param dev - The device structure.
 * @param mode - The SPI operation mode.
 *				 Accepted values: ad7770_INT_REG
 *								  ad7770_SD_CONV
 *								  ad7770_SAR_CONV
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_spi_op_mode(ad7770_dev *dev,
							   ad7770_spi_op_mode mode)
{
	int32_t ret;
	uint8_t cfg_2;
	uint8_t cfg_3;

	switch (mode) {
	case ad7770_SD_CONV:
		cfg_2 = 0;
		cfg_3 = ad7770_SPI_SLAVE_MODE_EN;
		break;
	case ad7770_SAR_CONV:
		cfg_2 = ad7770_SAR_DIAG_MODE_EN;
		cfg_3 = 0;
		break;
	default:	// ad7770_INT_REG
		cfg_2 = 0;
		cfg_3 = 0;
	}
	ret = ad7770_spi_int_reg_write_mask(dev,
										ad7770_REG_GENERAL_USER_CONFIG_2,
										ad7770_SAR_DIAG_MODE_EN,
										cfg_2);
										
	ret |= ad7770_spi_int_reg_write_mask(dev,
										 ad7770_REG_GENERAL_USER_CONFIG_3,
										 ad7770_SPI_SLAVE_MODE_EN,
										 cfg_3);
										 
	dev->spi_op_mode = mode;

	return ret;
}

/**
 * Get SPI operation mode.
 * @param dev - The device structure.
 * @param mode - The SPI operation mode.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_spi_op_mode(ad7770_dev *dev,
							   ad7770_spi_op_mode *mode)
{
	*mode = dev->spi_op_mode;

	return SUCCESS;
}

/**
 * Update the state of the MODEx pins according to the settings specified in
 * the device structure.
 * @param dev - The device structure.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_do_update_mode_pins(ad7770_dev *dev)
{
	int32_t ret;
	uint8_t option_index;
	uint8_t mode;

	if (!(dev->gain[ad7770_CH0] == dev->gain[ad7770_CH1] ==
		  dev->gain[ad7770_CH2] == dev->gain[ad7770_CH3] == ad7770_GAIN_1))
		goto error;

	if (!(dev->gain[ad7770_CH4] == dev->gain[ad7770_CH5] ==
		  dev->gain[ad7770_CH6] == dev->gain[ad7770_CH7]))
		goto error;

	switch (dev->dec_rate_int) {
	case 128:
		option_index = 0;
		break;
	case 256:
		option_index = 4;
		break;
	case 512:
		option_index = 8;
		break;
	case 1024:
		option_index = 12;
		break;
	default:
		goto error;
	}

	if (dev->pwr_mode == ad7770_HIGH_RES)
		if (dev->ref_type == ad7770_EXT_REF)
			mode = pin_mode_options[option_index + 0][dev->gain[ad7770_CH4]];
		else
			mode = pin_mode_options[option_index + 1][dev->gain[ad7770_CH4]];
	else
		if (dev->ref_type == ad7770_EXT_REF)
			mode = pin_mode_options[option_index + 2][dev->gain[ad7770_CH4]];
		else
			mode = pin_mode_options[option_index + 3][dev->gain[ad7770_CH4]];

	if (mode == 0xFF)
	  goto error;

	/*ret = gpio_set_value(&dev->gpio_dev,
					dev->gpio_mode0,
					((mode & 0x01) >> 0));
	ret |= gpio_set_value(&dev->gpio_dev,
					dev->gpio_mode1,
					((mode & 0x02) >> 1));
	ret |= gpio_set_value(&dev->gpio_dev,
					dev->gpio_mode2,
					((mode & 0x04) >> 2));
	ret |= gpio_set_value(&dev->gpio_dev,
					dev->gpio_mode3,
					((mode & 0x08) >> 3));*/

	/* All the pins that define the ad7770 configuration mode are re-evaluated
	 * every time SYNC_IN pin is pulsed. */
	/*ret |= gpio_set_value(&dev->gpio_dev, dev->gpio_sync_in, GPIO_LOW);
	mdelay(10);
	ret |= gpio_set_value(&dev->gpio_dev, dev->gpio_sync_in, GPIO_HIGH);

	return ret;*/
	return SUCCESS;

error:
	printf("%s: This setting can't be set in PIN control mode.\n",
		   __func__);
	return FAILURE;
}

/**
 * Set the state (enable, disable) of the channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param state - The channel state.
 * 				  Accepted values: ad7770_ENABLE
 * 								   ad7770_DISABLE
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_state(ad7770_dev *dev,
						 ad7770_ch ch,
						 ad7770_state state)
{
	int32_t ret;

	/*ret = ad7770_spi_int_reg_write_mask(dev,
										ad7770_REG_CH_DISABLE,
										ad7770_CH_DISABLE(0x1),
										ad7770_CH_DISABLE(state));*/
										
	ret = ad7770_spi_int_reg_write_mask(dev,
										ad7770_REG_CH_DISABLE,
										ad7770_CH_DISABLE(ch),
										(state == ad7770_DISABLE) ? ad7770_CH_DISABLE(ch) : 0);
										
	dev->state[ch] = state;

	return ret;
}

/**
 * Get the state (enable, disable) of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param state - The channel state.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_state(ad7770_dev *dev,
						 ad7770_ch ch,
						 ad7770_state *state)
{
	*state = dev->state[ch];

	return SUCCESS;
}

/**
 * Set the gain of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param gain - The gain value.
 * 				 Accepted values: ad7770_GAIN_1
 * 								  ad7770_GAIN_2
 * 								  ad7770_GAIN_4
 * 								  ad7770_GAIN_8
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_gain(ad7770_dev *dev,
						ad7770_ch ch,
						ad7770_gain gain)
{
	int32_t ret;

	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		if (ch <= ad7770_CH3) {
			dev->gain[ad7770_CH0] = gain;
			dev->gain[ad7770_CH1] = gain;
			dev->gain[ad7770_CH2] = gain;
			dev->gain[ad7770_CH3] = gain;
		} else {
			dev->gain[ad7770_CH4] = gain;
			dev->gain[ad7770_CH5] = gain;
			dev->gain[ad7770_CH6] = gain;
			dev->gain[ad7770_CH7] = gain;
		}
		ret = ad7770_do_update_mode_pins(dev);
	} else {
		dev->gain[ch] = gain;
		ret = ad7770_spi_int_reg_write_mask(dev,
											ad7770_REG_CH_CONFIG(ch),
											ad7770_CH_GAIN(0x3),
											ad7770_CH_GAIN(gain));
	}

	return ret;
}

/**
 * Get the gain of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param gain - The gain value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_gain(ad7770_dev *dev,
						ad7770_ch ch,
						ad7770_gain *gain)
{
	*gain = dev->gain[ch];

	return SUCCESS;
}

/**
 * Set the decimation rate.
 * @param dev - The device structure.
 * @param integer_val - The integer value.
 * @param decimal_val - The decimal value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_dec_rate(ad7770_dev *dev,
							uint16_t int_val,
							uint16_t dec_val)
{
	int32_t ret;
	uint8_t msb;
	uint8_t lsb;

	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		switch (int_val) {
		case 128:
			break;
		case 256:
			break;
		case 512:
			break;
		case 1024:
			break;
		default:
			printf("%s: This setting can't be set in PIN control mode.\n",
				   __func__);
			return FAILURE;
		}
		dev->dec_rate_int = int_val;
		dev->dec_rate_int = dec_val;
		ret = ad7770_do_update_mode_pins(dev);
	} else {
		msb = (int_val & 0x0F00) >> 8;
		lsb = (int_val & 0x00FF) >> 0;
		ret = ad7770_spi_int_reg_write(dev,
									   ad7770_REG_SRC_N_MSB,
									   msb);
		ret |= ad7770_spi_int_reg_write(dev,
										ad7770_REG_SRC_N_LSB,
										lsb);
		//dec_val = (dec_val * 65536) / 1000;
		msb = (dec_val & 0xFF00) >> 8;
		lsb = (dec_val & 0x00FF) >> 0;
		ret |= ad7770_spi_int_reg_write(dev,
										ad7770_REG_SRC_IF_MSB,
										msb);
		ret |= ad7770_spi_int_reg_write(dev,
										ad7770_REG_SRC_IF_LSB,
										lsb);
		dev->dec_rate_int = int_val;
		dev->dec_rate_int = dec_val;
	}

	return ret;
}

/**
 * Get the decimation rate.
 * @param dev - The device structure.
 * @param integer_val - The integer value.
 * @param decimal_val - The decimal value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_dec_rate(ad7770_dev *dev,
							uint16_t *int_val,
							uint16_t *dec_val)
{
	*int_val = dev->dec_rate_int;
	*dec_val = dev->dec_rate_int;

	return SUCCESS;
}

/**
 * Set the power mode.
 * @param dev - The device structure.
 * @param pwr_mode - The power mode.
 * 					 Accepted values: ad7770_HIGH_RES
 *									  ad7770_LOW_PWR
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_power_mode(ad7770_dev *dev,
							  ad7770_pwr_mode pwr_mode)
{
	int32_t ret;

	/*ret = ad7770_spi_int_reg_write_mask(dev,
										ad7770_REG_GENERAL_USER_CONFIG_1,
										ad7770_MOD_POWERMODE,
										pwr_mode ? ad7770_MOD_POWERMODE : 0);*/
										
	ret = ad7770_spi_int_reg_write_mask(dev,
										ad7770_REG_GENERAL_USER_CONFIG_1,
										ad7770_MOD_POWERMODE,
										(pwr_mode ==  ad7770_HIGH_RES) ? ad7770_MOD_POWERMODE : 0);
										
	dev->pwr_mode = pwr_mode;

	return ret;
}

/**
 * Get the power mode.
 * @param dev - The device structure.
 * @param pwr_mode - The power mode.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_power_mode(ad7770_dev *dev,
							  ad7770_pwr_mode *pwr_mode)
{
	*pwr_mode = dev->pwr_mode;

	return SUCCESS;
}

/**
 * Set the reference type.
 * @param dev - The device structure.
 * @param pwr_mode - The reference type.
 * 					 Accepted values: ad7770_EXT_REF
 *									  ad7770_INT_REF
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_reference_type(ad7770_dev *dev,
								  ad7770_ref_type ref_type)
{
	int32_t ret;

	ret = ad7770_spi_int_reg_write_mask(dev,
										ad7770_REG_GENERAL_USER_CONFIG_1,
										ad7770_PDB_REFOUT_BUF,
										ref_type ? ad7770_PDB_REFOUT_BUF : 0);
	dev->ref_type = ref_type;

	return ret;
}

/**
 * Get the reference type.
 * @param dev - The device structure.
 * @param pwr_mode - The reference type.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_reference_type(ad7770_dev *dev,
								  ad7770_ref_type *ref_type)
{
	*ref_type = dev->ref_type;

	return SUCCESS;
}

/**
 * Set the DCLK divider.
 * @param dev - The device structure.
 * @param div - The DCLK divider.
 *				Accepted values: ad7770_DCLK_DIV_1
 *								 ad7770_DCLK_DIV_2
 *								 ad7770_DCLK_DIV_4
 *								 ad7770_DCLK_DIV_8
 *								 ad7770_DCLK_DIV_16
 *								 ad7770_DCLK_DIV_32
 *								 ad7770_DCLK_DIV_64
 *								 ad7770_DCLK_DIV_128
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_dclk_div(ad7770_dev *dev,
							ad7768_dclk_div div)
{
	int32_t ret;

	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		/*ret = gpio_set_value(&dev->gpio_dev,
						dev->gpio_dclk0,
						((div & 0x01) >> 0));
		ret |= gpio_set_value(&dev->gpio_dev,
						dev->gpio_dclk1,
						((div & 0x02) >> 1));
		ret |= gpio_set_value(&dev->gpio_dev,
						dev->gpio_dclk2,
						((div & 0x04) >> 2));*/
	} else {
		/*ret = ad7770_spi_int_reg_write_mask(dev,
											ad7770_REG_CH_DISABLE,
											ad7770_DCLK_CLK_DIV(0x3),
											ad7770_DCLK_CLK_DIV(div));*/
											
		ret = ad7770_spi_int_reg_write_mask(dev,
											ad7770_REG_DOUT_FORMAT,
											ad7770_DCLK_CLK_DIV(0x7),
											ad7770_DCLK_CLK_DIV(div));
	}
	dev->dclk_div = div;

	return ret;
}

/**
 * Get the DCLK divider.
 * @param dev - The device structure.
 * @param div - The DCLK divider.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_dclk_div(ad7770_dev *dev,
							ad7768_dclk_div *div)
{
	*div = dev->dclk_div;

	return SUCCESS;
}

/**
 * Set the synchronization offset of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param sync_offset - The synchronization offset value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_sync_offset(ad7770_dev *dev,
							   ad7770_ch ch,
							   uint8_t sync_offset)
{
	int32_t ret;

	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	ret = ad7770_spi_int_reg_write(dev,
								   ad7770_REG_CH_SYNC_OFFSET(ch),
								   sync_offset);
	dev->sync_offset[ch] = sync_offset;

	return ret;
}

/**
 * Get the synchronization offset of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param sync_offset - The synchronization offset value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_sync_offset(ad7770_dev *dev,
							   ad7770_ch ch,
							   uint8_t *sync_offset)
{
	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	*sync_offset = dev->sync_offset[ch];

	return SUCCESS;
}

/**
 * Set the offset correction of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param offset - The offset value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_offset_corr(ad7770_dev *dev,
							   ad7770_ch ch,
							   uint32_t offset)
{
	int32_t ret;
	uint8_t upper_byte;
	uint8_t mid_byte;
	uint8_t lower_byte;

	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	upper_byte = (offset & 0xFF0000) >> 16;
	mid_byte = (offset & 0x00FF00) >> 8;
	lower_byte = (offset & 0x0000FF) >> 0;
	ret = ad7770_spi_int_reg_write(dev,
								   ad7770_REG_CH_OFFSET_UPPER_BYTE(ch),
								   upper_byte);
	ret |= ad7770_spi_int_reg_write(dev,
									ad7770_REG_CH_OFFSET_MID_BYTE(ch),
									mid_byte);
	ret |= ad7770_spi_int_reg_write(dev,
									ad7770_REG_CH_OFFSET_LOWER_BYTE(ch),
									lower_byte);
	dev->offset_corr[ch] = offset;

	return ret;
}

/**
 * Get the offset correction of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param offset - The offset value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_offset_corr(ad7770_dev *dev,
							   ad7770_ch ch,
							   uint32_t *offset)
{
	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	*offset = dev->offset_corr[ch];

	return SUCCESS;
}

/**
 * Set the gain correction of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param gain - The gain value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_gain_corr(ad7770_dev *dev,
							 ad7770_ch ch,
							 uint32_t gain)
{
	int32_t ret;
	uint8_t upper_byte;
	uint8_t mid_byte;
	uint8_t lower_byte;

	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	gain &= 0xFFFFFF;
	upper_byte = (gain & 0xff0000) >> 16;
	mid_byte = (gain & 0x00ff00) >> 8;
	lower_byte = (gain & 0x0000ff) >> 0;
	ret = ad7770_spi_int_reg_write(dev,
								   ad7770_REG_CH_GAIN_UPPER_BYTE(ch),
								   upper_byte);
	ret |= ad7770_spi_int_reg_write(dev,
									ad7770_REG_CH_GAIN_MID_BYTE(ch),
									mid_byte);
	ret |= ad7770_spi_int_reg_write(dev,
									ad7770_REG_CH_GAIN_LOWER_BYTE(ch),
									lower_byte);
	dev->gain_corr[ch] = gain;

	return ret;
}

/**
 * Get the gain correction of the selected channel.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: ad7770_CH0
 * 			   					ad7770_CH1
 * 			   					ad7770_CH2
 * 			   					ad7770_CH3
 * 			   					ad7770_CH4
 * 			   					ad7770_CH5
 * 			   					ad7770_CH6
 * 			   					ad7770_CH7
 * @param gain - The gain value.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_gain_corr(ad7770_dev *dev,
							 ad7770_ch ch,
							 uint32_t *gain)
{
	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	*gain = dev->gain_corr[ch];

	return SUCCESS;
}

/**
 * Set the reference buffer operation mode of the selected pin.
 * @param dev - The device structure.
 * @param refx_pin - The selected pin.
 * 					 Accepted values: ad7770_REFX_P
 * 									  ad7770_REFX_N
 * @param mode - The reference buffer operation mode.
 * 				 Accepted values: ad7770_REF_BUF_ENABLED
 * 								  ad7770_REF_BUF_PRECHARGED
 * 								  ad7770_REF_BUF_DISABLED
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_ref_buf_op_mode(ad7770_dev *dev,
								   ad7770_refx_pin refx_pin,
								   ad7770_ref_buf_op_mode mode)
{
	int32_t ret;
	uint8_t config_1;
	uint8_t config_2;

	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	if (refx_pin == ad7770_REFX_P) {
		switch (mode) {
		case ad7770_REF_BUF_ENABLED:
			config_1 = ad7770_REF_BUF_POS_EN;
			config_2 = 0;
			break;
		case ad7770_REF_BUF_PRECHARGED:
			config_1 = ad7770_REF_BUF_POS_EN;
			config_2 = ad7770_REFBUFP_PREQ;
			break;
		default:
			config_1 = 0;
			config_2 = 0;
		}
		ret = ad7770_spi_int_reg_write_mask(dev,
											ad7770_REG_BUFFER_CONFIG_1,
											ad7770_REF_BUF_POS_EN,
											config_1);
		ret |= ad7770_spi_int_reg_write_mask(dev,
											 ad7770_REG_BUFFER_CONFIG_2,
											 ad7770_REFBUFP_PREQ,
											 config_2);
	} else {
		switch (mode) {
		case ad7770_REF_BUF_ENABLED:
			config_1 = ad7770_REF_BUF_NEG_EN;
			config_2 = 0;
			break;
		case ad7770_REF_BUF_PRECHARGED:
			config_1 = ad7770_REF_BUF_NEG_EN;
			config_2 = ad7770_REFBUFN_PREQ;
			break;
		default:
			config_1 = 0;
			config_2 = 0;
		}
		ret = ad7770_spi_int_reg_write_mask(dev,
											ad7770_REG_BUFFER_CONFIG_1,
											ad7770_REF_BUF_NEG_EN,
											config_1);
		ret |= ad7770_spi_int_reg_write_mask(dev,
											 ad7770_REG_BUFFER_CONFIG_2,
											 ad7770_REFBUFN_PREQ,
											 config_2);
	}
	dev->ref_buf_op_mode[refx_pin] = mode;

	return ret;
}

/**
 * Get the reference buffer operation mode of the selected pin.
 * @param dev - The device structure.
 * @param refx_pin - The selected pin.
 * 					 Accepted values: ad7770_REFX_P
 * 									  ad7770_REFX_N
 * @param mode - The reference buffer operation mode.
 * 				 Accepted values: ad7770_REF_BUF_ENABLED
 * 								  ad7770_REF_BUF_PRECHARGED
 * 								  ad7770_REF_BUF_DISABLED
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_ref_buf_op_mode(ad7770_dev *dev,
								   ad7770_refx_pin refx_pin,
								   ad7770_ref_buf_op_mode *mode)
{
	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	*mode = dev->ref_buf_op_mode[refx_pin];

	return SUCCESS;
}

/**
 * Set the SAR ADC configuration.
 * @param dev - The device structure.
 * @param state - The SAR ADC state.
 * 				  Accepted values: ad7770_ENABLE
 * 								   ad7770_DISABLE
 * @param mux - The SAR mux input configuration.
 * 				Accepted values: ad7770_AUXAINP_AUXAINN
 *								 ad7770_DVBE_AVSSX
 *								 ad7770_REF1P_REF1N
 *								 ad7770_REF2P_REF2N
 *								 ad7770_REF_OUT_AVSSX
 *								 ad7770_VCM_AVSSX
 *								 ad7770_AREG1CAP_AVSSX_ATT
 *								 ad7770_AREG2CAP_AVSSX_ATT
 *								 ad7770_DREGCAP_DGND_ATT
 *								 ad7770_AVDD1A_AVSSX_ATT
 *								 ad7770_AVDD1B_AVSSX_ATT
 *								 ad7770_AVDD2A_AVSSX_ATT
 *								 ad7770_AVDD2B_AVSSX_ATT
 *								 ad7770_IOVDD_DGND_ATT
 *								 ad7770_AVDD4_AVSSX
 *								 ad7770_DGND_AVSS1A_ATT
 *								 ad7770_DGND_AVSS1B_ATT
 *								 ad7770_DGND_AVSSX_ATT
 *								 ad7770_AVDD4_AVSSX_ATT
 *								 ad7770_REF1P_AVSSX
 *								 ad7770_REF2P_AVSSX
 *								 ad7770_AVSSX_AVDD4_ATT
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_set_sar_cfg(ad7770_dev *dev,
						   ad7770_state state,
						   ad7770_sar_mux mux)
{
	int32_t ret;

	ret = ad7770_spi_int_reg_write_mask(dev,
										ad7770_REG_GENERAL_USER_CONFIG_1,
										ad7770_PDB_SAR,
										(state == ad7770_ENABLE) ?
												ad7770_PDB_SAR : 0);
	ret |= ad7770_spi_int_reg_write(dev,
									ad7770_REG_GLOBAL_MUX_CONFIG,
									ad7770_GLOBAL_MUX_CTRL(mux));
	dev->sar_state = state;
	dev->sar_mux = mux;

	return ret;
}

/**
 * Get the SAR ADC configuration.
 * @param dev - The device structure.
 * @param state - The SAR ADC state.
 * @param mux - The SAR mux input configuration.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_get_sar_cfg(ad7770_dev *dev,
						   ad7770_state *state,
						   ad7770_sar_mux *mux)
{
	*state = dev->sar_state;
	*mux = dev->sar_mux;

	return SUCCESS;
}

/**
 * Do a single SAR conversion.
 * @param dev - The device structure.
 * @param mux - The SAR mux input configuration.
 * 				Accepted values: ad7770_AUXAINP_AUXAINN
 *								 ad7770_DVBE_AVSSX
 *								 ad7770_REF1P_REF1N
 *								 ad7770_REF2P_REF2N
 *								 ad7770_REF_OUT_AVSSX
 *								 ad7770_VCM_AVSSX
 *								 ad7770_AREG1CAP_AVSSX_ATT
 *								 ad7770_AREG2CAP_AVSSX_ATT
 *								 ad7770_DREGCAP_DGND_ATT
 *								 ad7770_AVDD1A_AVSSX_ATT
 *								 ad7770_AVDD1B_AVSSX_ATT
 *								 ad7770_AVDD2A_AVSSX_ATT
 *								 ad7770_AVDD2B_AVSSX_ATT
 *								 ad7770_IOVDD_DGND_ATT
 *								 ad7770_AVDD4_AVSSX
 *								 ad7770_DGND_AVSS1A_ATT
 *								 ad7770_DGND_AVSS1B_ATT
 *								 ad7770_DGND_AVSSX_ATT
 *								 ad7770_AVDD4_AVSSX_ATT
 *								 ad7770_REF1P_AVSSX
 *								 ad7770_REF2P_AVSSX
 *								 ad7770_AVSSX_AVDD4_ATT
 * @param sar_code - SAR conversion code.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_do_single_sar_conv(ad7770_dev *dev,
								  ad7770_sar_mux mux,
								  uint16_t *sar_code)
{
	ad7770_spi_op_mode restore_spi_op_mode;
	ad7770_state restore_sar_state;
	int32_t ret;

	restore_spi_op_mode = dev->spi_op_mode;
	restore_sar_state = dev->sar_state;
	ret = ad7770_set_sar_cfg(dev, ad7770_ENABLE, mux);
	ret |= ad7770_set_spi_op_mode(dev, ad7770_SAR_CONV);
	//ret |= gpio_set_value(&dev->gpio_dev, dev->gpio_convst_sar, GPIO_LOW);
	//mdelay(10);	// Acquisition Time = min 500 ns
	bcm2835_gpio_clr(dev->gpio_convst);
	usleep(10000);
	//ret |= gpio_set_value(&dev->gpio_dev, dev->gpio_convst_sar, GPIO_HIGH);
	//mdelay(10);	// Conversion Time = max 3.4 us
	bcm2835_gpio_set(dev->gpio_convst);
	usleep(10000);
	ad7770_spi_sar_read_code(dev, mux, sar_code);
	ret |= ad7770_set_sar_cfg(dev, restore_sar_state, mux);
	ret |= ad7770_set_spi_op_mode(dev, restore_spi_op_mode);

	return ret;
}

/**
 * Do a SPI software reset.
 * @param dev - The device structure.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_do_spi_soft_reset(ad7770_dev *dev)
{
	uint8_t buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	int32_t ret;

	/* Keeping the SDI pin high during 64 consecutives clocks generates a
	   software reset */
	//ret = spi_write_and_read(&dev->spi_dev, buf, 8);
	ret = SUCCESS;
	bcm2835_spi_transfern(buf, 8);

	return ret;
}

/**
 * Set the state (enable, disable) of the SINC5 filter.
 * @param dev - The device structure.
 * @param state - The SINC5 filter state.
 * 				  Accepted values: ad7770_ENABLE
 * 								   ad7770_DISABLE
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7771_set_sinc5_filter_state(ad7770_dev *dev,
									  ad7770_state state)
{
	int32_t ret;

	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	ret = ad7770_spi_int_reg_write_mask(dev,
										ad7770_REG_GENERAL_USER_CONFIG_2,
										AD7771_FILTER_MODE,
										(state == ad7770_ENABLE) ?
												AD7771_FILTER_MODE : 0);
	dev->sinc5_state = state;

	return ret;
}

/**
 * Get the state (enable, disable) of the SINC5 filter.
 * @param dev - The device structure.
 * @param state - The SINC5 filter state.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7771_get_sinc5_filter_state(ad7770_dev *dev,
									  ad7770_state *state)
{
	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		printf("%s: This feature is not available in PIN control mode.\n",
			   __func__);
		return FAILURE;
	}

	*state = dev->sinc5_state;

	return SUCCESS;
}

/**
 * Initialize the device.
 * @param device - The device structure.
 * @param init_param - The structure that contains the device initial
 * 					   parameters.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_setup(ad7770_dev **device,
					 ad7770_init_param init_param)
{
	ad7770_dev *dev;
	uint8_t i, data_rst;
	int32_t ret = 0;

	dev = (ad7770_dev *)malloc(sizeof(*dev));
	if (!dev) {
		return FAILURE;
	}

	/* SPI */
	/*dev->spi_dev.chip_select = init_param.spi_chip_select;
	dev->spi_dev.mode = init_param.spi_mode;
	dev->spi_dev.device_id = init_param.spi_device_id;
	dev->spi_dev.type = init_param.spi_type;
	ret = spi_init(&dev->spi_dev);

	dev->gpio_dev.device_id = init_param.gpio_device_id;
	dev->gpio_dev.type = init_param.gpio_type;
	ret |= gpio_init(&dev->gpio_dev);*/

	/* GPIO */
	dev->gpio_start = init_param.gpio_start;
	dev->gpio_reset_n = init_param.gpio_reset_n;
	dev->gpio_test = init_param.gpio_test;
	dev->gpio_drdy_n = init_param.gpio_drdy_n;
	dev->gpio_convst = init_param.gpio_convst;
	dev->gpio_osc = init_param.gpio_osc;	
	
	// Hardware reset because of p.37 INTEGRATED LDOs
	// and control trough REG_GEN_ERR_REG_2 bit 5
	dev->spi_crc_en = ad7770_DISABLE; // to have no CRC problem with this 1st message
	uint32_t count=0;
	do{
		bcm2835_gpio_clr(dev->gpio_reset_n);
		usleep(10000);
		bcm2835_gpio_set(dev->gpio_reset_n);
		usleep(10000);
		ad7770_spi_int_reg_read_mask(dev, ad7770_REG_GEN_ERR_REG_2, 0x20, &data_rst);
		count++;
	}while(data_rst == 0x00);
	//printf("count = %d\n", count);
	/*ret |= gpio_set_direction(&dev->gpio_dev, dev->gpio_reset, GPIO_OUT);
	ret |= gpio_set_value(&dev->gpio_dev, dev->gpio_reset, GPIO_LOW);
	mdelay(10);	// RESET Hold Time = min 2 × MCLK
	ret |= gpio_set_value(&dev->gpio_dev, dev->gpio_reset, GPIO_HIGH);
	mdelay(10);	// RESET Rising Edge to First DRDY = min 225 us*/

	/* Device Settings */
	dev->ctrl_mode = init_param.ctrl_mode;
	dev->spi_crc_en = ad7770_DISABLE;
	dev->spi_op_mode = ad7770_SD_CONV;
	dev->sar_state = ad7770_DISABLE;
	dev->sar_mux = ad7770_AUXAINP_AUXAINN;

	// Enable CRC in SPI transmission trough ad7770_REG_GEN_ERR_REG_1_EN bit 0 
	if ((dev->ctrl_mode == ad7770_SPI_CTRL) && (init_param.spi_crc_en == ad7770_ENABLE)) {
		ret |= ad7770_spi_int_reg_read(dev,
							ad7770_REG_GEN_ERR_REG_1_EN,
							&dev->cached_reg_val[ad7770_REG_GEN_ERR_REG_1_EN]);
		ret |= ad7770_spi_int_reg_write_mask(dev,
							ad7770_REG_GEN_ERR_REG_1_EN,
							ad7770_SPI_CRC_TEST_EN,
							ad7770_SPI_CRC_TEST_EN);
		dev->spi_crc_en = ad7770_ENABLE;
	}
	//printf("test1\n");
	// Read all the registers and save them
	if (dev->ctrl_mode == ad7770_SPI_CTRL){
		for (i = ad7770_REG_CH_CONFIG(0); i <= ad7770_REG_SRC_UPDATE; i++){
			if(i == 0x1b) i++;
			dev->reset_reg_val[i] = reset_reg_val[i];
			ret |= ad7770_spi_int_reg_read(dev, i, &dev->cached_reg_val[i]);
		}
	}
	
	// Powerdown Vcm
	//ret |= ad7770_spi_int_reg_write_mask(dev, ad7770_REG_GENERAL_USER_CONFIG_1, ad7770_PDB_VCM, 0x00);
	
	// Set the drive strength to extra strong
	ret |= ad7770_spi_int_reg_write_mask(dev, ad7770_REG_GENERAL_USER_CONFIG_2, ad7770_SDO_DRIVE_STR(0x03), ad7770_SDO_DRIVE_STR(0x03));
	
	// Set the state of each channels (enable or disable)
	for (i = ad7770_CH0; i <= ad7770_CH7; i++) {
		dev->state[i] = init_param.state[i];
		if (dev->ctrl_mode == ad7770_SPI_CTRL)
			ret |= ad7770_set_state(dev, (ad7770_ch)i, dev->state[i]);
	}
	
	// Set the gain of each channels
	for (i = ad7770_CH0; i <= ad7770_CH7; i++) {
		dev->gain[i] = init_param.gain[i];
		if (dev->ctrl_mode == ad7770_SPI_CTRL)
			ret |= ad7770_set_gain(dev, (ad7770_ch)i, dev->gain[i]);
	}
	
	// Set the deciaml rate to configure the data rate
	dev->dec_rate_int = init_param.dec_rate_int;
	dev->dec_rate_dec = init_param.dec_rate_dec;
	if (dev->ctrl_mode == ad7770_SPI_CTRL)
		ret |= ad7770_set_dec_rate(dev, dev->dec_rate_int, dev->dec_rate_dec);
	// Update decimation (ODR)
	ret |= ad7770_spi_int_reg_write_mask(dev, ad7770_REG_SRC_UPDATE, 0x01, 0x01);
	usleep(1000);
	ret |= ad7770_spi_int_reg_write_mask(dev, ad7770_REG_SRC_UPDATE, 0x01, 0x00);
	usleep(1000);
	
	// Set the reference type
 	dev->ref_type = init_param.ref_type;
	if (dev->ctrl_mode == ad7770_SPI_CTRL)
		ret |= ad7770_set_reference_type(dev, dev->ref_type);
	
	// Set the power mode
	dev->pwr_mode = init_param.pwr_mode;
	/*if (dev->ctrl_mode == ad7770_SPI_CTRL)
		ret |= ad7770_set_reference_type(dev, dev->ref_type);*/
	if (dev->ctrl_mode == ad7770_SPI_CTRL)
		ret |= ad7770_set_power_mode(dev, dev->pwr_mode);
	
	if (dev->ctrl_mode == ad7770_PIN_CTRL) {
		ret |= ad7770_do_update_mode_pins(dev);
	}
	// Update power mode
	bcm2835_gpio_clr(dev->gpio_start);
	usleep(1000);
	bcm2835_gpio_set(dev->gpio_start);
	usleep(1000);
	
	// Set DCLK divider
	dev->dclk_div = init_param.dclk_div;
	ad7770_set_dclk_div(dev, dev->dclk_div);
	//printf("test2 ret = %d\n", ret);
	// Set sync. offset, gain offset and corr,
	for (i = ad7770_CH0; i <= ad7770_CH7; i++) {
		//printf("------------------------------------------------ ch = %d\n",i);
		dev->sync_offset[i] = init_param.sync_offset[i];
		dev->offset_corr[i] = init_param.offset_corr[i];
		dev->gain_corr[i] = init_param.gain_corr[i];
		if (dev->ctrl_mode == ad7770_SPI_CTRL) {
			ret |= ad7770_set_sync_offset(dev, (ad7770_ch)i,
								dev->sync_offset[i]);
			//printf("test sync\n");
			// Update phase
			bcm2835_gpio_clr(dev->gpio_start);
			usleep(1000);
			bcm2835_gpio_set(dev->gpio_start);
			usleep(1000);
			ret |= ad7770_set_offset_corr(dev, (ad7770_ch)i,
								dev->offset_corr[i]);
			//printf("test offset\n");
			// Update offset
			bcm2835_gpio_clr(dev->gpio_start);
			usleep(1000);
			bcm2835_gpio_set(dev->gpio_start);
			usleep(1000);					
			// Not shure we need to change the gain
			if(dev->gain_corr[i] != 1) ret |= ad7770_set_gain_corr(dev, (ad7770_ch)i,
								dev->gain_corr[i]);
			//printf("test gain\n"); 
			// Update gain
			bcm2835_gpio_clr(dev->gpio_start);
			usleep(1000);
			bcm2835_gpio_set(dev->gpio_start);
			usleep(1000);
		}
	}
	//printf("test6 ret = %d\n", ret);
	
	// Set sigma-delta in spi mode
	ret |= ad7770_set_spi_op_mode(dev, dev->spi_op_mode);
	//printf("test7 ret = %d\n", ret);
	
	
	/*bcm2835_gpio_clr(dev->gpio_start);
	usleep(10000);
	bcm2835_gpio_set(dev->gpio_start);
	usleep(10000);*/
	
	*device = dev;
	
	if (ret == SUCCESS)
		printf("ad7770 successfully initialized\n");
	else
		printf("ad7770 initialization error (%d)\n", ret);

	return ret;
}

/**
 * Control the state of all the register, and compare it with the reset value
 * @param dev - The device structure.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad7770_ctrl_all_reg(ad7770_dev *dev, bool all_reg)
{
	int i, start_reg;
	int32_t ret = 0;
	uint8_t	reg_val[ad7770_REG_SRC_UPDATE + 1];
	// Return in register mode, to control the ad7770
	ret |= ad7770_set_spi_op_mode(dev, ad7770_INT_REG);
	
	if(all_reg == true) start_reg = ad7770_REG_CH_CONFIG(0);
	else start_reg = ad7770_REG_CH_ERR_REG(0);
	
	for (i = start_reg; i <= ad7770_REG_SRC_UPDATE; i++){
			if(i == 0x1b) i++;
			if(ad7770_spi_int_reg_read(dev, i, &reg_val[i]) == FAILURE){
				//i = (ad7770_REG_SRC_UPDATE + 1);
				printf("Failure during registers reading\n");
				ret |= FAILURE;
			}
			else{
				/*printf("reg 0x%02X) 0x%02X, 0x%02X, 0x%02X", i, dev->reset_reg_val[i], dev->cached_reg_val[i], reg_val[i]);
				if(dev->reset_reg_val[i] == dev->cached_reg_val[i]) printf("  RESET Cached");
				else printf("              ");
				if(dev->reset_reg_val[i] == reg_val[i]) printf("  RESET Now");
				else printf("           ");
				if(reg_val[i] != dev->cached_reg_val[i]) printf("  PROBLEM");
				printf("\n");	*/
				if(reg_val[i] != dev->cached_reg_val[i]){
					printf("reg 0x%02X) 0x%02X, 0x%02X, 0x%02X", i, dev->reset_reg_val[i], dev->cached_reg_val[i], reg_val[i]);
					printf("  PROBLEM");
					printf("\n");
				}
			}											
	}
	
	// Set sigma-delta in spi mode
	ret |= ad7770_set_spi_op_mode(dev, ad7770_SD_CONV);	

	return ret;
}
