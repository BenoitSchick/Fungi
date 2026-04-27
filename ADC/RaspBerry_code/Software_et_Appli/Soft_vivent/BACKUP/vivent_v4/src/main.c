// main.c
//
// Author: Anthony Schluchin
// 
// This programm is used to save the data from the AD into a USB

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
#define IP_ADDRESS	"160.98.87.167" //"160.98.86.179"  

int main(int argc, char **argv)
{   	
	// Variables
	ad7770_dev *device;
	ad7770_init_param init_param;
	int i, flag_error;
	uint32_t j,decimation;
	char buf_rx[32];
    char buf_tx[32] = {0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0,0x80,0x0};
	int32_t data[9];
	uint32_t nbr_error = 0;
	uint32_t nbr_value = 0;
	
	
	//DHCP
	struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char data_str[100];
    char buffer[1024] = {0};
    
    //File
    FILE *write_ptr[9];
    FILE *config_ptr;
    char config_str[200];
    char file_name[50] = {0};
    size_t nbr_element;
    
    //Time
    time_t t = time(NULL);
    struct tm *tm;
    char time_str[100];
	
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
	init_param.dec_rate_int = 0x0080;	// data rate = 16kHz -> decimation = 2048/16 = 128
	init_param.dec_rate_dec = 0x0000;	// 4096 = 0x0080  0*2^16 = 0 = 0x0000
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
		
	//File opening
	DIR *mydir;
	struct dirent *myfile;
	char dir_name[50] = {0};
	
	mydir = opendir("/media/pi");
	while((myfile = readdir(mydir)) != NULL && strlen(myfile->d_name) <= 2) printf("File name: %s | size: %d\n", myfile->d_name, strlen(myfile->d_name));
	if(strlen(myfile->d_name) > 2) printf("USB detected: %s\n", myfile->d_name);
	else{
		printf("No USB connected\n");
		return -1;
	}
	
	t = time(NULL);
	tm = localtime(&t);
	strftime(time_str, 64, "%d%B_%Hh%Mmin%Ssec", tm);
	sprintf(dir_name, "/media/pi/%s/%s", myfile->d_name, time_str);
	printf(" directory = %s\n", dir_name);
	closedir(mydir);
	if(mkdir(dir_name, 0777) < 0){
		perror(dir_name);
		printf("Maybe name of USB key to long!\n");
		return -1;
	}
	
	for(i=0; i<8; i++){
		sprintf(file_name, "%s/channel%d", dir_name, i);
		write_ptr[i] = fopen(file_name, "wb");
		if(write_ptr[0] == 0) {
			sprintf(file_name, "%s opening", file_name);
			perror(file_name);
			return -1;
		}
		else printf("%s.bin successfully opened\n", file_name);
	}
	sprintf(file_name, "%s/error", dir_name);
	write_ptr[8] = fopen(file_name, "wb");
	if(write_ptr[8] == 0) {
		sprintf(file_name, "%s openning", file_name);
		perror(file_name);
		return -1;
	}
	else printf("%s.bin successfully opened\n", file_name);
	
	sprintf(file_name, "%s/config.txt", dir_name);
	config_ptr = fopen(file_name, "w");
	if(config_ptr == 0) {
		sprintf(file_name, "%s openning", file_name);
		perror(file_name);
		return -1;
	}
	else printf("%s successfully opened\n", file_name);
	
	sprintf(config_str, "Sampling frequency : 16kHz\n");
	fwrite(config_str, strlen(config_str), 1, config_ptr);
	sprintf(config_str, "Averaging on 32 samples\n");
	fwrite(config_str, strlen(config_str), 1, config_ptr);
	sprintf(config_str, "Storage frequency : 500Hz\n");
	fwrite(config_str, strlen(config_str), 1, config_ptr);
	sprintf(config_str, "Data format : 32bits / little endian / 1 file per channel per day\n");
	fwrite(config_str, strlen(config_str), 1, config_ptr);		
	
	bcm2835_gpio_clr(LED_RUN);
	bcm2835_gpio_set(LED_RDY);
	bcm2835_gpio_set_eds(TEST);
	decimation = 0;
	for(i=0; i<9; i++) data[i] = 0;
	nbr_value = 0;
	nbr_error = 0;
	flag_error = 0;
	t = time(NULL);
	tm = localtime(&t);
	strftime(time_str, 64, "%c", tm);
	sprintf(config_str, "Start time : %s\n", time_str);
	fwrite(config_str, strlen(config_str), 1, config_ptr);
	
	while(bcm2835_gpio_lev(SWITCH) == LOW){
		bcm2835_gpio_set_eds(DRDY_n);
		while(bcm2835_gpio_eds(DRDY_n) == LOW); // Waiting on signal DRDY to read data
		if(bcm2835_gpio_eds(TEST) == HIGH){		// If there is a problem
			bcm2835_gpio_clr(LED_ERR);
			flag_error = 1;
			bcm2835_gpio_set_eds(TEST);
		}
		else{									// Read data and sum them (32 times)
			bcm2835_spi_transfernb(buf_tx, buf_rx, 32);
			for(i = 0; i < 32; i += 4){
				data[i>>2] += ((buf_rx[i+1] << 24) | (buf_rx[i+2] << 16) | (buf_rx[i+3] << 8)) >> 8;
			}
		}
		
		decimation++;
		if(decimation >= 32){					
			if(flag_error > 0){				// Error
				nbr_error++;				// datas = 0, and error is set
				for(i=0; i<8; i++) data[i] = 0;
				data[8] = 10000000;
				flag_error = 0;
			}
			else{
				nbr_value++;				// No error
				for(i=0; i<8; i++) data[i] = (data[i] >> 5);	// Decimation of 32 on datas
			}
			
			// Save datas and errors in file
			for(i=0; i<9; i++){
				nbr_element = fwrite(&data[i], 4, 1, write_ptr[i]); 
				if(nbr_element != 1){ // 1 elements of 4 bytes
					printf("erreur nbr_element = %d\n", nbr_element);
				}
				data[i] = 0;
			}
			bcm2835_gpio_set(LED_ERR);
			decimation = 0;
		}
	}
	
	// Save number of errors and good samples in config.txt
	sprintf(config_str, "Number of good samples = %d\n", nbr_value);
	fwrite(config_str, strlen(config_str), 1, config_ptr);
	sprintf(config_str, "Number of errors = %d\n", nbr_error);
	fwrite(config_str, strlen(config_str), 1, config_ptr);
	t = time(NULL);
	tm = localtime(&t);
	strftime(time_str, 64, "%c", tm);
	sprintf(config_str, "Stop time : %s\n", time_str);
	fwrite(config_str, strlen(config_str), 1, config_ptr);
	
	printf("\n--------- CONTROL STATE REGISTERS-------------\n");	
	ad7770_ctrl_all_reg(device, true);
	printf("-------END CONTROL STATE REGISTERS------------\n");
	
	// Close files, SPI, library
	for(i=0; i<9; i++) fclose(write_ptr[i]);
	fclose(config_ptr);
	bcm2835_gpio_set(LED_RUN);
	bcm2835_gpio_set(LED_RDY);
	bcm2835_gpio_set(LED_ERR);
    bcm2835_gpio_clr_hen(DRDY_n);
    bcm2835_gpio_clr_hen(TEST);
    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}

