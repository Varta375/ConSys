#include "HardwareMonitor.h"
#include "lhwm-cpp-wrapper.h"
#include <exception>
#include <iostream>

void HardwareMonitor::Update() {
    // CPU temperature
    try {
        CpuTemperature = static_cast<int>(LHWM::GetSensorValue(CpuTempId));
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Update() for CPU Temp: " << e.what() << std::endl;
        CpuTemperature = -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in Update() for CPU Temp" << std::endl;
        CpuTemperature = -1;
    }

    // GPU temperature
    try {
        GpuTemperature = static_cast<int>(LHWM::GetSensorValue(GpuTempId));
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Update() for GPU Temp: " << e.what() << std::endl;
        GpuTemperature = -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in Update() for GPU Temp" << std::endl;
        GpuTemperature = -1;
    }

    // CPU load
    try {
        CpuLoad = static_cast<int>(LHWM::GetSensorValue(CpuTotalLoad));
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Update() for CPU Load: " << e.what() << std::endl;
        CpuLoad = -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in Update() for CPU Load" << std::endl;
        CpuLoad = -1;
    }

    // GPU load
    try {
        GpuLoad = static_cast<int>(LHWM::GetSensorValue(GpuCoreLoad));
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Update() for GPU Load: " << e.what() << std::endl;
        GpuLoad = -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in Update() for GPU Load" << std::endl;
        GpuLoad = -1;
    }
}

int HardwareMonitor::GetCpuTemperature() {
    try {
        return CpuTemperature;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in GetCpuTemperature(): " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in GetCpuTemperature()" << std::endl;
        return -1;
    }
}

int HardwareMonitor::GetGpuTemperature() {
    try {
        return GpuTemperature;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in GetGpuTemperature(): " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in GetGpuTemperature()" << std::endl;
        return -1;
    }
}

int HardwareMonitor::GetCpuLoad() {
    try {
        return CpuLoad;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in GetCpuLoad(): " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in GetCpuLoad()" << std::endl;
        return -1;
    }
}

int HardwareMonitor::GetGpuLoad() {
    try {
        return GpuLoad;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in GetGpuLoad(): " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown exception in GetGpuLoad()" << std::endl;
        return -1;
    }
}