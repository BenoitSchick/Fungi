// main.c
//
// Author: Anthony Schluchin

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <dirent.h> 
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
#define LED_ERR		RPI_V2_GPIO_P1_15
#define LED_RDY		RPI_V2_GPIO_P1_16
#define CONNECTED	RPI_V2_GPIO_P1_18
#define SWITCH		RPI_V2_GPIO_P1_12
#define LED_RUN		RPI_V2_GPIO_P1_22

#define PORT 		50005
#define IP_ADDRESS	"160.98.87.167" //"169.254.109.27"//"160.98.116.37"//"160.98.86.179"  

int main(int argc, char **argv)
{   	
	// Variables
	ad7770_dev *device;
	ad7770_init_param init_param;
	int i;
	uint32_t j,decimation;
	char buf_rx[32];
    char buf_tx[32] = {0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0};
	int32_t data[9];
	
	
	//DHCP
	struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char data_str[100];
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
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // CS0 in use
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // CS drived to '0'
    
    // GPIO initialisation
    bcm2835_gpio_set_pud(START, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RESET_n, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(TEST, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(DRDY_n, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(CONVST, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(CONNECTED, BCM2835_GPIO_PUD_DOWN);
    bcm2835_gpio_set_pud(SWITCH, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(LED_ERR, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(LED_RDY, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(LED_RUN, BCM2835_GPIO_PUD_OFF);
    
    bcm2835_gpio_fsel(START, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RESET_n, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(TEST, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(DRDY_n, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(CONVST, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(CONNECTED, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(SWITCH, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(LED_ERR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(LED_RDY, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(LED_RUN, BCM2835_GPIO_FSEL_OUTP);
    
    bcm2835_gpio_set(START);
    bcm2835_gpio_set(RESET_n);
    bcm2835_gpio_hen(TEST);
    bcm2835_gpio_set_eds(TEST);
    bcm2835_gpio_hen(DRDY_n);
    bcm2835_gpio_set_eds(DRDY_n);// bcm2835_gpio_eds(DRDY_n) = HIGH when '1' state is detected, then use bcm2835_gpio_set_eds(DRDY_n) to clear the flag
    bcm2835_gpio_set(CONVST);
    bcm2835_gpio_set(LED_ERR);
    bcm2835_gpio_set(LED_RDY);  
    bcm2835_gpio_set(LED_RUN);  
    
	// Waiting on switch
	bcm2835_gpio_clr(LED_RDY);
	if(bcm2835_gpio_lev(SWITCH) == HIGH) printf("Waiting on switch to start\n");
	while(bcm2835_gpio_lev(SWITCH) == HIGH) usleep(250000);
	bcm2835_gpio_set(LED_RDY);
    
	// Test if AD is connected to the Raspberry
	if(bcm2835_gpio_lev(CONNECTED)== LOW){
		printf("AD7770 is power off or not connected!\n");
		return -1;
	}
	
	// AD7770 inialisation
	init_param.gpio_start = START;
	init_param.gpio_reset_n = RESET_n;
	init_param.gpio_test = TEST;
	init_param.gpio_drdy_n = DRDY_n;
	init_param.gpio_convst = CONVST;
	init_param.gpio_connected = CONNECTED;	
	init_param.gpio_switch = SWITCH;
	init_param.gpio_led_err = LED_ERR;
	init_param.gpio_led_rdy = LED_RDY;
	init_param.gpio_led_run = LED_RUN;
	init_param.ctrl_mode = ad7770_SPI_CTRL;
	init_param.spi_crc_en = ad7770_ENABLE;
	for (i = ad7770_CH0; i <= ad7770_CH7; i++) init_param.state[i] = ad7770_ENABLE;
	for (i = ad7770_CH0; i <= ad7770_CH7; i++) init_param.gain[i] = ad7770_GAIN_1;
	init_param.dec_rate_int = 0x1900;	// data rate = 0.32kHz -> decimation = 2048/0.32 = 6400
	init_param.dec_rate_dec = 0x0000;	// 6400 = 0x1900  0*2^16 = 0 = 0
	//init_param.dec_rate_int = 0x0c80;	// data rate = 0.64kHz -> decimation = 2048/0.64 = 3200
	//init_param.dec_rate_dec = 0x0000;	// 3200 = 0xc80  0*2^16 = 0 = 0
	//init_param.dec_rate_int = 0x0080;	// data rate = 16kHz -> decimation = 2048/16= 128
	//init_param.dec_rate_dec = 0x0000;	// 128 = 0x0080  0*2^16 = 26'214 = 0x0000
	//init_param.dec_rate_int = 0x0fff;	// data rate = 0.5kHz -> decimation = 2048/0.5= 4096, but in doc. (p. 58) -> 4095
	//init_param.dec_rate_dec = 0x0000;	// 4095 = 0x0fff  0*2^16 = 0 = 0
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
	//while(1){
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
    
	
		while(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			printf("\nConnection Failed \n");
		}
		printf("DHCP successfully connected\n");
		
		usleep(50000);
		bcm2835_gpio_clr(LED_RUN);
		
		bcm2835_gpio_set_eds(TEST);

		decimation = 0;
		for(i=0; i<9; i++) data[i] = 0;
		while(bcm2835_gpio_lev(SWITCH) == LOW){
			if(bcm2835_gpio_eds(TEST) == HIGH){
				bcm2835_gpio_clr(LED_ERR);
				printf("\n TEST is high\n");
				sprintf(data_str, "0 0 0 0 0 0 0 0 -10000000");
				send(sock , data_str ,(strlen(data_str) + 1) , 0 );
				//if(ad7770_ctrl_all_reg(device, true) == FAILURE){
					// problem with registers !
				//}
				usleep(25000);
				sprintf(data_str, "0 0 0 0 0 0 0 0 10000000");
				send(sock , data_str ,(strlen(data_str) + 1) , 0 );
				usleep(25000);
				bcm2835_gpio_set_eds(TEST);
			}
			else{
				bcm2835_gpio_set(LED_ERR);
				bcm2835_gpio_set_eds(DRDY_n);
				while(bcm2835_gpio_eds(DRDY_n) == LOW);
				bcm2835_spi_transfernb(buf_tx, buf_rx, 32);
				for(i = 0; i < 32; i += 4){
					data[i>>2] += ((buf_rx[i+1] << 24) | (buf_rx[i+2] << 16) | (buf_rx[i+3] << 8)) >> 8;
				}
				//printf("%d) %d %d %d %d %d %d %d %d %d\n", decimation, data[0], data[1],
				//	 data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
				decimation++;
				if(decimation >= 32){
					for (i = 0; i < 9; i++) data[i] = (data[i] >> 5);
					data[0] = 0;
					data[1] = 0;
					data[2] = 0;
					data[4] = 0;
					data[5] = 0;
					data[6] = 0;
					data[7] = 0;
					//data[8] = 0;
					//printf("%d %d %d %d %d %d %d %d %d\n", data[0], data[1],
					// data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
					sprintf(data_str, "%d %d %d %d %d %d %d %d %d", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
					if(send(sock , data_str ,(strlen(data_str) + 1) , 0 ) < 0){
						perror("Problem");
						break;
					}
					decimation = 0;
					for (i = 0; i < 9; i++) data[i] = 0;
				}
			}
		}
		
		usleep(50000);
		
		printf("\n--------- CONTROL STATE REGISTERS-------------\n");	
		ad7770_ctrl_all_reg(device, true);
		printf("-------END CONTROL STATE REGISTERS------------\n");
		
		/*if(shutdown(sock, 2) < 0){
			perror("Socket closing");
			return -1;
		}
		else{
			printf("Socket successfully closed\n");
			sleep(2);
		}*/
		sleep(2);
	//}
	
	bcm2835_gpio_set(LED_RUN);
	bcm2835_gpio_set(LED_RDY);
	bcm2835_gpio_set(LED_ERR);
    bcm2835_gpio_clr_hen(DRDY_n);
    bcm2835_gpio_clr_hen(TEST);
    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}

