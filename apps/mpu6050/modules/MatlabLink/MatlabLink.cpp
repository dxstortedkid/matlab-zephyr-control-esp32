#include "MatlabLink/MatlabLink.hpp"
#include <cstring>

namespace MatlabLink {

uint16_t calculateCrc(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (int b = 0; b < 8; ++b) {
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
        }
    }
    return crc;
}

size_t pack(const Message &msg, uint8_t *out_frame) {
    if (msg.length > MAX_PAYLOAD_SIZE || out_frame == nullptr) {
        return 0;
    }

    out_frame[0] = SYNC_LOW;
    out_frame[1] = SYNC_HIGH;
    out_frame[2] = static_cast<uint8_t>(msg.id);
    out_frame[3] = msg.length;

    if (msg.length > 0) {
        std::memcpy(&out_frame[HEADER_SIZE], msg.payload, msg.length);
    }

    // CRC рассчитывается от ID, Length и Payload
    const uint16_t crc = calculateCrc(&out_frame[2], msg.length + 2);
    out_frame[HEADER_SIZE + msg.length]     = static_cast<uint8_t>(crc & 0xFF);
    out_frame[HEADER_SIZE + msg.length + 1] = static_cast<uint8_t>((crc >> 8) & 0xFF);

    return HEADER_SIZE + msg.length + CRC_SIZE;
}

void Parser::reset() {
    state_ = State::WaitSyncLow;
    rx_idx_ = 0;
    crc_recv_ = 0;
    rx_msg_ = Message{};
}

bool Parser::parseByte(uint8_t byte, Message &out_msg) {
    switch (state_) {
        case State::WaitSyncLow:
            if (byte == SYNC_LOW) {
                state_ = State::WaitSyncHigh;
            }
            break;

        case State::WaitSyncHigh:
            state_ = (byte == SYNC_HIGH) ? State::ReadId : State::WaitSyncLow;
            break;

        case State::ReadId:
            rx_msg_.id = static_cast<MessageId>(byte);
            state_ = State::ReadLen;
            break;

        case State::ReadLen:
            if (byte <= MAX_PAYLOAD_SIZE) {
                rx_msg_.length = byte;
                rx_idx_ = 0;
                state_ = (byte > 0) ? State::ReadPayload : State::ReadCrcLow;
            } else {
                reset();
            }
            break;

        case State::ReadPayload:
            rx_msg_.payload[rx_idx_++] = byte;
            if (rx_idx_ >= rx_msg_.length) {
                state_ = State::ReadCrcLow;
            }
            break;

        case State::ReadCrcLow:
            crc_recv_ = static_cast<uint16_t>(byte);
            state_ = State::ReadCrcHigh;
            break;

        case State::ReadCrcHigh: {
            crc_recv_ |= (static_cast<uint16_t>(byte) << 8);
            state_ = State::WaitSyncLow;

            // Расчет контрольной суммы от заголовка (ID + LEN) и полезной нагрузки
            const uint8_t hdr[2] = { static_cast<uint8_t>(rx_msg_.id), rx_msg_.length };
            uint16_t crc_calc = calculateCrc(hdr, 2);

            if (rx_msg_.length > 0) {
                for (size_t i = 0; i < rx_msg_.length; ++i) {
                    crc_calc ^= static_cast<uint16_t>(rx_msg_.payload[i]) << 8;
                    for (int b = 0; b < 8; ++b) {
                        crc_calc = (crc_calc & 0x8000) ? ((crc_calc << 1) ^ 0x1021) : (crc_calc << 1);
                    }
                }
            }

            if (crc_calc == crc_recv_) {
                out_msg = rx_msg_;
                return true;
            }
            break;
        }
    }
    return false;
}

} // namespace MatlabLink