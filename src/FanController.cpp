#include "FanController.h"
#include <windows.h>
#include <winreg.hpp>
#include <exception>
#include <iostream>
#include "HardwareMonitor.h"

bool FanController::SetCpuFanSpeed(DWORD percent) {
    try {
        winreg::RegKey key{ HKEY_LOCAL_MACHINE, regPath, KEY_WRITE };
        key.SetDwordValue(L"CPUFanPercentage", percent);
        return ApplyChangesViaPipe(FanType::CPU, percent);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in SetCpuFanSpeed(): " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown exception in SetCpuFanSpeed()" << std::endl;
        return false;
    }
}

bool FanController::SetGpuFanSpeed(DWORD percent) {
    try {
        winreg::RegKey key{ HKEY_LOCAL_MACHINE, regPath, KEY_WRITE };
        key.SetDwordValue(L"GPU1FanPercentage", percent);
        return ApplyChangesViaPipe(FanType::GPU, percent);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in SetGpuFanSpeed(): " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown exception in SetGpuFanSpeed()" << std::endl;
        return false;
    }
}

int FanController::GetCpuFanRPM() {
    try {
        auto v = GetSystemHealthValue(2); // CPU fan speed
        return v.value_or(-1);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in GetCpuFanRPM(): " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in GetCpuFanRPM()" << std::endl;
        return -1;
    }
}

int FanController::GetGpuFanRPM() {
    try {
        auto v = GetSystemHealthValue(6); // GPU fan speed
        return v.value_or(-1);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in GetGpuFanRPM(): " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in GetGpuFanRPM()" << std::endl;
        return -1;
    }
}

bool FanController::SetFanMode(int mode) {
    try {
        if (mode < 0 || mode > 2) return false;

        int CpuPercentage{ 0 };
        int GpuPercentage{ 0 };

        if (mode == 1) 
        { // Custom

            CpuPercentage = GetCpuFanRPM();
            GpuPercentage = GetGpuFanRPM();
        }
        else { // Max
            CpuPercentage = 100;
            GpuPercentage = 100;
        }

        ApplyChangesViaPipe(FanType::CPU, static_cast<DWORD>(CpuPercentage));
        ApplyChangesViaPipe(FanType::GPU, static_cast<DWORD>(GpuPercentage));
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in SetFanMode(): " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown exception in SetFanMode()" << std::endl;
        return false;
    }
}

bool FanController::ApplyChangesViaPipe(FanType fanType, DWORD percent) {
    HANDLE pipeHandle = INVALID_HANDLE_VALUE;
    try {
        // Формируем побитовый полезный груз (payload):
        // Тип вентилятора записывается в младший байт (0-7 биты),
        // процент мощности сдвигается на 8 бит влево (занимает 8-15 биты).
        // Явное приведение static_cast<uint64_t> предотвращает возможное переполнение при сдвиге.
        uint64_t fanControlPayload = (static_cast<uint64_t>(percent) << 8) | static_cast<uint64_t>(fanType);

        FanCommandPacket commandPacket;
        commandPacket.payload = fanControlPayload;

        pipeHandle = CreateFileA(
            R"(\\.\pipe\PredatorSense_service_namedpipe)",
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr
        );

        if (pipeHandle == INVALID_HANDLE_VALUE) {
            return false;
        }

        DWORD bytesWritten = 0;
        WriteFile(pipeHandle, &commandPacket, sizeof(commandPacket), &bytesWritten, nullptr);

        FlushFileBuffers(pipeHandle);

        uint8_t rawResponse[9] = {};
        DWORD bytesRead = 0;
        ReadFile(pipeHandle, rawResponse, sizeof(rawResponse), &bytesRead, nullptr);

        CloseHandle(pipeHandle);
        pipeHandle = INVALID_HANDLE_VALUE;

        return bytesWritten == sizeof(commandPacket);
    }
    catch (const std::exception& e) {
        if (pipeHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(pipeHandle);
        }
        std::cerr << "Exception in ApplyChangesViaPipe(): " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        if (pipeHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(pipeHandle);
        }
        std::cerr << "Unknown exception in ApplyChangesViaPipe()" << std::endl;
        return false;
    }
}

// Source: https://github.com/KRWCLASSIC/NitroSensual/blob/main/nitrosensual.py#L145
// Indexes: 1=CPU_Temp, 2=CPU_Fan_RPM, 6=GPU_Fan_RPM, 10=GPU_Temp
std::optional<int> FanController::GetSystemHealthValue(int index) {
    HANDLE pipeHandle = INVALID_HANDLE_VALUE;
    try {
        pipeHandle = CreateFileA(
            R"(\\.\pipe\PredatorSense_service_namedpipe)",
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr
        );

        if (pipeHandle == INVALID_HANDLE_VALUE) {
            return std::nullopt;
        }

        // Формируем код датчика для запроса:
        // Записываем единицу (флаг чтения) в младший байт,
        // а индекс датчика сдвигаем на 8 бит влево (в диапазон 8-15 бит).
        uint32_t sensorQueryPayload = 1 | (static_cast<uint32_t>(index) << 8);

        SensorQueryPacket queryPacket;
        queryPacket.payload = sensorQueryPayload;

        DWORD bytesWritten = 0;
        WriteFile(pipeHandle, &queryPacket, sizeof(queryPacket), &bytesWritten, nullptr);
        FlushFileBuffers(pipeHandle);

        SensorResponsePacket responsePacket;
        DWORD bytesRead = 0;
        ReadFile(pipeHandle, &responsePacket, sizeof(responsePacket), &bytesRead, nullptr);

        CloseHandle(pipeHandle);
        pipeHandle = INVALID_HANDLE_VALUE;

        if (bytesRead < sizeof(responsePacket)) {
            return std::nullopt;
        }

        uint64_t rawSensorData = responsePacket.payload;

        if ((rawSensorData & 0xFF) == 0) // Младший байт (rawSensorData & 0xFF) отвечает за статус успешности.
        { 
            // Сдвигаем данные на 8 бит вправо, чтобы убрать байт статуса,
            // и накладываем маску 0xFFFF для выделения 16-битного значения датчика (например, RPM или темп.).
            return static_cast<int>((rawSensorData >> 8) & 0xFFFF);
        }

        return std::nullopt;
    }
    catch (const std::exception& e) {
        if (pipeHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(pipeHandle);
        }
        std::cerr << "Exception in GetSystemHealthValue(): " << e.what() << std::endl;
        return std::nullopt;
    }
    catch (...) {
        if (pipeHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(pipeHandle);
        }
        std::cerr << "Unknown exception in GetSystemHealthValue()" << std::endl;
        return std::nullopt;
    }
}
// Source: https://github.com/KRWCLASSIC/NitroSensual/blob/main/nitrosensual.py#L145