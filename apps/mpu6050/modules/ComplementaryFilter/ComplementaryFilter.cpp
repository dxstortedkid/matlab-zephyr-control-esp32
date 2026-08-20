#include "ComplementaryFilter/ComplementaryFilter.hpp"
#include <algorithm>

/**
 * @brief Constructs a new ComplementaryFilter instance and clamps alpha to valid range.
 * 
 * @param alpha Weighting factor in range [0.0f, 1.0f].
 */
ComplementaryFilter::ComplementaryFilter(float alpha)
{
    setAlpha(alpha);
}

/**
 * @brief Fuses accelerometer tilt and gyroscope angular rate to estimate current angle.
 * 
 * @param accel_angle Angle calculated from accelerometer data (deg or rad).
 * @param gyro_rate Angular rate measured by gyroscope (deg/s or rad/s).
 * @param dt Time delta since previous update in seconds.
 * @return float Estimated fused angle.
 */
float ComplementaryFilter::update(float accel_angle, float gyro_rate, float dt)
{
    // Guard against non-positive time deltas
    if (dt <= 0.0f) {
        return angle_;
    }

    // Integrate gyroscope angular velocity over time
    const float gyro_angle = angle_ + (gyro_rate * dt);

    // Complementary fusion equation:
    // Low-pass on accelerometer tilt + High-pass on gyroscope integration
    angle_ = (alpha_ * gyro_angle) + ((1.0f - alpha_) * accel_angle);

    return angle_;
}

/**
 * @brief Resets the internal state to a baseline angle.
 * 
 * @param angle Target baseline angle (defaults to 0.0f).
 */
void ComplementaryFilter::reset(float angle)
{
    angle_ = angle;
}

/**
 * @brief Updates and clamps the alpha weighting coefficient.
 * 
 * @param alpha New alpha coefficient constrained within [0.0f, 1.0f].
 */
void ComplementaryFilter::setAlpha(float alpha)
{
    alpha_ = std::clamp(alpha, 0.0f, 1.0f);
}

/**
 * @brief Retrieves the latest filtered angle without running an update.
 * 
 * @return float Current estimated angle.
 */
float ComplementaryFilter::getAngle() const
{
    return angle_;
}