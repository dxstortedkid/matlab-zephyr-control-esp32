function [valid, timestamp_ms, raw_pitch, raw_roll, clean_pitch, clean_roll, gx, gy, gz] = parse_telemetry(raw_bytes)
% Декодер пакетов MatlabLink (FSM Parser + CRC-16 Check)

% Инициализация статических буферов состояния
persistent state rx_buf rx_idx expected_len msg_id crc_low
if isempty(state)
    state = uint8(0); % 0: SYNC1, 1: SYNC2, 2: ID, 3: LEN, 4: PAYLOAD, 5: CRC1, 6: CRC2
    rx_buf = uint8(zeros(1, 64));
    rx_idx = uint8(1);
    expected_len = uint8(0);
    msg_id = uint8(0);
    crc_low = uint8(0);
end

% Значения выходов по умолчанию
valid        = false;
timestamp_ms = uint32(0);
raw_pitch    = single(0);
raw_roll     = single(0);
clean_pitch  = single(0);
clean_roll   = single(0);
gx           = single(0);
gy           = single(0);
gz           = single(0);

% Обработка входящего массива байт
for i = 1:length(raw_bytes)
    b = raw_bytes(i);

    switch state
        case 0 % Ожидание SYNC_LOW (0x5A)
            if b == 90 % 0x5A
                state = uint8(1);
            end

        case 1 % Ожидание SYNC_HIGH (0xA5)
            if b == 165 % 0xA5
                state = uint8(2);
            else
                state = uint8(0);
            end

        case 2 % Чтение MessageId
            msg_id = b;
            state  = uint8(3);

        case 3 % Чтение Length
            if b <= 64
                expected_len = b;
                rx_idx       = uint8(1);
                if expected_len > 0
                    state = uint8(4);
                else
                    state = uint8(5);
                end
            else
                state = uint8(0);
            end

        case 4 % Чтение Payload
            rx_buf(rx_idx) = b;
            rx_idx = rx_idx + uint8(1);
            if rx_idx > expected_len
                state = uint8(5);
            end

        case 5 % Чтение CRC16 Low
            crc_low = b;
            state   = uint8(6);

        case 6 % Чтение CRC16 High и проверка
            crc_high = b;
            crc_received = bitor(uint16(crc_low), bitshift(uint16(crc_high), 8));
            state = uint8(0);

            % Расчет контрольной суммы от ID, LEN и тела данных
            calc_data = [msg_id, expected_len, rx_buf(1:expected_len)];
            crc_calc = uint16(65535); % 0xFFFF

            for k = 1:length(calc_data)
                crc_calc = bitxor(crc_calc, bitshift(uint16(calc_data(k)), 8));
                for bit = 1:8
                    if bitand(crc_calc, uint16(32768)) ~= 0 % 0x8000
                        crc_calc = bitxor(bitshift(crc_calc, 1), uint16(4129)); % 0x1021
                    else
                        crc_calc = bitshift(crc_calc, 1);
                    end
                end
            end

            % Пакет валиден: прямое преобразование памяти (Zero-Copy Typecast)
            if (crc_calc == crc_received) && (msg_id == 1) && (expected_len == 32)
                timestamp_ms = typecast(rx_buf(1:4),   'uint32');
                raw_pitch    = typecast(rx_buf(5:8),   'single');
                raw_roll     = typecast(rx_buf(9:12),  'single');
                clean_pitch  = typecast(rx_buf(13:16), 'single');
                clean_roll   = typecast(rx_buf(17:20), 'single');
                gx           = typecast(rx_buf(21:24), 'single');
                gy           = typecast(rx_buf(25:28), 'single');
                gz           = typecast(rx_buf(29:32), 'single');
                valid        = true;
            end
    end
end