#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#define MPU6050_NODE DT_ALIAS(mpu6050)

#if !DT_NODE_EXISTS(MPU6050_NODE)
#define MPU6050_NODE DT_NODELABEL(mpu6050)
#endif

int main(void)
{
    const struct device *imu = DEVICE_DT_GET(MPU6050_NODE);
    struct sensor_value accel[3];
    struct sensor_value gyro[3];
    struct sensor_value temp;
    int ret;

    printf("MPU6050 Zephyr Sensor API demo starting...\n");

    if (!device_is_ready(imu)) {
        printf("MPU6050 device is not ready\n");
        return 0;
    }

    while (1) {
        ret = sensor_sample_fetch(imu);
        if (ret != 0) {
            printf("sensor_sample_fetch failed: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }

        sensor_channel_get(imu, SENSOR_CHAN_ACCEL_XYZ, accel);
        sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, gyro);
        sensor_channel_get(imu, SENSOR_CHAN_DIE_TEMP, &temp);

        printf("ACC [m/s^2] x=%.2f y=%.2f z=%.2f\n",
               sensor_value_to_double(&accel[0]),
               sensor_value_to_double(&accel[1]),
               sensor_value_to_double(&accel[2]));

        printf("GYRO [rad/s] x=%.2f y=%.2f z=%.2f\n",
               sensor_value_to_double(&gyro[0]),
               sensor_value_to_double(&gyro[1]),
               sensor_value_to_double(&gyro[2]));

        printf("TEMP [C] %.2f\n\n", sensor_value_to_double(&temp));

        k_sleep(K_SECONDS(1));
    }
}
