//
// File: untitled.cpp
//
// Code generated for Simulink model 'untitled'.
//
// Model version                  : 1.1
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Sun Aug 16 21:36:18 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#include "untitled.h"

// Model step function
void untitled::step()
{
  // Gain: '<Root>/Gain' incorporates:
  //   Inport: '<Root>/In1'

  untitled_Y.Out1 = 10.0 * untitled_U.In1;

  // Saturate: '<Root>/Saturation'
  if (untitled_Y.Out1 > 0.5) {
    // Gain: '<Root>/Gain' incorporates:
    //   Outport: '<Root>/Out1'

    untitled_Y.Out1 = 0.5;
  } else if (untitled_Y.Out1 < -0.5) {
    // Gain: '<Root>/Gain' incorporates:
    //   Outport: '<Root>/Out1'

    untitled_Y.Out1 = -0.5;
  }

  // End of Saturate: '<Root>/Saturation'
}

// Model initialize function
void untitled::initialize()
{
  // (no initialization code required)
}

// Model terminate function
void untitled::terminate()
{
  // (no terminate code required)
}

const char_T* untitled::RT_MODEL_untitled_T::getErrorStatus() const
{
  return (errorStatus);
}

void untitled::RT_MODEL_untitled_T::setErrorStatus(const char_T* const volatile
  aErrorStatus)
{
  (errorStatus = aErrorStatus);
}

// Constructor
untitled::untitled() :
  untitled_U(),
  untitled_Y(),
  untitled_M()
{
  // Currently there is no constructor body generated.
}

// Destructor
// Currently there is no destructor body generated.
untitled::~untitled() = default;

// Real-Time Model get method
untitled::RT_MODEL_untitled_T * untitled::getRTM()
{
  return (&untitled_M);
}

//
// File trailer for generated code.
//
// [EOF]
//
