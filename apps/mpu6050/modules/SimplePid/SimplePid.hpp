#pragma once

/**
 * @brief Standard Proportional-Integral-Derivative (PID) controller implementation.
 */
class SimplePid
{
public:
    /**
     * @brief Constructs a new SimplePid controller.
     * 
     * @param kp Proportional gain coefficient
     * @param ki Integral gain coefficient
     * @param kd Derivative gain coefficient
     */
    explicit SimplePid(float kp = 0.0f, float ki = 0.0f, float kd = 0.0f);

    /**
     * @brief Computes the PID control effort for the current time step.
     * 
     * @param setpoint Target reference value
     * @param measured Current process variable / sensor measurement
     * @param dt Elapsed time since last update (in seconds)
     * @return float Calculated control output
     */
    [[nodiscard]] float Update(float setpoint, float measured, float dt);

    /**
     * @brief Resets the internal state (accumulated integral and previous error).
     */
    void Reset();

private:
    // Controller gains
    float kp_{0.0f};  ///< Proportional gain
    float ki_{0.0f};  ///< Integral gain
    float kd_{0.0f};  ///< Derivative gain

    // Controller state variables
    float integral_{0.0f};        ///< Accumulated integral error
    float previous_error_{0.0f};  ///< Error value from previous update cycle
};