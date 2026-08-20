#pragma once

/**
 * @brief Discrete first-order Low-Pass Filter (Exponential Moving Average).
 * 
 * Filter equation:
 *   output[n] = alpha * input[n] + (1.0 - alpha) * output[n - 1]
 */
class LowPassFilter
{
public:
    /**
     * @brief Constructs a new LowPassFilter instance.
     * 
     * @param alpha Smoothing factor in the range [0.0f, 1.0f].
     *              Lower values provide stronger smoothing with higher phase lag.
     */
    explicit LowPassFilter(float alpha = 1.0f);

    /**
     * @brief Feeds a new raw sample into the filter and computes the filtered value.
     * 
     * @param input Current raw measurement
     * @return float Filtered output value
     */
    [[nodiscard]] float update(float input);

    /**
     * @brief Resets the internal filtered state to a specific value.
     * 
     * @param initial_value Target baseline value (defaults to 0.0f)
     */
    void reset(float initial_value = 0.0f);

    /**
     * @brief Updates the smoothing factor alpha dynamically.
     * 
     * @param alpha New smoothing coefficient in range [0.0f, 1.0f]
     */
    void setAlpha(float alpha);

    /**
     * @brief Retrieves the latest filtered output without pushing a new sample.
     * 
     * @return float Current filtered output
     */
    [[nodiscard]] float getOutput() const;

private:
    float alpha_{1.0f};   ///< Smoothing coefficient (0.0f <= alpha <= 1.0f)
    float output_{0.0f};  ///< Filter state / current output
};