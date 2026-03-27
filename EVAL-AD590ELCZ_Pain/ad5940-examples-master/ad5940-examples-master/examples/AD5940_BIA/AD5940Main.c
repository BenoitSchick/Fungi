/*!
 *****************************************************************************
 @file:    AD5940Main.c
 @author:  Neo Xu
 @brief:   Used to control specific application and process data.
 -----------------------------------------------------------------------------

Copyright (c) 2017-2019 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 
*****************************************************************************/
/** 
 * @addtogroup AD5940_System_Examples
 * @{
 *  @defgroup BioElec_Example
 *  @{
  */
#include "ad5940.h"
#include "AD5940.h"
#include <stdio.h>
#include "string.h"
#include "math.h"
#include "BodyImpedance.h"

#define APPBUFF_SIZE 512

/********************************************************************************
 *	!!!!!!!!	PARAMETER TO MODIFY FOR CHANGING THE MEASUREMENT STYLE	!!!!!!!!	*
 ********************************************************************************/
 
#define TIME_BETWEEN_MEASURE (3) // Time between measurements in seconds
#define MEASURE_FREQ (1000.0) 	  // Frequence of the sin for the measurement
#define NB_SWEEP_POINTS 20

/********************************************************************************
 *	^^^^^^^^	PARAMETER TO MODIFY FOR CHANGING THE MEASUREMENT STYLE	^^^^^^^^	*
 ********************************************************************************/

uint32_t AppBuff[APPBUFF_SIZE];

/* GPIO edge detection: track previous pin state to detect rising edge only */
static uint32_t prev_pin4_state = 0;
 
/* Flag set on rising edge — consumed once per sweep */
static BoolFlag flag_GPIO = bFALSE;
 
/* Sweep is running: block any new trigger until it completes */
static BoolFlag sweep_running = bFALSE;


/* It's your choice here how to do with the data. Here is just an example to print them to UART */
int32_t BIAShowResult(uint32_t *pData, uint32_t DataCount)
{
  float freq;

  fImpPol_Type *pImp = (fImpPol_Type*)pData;
  AppBIACtrl(BIACTRL_GETFREQ, &freq);

  printf("%.2f,", freq);
  /*Process data*/
  for(int i=0;i<DataCount;i++)
  {
    printf("%.2f,%.2f\n",pImp[i].Magnitude,pImp[i].Phase*180/MATH_PI);
		//AD5940_Delay10us(1000);
  }
  return 0;
}

/* Initialize AD5940 basic blocks like clock */
static int32_t AD5940PlatformCfg(void)
{
  CLKCfg_Type clk_cfg;
  FIFOCfg_Type fifo_cfg;
  AGPIOCfg_Type gpio_cfg;
	SeqGpioTrig_Cfg seqgpiotrig_cfg;
	

  /* Use hardware reset */
  AD5940_HWReset();
  /* Platform configuration */
  AD5940_Initialize();
  /* Step1. Configure clock */
  clk_cfg.ADCClkDiv = ADCCLKDIV_1;
  clk_cfg.ADCCLkSrc = ADCCLKSRC_HFOSC;
  clk_cfg.SysClkDiv = SYSCLKDIV_1;
  clk_cfg.SysClkSrc = SYSCLKSRC_HFOSC;
  clk_cfg.HfOSC32MHzMode = bFALSE;
  clk_cfg.HFOSCEn = bTRUE;
  clk_cfg.HFXTALEn = bFALSE;
  clk_cfg.LFOSCEn = bTRUE;
  AD5940_CLKCfg(&clk_cfg);
  /* Step2. Configure FIFO and Sequencer*/
  fifo_cfg.FIFOEn = bFALSE;
  fifo_cfg.FIFOMode = FIFOMODE_FIFO;
  fifo_cfg.FIFOSize = FIFOSIZE_4KB;                       /* 4kB for FIFO, The reset 2kB for sequencer */
  fifo_cfg.FIFOSrc = FIFOSRC_DFT;
  fifo_cfg.FIFOThresh = 4;//AppBIACfg.FifoThresh;        /* DFT result. One pair for RCAL, another for Rz. One DFT result have real part and imaginary part */
  AD5940_FIFOCfg(&fifo_cfg);                             /* Disable to reset FIFO. */
  fifo_cfg.FIFOEn = bTRUE;  
  AD5940_FIFOCfg(&fifo_cfg);                             /* Enable FIFO here */
  
  /* Step3. Interrupt controller */
  
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);           /* Enable all interrupt in Interrupt Controller 1, so we can check INTC flags */
  AD5940_INTCCfg(AFEINTC_0, AFEINTSRC_DATAFIFOTHRESH|AFEINTSRC_CUSTOMINT2, bTRUE);   /* Interrupt Controller 0 will control GP0 to generate interrupt to MCU */
  AD5940_INTCClrFlag(AFEINTSRC_ALLINT);
  /* Step4: Reconfigure GPIO */
//  gpio_cfg.FuncSet = GP6_SYNC|GP5_SYNC|GP4_GPIO|GP2_TRIG|GP1_SYNC|GP0_INT;
//  gpio_cfg.InputEnSet = AGPIO_Pin2|AGPIO_Pin4;
//  gpio_cfg.OutputEnSet = AGPIO_Pin0|AGPIO_Pin1|AGPIO_Pin5|AGPIO_Pin6;
//  gpio_cfg.OutVal = 0;
//  gpio_cfg.PullEnSet = 0;

	gpio_cfg.FuncSet = GP4_GPIO | GP0_INT;
gpio_cfg.InputEnSet = AGPIO_Pin4;
gpio_cfg.OutputEnSet = AGPIO_Pin0;
gpio_cfg.PullEnSet = AGPIO_Pin4; // pull-down pour stabilité

	AD5940_AGPIOCfg(&gpio_cfg);
	
//	/* Allow GP2 falling edge to trigger sequence2 */
//	seqgpiotrig_cfg.bEnable = bTRUE;
//  seqgpiotrig_cfg.PinSel = AGPIO_Pin2;
//  seqgpiotrig_cfg.SeqPinTrigMode = SEQPINTRIGMODE_RISING;
//  AD5940_SEQGpioTrigCfg(&seqgpiotrig_cfg);
	
	

  
  AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK);  /* Allow AFE to enter sleep mode. */
  return 0;
}

/* !!Change the application parameters here if you want to change it to none-default value */
void AD5940BIAStructInit(void)
{
  AppBIACfg_Type *pBIACfg;
  
  AppBIAGetCfg(&pBIACfg);
  
  pBIACfg->SeqStartAddr = 0;
  pBIACfg->MaxSeqLen = 512; /** @todo add checker in function */
  
	//pBIACfg->SinFreq = MEASURE_FREQ;  /* Set measurement freq to 1.5kHz */
	
	/* Sweep Function Control */
	pBIACfg->SweepCfg.SweepEn = bTRUE;
	pBIACfg->SweepCfg.SweepStart = 1000;
	pBIACfg->SweepCfg.SweepStop = 100000;
	pBIACfg->SweepCfg.SweepLog = bFALSE;
	pBIACfg->SweepCfg.SweepPoints = NB_SWEEP_POINTS;
	pBIACfg->SweepCfg.SweepIndex = 0;
	
	
  pBIACfg->RcalVal = 10000.0;
  pBIACfg->DftNum = DFTNUM_8192;
  pBIACfg->NumOfData = -1;      /* Never stop until you stop it manually by AppBIACtrl() function */
  pBIACfg->BiaODR = 5;         /* ODR(Sample Rate) 20Hz */
  pBIACfg->FifoThresh = 4;      /* 4 */
  pBIACfg->ADCSinc3Osr = ADCSINC3OSR_2;
}


/* ---------------------------------------------------------------------------
 * GPIO polling — detects a RISING EDGE on GP4 only.
 * --------------------------------------------------------------------------*/
static void PollGPIOTrigger(void)
{
  uint32_t gpio_val  = AD5940_ReadReg(REG_AGPIO_GP0IN);
  uint32_t cur_state = (gpio_val & AGPIO_Pin4) ? 1u : 0u;
	
  if (cur_state == 1u && prev_pin4_state == 0u)
  {
    if (sweep_running == bFALSE)   /* Only arm if no sweep is in progress */
    {
      flag_GPIO = bTRUE;
    }
  }
  prev_pin4_state = cur_state;
}

static uint32_t sweep_point_count = 0;

void AD5940_Main(void)
{
  uint32_t IntCount;
  uint32_t count;
  uint32_t temp;
	AppBIACfg_Type *pBIACfg;
	AppBIAGetCfg(&pBIACfg);
  
  AD5940PlatformCfg();
  AD5940BIAStructInit(); /* Configure your parameters in this function */
  
  AppBIAInit(AppBuff, APPBUFF_SIZE);    /* Initialize BIA application. Provide a buffer, which is used to store sequencer commands */
  AppBIACtrl(BIACTRL_START, 0);  

	
	
	sweep_running = bTRUE;
	sweep_point_count = 0;
	
	while(1)
  {
		PollGPIOTrigger();
		
    /* Check if interrupt flag which will be set when interrupt occurred. */
    if(AD5940_GetMCUIntFlag())
    {
      AD5940_ClrMCUIntFlag(); /* Clear this flag */
			
			temp = APPBUFF_SIZE;
      AppBIAISR(AppBuff, &temp); /* Deal with it and provide a buffer to store data we got */
      BIAShowResult(AppBuff, 1); /* Show the results to UART */
			sweep_point_count++;
			
			if (sweep_point_count >= NB_SWEEP_POINTS)//(pBIACfg->SweepCfg.SweepIndex == 0)
      {
        AppBIACtrl(BIACTRL_SHUTDOWN, 0);
        sweep_running = bFALSE;
        flag_GPIO = bFALSE; 
				sweep_point_count = 0;
      }
    } 
		
    if (flag_GPIO == bTRUE && sweep_running == bFALSE)
    {
      flag_GPIO     = bFALSE;
      sweep_running = bTRUE;
			//AppBIAInit(AppBuff, APPBUFF_SIZE);
      AppBIAInit(0, 0);
      AppBIACtrl(BIACTRL_START, 0);
			//AD5940_Delay10us(100000);
    }
  }
}


/**
 * @}
 * @}
 * */
