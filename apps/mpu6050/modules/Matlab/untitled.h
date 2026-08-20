//
// File: untitled.h
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
#ifndef untitled_h_
#define untitled_h_
#include <cmath>
#include "rtwtypes.h"

// Class declaration for model untitled
class untitled final
{
  // public data and function members
 public:
  // External inputs (root inport signals with default storage)
  struct ExtU_untitled_T {
    real_T In1;                        // '<Root>/In1'
  };

  // External outputs (root outports fed by signals with default storage)
  struct ExtY_untitled_T {
    real_T Out1;                       // '<Root>/Out1'
  };

  // Real-time Model Data Structure
  struct RT_MODEL_untitled_T {
    const char_T * volatile errorStatus;
    const char_T* getErrorStatus() const;
    void setErrorStatus(const char_T* const volatile aErrorStatus);
  };

  // Copy Constructor
  untitled(untitled const&) = delete;

  // Assignment Operator
  untitled& operator= (untitled const&) & = delete;

  // Move Constructor
  untitled(untitled &&) = delete;

  // Move Assignment Operator
  untitled& operator= (untitled &&) = delete;

  // Real-Time Model get method
  untitled::RT_MODEL_untitled_T * getRTM();

  // Root inports set method
  void setExternalInputs(const ExtU_untitled_T *pExtU_untitled_T)
  {
    untitled_U = *pExtU_untitled_T;
  }

  // Root outports get method
  const ExtY_untitled_T &getExternalOutputs() const
  {
    return untitled_Y;
  }

  // model initialize function
  static void initialize();

  // model step function
  void step();

  // model terminate function
  static void terminate();

  // Constructor
  untitled();

  // Destructor
  ~untitled();

  // private data and function members
 private:
  // External inputs
  ExtU_untitled_T untitled_U;

  // External outputs
  ExtY_untitled_T untitled_Y;

  // Real-Time Model
  RT_MODEL_untitled_T untitled_M;
};

//-
//  The generated code includes comments that allow you to trace directly
//  back to the appropriate location in the model.  The basic format
//  is <system>/block_name, where system is the system number (uniquely
//  assigned by Simulink) and block_name is the name of the block.
//
//  Use the MATLAB hilite_system command to trace the generated code back
//  to the model.  For example,
//
//  hilite_system('<S3>')    - opens system 3
//  hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
//
//  Here is the system hierarchy for this model
//
//  '<Root>' : 'untitled'

#endif                                 // untitled_h_

//
// File trailer for generated code.
//
// [EOF]
//
