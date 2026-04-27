// main.c
//
// Author: Maxime Charrière
// 
// This programm is used to save the data from the ADC to local file
// and also to send messages to the Python GUI through message queue

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/times.h>
#include <dirent.h> 
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <bcm2835.h>
#include "../include/ad7124.h"
#include "../include/spi.h"
#include "../include/linux_spi.h"
#include "../include/ad7124_regs.h"
#include "../include/myError.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

// DEFINE
#define SYNC_n	RPI_V2_GPIO_P1_31
#define NBR_CHANNELS	2
#define BUFFSIZE_MQUEUE 100

// SPS Filter Word (dec) value for Sinc^4 or sinc^3 filter
#define SPEED_19200_SPS  0x002
#define SPEED_3200_SPS   0x006
#define SPEED_1600_SPS   0x003
#define SPEED_1000_SPS   0x013
#define SPEED_500_SPS    0x026
#define SPEED_50_SPS     0x180

// Filter mode define
#define sinc4_Filter                0x0		// Sinc^4 Filter
#define sinc3_Filter                0x2		// Sinc^3 Filter
#define sinc4_fast_settling_Filter  0x4		// Sinc^4+sinc^1 Filter
#define sinc3_fast_settling_Filter  0x5		// Sinc^3+sinc^1 Filter

// AD7124-4 power mode define
#define	LOW_POWER_MODE	0x0
#define	MID_POWER_MODE	0x1
#define	FULL_POWER_MODE	0x3

// SPI parameter definition
#define	SPI_AD7124_SPEED	3600000	// 3.6 MHz
#define	SPI_AD7124_CHIP_SELECT	0	// Chip Select 0
#define SPI_DEVICE_ID		3	// SPI device ID 3 -> SPI6


/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

// Task for the data acquisition prototype
void *dataAcquisitionTask (void *ptr);

/******************************************************************************/
/****************** Global variables and Types Declarations *******************/
/******************************************************************************/

// Arguments basic values
uint16_t samplingFrequency = 500;
uint8_t channel = 0;
uint16_t timeMeasure = 30;

// Time variables
time_t t;
struct tm *tm;
char time_str[30] = {0};

// File variables
FILE *write_ptr[NBR_CHANNELS];
FILE *config_ptr;	
char config_str[200] = {0};
char data_str[200] = {0};
char file_name[150] = {0};
char dir_name[100] = {0};

// ADC variables
uint32_t nbr_value = 0;
uint32_t nbr_error = 0;

// Struct for message queue
struct vmsgbuf {
      long mwidth;
      char message[BUFFSIZE_MQUEUE];
      };

// Struct for arg of aquisition thread
struct thread_arg {
      struct ad7124_dev *ad7124_handler;
      uint16_t timeout;
      };

/***************************************************************************//**
 * @brief Read the conversion from the ADC. If an error occur, the thread is
 *        exited and an error code is returned.
 *
 * @param ptr   - Pointer to the struct that contains the handler of the instance
 *                of the driver and the timeout time for the waiting of conv ready
*******************************************************************************/
void *dataAcquisitionTask (void *ptr)
{
  // variable declaration
  struct thread_arg *pThread_arg;
  int32_t data;
  int32_t data_read;
  int32_t status_reg;
  int32_t error_reg;
  time_t startTime;
  time_t currentTime;
  
  //return of functions variable for checks
  int32_t ret = -1;
  
  pThread_arg = (struct thread_arg*)ptr;
  
  if(!pThread_arg)
  {
    printf("Invalid pointer\n");
    return (void*)INVALID_POINTER;
  }
  
  // Get the start time
  time(&startTime);

  
  // Infinite loop (Until the thread is cancelled)
  while(1)
  {
    //Waiting on conversion ready
    ret = ad7124_wait_for_conv_ready(pThread_arg->ad7124_handler, pThread_arg->timeout);
    
    if(ret == -3)
    {
      printf("Timeout occured on wait for convertion ready! Retry\n");
      nbr_error++;
      fflush(stdout);
    }
    else if (ret < 0)
    {
      printf("Error while waiting for convertion ready! Exiting thread....\n");
      fflush(stdout);
      return (void*)CONV_RDY_ERROR;
    }
    else
    {
      // Read data and fill the array	
      ret = ad7124_read_data(pThread_arg->ad7124_handler, &data_read);
      if (ret < 0)
      {
        printf("Error while reading data! Exiting thread....\n");
        fflush(stdout);
	return (void*)DATA_RD_ERROR;
      }
      
      data = (data_read & 0x00FFFFFF);  // Mask for the data bits
      status_reg = (data_read & 0xFF000000)>>24;  // Mask for the status bits

      // Check if error occured, if not increment nbr_value counter
      uint8_t status_ok_value = 0x00;
      
      
      if(status_reg != status_ok_value)
      {
	printf("status reg = 0x%x", status_reg);
        // Check if the ADC was ready to send conversion
        if (status_reg & AD7124_STATUS_REG_RDY)
        {
          printf("ADC conversion was not ready even after a conv_ready check! Exiting thread....\n");
          fflush(stdout);
          return (void*)NRDY_CONV_RDY_ERROR;
        }
        // Check if an error occured
        else if (status_reg & AD7124_STATUS_REG_ERROR_FLAG)
        {
          printf("An ADC error is occured! Error:\n");
          fflush(stdout);
          ret = ad7124_read_data(pThread_arg->ad7124_handler, &error_reg);
          if (ret < 0)
          {
            printf("Error while reading error register! Exiting thread....\n");
            fflush(stdout);
            return (void*)DATA_RD_ERROR;
          }
          if(error_reg & AD7124_ERR_REG_ADC_CONV_ERR) printf("Error occured during a convertion! Exiting thread....\n");
          if(error_reg & AD7124_ERR_REG_ADC_SAT_ERR) printf("Modulator got saturated during conversion! Exiting thread....\n");
          if(error_reg & AD7124_ERR_REG_AINP_OV_ERR) printf("Overvoltage detected on AINx +! Exiting thread....\n");
          if(error_reg & AD7124_ERR_REG_AINP_UV_ERR) printf("Undervoltage detected on AINx +! Exiting thread....\n");
          if(error_reg & AD7124_ERR_REG_AINM_OV_ERR) printf("Overvoltage detected on AINx -! Exiting thread....\n");
          if(error_reg & AD7124_ERR_REG_AINM_UV_ERR) printf("Undervoltage detected on AINx -! Exiting thread....\n");
          return (void*)ADC_ERROR_REG_FLAG;
        }
        // Check if the Power-on reset flag is active
        else if (status_reg & AD7124_STATUS_REG_POR_FLAG)
        {
          printf("ADC Power-on reset flag is active! Exiting thread....\n");
          fflush(stdout);
          return (void*)PWRON_FLAG_ERROR;
        }
        // Check which channel is active (if none of the above check was true the only alternative is that a wrong channel is active)
        else
        {
          uint8_t active_channel = AD7124_STATUS_REG_CH_ACTIVE(status_reg);
          printf("Possible wrong channel setting (CH %d) is active, something wrong happened! Exiting thread....\n", active_channel);
          fflush(stdout);
          return (void*)WRONG_CHANNEL;
        }
      }
        else
      {
        nbr_value++;
      }

      // Save datas in file
      if (channel == 0)
      {
        fwrite(&data, 4, 1, write_ptr[0]);
        data = 0;	// Set data value to 0, as only 1 channel is active
        fwrite(&data, 4, 1, write_ptr[1]);
      }
      else if (channel == 1)
      {
        fwrite(&data, 4, 1, write_ptr[1]);
        data = 0;	// Set data value to 0, as only 1 channel is active
        fwrite(&data, 4, 1, write_ptr[0]);
      }
    }
    
    time(&currentTime);
    if((currentTime-startTime) >= timeMeasure){
      return (void*)0;
    }    
  }
}

int main(int argc, char **argv)
{   
  printf("Starting AD7124-4 conversion routine\n");
  fflush(stdout);
  	
  // If arguments used
  if (argc >= 2)
	  samplingFrequency = atoi(argv[1]);
  if (argc >= 3)
	  channel = atoi(argv[2]);
  if (argc >= 4)
	  timeMeasure = atoi(argv[3]);
  
  struct ad7124_dev my_ad7124;                    // A new driver instance
  struct ad7124_dev *ad7124_handler = &my_ad7124; // A driver handle to pass around
  uint16_t timeout = 1000;                        // Number of tries before a function times out
  int16_t ret = 0;                                // Return value
  struct spi_init_param *p_my_spi_init = malloc(sizeof(struct spi_init_param ));          // SPI init setting
  struct linux_spi_init_param *my_linux_spi_init = malloc(sizeof(struct linux_spi_init_param )); // Linux SPi init
  struct ad7124_init_param *p_my_ad7124_init = malloc(sizeof(struct ad7124_init_param )); // ADC init setting
  uint16_t timeout_for_acquisition = 2000;	// Timeout value for ad7124_wait_for_conv_ready() in aquisition thread
  
  my_linux_spi_init->device_id = SPI_DEVICE_ID;	// set the device ID

  // SPI Init parameters
  p_my_spi_init->max_speed_hz = SPI_AD7124_SPEED;
  p_my_spi_init->chip_select = SPI_AD7124_CHIP_SELECT;      
  p_my_spi_init->mode = SPI_MODE_3; // ADC7124 use SPI mode 3
  p_my_spi_init->platform_ops = &linux_spi_platform_ops; // Link to the Linux platform specific SPI platform ops structure
  p_my_spi_init->extra = (void*)my_linux_spi_init;  // Linux SPI init value
  
  printf("p_my_spi_init initialized\n");
  fflush(stdout);
  
  // ADC Init parameters
  p_my_ad7124_init->spi_init = p_my_spi_init; // Set SPI init parameter
  p_my_ad7124_init->regs = ad7124_regs; // Set init register value
  p_my_ad7124_init->spi_rdy_poll_cnt = timeout; // Set timeout
  
  printf("p_my_ad7124_init initialized\n");
  fflush(stdout);
  
  // AD7124-4 initalisation
  // Creation of the Filter register config value
  uint32_t filter_setting = 0x00000000;
  filter_setting = AD7124_FILT_REG_FILTER(sinc3_Filter);
  
  if (samplingFrequency == 19200)
  {
    filter_setting |= AD7124_FILT_REG_FS(SPEED_19200_SPS);
  }
  else if (samplingFrequency == 3200)
  {
    filter_setting |= AD7124_FILT_REG_FS(SPEED_3200_SPS);
  }
  else if (samplingFrequency == 1600) 
  {
    filter_setting |= AD7124_FILT_REG_FS(SPEED_1600_SPS);
  }
  else if (samplingFrequency == 1000)
  {
    filter_setting |= AD7124_FILT_REG_FS(SPEED_1000_SPS);
  }
  else if (samplingFrequency == 500)
  {
    filter_setting |= AD7124_FILT_REG_FS(SPEED_500_SPS);
  }
  else if (samplingFrequency == 50)
  {
    filter_setting |= AD7124_FILT_REG_FS(SPEED_50_SPS);
  }
  else
  {
    filter_setting |= AD7124_FILT_REG_FS(SPEED_500_SPS); // default SPS -> 500
  }
  
  printf("filter_setting initialized with value: 0x%x\n", filter_setting);
  fflush(stdout);
  
  // Creation of the control register config value
  uint16_t control_setting = 0x0000;
  
  control_setting |= AD7124_ADC_CTRL_REG_POWER_MODE(FULL_POWER_MODE);
  control_setting |= AD7124_ADC_CTRL_REG_DATA_STATUS;  
  
  printf("control_setting initialized with value: 0x%x\n", control_setting);
  fflush(stdout);
  
  // Creation of the channel register config value
  uint16_t channel_setting = 0x0000;
  channel_setting |= AD7124_CH_MAP_REG_CH_ENABLE | AD7124_CH_MAP_REG_SETUP(0x0);
  
  printf("channel %d selected", channel);
  
  if (channel == 0) channel_setting |= AD7124_CH_MAP_REG_AINP(0x00) | AD7124_CH_MAP_REG_AINM(0x01);   // Select AIN0+ and AIN1-
  else if (channel == 1) channel_setting |= AD7124_CH_MAP_REG_AINP(0x02) | AD7124_CH_MAP_REG_AINM(0x03);  // Select AIN2+ and AIN3-
  else channel_setting |= AD7124_CH_MAP_REG_AINP(0x00) | AD7124_CH_MAP_REG_AINM(0x01); // default AIN0+ and AIN1 -
  
  printf("channel_setting initialized with value: 0x%x\n", channel_setting);
  fflush(stdout);
  
  // Creation of the Configuration register config value
  uint32_t config_setting = 0x00000000;
  
  config_setting |= AD7124_CFG_REG_BIPOLAR | AD7124_CFG_REG_REF_SEL(0) |
		    AD7124_CFG_REG_PGA(0);
  
  // Creation of the Error_EN register config value
  // Usefull error check are enabled
  uint32_t error_en_setting = 0x00000000;
  error_en_setting |= AD7124_ERREN_REG_SPI_IGNORE_ERR_EN | AD7124_ERREN_REG_AINM_UV_ERR_EN | 
                      AD7124_ERREN_REG_AINM_OV_ERR_EN | AD7124_ERREN_REG_AINP_UV_ERR_EN | 
                      AD7124_ERREN_REG_AINP_OV_ERR_EN | AD7124_ERREN_REG_ADC_SAT_ERR_EN;
		      
  printf("error_en_setting initialized with value: 0x%x\n", error_en_setting);
  fflush(stdout);
                      

  // Initialize AD7124 device with default values (init SPI and setup AD7124)
  ret = ad7124_setup(&ad7124_handler, p_my_ad7124_init);
  if (ret < 0)
  {
    printf("AD7124 Setup failed! ErrNo: %d\n", ret);
    fflush(stdout);
    return SETUP_FAILED;
  }
  else
  {
    printf("AD7124 Setup successfull\n");
    fflush(stdout);
  }

  ret = ad7124_wait_to_power_on(ad7124_handler, timeout);
  if (ret < 0)
  {
    printf("AD7124 power on failed! ErrNo: %d\n", ret);
    fflush(stdout);
    return PWRON_FAILED;
  }
  else
  {
    printf("AD7124 power on successfull\n");
    fflush(stdout);
  }
  
  // Variable to check if write was successfull
  struct ad7124_st_reg check_sended_value[AD7124_REG_NO];
  for (enum ad7124_registers regNr = AD7124_Status; (regNr < AD7124_REG_NO) && !(ret < 0); regNr++)
  {
    check_sended_value[regNr] = ad7124_regs[regNr];
  }
  
  // Write the new register value
  // Filter register 0-7
  for(enum ad7124_registers i=AD7124_Filter_0; i<=AD7124_Filter_7; i++)
  {    
    ad7124_handler->regs[i].value = filter_setting;
    ret = ad7124_write_register(ad7124_handler, ad7124_handler->regs[i]);
    if (ret < 0)
      {
        printf("Filter register %d write failed! ErrNo: %d\n", i, ret);
        fflush(stdout);
        return REGISTER_WR_FAILED;
      }
    else
      {
        ad7124_read_register(ad7124_handler, &check_sended_value[i]);
        printf("Filter register 0x%x write successfull\n", i);
        printf("Filter register value send: 0x%x\n", ad7124_handler->regs[i].value);
        printf("Register 0x%x value: 0x%x\n", i, check_sended_value[i].value);
        fflush(stdout);
      }
  }
  
  // Channel register 0 and 1, the other ch register remain default value
  uint16_t default_val = 0x0001;
  if (channel == 0)
  {
    ad7124_handler->regs[AD7124_Channel_0].value = channel_setting;
    ad7124_handler->regs[AD7124_Channel_1].value = default_val;
  }
  else if (channel == 1)
  {
    //ad7124_handler->regs[AD7124_Channel_0].value = default_val;
    //ad7124_handler->regs[AD7124_Channel_1].value = channel_setting;
    ad7124_handler->regs[AD7124_Channel_0].value = channel_setting;
    ad7124_handler->regs[AD7124_Channel_1].value = default_val;
  }
  
  for(enum ad7124_registers i=AD7124_Channel_0; i<=AD7124_Channel_1; i++)
  {
    ret = ad7124_write_register(ad7124_handler, ad7124_handler->regs[i]);
    if (ret < 0)
      {
        printf("Channel register 0x%x write failed! ErrNo: %d\n", i, ret);
        fflush(stdout);
        return REGISTER_WR_FAILED;
      }
    else
      {
        ad7124_read_register(ad7124_handler, &check_sended_value[i]);
        printf("Channel register %d write successfull\n", i);
        printf("Channel register value send: 0x%x\n", ad7124_handler->regs[i].value);
        printf("Register 0x%x value: 0x%x\n", i, check_sended_value[i].value);
        fflush(stdout);
      }
  }
  
  // ADC control register
  ad7124_handler->regs[AD7124_ADC_Control].value = control_setting;
  ret = ad7124_write_register(ad7124_handler, ad7124_handler->regs[AD7124_ADC_Control]);
  if (ret < 0)
    {
      printf("Control register write failed! ErrNo: %d\n", ret);
      fflush(stdout);
      return REGISTER_WR_FAILED;
    }
  else
    {
      ad7124_read_register(ad7124_handler, &check_sended_value[AD7124_ADC_Control]);
      printf("Control register write successfull\n");
      printf("Control register value send: 0x%x\n", ad7124_handler->regs[AD7124_ADC_Control].value);
      printf("Register 0x%x value: 0x%x\n", AD7124_ADC_Control, check_sended_value[AD7124_ADC_Control].value);
      fflush(stdout);
    }
    
  // ADC Configuration register
  ad7124_handler->regs[AD7124_Config_0].value = config_setting;
  ret = ad7124_write_register(ad7124_handler, ad7124_handler->regs[AD7124_Config_0]);
  if (ret < 0)
    {
      printf("Configuration register write failed! ErrNo: %d\n", ret);
      fflush(stdout);
      return REGISTER_WR_FAILED;
    }
  else
    {
      ad7124_read_register(ad7124_handler, &check_sended_value[AD7124_Error_En]);
      printf("Configuration register write successfull\n");
      printf("Configuration register value send: 0x%x\n", ad7124_handler->regs[AD7124_Error_En].value);
      printf("Register 0x%x value: 0x%x\n", AD7124_Error_En, check_sended_value[AD7124_Error_En].value);
      fflush(stdout);
    }
    
  // ADC error EN register
  ad7124_handler->regs[AD7124_Error_En].value = error_en_setting;
  ret = ad7124_write_register(ad7124_handler, ad7124_handler->regs[AD7124_Error_En]);
  if (ret < 0)
    {
      printf("Error EN register write failed! ErrNo: %d\n", ret);
      fflush(stdout);
      return REGISTER_WR_FAILED;
    }
  else
    {
      ad7124_read_register(ad7124_handler, &check_sended_value[AD7124_Error_En]);
      printf("Error EN register write successfull\n");
      printf("Error EN register value send: 0x%x\n", ad7124_handler->regs[AD7124_Error_En].value);
      printf("Register 0x%x value: 0x%x\n", AD7124_Error_En, check_sended_value[AD7124_Error_En].value);
      fflush(stdout);
    }
  
  // Get a message queue identifier (send)
  int msqidSend;
  struct vmsgbuf msgQueueSendData;
  msgQueueSendData.mwidth = 1;
  memset(msgQueueSendData.message,0,BUFFSIZE_MQUEUE);
  if ( -1 == (msqidSend = msgget((key_t)1234, IPC_CREAT | 0666)))
  {
	  perror("msgget() failed (send)");
	  return -1;
  }

  // Get a message queue identifier (receive)
  int msqidReceive;
  struct vmsgbuf msgQueueReceiveData;
  msgQueueReceiveData.mwidth = 1;
  memset(msgQueueReceiveData.message,0,BUFFSIZE_MQUEUE);
  if ( -1 == (msqidReceive = msgget((key_t)2345, IPC_CREAT | 0666)))
  {
	  perror("msgget() failed (receive)");
	  return -1;
  }
  
  // Define the time at te begining of the program
  t = time(NULL);
  tm = localtime(&t);
  strftime(time_str, 64, "%d%B%Hh%Mmin%Ssec", tm);

  // Create a new directory for the measure
  sprintf(dir_name, "/home/pi/Desktop/data/notOnCloud/%s", time_str);
  printf(" directory = %s\n", dir_name);
  fflush(stdout);
  umask(0);
  if(mkdir(dir_name, 0777) < 0)
  {
	  perror("creating directoy failed");
	  return DIR_CREATION_FAILED;
  }

  // Create a file per channel
  for(int i=0; i<NBR_CHANNELS; i++)
  {
	  sprintf(file_name, "%s/channel%d", dir_name, i);
	  write_ptr[i] = fopen(file_name, "wb");
	  if(write_ptr[0] == 0)
    {
		  perror("opening channel file failed");
		  return FILE_OPEN_FAILED;
	  }
	  else
    {
		  printf("%s.bin successfully opened\n", file_name);
		  fflush(stdout);
	  }
  }

  // Create a file for the configuration
  sprintf(file_name, "%s/config.txt", dir_name);
  config_ptr = fopen(file_name, "w");
  if(config_ptr == 0)
  {
	  perror("opening config file failed");
	  return FILE_OPEN_FAILED;
  }
  else
  {
	  printf("%s successfully opened\n", file_name);
	  fflush(stdout);
  }

  // Write the configuration to the file	
  sprintf(config_str, "Sampling frequency : %iHz\n", samplingFrequency);
  fwrite(config_str, strlen(config_str), 1, config_ptr);
  sprintf(config_str, "Data format : 32bits / little endian / 1 file per channel\n");
  fwrite(config_str, strlen(config_str), 1, config_ptr);
  
  // Write start time to the configuration file
  t = time(NULL);
  tm = localtime(&t);
  strftime(time_str, 64, "%c", tm);
  sprintf(config_str, "Start time : %s\n", time_str);
  fwrite(config_str, strlen(config_str), 1, config_ptr);	

  // Inform of the acquisition beginning
  printf("Beginning of the data acquisition\n");
  fflush(stdout);

  // Reset variables
  nbr_value = 0;

  // pThread id variable
  pthread_t tid_acquisition;
  
  //pThread arg variable
  struct thread_arg *pThread_arg = malloc(sizeof(struct thread_arg));
  
  pThread_arg->ad7124_handler = ad7124_handler;
  pThread_arg->timeout = timeout_for_acquisition;
  
  // Create the thread for the data acquisition
  if(pthread_create(&tid_acquisition, NULL, dataAcquisitionTask, (void*)pThread_arg))
  {
	  perror("Error when creating the acquisition thread\n");
	  return THREAD_CREATION_ERROR;
  }

  //// pthread thread return value
  void *status = NULL;
  pthread_join(tid_acquisition, &status);
  
  // Handle the eventual error from the conversionThread
  if((int)status != 0){
    printf("Thread returned val: %d\n", (int)status);
    sprintf(msgQueueSendData.message, "Thread Error\n");
    
    // Send a message through the message queue
    if ( -1 == msgsnd(msqidSend, (struct msgbuf *)&msgQueueSendData, BUFFSIZE_MQUEUE, 0)){
	    perror("msgsnd() failed");
	    return MSG_SEND_FAILED;
    }
    
    sprintf(msgQueueSendData.message, "Error code: %d\n", (int)status);
    
    // Send a message through the message queue
    if ( -1 == msgsnd(msqidSend, (struct msgbuf *)&msgQueueSendData, BUFFSIZE_MQUEUE, 0)){
	    perror("msgsnd() failed");
	    return MSG_SEND_FAILED;
    }
      
    return (int)status;
  }
  
  // Save number of samples in config.txt
  sprintf(config_str, "Number of samples = %d\n", nbr_value);
  fwrite(config_str, strlen(config_str), 1, config_ptr);
  
  // Save number of errors in config.txt
  sprintf(config_str, "Number of error = %d\n", nbr_error);
  fwrite(config_str, strlen(config_str), 1, config_ptr);

  // Inform of the acquisition end
  printf("End of the data acquisition\n");
  fflush(stdout);

  // Write stop time
  t = time(NULL);
  tm = localtime(&t);
  strftime(time_str, 64, "%c", tm);
  sprintf(config_str, "Stop time : %s\n", time_str);
  fwrite(config_str, strlen(config_str), 1, config_ptr);

  // Get location information
  if ( -1 == msgrcv(msqidReceive, (struct msgbuf *)&msgQueueReceiveData, BUFFSIZE_MQUEUE, msgQueueSendData.mwidth,0 ))
  {
	  perror ("msgrcv() failed");
	  return MSG_RECEIVE_FAILED;
  }
  sprintf(config_str, "Location information : %s\n", msgQueueReceiveData.message);
  printf("Received from the message queue : %s\n", msgQueueReceiveData.message);
  fflush(stdout);
  fwrite(config_str, strlen(config_str), 1, config_ptr);

  // -------------------------------------------------
  // RUN ML ALGORITHM ON CHANNEL X
  // result = ML(write_ptr[channel]);
  // -------------------------------------------------

  // Close files
  for(int i=0; i<NBR_CHANNELS; i++) fclose(write_ptr[i]);

  // String that contain the result
  sprintf(msgQueueSendData.message, "it works!\n");

  // Write the result in the config file
  sprintf(config_str, "Diagnosis result on channel%i : %s\n", channel, msgQueueSendData.message);
  fwrite(config_str, strlen(config_str), 1, config_ptr);

  // Send a message through the message queue
  if ( -1 == msgsnd(msqidSend, (struct msgbuf *)&msgQueueSendData, BUFFSIZE_MQUEUE, 0))
  {
	  perror("msgsnd() failed");
	  return MSG_SEND_FAILED;
  }

  // Inform of the acquisition end
  printf("Send result through the message queue\n");
  fflush(stdout);

  // Remove messageQueues
  if ( -1 == msgctl(msqidReceive, IPC_RMID,0))
  {
	  perror("msgctl() failed (Receiver)");
	  return MSG_QUEUE_REMOVE_FAILED;
  }
  if ( -1 == msgctl(msqidSend, IPC_RMID,0))
  {
	  perror("msgctl() failed (Sender)");
	  return MSG_QUEUE_REMOVE_FAILED;
  }

  // Close files
  fclose(config_ptr);

  // Close SPI and GPIO
  bcm2835_close();
  ad7124_remove(ad7124_handler);
  
  // Free allocated memory
  free(p_my_spi_init);
  free(my_linux_spi_init);
  free(p_my_ad7124_init);
  free(pThread_arg);
  
  // End of the program
  printf("End of AD7124-4 conversion routine\n");
  fflush(stdout);
  return 0;
}
