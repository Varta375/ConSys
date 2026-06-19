#include "PowerManager.h"
#include <exception>
#include <iostream>
#include <powersetting.h>

bool PowerManager::SetEcoMode() {
    try {
        DWORD result = PowerSetActiveScheme(nullptr, &ECO_GUID);
        return (result == ERROR_SUCCESS);
    } catch (const std::exception& e) {
        std::cerr << "Exception in SetEcoMode(): " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in SetEcoMode()" << std::endl;
        return false;
    }
}

bool PowerManager::SetBalanceMode() {
    try {
        DWORD result = PowerSetActiveScheme(nullptr, &BALANCED_GUID);
        return (result == ERROR_SUCCESS);
    } catch (const std::exception& e) {
        std::cerr << "Exception in SetBalanceMode(): " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in SetBalanceMode()" << std::endl;
        return false;
    }
}

bool PowerManager::SetPerformanceMode() {
    try {
        DWORD result = PowerSetActiveScheme(nullptr, &PERFORMANCE_GUID);
        return (result == ERROR_SUCCESS);
    } catch (const std::exception& e) {
        std::cerr << "Exception in SetPerformanceMode(): " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in SetPerformanceMode()" << std::endl;
        return false;
    }
}