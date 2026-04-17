#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define CS_PIN 7 
#define SCK_PIN 4
#define MOSI_PIN 5
#define MISO_PIN 6

// Registers
static const uint8_t REG_DEVID = 0x00;
static const uint8_t REG_POWER_CTL = 0x2D;
static const uint8_t REG_DATAX0 = 0x32;

// Other constants
static const uint8_t DEVID = 0xE5;
static const float SENSITIVITY_2G = 1.0 / 256;  // (g/LSB)
static const float EARTH_GRAVITY = 9.80665;     // Earth's gravity in [m/s^2]



/* #include "pico_spi.h" */
/* #include "no_os_spi.h" */

/* struct no_os_spi_desc *spi_desc; */

/* // Paramètres spécifiques au Pico (broches) */
/* struct pico_spi_init_param pico_spi_extra = { */
/*     .spi_tx_pin  = SPI0_TX_GP19,   // MOSI */
/*     .spi_rx_pin  = SPI0_RX_GP16,   // MISO */
/*     .spi_sck_pin = SPI0_SCK_GP18,  // CLK */
/*     .spi_cs_pin  = SPI0_CS_GP17,   // CS */
/* }; */

/* // Paramètres généraux SPI */
/* struct no_os_spi_init_param spi_param = { */
/*     .device_id     = 0,              // spi0 */
/*     .max_speed_hz  = 1000000,        // 1 MHz */
/*     .mode          = NO_OS_SPI_MODE_3, // AD7777 : CPOL=1, CPHA=1 */
/*     .extra         = &pico_spi_extra, */
/*     .platform_ops  = &pico_spi_ops, */
/* }; */


/*******************************************************************************
 * Function Declarations
 */
void reg_write( spi_inst_t *spi, 
                const uint cs, 
                const uint8_t reg, 
                const uint8_t data);

int reg_read(  spi_inst_t *spi,
                const uint cs,
                const uint8_t reg,
                uint8_t *buf,
                uint8_t nbytes);

/*******************************************************************************
 * Function Definitions
 */

// Write 1 byte to the specified register
void reg_write( spi_inst_t *spi, 
                const uint cs, 
                const uint8_t reg, 
                const uint8_t data) {

    uint8_t msg[2];
                
    // Construct message (set ~W bit low, MB bit low)
    msg[0] = 0x00 | reg;
    msg[1] = data;

    // Write to register
    gpio_put(cs, 0);
    spi_write_blocking(spi, msg, 2);
    gpio_put(cs, 1);
}

// Read byte(s) from specified register. If nbytes > 1, read from consecutive
// registers.
int reg_read(  spi_inst_t *spi,
                const uint cs,
                const uint8_t reg,
                uint8_t *buf,
                const uint8_t nbytes) {

    int num_bytes_read = 0;
    uint8_t mb = 0;

    // Determine if multiple byte (MB) bit should be set
    if (nbytes < 1) {
        return -1;
    } else if (nbytes == 1) {
        mb = 0;
    } else {
        mb = 1;
    }

    // Construct message (set ~W bit high)
    uint8_t msg = 0x80 | (mb << 6) | reg;

    // Read from register
    gpio_put(cs, 0);
    spi_write_blocking(spi, &msg, 1);
    num_bytes_read = spi_read_blocking(spi, 0, buf, nbytes);
    gpio_put(cs, 1);

    return num_bytes_read;
}

/*******************************************************************************
 * Main
 */
int main() {

    /* int16_t acc_x; */
    /* int16_t acc_y; */
    /* int16_t acc_z; */
    /* float acc_x_f; */
    /* float acc_y_f; */
    /* float acc_z_f; */

    // Pins

    // Buffer to store raw reads
    uint8_t data[6];

    // Ports
    spi_inst_t *spi = spi0;

    // Initialize chosen serial port
    stdio_init_all();

    // Initialize CS pin high
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1);

    // Initialize SPI port at 1 MHz
    spi_init(spi, 1000 * 1000);

    // Set SPI format
    spi_set_format( spi0,   // SPI instance
                    8,      // Number of bits per transfer
                    1,      // Polarity (CPOL)
                    1,      // Phase (CPHA)
                    SPI_MSB_FIRST);

    // Initialize SPI pins
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MISO_PIN, GPIO_FUNC_SPI);

    /* // Workaround: perform throw-away read to make SCK idle high */
    /* reg_read(spi, CS_PIN, REG_DEVID, data, 1); */

    /* // Read device ID to make sure that we can communicate with the ADXL343 */
    /* reg_read(spi, CS_PIN, REG_DEVID, data, 1); */
    /* if (data[0] != DEVID) { */
    /*     printf("ERROR: Could not communicate \r\n"); */
    /*     while (true); */
    /* } */
    
    /* // Read Power Control register */
    /* reg_read(spi, CS_PIN, REG_POWER_CTL, data, 1); */
    /* printf("0x␂X\r\n", data[0]); */

    /* // Tell ADXL343 to start taking measurements by setting Measure bit to high */
    /* data[0] |= (1 << 3); */
    /* reg_write(spi, cs_pin, REG_POWER_CTL, data[0]); */

    /* // Test: read Power Control register back to make sure Measure bit was set */
    /* reg_read(spi, cs_pin, REG_POWER_CTL, data, 1); */
    /* printf("0x␂X\r\n", data[0]); */

    /* // Wait before taking measurements */
    /* sleep_ms(2000); */

    // Loop forever
    while (true) {

        /* // Read X, Y, and Z values from registers (16 bits each) */
        /* reg_read(spi, cs_pin, REG_DATAX0, data, 6); */

        /* // Convert 2 bytes (little-endian) into 16-bit integer (signed) */
        /* acc_x = (int16_t)((data[1] << 8) | data[0]); */
        /* acc_y = (int16_t)((data[3] << 8) | data[2]); */
        /* acc_z = (int16_t)((data[5] << 8) | data[4]); */

        /* // Convert measurements to [m/s^2] */
        /* acc_x_f = acc_x * SENSITIVITY_2G * EARTH_GRAVITY; */
        /* acc_y_f = acc_y * SENSITIVITY_2G * EARTH_GRAVITY; */
        /* acc_z_f = acc_z * SENSITIVITY_2G * EARTH_GRAVITY; */

        /* // Print results */
        /* printf("X: %.2f | Y: %.2f | Z: %.2f\r\n", acc_x_f, acc_y_f, acc_z_f); */

        /* sleep_ms(100); */
    }
}
