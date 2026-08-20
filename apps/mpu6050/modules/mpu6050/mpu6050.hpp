#pragma once

// Forward declaration of Zephyr device struct to avoid unnecessary header dependencies
struct device;

/**
 * @brief 6-Axis Inertial Measurement Unit (IMU) data container.
 */
struct ImuData
{
    // Accelerometer readings (m/s²)
    float ax{0.0f};
    float ay{0.0f};
    float az{0.0f};

    // Gyroscope readings (rad/s)
    float gx{0.0f};
    float gy{0.0f};
    float gz{0.0f};
};

/**
 * @brief C++ abstraction layer for the InvenSense MPU6050 6-DOF IMU sensor.
 */
class Mpu6050
{
public:
    Mpu6050() = default;

    /**
     * @brief Initializes the sensor device binding and verifies hardware readiness.
     *
     * @return true If the device is ready.
     * @return false If the device is not available.
     */
    [[nodiscard]] bool init();

    /**
     * @brief Reads and returns calibrated sensor data.
     *
     * @param[out] data Destination for the corrected measurements.
     * @return true If the sample was read successfully.
     * @return false If the sensor read failed.
     */
    [[nodiscard]] bool read(ImuData& data);

    /**
     * @brief Calculates sensor offsets while the sensor is stationary.
     *
     * The sensor must remain completely still during calibration.
     *
     * @param samples Number of samples used to calculate the average offset.
     * @return true If calibration completed successfully.
     * @return false If a sensor read failed.
     */
    [[nodiscard]] bool calibrate(int samples);

    /**
     * @brief Resets all calibration offsets to zero.
     */
    void resetCalibration();

private:
    /**
     * @brief Reads raw sensor data without applying calibration offsets.
     */
    [[nodiscard]] bool readRaw(ImuData& data);

private:
    const struct device* dev_{nullptr};

    // Calibration offsets.
    ImuData offset_{};
};