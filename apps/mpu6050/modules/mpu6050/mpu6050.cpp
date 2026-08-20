#include "mpu6050/mpu6050.hpp"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#define MPU6050_NODE DT_NODELABEL(mpu6050)

/**
 * @brief Initializes the MPU6050 device binding and verifies hardware readiness.
 */
bool Mpu6050::init()
{
    dev_ = DEVICE_DT_GET(MPU6050_NODE);

    if (!device_is_ready(dev_)) {
        return false;
    }

    return true;
}

/**
 * @brief Reads raw data from the MPU6050 without applying calibration.
 */
bool Mpu6050::readRaw(ImuData& data)
{
    if (dev_ == nullptr) {
        return false;
    }

    if (sensor_sample_fetch(dev_) < 0) {
        return false;
    }

    sensor_value accel[3];
    sensor_value gyro[3];

    if (sensor_channel_get(
            dev_,
            SENSOR_CHAN_ACCEL_XYZ,
            accel) < 0) {
        return false;
    }

    if (sensor_channel_get(
            dev_,
            SENSOR_CHAN_GYRO_XYZ,
            gyro) < 0) {
        return false;
    }

    data.ax = static_cast<float>(
        sensor_value_to_double(&accel[0]));

    data.ay = static_cast<float>(
        sensor_value_to_double(&accel[1]));

    data.az = static_cast<float>(
        sensor_value_to_double(&accel[2]));

    data.gx = static_cast<float>(
        sensor_value_to_double(&gyro[0]));

    data.gy = static_cast<float>(
        sensor_value_to_double(&gyro[1]));

    data.gz = static_cast<float>(
        sensor_value_to_double(&gyro[2]));

    return true;
}

/**
 * @brief Reads calibrated data.
 */
bool Mpu6050::read(ImuData& data)
{
    ImuData raw{};

    if (!readRaw(raw)) {
        return false;
    }

    data.ax = raw.ax - offset_.ax;
    data.ay = raw.ay - offset_.ay;
    data.az = raw.az - offset_.az;

    data.gx = raw.gx - offset_.gx;
    data.gy = raw.gy - offset_.gy;
    data.gz = raw.gz - offset_.gz;

    return true;
}

/**
 * @brief Calculates calibration offsets.
 *
 * MPU6050 must remain completely stationary during calibration.
 */
bool Mpu6050::calibrate(int samples)
{
    if (samples <= 0) {
        return false;
    }

    ImuData sum{};

    for (int i = 0; i < samples; ++i) {
        ImuData raw{};

        if (!readRaw(raw)) {
            return false;
        }

        sum.ax += raw.ax;
        sum.ay += raw.ay;
        sum.az += raw.az;

        sum.gx += raw.gx;
        sum.gy += raw.gy;
        sum.gz += raw.gz;

        k_msleep(5);
    }

    const float count = static_cast<float>(samples);

    const float average_ax = sum.ax / count;
    const float average_ay = sum.ay / count;
    const float average_az = sum.az / count;

    const float average_gx = sum.gx / count;
    const float average_gy = sum.gy / count;
    const float average_gz = sum.gz / count;

    /*
     * Accelerometer:
     *
     * When the sensor is lying flat:
     * X ≈ 0
     * Y ≈ 0
     * Z ≈ +9.81 m/s²
     */
    offset_.ax = average_ax;
    offset_.ay = average_ay;
    offset_.az = average_az - 9.80665f;

    /*
     * Gyroscope:
     *
     * When stationary:
     * X ≈ 0
     * Y ≈ 0
     * Z ≈ 0
     */
    offset_.gx = average_gx;
    offset_.gy = average_gy;
    offset_.gz = average_gz;

    return true;
}

/**
 * @brief Resets all calibration offsets.
 */
void Mpu6050::resetCalibration()
{
    offset_ = ImuData{};
}