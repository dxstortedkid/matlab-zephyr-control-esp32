#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#include <cmath>
#include <cstring>

#include "ComplementaryFilter/ComplementaryFilter.hpp"
#include "LowPassFilter/LowPassFilter.hpp"
#include "mpu6050/mpu6050.hpp"
#include "MatlabLink/MatlabLink.hpp"

// =============================================================================
// Application Constants & Tuning Parameters
// =============================================================================

/**
 * @brief Main processing loop period in milliseconds (10 ms -> 100 Hz rate).
 */
static constexpr int32_t SAMPLE_PERIOD_MS = 10;

/**
 * @brief Sampling period in seconds for numerical integration steps.
 */
static constexpr float SAMPLE_PERIOD_S = 0.01f;

/**
 * @brief Number of stationary samples collected for zero-offset bias calibration.
 */
static constexpr int CALIBRATION_SAMPLES = 500;

/**
 * @brief First-order low-pass filter smoothing coefficient for raw accelerometer.
 */
static constexpr float FILTER_ALPHA = 0.1f;

/**
 * @brief Complementary filter weighting factor.
 */
static constexpr float COMPLEMENTARY_ALPHA = 0.98f;

/**
 * @brief Multiplier to convert angles from radians to degrees.
 */
static constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;

// =============================================================================
// Telemetry Payload Structure (Zero-Copy Little-Endian)
// =============================================================================

#pragma pack(push, 1)
struct TelemetryPayload
{
    uint32_t timestamp_ms{0};     ///< System uptime in milliseconds
    float    raw_pitch_deg{0.0f};  ///< Accelerometer pitch before fusion (deg)
    float    raw_roll_deg{0.0f};   ///< Accelerometer roll before fusion (deg)
    float    clean_pitch_deg{0.0f};///< Fused complementary pitch (deg)
    float    clean_roll_deg{0.0f}; ///< Fused complementary roll (deg)
    float    gyro_x{0.0f};         ///< Angular rate around X-axis (rad/s)
    float    gyro_y{0.0f};         ///< Angular rate around Y-axis (rad/s)
    float    gyro_z{0.0f};         ///< Angular rate around Z-axis (rad/s)
};
#pragma pack(pop)

static const struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/**
 * @brief Serializes and transmits the telemetry message using the MatlabLink protocol.
 */
static inline void send_telemetry(const TelemetryPayload &payload)
{
    if (!device_is_ready(uart_dev)) {
        return;
    }

    MatlabLink::Message msg;
    msg.id = MatlabLink::MessageId::Telemetry;
    msg.length = sizeof(TelemetryPayload);
    std::memcpy(msg.payload, &payload, sizeof(TelemetryPayload));

    uint8_t tx_frame[MatlabLink::MAX_FRAME_SIZE];
    const size_t frame_len = MatlabLink::pack(msg, tx_frame);

    for (size_t i = 0; i < frame_len; ++i) {
        uart_poll_out(uart_dev, tx_frame[i]);
    }
}

// =============================================================================
// Geometric Angle Estimation Functions
// =============================================================================

/**
 * @brief Computes the pitch (tilt along Y-axis) angle from linear acceleration.
 */
static inline float calculatePitch(float ax, float ay, float az)
{
    return std::atan2(-ax, std::sqrt((ay * ay) + (az * az)));
}

/**
 * @brief Computes the roll (tilt along X-axis) angle from linear acceleration.
 */
static inline float calculateRoll(float ax, float ay, float az)
{
    (void)ax;
    return std::atan2(ay, az);
}

// =============================================================================
// Application Entry Point
// =============================================================================

int main()
{
    printk("\n[INIT] Starting MPU6050 attitude estimation & MatlabLink streamer...\n");

    // 1. Hardware Initialization
    Mpu6050 imu;
    if (!imu.init()) {
        printk("[ERROR] MPU6050 initialization failed! Halting.\n");
        return -1;
    }
    printk("[INIT] MPU6050 initialized successfully.\n");

    // 2. Sensor Zero-Offset Calibration
    printk("[INIT] Keep MPU6050 still. Calibrating (%d samples)...\n", CALIBRATION_SAMPLES);
    if (!imu.calibrate(CALIBRATION_SAMPLES)) {
        printk("[ERROR] MPU6050 calibration failed! Halting.\n");
        return -1;
    }
    printk("[INIT] Calibration complete. Launching 100 Hz binary MatlabLink stream...\n");

    // 3. Filters Initialization
    LowPassFilter acc_x_filter(FILTER_ALPHA);
    LowPassFilter acc_y_filter(FILTER_ALPHA);
    LowPassFilter acc_z_filter(FILTER_ALPHA);

    ComplementaryFilter pitch_filter(COMPLEMENTARY_ALPHA);
    ComplementaryFilter roll_filter(COMPLEMENTARY_ALPHA);

    // Settle delay before starting UART streaming
    k_msleep(200);

    int64_t next_cycle_time = k_uptime_get();
    TelemetryPayload telemetry_data{};

    // 4. Main Processing Loop (100 Hz)
    while (true) {
        next_cycle_time += SAMPLE_PERIOD_MS;

        ImuData data{};
        if (!imu.read(data)) {
            k_msleep(1);
            continue;
        }

        // 4.1 Suppress high-frequency accelerometer vibrations
        const float ax = acc_x_filter.update(data.ax);
        const float ay = acc_y_filter.update(data.ay);
        const float az = acc_z_filter.update(data.az);

        // 4.2 Pure accelerometer static angles
        const float raw_pitch_rad = calculatePitch(ax, ay, az);
        const float raw_roll_rad  = calculateRoll(ax, ay, az);

        // 4.3 Fused clean attitude angles
        const float pitch_rad = pitch_filter.update(raw_pitch_rad, data.gy, SAMPLE_PERIOD_S);
        const float roll_rad  = roll_filter.update(raw_roll_rad, data.gx, SAMPLE_PERIOD_S);

        // 4.4 Fill telemetry payload
        telemetry_data.timestamp_ms     = static_cast<uint32_t>(k_uptime_get());
        telemetry_data.raw_pitch_deg   = raw_pitch_rad * RAD_TO_DEG;
        telemetry_data.raw_roll_deg    = raw_roll_rad * RAD_TO_DEG;
        telemetry_data.clean_pitch_deg = pitch_rad * RAD_TO_DEG;
        telemetry_data.clean_roll_deg  = roll_rad * RAD_TO_DEG;
        telemetry_data.gyro_x          = data.gx;
        telemetry_data.gyro_y          = data.gy;
        telemetry_data.gyro_z          = data.gz;

        // 4.5 Transmit binary frame via MatlabLink (0x5A, 0xA5, ID, LEN, DATA, CRC16)
        send_telemetry(telemetry_data);

        // 4.6 Fixed-step 10 ms scheduling
        const int64_t current_time = k_uptime_get();
        const int32_t remaining_sleep_ms = static_cast<int32_t>(next_cycle_time - current_time);

        if (remaining_sleep_ms > 0) {
            k_msleep(remaining_sleep_ms);
        }
    }

    return 0;
}