cmake_minimum_required(VERSION 3.20)
project(ConSys)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)

set(DOCTEST_WITH_TESTS OFF CACHE INTERNAL "")
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.5.2
)

FetchContent_Declare(winreg
    GIT_REPOSITORY https://github.com/GiovanniDicanio/WinReg.git
    GIT_TAG v6.4.0
)
FetchContent_MakeAvailable(doctest winreg)

add_library(winreg_iface INTERFACE)
target_include_directories(winreg_iface INTERFACE "${winreg_SOURCE_DIR}/WinReg")

add_executable(ConSys
    src/HardwareMonitor.cpp
    src/FanController.cpp
    src/PowerManager.cpp
    src/ProcessManager.cpp
)

target_include_directories(ConSys PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

target_link_libraries(ConSys PRIVATE
    winreg_iface        
    ${CMAKE_CURRENT_SOURCE_DIR}/external/lhwm-cpp-wrapper.lib
    mscoree.lib
    PowrProf.lib
)

set_target_properties(ConSys PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator'\""
)

file(GLOB LHM_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/external/*.dll")
if(LHM_DLLS)
    add_custom_command(TARGET ConSys POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LHM_DLLS} $<TARGET_FILE_DIR:ConSys>
    )
endif()

add_custom_command(TARGET ConSys POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/external/ryzenadj.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/WinRing0x64.sys
        ${CMAKE_CURRENT_SOURCE_DIR}/external/WinRing0x64.dll
        $<TARGET_FILE_DIR:ConSys>
)

add_executable(ConSys_tests
    tests/ConSys_tests.cpp
    src/HardwareMonitor.cpp
    src/FanController.cpp
    src/PowerManager.cpp
    src/ProcessManager.cpp
)

target_include_directories(ConSys_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

target_link_libraries(ConSys_tests PRIVATE
    doctest::doctest      
    winreg_iface
    ${CMAKE_CURRENT_SOURCE_DIR}/external/lhwm-cpp-wrapper.lib
    mscoree.lib
    PowrProf.lib
    advapi32.lib
)

set_target_properties(ConSys_tests PROPERTIES
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator'\""
)

add_custom_command(TARGET ConSys_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/external/lhwm-cpp-wrapper.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/external/LibreHardwareMonitorLib.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/external/dummy_test.exe
        $<TARGET_FILE_DIR:ConSys_tests>
)

enable_testing()
add_test(NAME ConSysTests COMMAND ConSys_tests)