#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>

#include <bme680_reg.h>

#define BME680_ADDR 0x77

#define I2C_LABEL i2c0

#define PAR_T1_LSB 0xE9
#define PAR_T2_LSB 0x8A
#define PAR_T3 0x8C

int main(void)
{
    const struct device *i2c_bus = DEVICE_DT_GET(DT_NODELABEL(I2C_LABEL));

    if (!device_is_ready(i2c_bus))
    {
        printf("I2C not ready\n");
        return -1;
    }

    uint8_t id;
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, BME680_ID, &id);
    printf("Everything is working.\n");
    printf("Chip ID: 0x%02X\n", id);

    uint8_t lsb, msb;


    //From datasheet
    /* par_t1 is uint16 from 0xE9/0xEA */
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, PAR_T1_LSB, &lsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, PAR_T1_LSB + 1, &msb);
    uint16_t par_t1 = ((uint16_t)msb << 8) | lsb;

    /* par_t2 is int16 from 0x8A/0x8B */
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, PAR_T2_LSB, &lsb);
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, PAR_T2_LSB + 1, &msb);
    int16_t par_t2 = (int16_t)(((uint16_t)msb << 8) | lsb);

    /* par_t3 is int8 from 0x8C */
    i2c_reg_read_byte(i2c_bus, BME680_ADDR, PAR_T3, &lsb);
    int8_t par_t3 = (int8_t)lsb;

    printf("calib: par_t1=%u par_t2=%d par_t3=%d\n", par_t1, par_t2, par_t3);

    while (1)
    {

        i2c_reg_write_byte(i2c_bus, BME680_ADDR, BME680_CTRL_MEAS, 0x41);

        k_sleep(K_MSEC(100));

        uint8_t data[3];
        i2c_burst_read(i2c_bus, BME680_ADDR, BME680_TEMP_MSB, data, 3);

        uint32_t temp_adc =
            (data[0] << 12) |
            (data[1] << 4) |
            (data[2] >> 4);

        printf("Raw temp ADC: %u\n", temp_adc);

        // From datasheet

        double var1 = (((double)temp_adc / 16384.0) - ((double)par_t1 / 1024.0)) * (double)par_t2;
        double var2 = ((((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0)) *
                       (((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0))) *
                      ((double)par_t3 * 16.0);

        double t_fine = var1 + var2;
        double temp_comp = t_fine / 5120.0;

        printf("Temperature: %.2f C\n", temp_comp);

        k_sleep(K_SECONDS(3));
    }
}