# Action Plan & Roadmap: MatlabSimpleLink Protocol

Breaking down the task into atomic steps for the `v0.1` implementation, while establishing a backlog of architectural improvements for future releases (`v0.2+`). 

*Print this out, hang it above your monitor, and check off items as they are completed.*

---

### Phase 1: Target Side (ESP32-C3 / Zephyr RTOS / C++)
- [ ] **Create `Protocol.hpp`**: Define sync byte constants (`0xAA`, `0x55`) and `MessageId` enumerations.
- [ ] **Define Payload Structures**: Implement `TelemetryFrame` (24 bytes) and `TuningFrame` (16 bytes), including the `reserved` padding fields.
- [ ] **Enforce Memory Packing**: Wrap the structures in `#pragma pack(push, 1)` and `#pragma pack(pop)` directives to prevent compiler padding.
- [ ] **Implement Safety Checks**: Add `static_assert(sizeof(...) == ...)` to guarantee structure footprints at compile time.
- [ ] **Develop RX FSM Parser**: Write a non-blocking finite-state machine (`switch-case`) processing byte-by-byte (States: `Wait_0xAA`, `Wait_0x55`, `Read_ID`, `Read_LEN`, `Read_Payload`).
- [ ] **Develop TX Packing Routine**: Write a serialization function that takes a `TelemetryFrame` struct, prepends the 4-byte header, and returns a flat `uint8_t` array for transmission.
- [ ] **Integrate into Hard Real-Time Loop (100 Hz)**: 
  - Invoke the FSM parser BEFORE executing `controller.step()`.
  - Upon successful `0x10` packet parsing, atomically update model coefficients (protected by a mutex).
  - Construct telemetry and transmit via UART immediately AFTER `controller.step()`.

### Phase 2: Host Side (MATLAB / Simulink)
- [ ] **Develop Telemetry Parser M-script (`parse_telemetry.m`)**: Utilize `persistent` variables within the MATLAB Function block to retain FSM states across discrete simulation steps.
- [ ] **Configure `Serial Receive` Simulink Block**: 
  - Disable the `Header` and `End token` (terminator) checkboxes. 
  - Set *Data type* to `uint8`.
  - Set *Data size* to `[28 1]` (4 bytes header + 24 bytes `TelemetryFrame`).
- [ ] **Develop Tuning Packer M-script (`pack_tuning.m`)**: Write a MATLAB function that serializes `single` values ($K_p$, $K_d$, $Target$) into a `uint8` array using `typecast`, and prepends the protocol header.
- [ ] **Configure `Serial Send` Simulink Block**: Route the output array of `pack_tuning.m` directly into this block.
- [ ] **Construct Dashboard**: Add UI elements (Knobs/Sliders) for on-the-fly tuning and a Scope block for real-time visualization of `pitch`, `roll`, and `motor_out`.

---

### Phase 3: Architectural Improvements Backlog (TODO v0.2+)
*Note: Implement these features ONLY after the baseline v0.1 is operational and the cube demonstrates stable balancing.*

- [ ] **CRC-16 (CCITT-FALSE) Integration**: Data corruption protection. Append a 2-byte checksum to the frame and verify it within the FSM before applying the payload.
- [ ] **Watchdog Timer (Failsafe)**: Configure a software/hardware timer in Zephyr that resets upon every successful tuning/heartbeat packet reception. If the timer exceeds 500 ms (cable disconnection or PC freeze), trigger an interrupt to hardware-disable motor PWM (Safe State).
- [ ] **Asynchronous UART (Interrupts / DMA)**: Migrate away from blocking/polling `uart_poll_in` and `uart_poll_out`. Implement `uart_irq_rx_ready()` to sink incoming bytes into a Ring Buffer via ISR, freeing up CPU cycles in the primary 100 Hz thread.
- [ ] **Acknowledgment (ACK Protocol)**: Upon receiving a tuning payload (ID `0x10`), the MCU must transmit a zero-payload ACK frame (ID `0x20`) to update a Simulink UI indicator ("Settings Applied").
- [ ] **Non-Volatile Storage (NVS / Flash)**: Introduce packet ID `0x12` (`Save_To_Flash`). Upon receipt, the MCU commits the current successful PID coefficients to internal Flash memory, enabling standalone balancing operations on power-up without MATLAB.
- [ ] **Variable Rate Telemetry (Stream Separation)**: Transmit critical physical data at 100 Hz (ID `0x01`), and secondary system data (battery voltage, CPU load, error flags) at a lower frequency like 1-10 Hz (ID `0x02`) to prevent UART bus saturation.