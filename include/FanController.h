#ifndef FanController_H
#define FanController_H

#include <windows.h>
#include <cstdint>
#include <optional>
#include <string>

/**
 * @enum FanType
 * @brief Тип вентилятора для отправки команд через именованный канал
 */
enum class FanType {
    CPU = 1, ///< Вентилятор центрального процессора
    GPU = 4  ///< Вентилятор дискретной видеокарты
};

#pragma pack(push, 1)

/**
 * @struct FanCommandPacket
 * @brief Структура пакета команды для управления вентилятором (размер 15 байт)
 */
struct FanCommandPacket {
    uint16_t commandCode = 16; ///< Код команды службы (всегда 16)
    uint8_t argumentCount = 1; ///< Количество передаваемых аргументов (всегда 1)
    uint32_t argumentLength = 8; ///< Размер полезной нагрузки аргумента в байтах (всегда 8 байт для uint64_t)
    uint64_t payload; ///< Данные (упакованные вместе процент скорости и тип вентилятора)
};

/**
 * @struct SensorQueryPacket
 * @brief Структура пакета запроса данных датчиков системы (размер 11 байт)
 */
struct SensorQueryPacket {
    uint16_t commandCode = 13; ///< Код команды службы (всегда 13)
    uint8_t argumentCount = 1; ///< Количество аргументов (всегда 1)
    uint32_t argumentLength = 4; ///< Размер аргумента в байтах (всегда 4 байта для uint32_t)
    uint32_t payload; ///< Данные запроса (код датчика)
};

/**
 * @struct SensorResponsePacket
 * @brief Структура пакета ответа от службы датчиков (размер 13 байт)
 */
struct SensorResponsePacket {
    uint8_t header[5]; ///< Служебный заголовок ответа от службы (5 байт)
    uint64_t payload; ///< Полезная нагрузка с показаниями датчика (8 байт)
};

#pragma pack(pop)

/**
 * @class FanController
 * @brief Класс для управления вентиляторами и их мониторинга для ноутбуков Acer Nitro 5 AN515-45 (Ryzen 5 5600H RTX 3060)
 * @details Управляет оборотами через запись в реестр HKLM\SOFTWARE\OEM\NitroSense\FanControl и
 *          отправку структурированных команд службе PSsvc через именованный канал
 *          Идея реализации базируется на решениях проекте NitroSensual от KRWCLASSIC
 * @see https://github.com/KRWCLASSIC/NitroSensual
 */
class FanController {
public:
    /**
     * @brief Установка скорости оборотов вентилятора процессора (CPU)
     * @param percentage Скорость вращения в процентах (0-100)%
     * @details Записывает значение в реестр под именем "CPUFanPercentage" и мгновенно применяет
     *          его, отправляя команду через именованный канал
     * @return true, если значение успешно записано и передано службе; иначе false
     * @warning Настройка применится аппаратно только в том случае, если в NitroSense выбран режим работы "Custom"
     * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
     */
    bool SetCpuFanSpeed(DWORD percentage);

    /**
     * @brief Установка скорости оборотов вентилятора дискретной видеокарты (GPU)
     * @param percentage Скорость вращения в процентах (0-100)%
     * @details Записывает значение в реестр под именем "GPU1FanPercentage" и мгновенно применяет
     *          его, отправляя команду через именованный канал
     * @return true, если значение успешно записано и передано службе; иначе false
     * @warning Настройка применится аппаратно только в том случае, если в NitroSense выбран режим работы "Custom"
     * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
     */
    bool SetGpuFanSpeed(DWORD percentage);

    /**
     * @brief Установка режима работы вентиляторов ноутбука (Custom, Max)
     * @param mode Режим работы: 1 - Custom, 2 - Max
     * @details Изменяет значение "CurrentFanMode" в реестре и отправляет пакеты инициализации
     *          выбранного режима обоим вентиляторам через именованный канал
     * @return true, если режим успешно изменен; false при передаче недопустимого значения режима
     * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
     */
    bool SetFanMode(int mode);

    /**
     * @brief Получение текущей реальной скорости вращения вентилятора процессора
     * @details Считывает актуальные показания тахометра CPU (индекс датчика 2) через именованный канал службы
     * @return Текущая скорость в оборотах в минуту (RPM), либо -1 при ошибке связи со службой
     * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
     */
    int GetCpuFanRPM();

    /**
     * @brief Получение текущей реальной скорости вращения вентилятора дискретной видеокарты
     * @details Считывает актуальные показания тахометра GPU (индекс датчика 6) через именованный канал службы
     * @return Текущая скорость в оборотах в минуту (RPM), либо -1 при ошибке связи со службой
     * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
     */
    int GetGpuFanRPM();

    /**
     * @brief Отправка команды на применение скорости вентилятора через именованный канал
     * @param fanType Тип целевого вентилятора (CPU или GPU)
     * @param percent Скорость вращения в процентах (0-100)%
     * @details Упаковывает параметры в 64-битную нагрузку, формирует структуру FanCommandPacket (15 байт)
     *          и отправляет её в канал \\.\pipe\PredatorSense_service_namedpipe. Считывает 9-байтовый ответ
     * @return true, если пакет был успешно записан в канал.
     * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
     */
    bool ApplyChangesViaPipe(FanType fanType, DWORD percent);

private:
    const std::wstring regPath = L"SOFTWARE\\OEM\\NitroSense\\FanControl"; ///< Путь к ветке реестра настроек вентиляторов HKLM

    /**
     * @brief Внутренний метод запроса показателей датчиков системы через канал службы
     * @param index Индекс системного датчика (1=CPU_Temp, 2=CPU_Fan_RPM, 6=GPU_Fan_RPM, 10=GPU_Temp, используются только 2 и 6 из-за наличия LibreHardwareMonitor)
     * @details Отправляет пакет SensorQueryPacket (11 байт) в именованный канал службы и считывает
     *          структурированный ответ SensorResponsePacket (13 байт). Декодирует полезную нагрузку датчика
     * @return Декодированное целочисленное значение показателя датчика либо std::nullopt в случае ошибки
     * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
     */
    std::optional<int> GetSystemHealthValue(int index);
};

#endif