#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include <string.h>
#include "hardware/pio.h"
#include "spi_read_adc.pio.h"
#include "tusb.h"
#include <stdatomic.h>
#include "pico/multicore.h"

#define SPI_SCK0_PIN 2
#define SPI_MOSI0_PIN 3
#define SPI_MISO0_PIN 4
#define SPI_CS0_PIN 1

#define AD7770_RESET 6 
#define AD7770_DRDY0 5
#define AD7770_START 7

#define AD7770_DRDY1 13
#define SPI_SCK1_PIN 10
#define SPI_MOSI1_PIN 11
#define SPI_MISO1_PIN 12
#define SPI_CS1_PIN 9

#define BUF_BYTES   (1 << 14)   // 4 KiB per buffer, tune to be multiple of 64
#define CDC_ITF		0

static uint8_t buf0[BUF_BYTES];
static uint8_t buf1[BUF_BYTES];

static volatile PIO pio_zero;
static volatile uint sm_zero;
static volatile uint offset_zero; 

static volatile PIO pio_one;
static volatile uint sm_one;
static volatile uint offset_one; 

bool startup_done = false;

void core1_main(void) {
	const uint gpio_dbg = 16;
    gpio_init(gpio_dbg);
    gpio_set_dir(gpio_dbg, GPIO_OUT);
    gpio_put(gpio_dbg, 0);
	
	const uint gpio_buf = 21;
	gpio_init(gpio_buf);
	gpio_set_dir(gpio_buf, GPIO_OUT);
	gpio_put(gpio_buf, 0);

    uint8_t txbuf[BUF_BYTES];
    int32_t nbytes_to_process = -1;
    int idx = 0;

    const size_t CHUNK_MAX = 64;
	const size_t FLUSH_THRESH = 64;

    while (1) {
        tud_task();

		if((tud_cdc_n_connected(CDC_ITF)) && (tud_cdc_n_available(CDC_ITF) > 0)){
			uint32_t cmd;
			if ((tud_cdc_n_read(CDC_ITF, &cmd, 1) > 0) && (cmd == 'S')) multicore_fifo_push_blocking('1'); 
		}
		
        if (multicore_fifo_rvalid()) {
            int bufid = (int)multicore_fifo_pop_blocking();
            memcpy(txbuf, (bufid == 0) ? buf0 : buf1, BUF_BYTES);
			gpio_put(gpio_buf, bufid);
            nbytes_to_process = BUF_BYTES;
            idx = 0;
        }

        if (nbytes_to_process > 0) {
            if (!tud_cdc_n_connected(0)) {
                continue;
            }

			gpio_put(gpio_dbg, 1);            
			size_t queued_since_flush = 0;        // batch counter
            while (nbytes_to_process > 0) {
                tud_task();

                int avail = tud_cdc_n_write_available(CDC_ITF);
                if (avail > 0) {
                    size_t to_write = (size_t)avail;
                    if (to_write > CHUNK_MAX) to_write = CHUNK_MAX;
                    if (to_write > (size_t)nbytes_to_process) to_write = (size_t)nbytes_to_process;

                    tud_cdc_n_write(CDC_ITF, txbuf + idx, to_write);

                    idx += to_write;
                    nbytes_to_process -= (int32_t)to_write;


                    queued_since_flush += to_write;

                    // flush only when we've queued enough or when block finished
                    if (queued_since_flush >= FLUSH_THRESH || nbytes_to_process == 0) {
                        tud_cdc_n_write_flush(CDC_ITF);
                        queued_since_flush = 0;
                    }
                    continue;
                }
            }
			gpio_put(gpio_dbg, 0);
            nbytes_to_process = -1;
        } 
    }
}

int main()
{
	tusb_init();
	stdio_init_all(); // Initialize stdio for debugging
	sleep_ms(1000);
	//printf("starting.\r\n");
	
	//init SPI bus
	spi_init(spi0, 200 * 1000); // 20 MHz SPI clock
	spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0 , SPI_MSB_FIRST);
	gpio_set_function(SPI_SCK0_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SPI_MOSI0_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SPI_MISO0_PIN, GPIO_FUNC_SPI);
	gpio_init(SPI_CS0_PIN);
	gpio_set_dir(SPI_CS0_PIN, GPIO_OUT);
	gpio_put(SPI_CS0_PIN, 1);  // Deselect the chip (active low)

	//init SPI bus
	spi_init(spi1, 200 * 1000); // 20 MHz SPI clock
	spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0 , SPI_MSB_FIRST);
	gpio_set_function(SPI_SCK1_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SPI_MOSI1_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SPI_MISO1_PIN, GPIO_FUNC_SPI);
	gpio_init(SPI_CS1_PIN);
	gpio_set_dir(SPI_CS1_PIN, GPIO_OUT);
	gpio_put(SPI_CS1_PIN, 1);  // Deselect the chip (active low)

	//init AD7770 specific pins
	gpio_init(AD7770_RESET);
	gpio_set_dir(AD7770_RESET, GPIO_OUT);
	gpio_put(AD7770_RESET, 0);

	gpio_init(AD7770_START);
	gpio_set_dir(AD7770_START, GPIO_OUT);
	gpio_put(AD7770_START, 0);

	gpio_init(AD7770_DRDY0);
	gpio_set_dir(AD7770_DRDY0, GPIO_IN);

	gpio_init(AD7770_DRDY1);
	gpio_set_dir(AD7770_DRDY1, GPIO_IN);

	//hw reset AD7770
	gpio_put(AD7770_RESET, 0);
	sleep_ms(1);
	gpio_put(AD7770_RESET, 1);
	sleep_ms(10);

	uint8_t txBuf [] = {0x11, 0x74};
	uint8_t txBuf1 [] = {0x13, 0x90};
	uint8_t txBuf2 [] = {0x60, 0x00};
	uint8_t txBuf3 [] = {0x61, 0x80};
	uint8_t txBuf4 [] = {0x64, 0x01};

	gpio_put(SPI_CS0_PIN, 0);
	spi_write_blocking(spi0, txBuf, 2);
	gpio_put(SPI_CS0_PIN, 1);
	gpio_put(SPI_CS1_PIN, 0);
	spi_write_blocking(spi1, txBuf, 2);
	gpio_put(SPI_CS1_PIN, 1);
	sleep_ms(1);
	gpio_put(SPI_CS0_PIN, 0);
	spi_write_blocking(spi0, txBuf1, 2);
	gpio_put(SPI_CS0_PIN, 1);
	gpio_put(SPI_CS1_PIN, 0);
	spi_write_blocking(spi1, txBuf1, 2);
	gpio_put(SPI_CS1_PIN, 1);
	sleep_ms(1);
	gpio_put(SPI_CS0_PIN, 0);
	spi_write_blocking(spi0, txBuf2, 2);
	gpio_put(SPI_CS0_PIN, 1);
	gpio_put(SPI_CS1_PIN, 0);
	spi_write_blocking(spi1, txBuf2, 2);
	gpio_put(SPI_CS1_PIN, 1);
	sleep_ms(1);
	gpio_put(SPI_CS0_PIN, 0);
	spi_write_blocking(spi0, txBuf3, 2);
	gpio_put(SPI_CS0_PIN, 1);
	gpio_put(SPI_CS1_PIN, 0);
	spi_write_blocking(spi1, txBuf3, 2);
	gpio_put(SPI_CS1_PIN, 1);
	sleep_ms(1);
	gpio_put(SPI_CS0_PIN, 0);
	spi_write_blocking(spi0, txBuf4, 2);
	gpio_put(SPI_CS0_PIN, 1);
	gpio_put(SPI_CS1_PIN, 0);
	spi_write_blocking(spi1, txBuf4, 2);
	gpio_put(SPI_CS1_PIN, 1);
	sleep_ms(1);
	txBuf4[1] = 0x00;
	gpio_put(SPI_CS0_PIN, 0);
	spi_write_blocking(spi0, txBuf4, 2);
	gpio_put(SPI_CS0_PIN, 1);
	gpio_put(SPI_CS1_PIN, 0);
	spi_write_blocking(spi1, txBuf4, 2);
	gpio_put(SPI_CS1_PIN, 1);
	sleep_ms(1);

	spi_deinit(spi0);
	spi_deinit(spi1);

    bool success = pio_claim_free_sm_and_add_program_for_gpio_range( \
					&spi_read_adc_program, &pio_zero, &sm_zero, &offset_zero, SPI_CS1_PIN, 5, true);

	spi_read_adc_init(pio_zero, sm_zero, offset_zero, 25000000, SPI_CS1_PIN, SPI_MISO1_PIN);

   	success = pio_claim_free_sm_and_add_program_for_gpio_range( \
					&spi_read_adc1_program, &pio_one, &sm_one, &offset_one, SPI_CS0_PIN, 5, true);

	spi_read_adc1_init(pio_one, sm_one, offset_one, 25000000, SPI_CS0_PIN, SPI_MISO0_PIN);
	
	sleep_ms(10);

    	multicore_launch_core1(core1_main);

	uint32_t cmd = multicore_fifo_pop_blocking();

	uint32_t bufid = 0; // start in buffer0
	gpio_put(AD7770_START, 1);

	//printf("adc running.\r\n");
    while (1) {
        uint8_t *dst = (bufid == 0) ? buf0 : buf1;
        size_t written = 0;
        while (written < BUF_BYTES) {
			if(pio_interrupt_get(pio_zero, 0)){
				pio_interrupt_clear(pio_zero, 0);
				while(!pio_sm_is_rx_fifo_empty(pio_zero, sm_zero)){
					uint32_t data = pio_sm_get(pio_zero, sm_zero);
                    dst[written + 0] = (uint8_t)((data >> 10) & 0xFF);
                    dst[written + 1] = (uint8_t)((data >> 18) & 0x3F);
					dst[written + 1] += ((uint8_t)(data >> 24) >= 0x80) ? 0x80 : 0;
					dst[written + 1] += ((uint8_t)(data >> 24) >= 0x80) ? 0x80 : 0;
                    written += 2;
				}
			}
			if(pio_interrupt_get(pio_one, 1)){
				pio_interrupt_clear(pio_one, 1);
				while(!pio_sm_is_rx_fifo_empty(pio_one, sm_one)){
					uint32_t data = pio_sm_get(pio_one, sm_one);
                    dst[written + 0] = (uint8_t)((data >> 10) & 0xFF);
                    dst[written + 1] = (uint8_t)((data >> 18) & 0x3F);
					dst[written + 1] += ((uint8_t)(data >> 24) >= 0x80) ? 0xC0 : 0x40;
                    written += 2;
				}
			}
        }
        // notify core1 via multicore FIFO (send buffer id 0 or 1)
        multicore_fifo_push_blocking(bufid);
        // loop back to fill the other buffer
		bufid = (bufid ==0) ? 1 : 0;
    }
}
