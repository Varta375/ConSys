#pragma once

#include "FanController.h"
#include "HardwareMonitor.h"
#include "PowerManager.h"
#include "ProcessManager.h"
#include "ui_consysgui.h"
#include <QMainWindow>
#include <QTimer>

class ConSysGUI : public QMainWindow
{
    Q_OBJECT

public:
    ConSysGUI(QWidget* parent = nullptr);
    ~ConSysGUI();

private slots:
    void EcoModeButtonClicked();
    void BalanceModeButtonClicked();
    void PerformanceModeButtonClicked();

    void TemperatureLimitButtonClicked();

    void CpuFansSpeedCustomChange();
    void GpuFansSpeedCustomChange();

    void SetMaxFansMode();
    void SetCustomFansMode();

    void StopProcessButtonClicked();

    void UpdateMonitoring();

private:
    Ui::ConSysGUIClass ui;

    HardwareMonitor* hwMonitor;
    FanController* FanControl;
    PowerManager* PowerMode;
    ProcessManager* ProcessManagerTable;

    QTimer* UpdateTimer;

    void RefreshProcessTable();
};