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
)

target_include_directories(ConSys PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

target_link_libraries(ConSys PRIVATE       
    ${CMAKE_CURRENT_SOURCE_DIR}/external/lhwm-cpp-wrapper.lib
    mscoree.lib
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