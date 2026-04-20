/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main function
 *   @author Benoit Schick (benoit.schick@hefr.ch)
********************************************************************************/

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "no-OS/drivers/platform/pico/pico_spi.h"
#include "no-OS/include/no_os_spi.h"
#include "no-OS/drivers/adc/ad7779/ad7779.h"


// --- Pinout Raspberry Pi -----------------------------
#define GPIO_RESET 0
#define GPIO_MODE0 1
#define GPIO_MODE1 2
#define GPIO_MODE2 3
#define GPIO_MODE3 4
#define GPIO_DCLK0 5
#define GPIO_DCLK1 6
#define GPIO_DCLK2 7
#define GPIO_SYNC_IN 8
#define GPIO_CONVST_SAR 9

// --- Registers -----------------------------
/* static const uint8_t REG_DEVID = 0x00; */
/* static const uint8_t REG_POWER_CTL = 0x2D; */
/* static const uint8_t REG_DATAX0 = 0x32; */



/* struct no_os_spi_desc *pico_spi_desc; */

// --- Pico SPI Settings -----------------------------
struct pico_spi_init_param pico_spi_pin = {
    .spi_tx_pin  = SPI0_TX_GP19,   // MOSI
    .spi_rx_pin  = SPI0_RX_GP16,   // MISO
    .spi_sck_pin = SPI0_SCK_GP18,  // CLK
    .spi_cs_pin  = SPI0_CS_GP17,   // CS
};

// --- Pico GPIO Settings -----------------------------
struct no_os_gpio_init_param gpio_reset_ip =
        { .number = GPIO_RESET, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_mode0_ip =
        { .number = GPIO_MODE0, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_mode1_ip =
        { .number = GPIO_MODE1, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_mode2_ip =
        { .number = GPIO_MODE2, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_mode3_ip =
        { .number = GPIO_MODE3, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_dclk0_ip =
        { .number = GPIO_DCLK0, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_dclk1_ip =
        { .number = GPIO_DCLK1, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_dclk2_ip =
        { .number = GPIO_DCLK2, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_sync_in_ip =
        { .number = GPIO_SYNC_IN, .platform_ops = &pico_gpio_ops, .extra = NULL };
struct no_os_gpio_init_param gpio_convst_sar_ip =
        { .number = GPIO_CONVST_SAR, .platform_ops = &pico_gpio_ops, .extra = NULL };

// --- General SPI settings -----------------------------
struct no_os_spi_init_param spi_init_param = {
    .device_id     		= 0,              // spi0
    .max_speed_hz  		= 1000000,        // 1 MHz
    /* .chip_select 		= , */
    .mode         		= NO_OS_SPI_MODE_0, // AD7777 : CPOL=1, CPHA=1
    .bit_order 			= NO_OS_SPI_BIT_ORDER_MSB_FIRST,
    .lanes 			= NO_OS_SPI_SINGLE_LANE,
    .extra         		= &pico_spi_pin,
    .platform_ops  		= &pico_spi_ops,
};

// --- ADC AD7779 settings -----------------------------
ad7779_dev* adc_dev; 

ad7779_init_param adc_init_param = {
	.spi_init = spi_init_param;
	.gpio_reset      = gpio_reset_ip,
        .gpio_mode0      = gpio_mode0_ip,
        .gpio_mode1      = gpio_mode1_ip,
        .gpio_mode2      = gpio_mode2_ip,
        .gpio_mode3      = gpio_mode3_ip,
        .gpio_dclk0      = gpio_dclk0_ip,
        .gpio_dclk1      = gpio_dclk1_ip,
        .gpio_dclk2      = gpio_dclk2_ip,
        .gpio_sync_in    = gpio_sync_in_ip,
        .gpio_convst_sar = gpio_convst_sar_ip,
	.ctrl_mode       = AD7779_SPI_CTRL, //Pin or SPI control mode
        .spi_crc_en      = AD7779_DISABLE,
        .read_from_cache = true,
        .state = {
            AD7779_ENABLE, AD7779_ENABLE, AD7779_ENABLE, AD7779_ENABLE,
            AD7779_ENABLE, AD7779_ENABLE, AD7779_ENABLE, AD7779_ENABLE
        },
        .gain = {
            AD7779_GAIN_1, AD7779_GAIN_1, AD7779_GAIN_1, AD7779_GAIN_1,
            AD7779_GAIN_1, AD7779_GAIN_1, AD7779_GAIN_1, AD7779_GAIN_1
        },
        .dec_rate_int = 512, //decimation rate
        .dec_rate_dec = 0,
        .ref_type     = AD7779_EXT_REF,
        .pwr_mode     = AD7779_HIGH_RES,
        .dclk_div     = AD7779_DCLK_DIV_1,
};

int main()
{
	//init Raspberry Pi Pico
	/* int ret1 = pico_spi_init(&pico_spi_desc, &pico_spi_init_param); */
	
	//init ADC AD7779
	int ret = ad7779_init(&adc_dev, adc_init_param);
	if (ret != 0) {return -1;}


	uint8_t reg_val;
    	ad7779_spi_int_reg_read(dev, AD7779_REG_GENERAL_USER_CONFIG_1, &reg_val);
    	printf("GENERAL_USER_CONFIG_1 = 0x%02X (attendu 0x60)\n", reg_val);

	ad7779_set_spi_op_mode(dev, AD7779_SD_CONV); //Switch to sigma-delta acquisition mode

	ad7779_remove(adc_dev)
	return 0;
}
