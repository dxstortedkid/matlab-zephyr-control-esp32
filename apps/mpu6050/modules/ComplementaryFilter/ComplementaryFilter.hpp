#pragma once

/**
 * @brief Discrete Complementary Filter for IMU sensor fusion.
 * 
 * Combines high-frequency gyroscope integration with low-frequency accelerometer tilt:
 *   angle = alpha * (angle + gyro_rate * dt) + (1.0f - alpha) * accel_angle
 */
class ComplementaryFilter
{
public:
    /**
     * @brief Constructs a new ComplementaryFilter instance.
     * 
     * @param alpha Weight factor in the range [0.0f, 1.0f].
     *              Higher values (e.g., 0.98) trust gyroscope integration more;
     *              lower values rely more heavily on accelerometer tilt.
     */
    explicit ComplementaryFilter(float alpha = 0.98f);

    /**
     * @brief Updates the fused angle estimation with fresh sensor samples.
     * 
     * @param accel_angle Angle calculated from accelerometer measurements (deg or rad).
     * @param gyro_rate Angular rate measured by gyroscope (deg/s or rad/s).
     * @param dt Time delta since previous sample in seconds.
     * @return float Fused orientation angle.
     */
    [[nodiscard]] float update(float accel_angle, float gyro_rate, float dt);

    /**
     * @brief Resets the internal state to a baseline angle.
     * 
     * @param angle Target baseline angle (defaults to 0.0f).
     */
    void reset(float angle = 0.0f);

    /**
     * @brief Dynamically updates the weighting coefficient alpha.
     * 
     * @param alpha New coefficient value in the range [0.0f, 1.0f].
     */
    void setAlpha(float alpha);

    /**
     * @brief Retrieves the latest filtered angle without running an update.
     * 
     * @return float Current estimated angle.
     */
    [[nodiscard]] float getAngle() const;

private:
    float alpha_{0.98f};  ///< Gyroscope trust coefficient (0.0f <= alpha <= 1.0f)
    float angle_{0.0f};   ///< Current fused angle state
};