#include "LowPassFilter/LowPassFilter.hpp"
#include <algorithm>

/**
 * @brief Constructs a new LowPassFilter instance and clamps alpha to valid range.
 * 
 * @param alpha Smoothing factor coefficient in range [0.0f, 1.0f].
 */
LowPassFilter::LowPassFilter(float alpha)
    : output_(0.0f)
{
    setAlpha(alpha);
}

/**
 * @brief Feeds a new raw sample into the filter and computes the filtered value.
 * 
 * @param input Current raw input sample
 * @return float Smoothed output value
 */
float LowPassFilter::update(float input)
{
    // Single-multiplication form: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
    output_ += alpha_ * (input - output_);

    return output_;
}

/**
 * @brief Resets the filter's internal state to a baseline value.
 * 
 * @param initial_value Starting baseline value (defaults to 0.0f)
 */
void LowPassFilter::reset(float initial_value)
{
    output_ = initial_value;
}

/**
 * @brief Updates and bounds the smoothing factor alpha.
 * 
 * @param alpha New alpha value to be clamped to [0.0f, 1.0f]
 */
void LowPassFilter::setAlpha(float alpha)
{
    alpha_ = std::clamp(alpha, 0.0f, 1.0f);
}

/**
 * @brief Retrieves the latest filtered output without modifying the internal state.
 * 
 * @return float Current filtered output
 */
float LowPassFilter::getOutput() const
{
    return output_;
}