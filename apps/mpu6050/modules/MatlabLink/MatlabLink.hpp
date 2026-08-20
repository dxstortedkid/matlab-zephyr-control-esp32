#pragma once

#include <cstdint>
#include <cstddef>

namespace MatlabLink {

constexpr uint8_t SYNC_LOW  = 0x5A;
constexpr uint8_t SYNC_HIGH = 0xA5;

constexpr size_t MAX_PAYLOAD_SIZE = 64;
constexpr size_t HEADER_SIZE      = 4; // SYNC_LOW + SYNC_HIGH + ID + LEN
constexpr size_t CRC_SIZE         = 2; // CRC_LOW + CRC_HIGH
constexpr size_t MAX_FRAME_SIZE   = HEADER_SIZE + MAX_PAYLOAD_SIZE + CRC_SIZE;

enum class MessageId : uint8_t {
    Telemetry    = 0x01,
    Status       = 0x02,

    SetParameter = 0x10,
    GetParameter = 0x11,

    Ack          = 0x20,
    Error        = 0x21,
};

struct Message {
    MessageId id{MessageId::Telemetry};
    uint8_t   length{0};
    uint8_t   payload[MAX_PAYLOAD_SIZE]{0};
};

/**
 * @brief Расчет контрольной суммы CRC-16/CCITT-FALSE.
 */
uint16_t calculateCrc(const uint8_t *data, size_t len);

/**
 * @brief Упаковка контейнера Message в бинарный фрейм для UART.
 * @return Количество записанных байт (0 при ошибке).
 */
size_t pack(const Message &msg, uint8_t *out_frame);

/**
 * @brief Потоковый конечный автомат для неблокирующего разбора байт.
 */
class Parser {
public:
    Parser() = default;

    void reset();
    bool parseByte(uint8_t byte, Message &out_msg);

private:
    enum class State : uint8_t {
        WaitSyncLow,
        WaitSyncHigh,
        ReadId,
        ReadLen,
        ReadPayload,
        ReadCrcLow,
        ReadCrcHigh
    };

    State    state_{State::WaitSyncLow};
    Message  rx_msg_{};
    size_t   rx_idx_{0};
    uint16_t crc_recv_{0};
};

} // namespace MatlabLink