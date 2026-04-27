// spin.c
//
// Example program for bcm2835 library
// Shows how to interface with SPI to transfer a number of bytes to and from an SPI device
//
// After installing bcm2835, you can build this 
// with something like:
// gcc -o spin spin.c -l bcm2835
// sudo ./spin
//
// Or you can test it before installing with:
// gcc -o spin -I ../../src ../../src/bcm2835.c spin.c
// sudo ./spin
//
// Author: Mike McCauley
// Copyright (C) 2012 Mike McCauley
// $Id: RF22.h,v 1.21 2012/05/30 01:51:25 mikem Exp $

#include <stdio.h>
#include <unistd.h>
#include <bcm2835.h>
#include "../include/ad7770.h"

#define START		RPI_V2_GPIO_P1_03
#define RESET_n		RPI_V2_GPIO_P1_05
#define TEST		RPI_V2_GPIO_P1_07
#define DRDY_n		RPI_V2_GPIO_P1_11
#define CONVST		RPI_V2_GPIO_P1_13
#define OSC			RPI_V2_GPIO_P1_15

int main(int argc, char **argv)
{   	
	// Variables
	ad7770_dev *device;
	ad7770_init_param init_param;
	int i,j;
	
	// Library bcm2835 initialisation
    if (!bcm2835_init())
    {
      printf("bcm2835_init failed. Are you running as root??\n");
      return 1;
    }

	// SPI initialisation
    if (!bcm2835_spi_begin())
    {
      printf("bcm2835_spi_begin failed. Are you running as root??\n");
      return 1;
    }
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // transmission: MSB first
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // mode 0
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16); 	  // 16 MHz
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8); 	  // 31.25 MHz
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // CS0 in use
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // CS drived to '0'
    
    // GPIO initialisation
    bcm2835_gpio_set_pud(START, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RESET_n, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(TEST, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(DRDY_n, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(CONVST, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(OSC, BCM2835_GPIO_PUD_OFF);
    
    bcm2835_gpio_fsel(START, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RESET_n, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(TEST, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(DRDY_n, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(CONVST, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OSC, BCM2835_GPIO_FSEL_OUTP);
    
    bcm2835_gpio_set(START);
    bcm2835_gpio_set(RESET_n);
    bcm2835_gpio_set_eds(DRDY_n);// bcm2835_gpio_eds(DRDY_n) = HIGH when '1' state is detected, then use bcm2835_gpio_set_eds(DRDY_n) to clear the flag
    bcm2835_gpio_hen(DRDY_n);
    bcm2835_gpio_set(CONVST);
    bcm2835_gpio_set(OSC);  
	
	// AD7770 inialisation
	init_param.gpio_start = START;
	init_param.gpio_reset_n = RESET_n;
	init_param.gpio_test = TEST;
	init_param.gpio_drdy_n = DRDY_n;
	init_param.gpio_convst = CONVST;
	init_param.gpio_osc = OSC;	
	init_param.ctrl_mode = ad7770_SPI_CTRL;
	init_param.spi_crc_en = ad7770_ENABLE;
	for (i = ad7770_CH0; i <= ad7770_CH7; i++) init_param.state[i] = ad7770_ENABLE;
	for (i = ad7770_CH0; i <= ad7770_CH7; i++) init_param.gain[i] = ad7770_GAIN_1;
	init_param.dec_rate_int = 0x0066;	// data rate = 20kHz -> decimation = 2048/20 = 102.4
	init_param.dec_rate_dec = 0x6666;	// 102 = 0x66  0.4*2^16 = 26'214 = 0x6666
	init_param.ref_type = ad7770_EXT_REF;
	init_param.pwr_mode = ad7770_HIGH_RES;
	init_param.dclk_div = ad7770_DCLK_DIV_1;
	for (i = ad7770_CH0; i <= ad7770_CH7; i++){
		init_param.sync_offset[i] = 0;
		init_param.offset_corr[i] = 0;
		init_param.gain_corr[i] = 0x555555;
	}
	init_param.ref_buf_op_mode[1] = ad7770_REF_BUF_DISABLED;
	init_param.ref_buf_op_mode[2] = ad7770_REF_BUF_DISABLED;
	init_param.sinc5_state = ad7770_DISABLE;
	
	if (ad7770_setup(&device, init_param) != SUCCESS) return 0;
    
    char buf_rx[32];
    char buf_tx[32] = {0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0};
	int32_t value;
	
	bcm2835_gpio_clr(OSC);
	
	for(j = 0; j < 50; j++){
		bcm2835_gpio_set_eds(DRDY_n);
		while(bcm2835_gpio_eds(DRDY_n) == LOW);
		//bcm2835_gpio_clr(OSC);
		bcm2835_spi_transfernb(buf_tx, buf_rx, 32);
		/*for(i = 0; i < 32; i += 4){
			value = ((buf_rx[i+1] << 24) | (buf_rx[i+2] << 16) | (buf_rx[i+3] << 8)) >> 8;
			printf("%d) alert = %d, ch = %d, CRC = %d,  value = %d,  %02X %02X %02X %02X\n",
			 (i/4), ((buf_rx[i] >> 7) & 0x01), ((buf_rx[i] >> 4) & 0x07), (buf_rx[i] & 0x0f), value, buf_rx[i], buf_rx[i+1], buf_rx[i+2], buf_rx[i+3]);							
		}
		printf("\n");*/
		//bcm2835_gpio_set(OSC);
	}
	bcm2835_gpio_set(OSC);
	
	for(i = 0; i < 32; i += 4){
		value = ((buf_rx[i+1] << 24) | (buf_rx[i+2] << 16) | (buf_rx[i+3] << 8)) >> 8;
		printf("%d) alert = %d, ch = %d, CRC = %d,  value = %d,  %02X %02X %02X %02X\n",
		 (i/4), ((buf_rx[i] >> 7) & 0x01), ((buf_rx[i] >> 4) & 0x07), (buf_rx[i] & 0x0f), value, buf_rx[i], buf_rx[i+1], buf_rx[i+2], buf_rx[i+3]);							
	}
	printf("\n");
	
	ad7770_set_spi_op_mode(device, ad7770_INT_REG);
	uint8_t	reg_val[ad7770_REG_SRC_UPDATE + 1];
	
	for (i = ad7770_REG_CH_CONFIG(0); i <= ad7770_REG_SRC_UPDATE; i++){
			if(i == 0x1b) i++;
			if(ad7770_spi_int_reg_read(device, i, &reg_val[i]) == FAILURE){
				i = (ad7770_REG_SRC_UPDATE + 1);
				printf("Failure during regsiters reading\n");
			}
			else{
				/*printf("reg 0x%02X) 0x%02X, 0x%02X, 0x%02X", i, device->reset_reg_val[i], device->cached_reg_val[i], reg_val[i]);
				if(device->reset_reg_val[i] == device->cached_reg_val[i]) printf("  RESET Cached");
				else printf("              ");
				if(device->reset_reg_val[i] == reg_val[i]) printf("  RESET Now");
				else printf("           ");
				if(reg_val[i] != device->cached_reg_val[i]) printf("  PROBLEM");
				printf("\n");	*/
				if(reg_val[i] != device->cached_reg_val[i]){
					printf("reg 0x%02X) 0x%02X, 0x%02X, 0x%02X", i, device->reset_reg_val[i], device->cached_reg_val[i], reg_val[i]);
					printf("  PROBLEM");
					printf("\n");
				}
			}											
	}
	    
    bcm2835_gpio_clr_hen(DRDY_n);
    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}

