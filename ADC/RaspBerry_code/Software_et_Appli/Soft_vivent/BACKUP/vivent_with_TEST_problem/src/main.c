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
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <bcm2835.h>
#include "../include/ad7770.h"

#define START		RPI_V2_GPIO_P1_03
#define RESET_n		RPI_V2_GPIO_P1_05
#define TEST		RPI_V2_GPIO_P1_07
#define DRDY_n		RPI_V2_GPIO_P1_11
#define CONVST		RPI_V2_GPIO_P1_13
#define OSC_1		RPI_V2_GPIO_P1_15
#define OSC_2		RPI_V2_GPIO_P1_16

#define PORT 		50005
#define IP_ADDRESS	"160.98.87.167" //"160.98.86.179"  

int main(int argc, char **argv)
{   	
	// Variables
	ad7770_dev *device;
	ad7770_init_param init_param;
	int i;
	uint32_t j,k;
	char buf_rx[32];
    char buf_tx[32] = {0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0};
	int32_t data[9];
	int32_t value;
	int count_test = 0;
	int flag_test = 0;
	
	
	//DHCP
	struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char hello[100];
    char buffer[1024] = {0};
	
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
    bcm2835_gpio_set_pud(OSC_1, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(OSC_2, BCM2835_GPIO_PUD_OFF);
    
    bcm2835_gpio_fsel(START, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RESET_n, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(TEST, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(DRDY_n, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(CONVST, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OSC_1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OSC_2, BCM2835_GPIO_FSEL_OUTP);
    
    bcm2835_gpio_set(START);
    bcm2835_gpio_set(RESET_n);
    bcm2835_gpio_set_eds(TEST);
    bcm2835_gpio_hen(TEST);
    bcm2835_gpio_set_eds(DRDY_n);// bcm2835_gpio_eds(DRDY_n) = HIGH when '1' state is detected, then use bcm2835_gpio_set_eds(DRDY_n) to clear the flag
    bcm2835_gpio_hen(DRDY_n);
    bcm2835_gpio_set(CONVST);
    bcm2835_gpio_set(OSC_1);
    bcm2835_gpio_set(OSC_2);  
	
	// AD7770 inialisation
	init_param.gpio_start = START;
	init_param.gpio_reset_n = RESET_n;
	init_param.gpio_test = TEST;
	init_param.gpio_drdy_n = DRDY_n;
	init_param.gpio_convst = CONVST;
	init_param.gpio_osc = OSC_1;	
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
		init_param.gain_corr[i] = 1;//0x555555;
	}
	init_param.ref_buf_op_mode[1] = ad7770_REF_BUF_DISABLED;
	init_param.ref_buf_op_mode[2] = ad7770_REF_BUF_DISABLED;
	init_param.sinc5_state = ad7770_DISABLE;
	
	if (ad7770_setup(&device, init_param) != SUCCESS) return 0;
	
	//DHCP initialisation
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    else printf("Socket successfully created\n");
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, IP_ADDRESS, &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    else printf("IP adress valid\n");
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    else printf("DHCP successfully connected\n");
    
    bcm2835_gpio_clr(OSC_1);
    usleep(50000);
    bcm2835_gpio_set(OSC_1);
    
    bcm2835_gpio_set_eds(TEST);
	k = 0;
	while(1){//	for(j = 0; j < 0x3ffff; j++){	//0x3ffff
		if(bcm2835_gpio_eds(TEST) == HIGH){
			bcm2835_gpio_clr(OSC_1);
			bcm2835_gpio_set(OSC_1);
			bcm2835_gpio_set_eds(TEST);
			count_test = 1;
			flag_test = 1;
		}
		else{
			if(count_test != 0){
				ad7770_ctrl_all_reg(device, true);
				ad7770_ctrl_all_reg(device, true);
				count_test = 0;
			}
			flag_test = 0;
		}
		bcm2835_gpio_set_eds(DRDY_n);
		while(bcm2835_gpio_eds(DRDY_n) == LOW);
		//bcm2835_gpio_clr(OSC_1);
		bcm2835_spi_transfernb(buf_tx, buf_rx, 32);
		for(i = 0; i < 32; i += 4){
			data[i>>2] = ((buf_rx[i+1] << 24) | (buf_rx[i+2] << 16) | (buf_rx[i+3] << 8)) >> 8;
		}
		k++;
		if(k >= 2000){
			data[0] = 0;
			data[1] = 0;
			//data[2] = 0; // problem on this channel on 1st AD7770_EVAL
			data[3] = 0;
			data[4] = 0;
			data[5] = 0;
			data[6] = 0;
			data[7] = 0;
			if(flag_test > 0){
				if(data[8] > 0) data[8] = -10000000;
				else data[8] = 10000000;
			}
			else data[8] = 0;
			sprintf(hello, "%d %d %d %d %d %d %d %d %d", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
			send(sock , hello ,(strlen(hello) + 1) , 0 );
			k = 0;
			printf("%s\n",hello);
		}
		//valread = read( sock , buffer, 1024);
		/*for(i = 0; i < 32; i += 4){
			value = ((buf_rx[i+1] << 24) | (buf_rx[i+2] << 16) | (buf_rx[i+3] << 8)) >> 8;
			printf("%d) alert = %d, ch = %d, CRC = %d,  value = %d,  %02X %02X %02X %02X\n",
			 (i/4), ((buf_rx[i] >> 7) & 0x01), ((buf_rx[i] >> 4) & 0x07), (buf_rx[i] & 0x0f), value, buf_rx[i], buf_rx[i+1], buf_rx[i+2], buf_rx[i+3]);							
		}
		printf("\n");*/
		//bcm2835_gpio_set(OSC_1);
	}
	
	/*printf("%s\n",hello );
	for(i=0;i<32;i+=4) printf("0x%02X 0x%02X 0x%02X 0x%02X\n", buf_rx[i], buf_rx[i+1], buf_rx[i+2], buf_rx[i+3]);
	
	for(i = 0; i < 32; i += 4){
		value = ((buf_rx[i+1] << 24) | (buf_rx[i+2] << 16) | (buf_rx[i+3] << 8)) >> 8;
		printf("%d) alert = %d, ch = %d, CRC = %d,  value = %d,  %02X %02X %02X %02X\n",
		 (i/4), ((buf_rx[i] >> 7) & 0x01), ((buf_rx[i] >> 4) & 0x07), (buf_rx[i] & 0x0f), value, buf_rx[i], buf_rx[i+1], buf_rx[i+2], buf_rx[i+3]);							
	}*/
	
	bcm2835_gpio_clr(OSC_1);
    usleep(50000);
    bcm2835_gpio_set(OSC_1);
	
	printf("\n--------- CONTROL STATE REGISTERS-------------\n");	
	ad7770_ctrl_all_reg(device, true);
	printf("-------END CONTROL STATE REGISTERS------------\n");
	    
    bcm2835_gpio_clr_hen(DRDY_n);
    bcm2835_gpio_clr_hen(TEST);
    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}

