#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "FanController.h"
#include "ProcessManager.h"
#include "HardwareMonitor.h"
#include "PowerManager.h"
#include "lhwm-cpp-wrapper.h"

// PowerManager tests
TEST_CASE("PowerManager - SetEcoMode positive") 
{
	PowerManager pmTest;
	CHECK(pmTest.SetEcoMode() == true);

}

TEST_CASE("PowerManager - SetEcoMode fake GUID (negative)")
{
	static constexpr GUID ECO_GUID_FAKE = { 0xb1842138 , 0x3451, 0x4aab, {0xdc, 0x21, 0x37, 0x15, 0x56, 0x43, 0x2d, 0x4a} }; // Random symbols
	DWORD result = PowerSetActiveScheme(nullptr, &ECO_GUID_FAKE);
	CHECK(result != ERROR_SUCCESS);
}

TEST_CASE("PowerManager - SetBalanceMode positive")
{
	PowerManager pmTest;
	CHECK(pmTest.SetBalanceMode() == true);
	Sleep(1000);
	pmTest.SetEcoMode();

}

TEST_CASE("PowerManager - SetBalanceMode fake GUID (negative)")
{
	static constexpr GUID BALANCE_GUID_FAKE = { 0xb1842138 , 0x3451, 0x4aab, {0xdc, 0x21, 0x37, 0x15, 0x56, 0x43, 0x2d, 0x4a} }; // Random symbols
	DWORD result = PowerSetActiveScheme(nullptr, &BALANCE_GUID_FAKE);
	CHECK(result != ERROR_SUCCESS);
}

TEST_CASE("PowerManager - SetPerformanceMode positive")
{
	PowerManager pmTest;
	CHECK(pmTest.SetPerformanceMode() == true);
	Sleep(1000);
	pmTest.SetEcoMode();

}

TEST_CASE("PowerManager - SetPerformanceMode fake GUID (negative)")
{
	static constexpr GUID PERFORMANCE_GUID_FAKE = { 0xb1842138 , 0x3451, 0x4aab, {0xdc, 0x21, 0x37, 0x15, 0x56, 0x43, 0x2d, 0x4a} };
	DWORD result = PowerSetActiveScheme(nullptr, &PERFORMANCE_GUID_FAKE);
	CHECK(result != ERROR_SUCCESS);
}

// HardwareMonitor tests
TEST_CASE("HardwareMonitor - Update (positive)")
{
	HardwareMonitor hwtest;
	hwtest.Update();

	int cpuTemp = hwtest.GetCpuTemperature();
	int gpuTemp = hwtest.GetGpuTemperature();

	// Temperatures are in valid range
	CHECK(cpuTemp >= 0);
	CHECK(cpuTemp <= 120);
	CHECK(gpuTemp >= 0);
	CHECK(gpuTemp <= 120);
}

TEST_CASE("HardwareMonitor - All sensors show 0 before Update() (positive)")
{
	HardwareMonitor hwmTest;

	CHECK(hwmTest.GetCpuTemperature() == 0);
	CHECK(hwmTest.GetGpuTemperature() == 0);
	CHECK(hwmTest.GetCpuLoad() == 0);
	CHECK(hwmTest.GetGpuLoad() == 0);
}

TEST_CASE("HardwareMonitor - Reading fake IDs (negative)")
{
	HardwareMonitor hwTest;
	const std::string CpuTempIdFake = "/FAKEcpu/0/FAKEtemperature/5";
	const std::string GpuTempIdFake = "/FAKEgpu-nvidia/0/FAKEtemperature/32";
	const std::string CpuTotalLoadFake = "/FAKEamdcpu/0/FAKEload/14";
	const std::string GpuCoreLoadFake = "/FAKEgpu-nvidia/0/FAKEload/123";

	int CpuTemperature = static_cast<int>(LHWM::GetSensorValue(CpuTempIdFake));
	int GpuTemperature = static_cast<int>(LHWM::GetSensorValue(GpuTempIdFake));

	int CpuLoad = static_cast<int>(LHWM::GetSensorValue(CpuTotalLoadFake));
	int GpuLoad = static_cast<int>(LHWM::GetSensorValue(GpuCoreLoadFake));

	// Fake IDs => 0 information about temperature & load sensors
	CHECK(CpuTemperature == 0);
	CHECK(GpuTemperature == 0);
	CHECK(CpuLoad == 0);
	CHECK(GpuLoad == 0);
}

// FanController tests
TEST_CASE("FanController - SetCpuFanSpeed & GetCpuFanRPM positive")
{
	FanController fcTest;
	fcTest.SetCpuFanSpeed(30);
	int rpm = fcTest.GetCpuFanRPM();
	CHECK(rpm >= 0);
	CHECK(rpm <= 6000);

}

TEST_CASE("FanController - SetCpuFanSpeed invalid percentage (negative)")
{
	FanController fcTest;
	fcTest.SetCpuFanSpeed(120); // 120% > 100%
	int rpm = fcTest.GetCpuFanRPM();
	CHECK(rpm >= 0);
	CHECK(rpm <= 6000);
}

TEST_CASE("FanController - SetGpuFanSpeed & GetGpuFanRPM positive")
{
	FanController fcTest;
	fcTest.SetGpuFanSpeed(30);
	int rpm = fcTest.GetGpuFanRPM();
	CHECK(rpm >= 0);
	CHECK(rpm <= 6000);

}

TEST_CASE("FanController - SetGpuFanSpeed invalid percentage (negative)")
{
	FanController fcTest;
	fcTest.SetGpuFanSpeed(120);
	int rpm = fcTest.GetGpuFanRPM();
	CHECK(rpm >= 0);
	CHECK(rpm <= 6000);
}

TEST_CASE("FanController - ApplyChangesViaPipe check result (positive)")
{
	FanController fcTest;

	fcTest.ApplyChangesViaPipe(FanType::CPU, 30);
	int rpm = fcTest.GetCpuFanRPM();

	CHECK(rpm >= 0);
	CHECK(rpm <= 6000);
	
}
// ProcessManager tests
TEST_CASE("ProcessManager - GetList is not empty (positive)")
{
	ProcessManager pmTest;
	const auto result{ pmTest.GetList(true) };
	CHECK(result.size() > 0);
}

TEST_CASE("ProcessManager - GetList cannot return processes list without Refresh() (negative)")
{
	ProcessManager pmTest;
	const auto result{ pmTest.GetList(false) };
	CHECK(result.size() == 0);
}

TEST_CASE("ProcessManager - StopProcess for valid PID (positive)")
{
	ProcessManager pmTest;
	STARTUPINFOA StartUpInfo;
	PROCESS_INFORMATION pi;
	
	char cmd[] = "dummy_test.exe";

	BOOL success = CreateProcessA(nullptr, cmd, nullptr, nullptr, false, 0, nullptr, nullptr,&StartUpInfo,&pi);

	Sleep(1000);

	DWORD TestPID = pi.dwProcessId;

	pmTest.GetList(true);
	CHECK(pmTest.StopProcess(TestPID) == true);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	Sleep(500);
}

TEST_CASE("ProcessManager - StopProcess for invalid PID (negative)")
{
	ProcessManager pmTest;
	pmTest.Refresh();

	bool result = pmTest.StopProcess(12345678);
	CHECK(result == false);
}