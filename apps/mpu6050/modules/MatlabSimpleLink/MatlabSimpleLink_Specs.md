# Specification: MatlabSimpleLink Protocol v0.1

**Status:** Draft  
**Target Architecture:** ESP32-C3 (Zephyr RTOS) <-> x86 (MATLAB/Simulink)  
**Architectural Rationale:** Minimal framing overhead (4 bytes per frame), deterministic O(1) parsing time, non-blocking execution. Designed explicitly for hard real-time constraints (100 Hz control loop). CRC, ACK, and Watchdog mechanisms are intentionally omitted in this revision to conserve CPU cycles and simplify the initial FSM implementation.

---

## 1. Physical Layer (PHY)
*   **Transport:** UART
*   **Configuration:** 115200 Baud (sufficient margin for 100 Hz), 8 Data bits, No Parity, 1 Stop bit (8-N-1).
*   **Byte Order:** Strict Little-Endian. Both the ESP32-C3 and x86 architectures natively utilize Little-Endian. No byte-swapping is permitted; memory is cast as-is (type punning).
*   **Buffer/RAM Limits:** The `PAYLOAD` size is strictly bounded to a maximum of `64` bytes (expanded for future extensibility) to prevent stack buffer overflows and guarantee deterministic memory allocation.

## 2. Data Link Layer (Framing)
The frame consists of a 4-byte header followed by a variable-length payload.

| Offset | Field     | Size   | Value / Description |
| :---   | :---      | :---   | :--- |
| `0x00` | `SYNC_1`  | 1 byte | `0xAA` (Binary `10101010`) — Hardware-optimized frame start marker. |
| `0x01` | `SYNC_2`  | 1 byte | `0x55` (Binary `01010101`) — Hardware-optimized sync confirmation. |
| `0x02` | `MSG_ID`  | 1 byte | Message Identifier (defines payload structure routing). |
| `0x03` | `LEN`     | 1 byte | `PAYLOAD` length in bytes ($0 \le LEN \le 64$). |
| `0x04` | `PAYLOAD` | `LEN`  | Raw memory dump (Type-punned data). |

**FSM Parser Constraints:**
The finite-state machine (FSM) must continuously scan for the `0xAA` -> `0x55` sequence. Upon successful synchronization, it reads `MSG_ID` and `LEN`. 
*Safety Interlock:* If `LEN > 64`, the FSM **MUST** immediately discard the data and reset to the `SYNC_1` search state to mitigate bus corruption and Out-of-Memory (OOM) conditions.

## 3. Message Dictionary (Message IDs)

*Note: Payloads have been intentionally padded with reserved fields to allow seamless future expansion without breaking backward compatibility of the parser.*

### 3.1. Downlink (MCU -> MATLAB): ID `0x01` [Telemetry]
*   **Mode:** Periodic (Timer-driven / Thread wakeup).
*   **Frequency:** 100 Hz.
*   **Payload Length:** 24 bytes.
*   **Structure:**
    *   `[0x00..0x03]` `float pitch` (Pitch angle)
    *   `[0x04..0x07]` `float roll` (Roll angle)
    *   `[0x08..0x0B]` `float gyro_y` (Y-axis angular rate)
    *   `[0x0C..0x0F]` `float motor_out` (Control effort/output)
    *   `[0x10..0x17]` `uint32_t reserved[2]` (Reserved for future states, **MUST** be zeroed out)

### 3.2. Uplink (MATLAB -> MCU): ID `0x10` [Tuning]
*   **Mode:** Asynchronous (Event-driven / ISR RX callback).
*   **Frequency:** On-demand (Triggered by Dashboard changes in Simulink).
*   **Payload Length:** 16 bytes.
*   **Structure:**
    *   `[0x00..0x03]` `float kp` (Proportional gain)
    *   `[0x04..0x07]` `float kd` (Derivative gain)
    *   `[0x08..0x0B]` `float target_angle` (Setpoint angle)
    *   `[0x0C..0x0F]` `uint32_t reserved` (Reserved for future parameters/flags, **MUST** be zeroed out)

## 4. Explicit Serialization (Manual Packing)
To eliminate compiler dependency, prevent strict-aliasing violations, and avoid Hardware Alignment Faults, the use of `#pragma pack` is **strictly prohibited**. 

All protocol payloads must be serialized and deserialized manually into/from flat `uint8_t` buffers using explicit memory copying. 

**Serialization Principle (Type Punning via `memcpy`):**
In C++, the only standard-compliant way to perform type punning (e.g., extracting bytes from a `float`) without undefined behavior is using `std::memcpy`.

**C++ Implementation Mandate (Target Side):**