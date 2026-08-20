# Control System — MATLAB/Simulink + Zephyr + ESP32-C3

Embedded control-system development using **MATLAB/Simulink**, **Zephyr RTOS** and **ESP32-C3**.

The project focuses on taking a control algorithm from simulation to real embedded hardware using a model-based development workflow.

## Workflow

```text
MATLAB / Simulink
        │
        ├── SIL ──► Software-in-the-Loop
        │
        ├── HIL ──► Hardware-in-the-Loop
        │
        ▼
Generated C/C++ code
        │
        ▼
Zephyr RTOS
        │
        ▼
ESP32-C3
```

### SIL — Software-in-the-Loop

The controller is tested using generated software code without requiring the physical target hardware.

This allows the control algorithm to be validated against the Simulink model before deployment.

### HIL — Hardware-in-the-Loop

The controller runs on the **ESP32-C3**, while the plant/system being controlled is simulated.

HIL testing is used to validate real-time behavior, interfaces and controller performance before testing on the actual physical system.

## Project Structure

```text
apps/           # Zephyr applications
docs/           # Documentation
west-manifest/  # Zephyr workspace configuration
.vscode/        # VS Code configuration
```

## Stack

* MATLAB / Simulink
* SIL / HIL testing
* C/C++ code generation
* Zephyr RTOS
* ESP32-C3
* West / CMake

## Goal

Create a reproducible workflow:

**Model → SIL → HIL → Embedded Hardware**

with the same control algorithm moving from simulation to a real-time embedded target.
