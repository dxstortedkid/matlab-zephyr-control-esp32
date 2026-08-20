============================================================
ESP32-C3 <-> MATLAB / SIMULINK
BINARY BIDIRECTIONAL COMMUNICATION
ARCHITECTURE
============================================================


1. ЦЕЛЬ
------------------------------------------------------------

На текущем этапе разрабатывается только система двусторонней
передачи данных между ESP32-C3 под управлением Zephyr RTOS
и MATLAB / Simulink.

Система должна обеспечивать:

    ESP32 -> MATLAB
        передача telemetry / данных

    MATLAB -> ESP32
        передача параметров / команд

Передача должна быть:

    - бинарной;
    - двусторонней;
    - надежной;
    - расширяемой;
    - с проверкой целостности;
    - без динамического выделения памяти;
    - пригодной для realtime-системы.


2. ОБЩАЯ СХЕМА
------------------------------------------------------------

                  HOST
             MATLAB / SIMULINK
                    |
                    |
             MATLAB Serial API
                    |
                    |
               UART / USB
                    |
                    |
                    v
              ESP32-C3
             Zephyr RTOS


Направление передачи:

    MATLAB
       |
       | Host -> Device
       v
    ESP32


Обратное направление:

    ESP32
       |
       | Device -> Host
       v
    MATLAB


Физически используется один полнодуплексный UART.

TX и RX работают независимо:

    ESP32 TX ---> MATLAB RX
    ESP32 RX <--- MATLAB TX


3. УРОВНИ АРХИТЕКТУРЫ ESP32
------------------------------------------------------------

ESP32 часть разделяется на четыре уровня:

    1. UART Driver
    2. RX/TX Transport
    3. Protocol
    4. Application


Схема:

    +-----------------------------+
    |        Application          |
    +-------------+---------------+
                  |
                  v
    +-----------------------------+
    |          Protocol           |
    | Frame / Message / CRC       |
    +-------------+---------------+
                  |
                  v
    +-----------------------------+
    |       Transport Layer       |
    |       RX / TX buffers       |
    +-------------+---------------+
                  |
                  v
    +-----------------------------+
    |       Zephyr UART API       |
    +-----------------------------+
                  |
                  v
                  UART


4. UART DRIVER LAYER
------------------------------------------------------------

Самый нижний уровень.

Он отвечает только за физическую передачу байтов.

Его задача:

    transmit bytes
    receive bytes


Этот слой не должен знать:

    что такое telemetry;
    что такое PID;
    что такое parameter;
    что такое MATLAB;
    что такое packet.


Для него существует только:

    uint8_t


Например:

    UART TX:

        0x5A
        0xA5
        0x01
        0x20
        ...


UART driver не занимается разбором этих данных.


5. TRANSPORT LAYER
------------------------------------------------------------

Следующий уровень отвечает за безопасную работу
с потоком байтов UART.

UART представляет собой поток:

    5A A5 01 20 FF 00 12 ...


UART не знает понятия "пакет".

Поэтому Transport Layer должен передать поток
в Protocol Layer.


На RX:

    UART
      |
      v
    RX buffer
      |
      v
    Protocol parser


На TX:

    Application
      |
      v
    Protocol serializer
      |
      v
    TX buffer
      |
      v
    UART


6. RX BUFFER
------------------------------------------------------------

RX buffer используется для накопления входящих байтов.

Буфер имеет фиксированный размер.

Например:

    uint8_t rx_buffer[256];


Динамическая память не используется.

UART может принять:

    1 байт
    5 байт
    20 байт
    100 байт


Parser должен уметь продолжить обработку
после получения следующей порции данных.


Например первая часть:

    5A A5 01


Вторая часть:

    20 00 00 80 3F ...


Parser должен сохранить состояние между вызовами.


7. PROTOCOL LAYER
------------------------------------------------------------

Protocol Layer отвечает за формирование и разбор кадров.

Основной формат:

    +--------+--------+--------+----------+--------+
    | SYNC   | MSG_ID | LENGTH | PAYLOAD  | CRC16  |
    | 2 byte | 1 byte | 1 byte | N bytes  | 2 byte |
    +--------+--------+--------+----------+--------+


Размер кадра:

    2 + 1 + 1 + N + 2

    = N + 6 bytes


8. SYNC
------------------------------------------------------------

SYNC используется для поиска начала кадра.

Значение:

    0x5A
    0xA5


Полный sync:

    5A A5


Parser постоянно ищет:

    5A A5


Если получен случайный мусор:

    11 42 91 00 5A 31 72 ...


Parser продолжает искать:

    5A A5


Это позволяет восстановить синхронизацию
после поврежденного кадра.


9. MSG_ID
------------------------------------------------------------

MSG_ID определяет тип сообщения.

Например:

    0x01 = TELEMETRY

    0x10 = SET_PARAMETER

    0x11 = GET_PARAMETER

    0x20 = ACK

    0x21 = ERROR


MSG_ID не определяет физический формат кадра.

Он определяет смысл PAYLOAD.


Например:

    MSG_ID = 0x10

означает:

    SET_PARAMETER


10. PAYLOAD_LENGTH
------------------------------------------------------------

LENGTH содержит количество байт PAYLOAD.

Например:

    LENGTH = 12


означает:

    следующие 12 байт являются PAYLOAD.


Максимальный размер:

    128 bytes


Таким образом parser заранее знает,
сколько байт необходимо получить.


11. PAYLOAD
------------------------------------------------------------

PAYLOAD содержит данные сообщения.

Например:

    SET_PARAMETER


может содержать:

    parameter_id
    value


Например:

    uint16_t parameter_id
    float value


В байтах:

    2 + 4 = 6 bytes


PAYLOAD не содержит:

    SYNC
    MSG_ID
    LENGTH
    CRC


Это исключительно данные сообщения.


12. CRC16
------------------------------------------------------------

Последние два байта кадра:

    CRC16_LOW
    CRC16_HIGH


Используется:

    CRC-16/CCITT-FALSE


Параметры:

    Polynomial:
        0x1021

    Initial value:
        0xFFFF


CRC рассчитывается над:

    MSG_ID
    LENGTH
    PAYLOAD


SYNC в CRC не входит.


Например:

    SYNC
      |
      | не участвует
      v

    5A A5

    MSG_ID
      |
    LENGTH
      |
    PAYLOAD
      |
      +----> CRC calculation


13. ПОЛНЫЙ ПРИМЕР КАДРА
------------------------------------------------------------

Допустим отправляется:

    MSG_ID = 0x10

    PAYLOAD:

        parameter_id = 5
        value = 2.5


Структура:

    SYNC:
        5A A5

    MSG_ID:
        10

    LENGTH:
        06

    PAYLOAD:
        05 00
        00 00 20 40

    CRC:
        XX XX


Итог:

    5A A5 10 06
    05 00 00 00 20 40
    XX XX


14. RX PARSER
------------------------------------------------------------

RX parser является конечным автоматом.


Состояния:

    WAIT_SYNC_1
    WAIT_SYNC_2
    READ_MSG_ID
    READ_LENGTH
    READ_PAYLOAD
    READ_CRC_LOW
    READ_CRC_HIGH
    VALIDATE


Логика:


    WAIT_SYNC_1

        получил 0x5A
             |
             v
        WAIT_SYNC_2


    WAIT_SYNC_2

        получил 0xA5
             |
             v
        READ_MSG_ID

        другой байт:
             |
             v
        WAIT_SYNC_1


    READ_MSG_ID
             |
             v
        READ_LENGTH
             |
             v
        READ_PAYLOAD
             |
             v
        READ_CRC


После получения полного кадра:

    CRC valid
         |
         v
    MESSAGE_READY


Если CRC invalid:

    FRAME_ERROR
         |
         v
    parser reset


15. MESSAGE OBJECT
------------------------------------------------------------

После успешного parsing кадра
необходимо получить абстрактное сообщение.

Например:

    struct Message
    {
        uint8_t id;
        uint8_t length;
        uint8_t payload[128];
    };


Это уже не UART.

Это объект протокола.


UART передал:

    bytes


Protocol parser преобразовал:

    bytes

в:

    Message


16. MESSAGE DISPATCHER
------------------------------------------------------------

После успешного parsing сообщение передается
в Message Dispatcher.


Например:

    Message
       |
       v
    msg.id
       |
       +---- 0x10 ---> SET_PARAMETER
       |
       +---- 0x11 ---> GET_PARAMETER
       |
       +---- 0x20 ---> ACK
       |
       +---- 0x21 ---> ERROR


Dispatcher не должен заниматься поиском SYNC
или вычислением CRC.

Это уже сделал Protocol Layer.


17. TX PATH
------------------------------------------------------------

Передача от ESP32 к MATLAB имеет обратную структуру.


Application:

    telemetry data


       |
       v

Protocol Serializer


       |
       v

Frame:

    SYNC
    MSG_ID
    LENGTH
    PAYLOAD
    CRC


       |
       v

TX buffer


       |
       v

UART


       |
       v

MATLAB


18. SERIALIZER
------------------------------------------------------------

Serializer получает:

    message ID
    payload


и создаёт:

    byte array


Например:

    Message:

        id = 0x01

        payload = telemetry


Serializer добавляет:

    SYNC
    MSG_ID
    LENGTH
    PAYLOAD
    CRC


На выходе:

    uint8_t frame[MAX_FRAME_SIZE];


19. ДВУСТОРОННИЙ ОБМЕН
------------------------------------------------------------

Протокол является симметричным.

Один и тот же формат кадра используется
в обоих направлениях.


MATLAB -> ESP32:

    SET_PARAMETER


ESP32 -> MATLAB:

    TELEMETRY


MATLAB -> ESP32:

    COMMAND


ESP32 -> MATLAB:

    ACK


То есть физически нет отдельного протокола
для TX и RX.


20. ТИПЫ СООБЩЕНИЙ
------------------------------------------------------------

Минимальная версия:


ESP32 -> MATLAB:

    0x01 TELEMETRY
    0x02 STATUS
    0x20 ACK
    0x21 ERROR


MATLAB -> ESP32:

    0x10 SET_PARAMETER
    0x11 GET_PARAMETER
    0x12 COMMAND


Список MSG_ID должен быть централизован.

Например:

    protocol/message_id.hpp


21. PARAMETER MESSAGE
------------------------------------------------------------

MATLAB должен иметь возможность отправлять
параметры на ESP32.


Например:

    SET_PARAMETER


Payload:

    parameter_id
    value


Формат:

    uint16_t parameter_id
    float value


Например:

    parameter_id = 1
    value = 0.25


ESP32 получает:

    ID = 1
    value = 0.25


После успешной обработки ESP32 отправляет:

    ACK


22. ACK
------------------------------------------------------------

ACK нужен для подтверждения приема команды.


Например:


MATLAB:

    SET_PARAMETER
    parameter_id = 1
    value = 0.25


ESP32:

    ACK
    message_id = SET_PARAMETER


Это позволяет MATLAB понять:

    команда была получена
    CRC корректен
    сообщение обработано


В будущем ACK можно дополнить:

    sequence number
    error code


23. ERROR
------------------------------------------------------------

Если ESP32 получил корректный кадр,
но не смог обработать сообщение,
отправляется ERROR.


Причины:

    UNKNOWN_MESSAGE
    INVALID_LENGTH
    INVALID_PARAMETER
    INVALID_VALUE
    INTERNAL_ERROR


Важно различать:


    CRC ERROR

и

    APPLICATION ERROR


CRC ERROR:

    кадр поврежден.


APPLICATION ERROR:

    кадр технически корректен,
    но его содержимое невозможно обработать.


24. SEQUENCE NUMBER
------------------------------------------------------------

На следующей версии протокола желательно добавить
sequence number.


Например:

    SEQ = uint8_t


Формат станет:


    SYNC
    MSG_ID
    SEQ
    LENGTH
    PAYLOAD
    CRC


Sequence number позволяет MATLAB понимать:

    какой пакет был отправлен;
    какой пакет подтвержден;
    потерялся ли пакет;
    в каком порядке пришли данные.


На первом этапе sequence number можно не реализовывать.


25. TIMESTAMP
------------------------------------------------------------

Для telemetry желательно иметь timestamp.


Например:

    uint32_t timestamp_ms


MATLAB сможет построить:

    time
      |
      v
    telemetry


Timestamp должен формироваться на ESP32,
а не на MATLAB.


Причина:

    MATLAB получает пакет с задержкой,
    поэтому время получения пакета
    не равно времени измерения.


26. MATLAB SIDE
------------------------------------------------------------

На стороне MATLAB необходимо реализовать
два основных компонента:


    TX:

        данные MATLAB
             |
             v
        pack_frame()
             |
             v
        uint8 array
             |
             v
        serialport.write()


    RX:

        serialport.read()
             |
             v
        byte stream
             |
             v
        parser
             |
             v
        message
             |
             v
        payload


MATLAB parser также должен быть
конечным автоматом.


27. MATLAB НЕ ДОЛЖЕН ПАРСИТЬ СТРОКИ
------------------------------------------------------------

Не используется:

    "KP=2.5;KI=0.1;KD=0.03\n"


Используется:

    binary frame


Преимущества:

    - меньше данных;
    - нет string parsing;
    - предсказуемое время обработки;
    - проще синхронизация;
    - CRC;
    - фиксированный формат;
    - удобнее для Simulink.


28. ПАМЯТЬ
------------------------------------------------------------

На ESP32 запрещается динамическое выделение памяти
в realtime communication path.


Используются:

    static buffers


Например:

    RX_BUFFER_SIZE
        256 bytes

    TX_BUFFER_SIZE
        256 bytes

    MAX_PAYLOAD_SIZE
        128 bytes

    MAX_FRAME_SIZE
        134 bytes


MAX_FRAME_SIZE:

    2 SYNC
    + 1 MSG_ID
    + 1 LENGTH
    + 128 PAYLOAD
    + 2 CRC

    = 134 bytes


29. ПОТОКИ
------------------------------------------------------------

На начальном этапе достаточно разделить
прием и обработку логически.


Вариант:


    UART RX
       |
       v
    RX buffer
       |
       v
    Parser
       |
       v
    Message


TX:

    Message
       |
       v
    Serializer
       |
       v
    TX buffer
       |
       v
    UART


Позже это можно разделить на Zephyr threads:


    Communication RX Thread

            |

            v

        RX Parser

            |

            v

       Message Queue

            |

            v

       Application


и:


    Application

        |

        v

    TX Message Queue

        |

        v

    Communication TX Thread

        |

        v

       UART


30. MESSAGE QUEUE
------------------------------------------------------------

Для связи между communication thread
и application thread используется Zephyr message queue.


Например:


    RX Thread
        |
        v
    k_msgq_put()
        |
        v
    Message Queue
        |
        v
    Application
        |
        v
    k_msgq_get()


Это предотвращает прямое изменение
application state из UART parser.


31. ПРИОРИТЕТЫ
------------------------------------------------------------

Communication thread не должен блокировать
другие части приложения.


UART может получать данные в любой момент.


Поэтому:

    UART RX
       |
       v
    parse
       |
       v
    queue


А приложение самостоятельно решает,
когда обработать сообщение.


32. ОШИБКИ СИНХРОНИЗАЦИИ
------------------------------------------------------------

Parser должен уметь восстанавливаться
после поврежденного кадра.


Например:

    5A A5 10 06 ...
                ^
                поврежден


CRC не совпал.


Parser:

    discard frame
        |
        v
    search for next
    5A A5


Таким образом один поврежденный кадр
не ломает дальнейшую коммуникацию.


33. НЕПРАВИЛЬНАЯ LENGTH
------------------------------------------------------------

Если:

    LENGTH > MAX_PAYLOAD_SIZE


кадр немедленно отбрасывается.


Например:

    LENGTH = 200


при:

    MAX_PAYLOAD_SIZE = 128


Результат:

    INVALID_LENGTH


Parser возвращается
в состояние поиска SYNC.


34. ОСНОВНЫЕ КОМПОНЕНТЫ ESP32
------------------------------------------------------------

На ESP32 создаются:


    protocol/
        protocol.hpp
        protocol.cpp

        frame.hpp

        crc16.hpp
        crc16.cpp

        message_id.hpp


    communication/
        uart.hpp
        uart.cpp


На первом этапе этого достаточно.


35. ОТВЕТСТВЕННОСТЬ КОМПОНЕНТОВ
------------------------------------------------------------


CRC16:

    принимает bytes
    возвращает CRC


Frame Parser:

    принимает bytes
    возвращает Message


Frame Serializer:

    принимает Message
    возвращает bytes


UART:

    передает bytes


Dispatcher:

    принимает Message
    определяет тип сообщения


Application:

    решает, что делать с Message


Ни один компонент не должен выполнять
чужую ответственность.


36. ПОЛНЫЙ RX PIPELINE
------------------------------------------------------------


MATLAB
   |
   | binary frame
   v
UART
   |
   v
RX buffer
   |
   v
Frame Parser
   |
   +---- invalid sync ---> discard
   |
   +---- invalid length -> discard
   |
   +---- invalid CRC ----> discard
   |
   v
Valid Message
   |
   v
Message Queue
   |
   v
Application
   |
   v
ACK / response


37. ПОЛНЫЙ TX PIPELINE
------------------------------------------------------------


Application
   |
   v
Message
   |
   v
Frame Serializer
   |
   v
CRC16
   |
   v
TX buffer
   |
   v
UART
   |
   v
MATLAB


38. ПЕРВАЯ ВЕРСИЯ ПРОЕКТА
------------------------------------------------------------

На первом этапе не реализуются:

    - MPU6050;
    - PID;
    - motor;
    - Simulink generated algorithm;
    - control system.


Реализуется только:


    ESP32-C3
        |
        +-- Zephyr
        |
        +-- UART
        |
        +-- Binary Protocol
        |
        +-- CRC16
        |
        +-- RX parser
        |
        +-- TX serializer
        |
        +-- Message Queue
        |
        +-- MATLAB communication


39. ПЕРВЫЙ ТЕСТ
------------------------------------------------------------

Самый первый тест должен быть максимально простым.


MATLAB:

    отправить:

        SET_PARAMETER

    parameter_id = 1
    value = 123.456


ESP32:

    получить frame
        |
        v
    проверить CRC
        |
        v
    распарсить
        |
        v
    вывести через printk:

        ID = 1
        VALUE = 123.456


После этого:

    ESP32 -> MATLAB

    ACK


Если это работает,
значит базовый двусторонний транспорт работает.


40. ВТОРОЙ ТЕСТ
------------------------------------------------------------

ESP32 периодически отправляет:

    TELEMETRY


Например:

    timestamp
    value1
    value2
    value3


MATLAB:

    принимает пакеты
    |
    v
    распаковывает
    |
    v
    строит график


Таким образом проверяется
обратное направление.


41. КОНЕЧНАЯ ЦЕЛЬ ЭТОГО СЛОЯ
------------------------------------------------------------

После завершения communication layer
должна существовать полностью независимая
система:


    MATLAB / Simulink
           ^
           |
        Binary
        Protocol
           |
           v
        ESP32


Обе стороны знают только:

    Frame Format
    MSG_ID
    Payload Format
    CRC


Всё остальное является application logic
и может развиваться независимо.


42. ГЛАВНЫЙ ПРИНЦИП
------------------------------------------------------------

Communication Layer не знает,
что передается.


Он знает только:


    BYTES
      |
      v
    FRAME
      |
      v
    MESSAGE


Например сегодня:

    PARAMETER


завтра:

    PID


послезавтра:

    TELEMETRY


Communication Layer при этом
не меняется.

Меняется только:

    MSG_ID
    PAYLOAD definition
    Application handler


============================================================
END OF COMMUNICATION ARCHITECTURE
============================================================