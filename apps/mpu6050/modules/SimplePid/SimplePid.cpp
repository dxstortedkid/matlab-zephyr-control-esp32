#include "SimplePid/SimplePid.hpp"

/**
 * @brief Constructs a new SimplePid controller.
 * 
 * @param kp Proportional gain coefficient
 * @param ki Integral gain coefficient
 * @param kd Derivative gain coefficient
 */
SimplePid::SimplePid(float kp, float ki, float kd)
    : kp_(kp),
      ki_(ki),
      kd_(kd),
      integral_(0.0f),
      previous_error_(0.0f)
{
}

/**
 * @brief Calculates the PID controller output based on the current error.
 * 
 * @param setpoint The desired target value
 * @param measured The current actual/measured value
 * @param dt Time delta since the last update in seconds
 * @return float Computed control effort/output
 */
float SimplePid::Update(float setpoint, float measured, float dt)
{
    // Protect against division by zero and negative time deltas
    if (dt <= 0.0f) {
        return 0.0f;
    }

    const float error = setpoint - measured;

    // Proportional & Integral accumulation
    integral_ += error * dt;

    // Derivative calculation (rate of error change)
    const float derivative = (error - previous_error_) / dt;

    // Compute final control output
    const float output = (kp_ * error) + (ki_ * integral_) + (kd_ * derivative);

    // Save state for the next iteration
    previous_error_ = error;

    return output;
}

/**
 * @brief Resets the internal state (accumulated integral and previous error).
 */
void SimplePid::Reset()
{
    integral_ = 0.0f;
    previous_error_ = 0.0f;
}