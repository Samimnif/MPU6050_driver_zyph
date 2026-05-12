#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

#include "mpu6050.h"

#define I2C_NODE DT_NODELABEL(i2c0)

int main(void)
{
    const struct device *i2c_bus = DEVICE_DT_GET(I2C_NODE);
    mpu6050_t imu;
    mpu6050_data_t data;
    uint8_t chip_id;

    printf("MPU6050 Zephyr test starting...\n");

    int ret = mpu6050_init(&imu, i2c_bus, MPU6050_I2C_ADDR_0);
    if (ret != 0) {
        printf("MPU6050 init failed: %d\n", ret);
        return ret;
    }

    ret = mpu6050_read_who_am_i(&imu, &chip_id);
    if (ret == 0) {
        printf("MPU6050 WHO_AM_I: 0x%02X\n", chip_id);
    }

    while (1) {
        ret = mpu6050_read(&imu, &data);
        if (ret == 0) {
            printf("ACC[g]  x=%.2f y=%.2f z=%.2f\n",
                   data.accel_x_g, data.accel_y_g, data.accel_z_g);
            printf("GYRO[dps] x=%.2f y=%.2f z=%.2f\n",
                   data.gyro_x_dps, data.gyro_y_dps, data.gyro_z_dps);
            printf("TEMP[C] %.2f\n\n", data.temperature_c);
        } else {
            printf("MPU6050 read failed: %d\n", ret);
        }

        k_sleep(K_SECONDS(1));
    }
}