#include "stdafx.h"
#include "consysgui.h"
#include <QMainWindow>
#include <QTimer>

ConSysGUI::ConSysGUI(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    hwMonitor = new HardwareMonitor();
    PowerMode = new PowerManager();
    FanControl = new FanController();
    ProcessManagerTable = new ProcessManager();

    ProcessManagerTable->Refresh();
    RefreshProcessTable();

    connect(ui.ApplyEcoMode, &QPushButton::clicked, this, [this]() { EcoModeButtonClicked(); });
    connect(ui.ApplyBalanceMode, &QPushButton::clicked, this, [this]() { BalanceModeButtonClicked(); });
    connect(ui.ApplyPerformanceMode, &QPushButton::clicked, this, [this]() { PerformanceModeButtonClicked(); });

    // Temperature limit
    connect(ui.ApplyTemperatureLimit, &QPushButton::clicked, this, [this]() { TemperatureLimitButtonClicked(); });

    // Fans sliders
    connect(ui.HorizontalCpuFansSlider, &QSlider::sliderReleased, this, [this]() { CpuFansSpeedCustomChange(); });
    connect(ui.HorizontalGpuFansSlider, &QSlider::sliderReleased, this, [this]() { GpuFansSpeedCustomChange(); });

    // Fans modes
    connect(ui.ApplyMaxFansMode, &QPushButton::clicked, this, [this]() { SetMaxFansMode(); });
    connect(ui.ApplyCustomFansMode, &QPushButton::clicked, this, [this]() { SetCustomFansMode(); });

    // Processes 
    connect(ui.StopProcessButton, &QPushButton::clicked, this, [this]() { StopProcessButtonClicked(); });
    connect(ui.RefreshPushButton, &QPushButton::clicked, this, [this]() {RefreshProcessTable();});

    // Таймер обновления
    UpdateTimer = new QTimer(this);
    connect(UpdateTimer, &QTimer::timeout, this, [this]() { UpdateMonitoring(); });
    UpdateTimer->start(2000);
}

ConSysGUI::~ConSysGUI()
{
    delete hwMonitor;
    delete PowerMode;
    delete FanControl;
    delete ProcessManagerTable;
}

// Power modes
void ConSysGUI::EcoModeButtonClicked()
{
    PowerMode->SetEcoMode();
}

void ConSysGUI::BalanceModeButtonClicked()
{
    PowerMode->SetBalanceMode();
}

void ConSysGUI::PerformanceModeButtonClicked()
{
    PowerMode->SetPerformanceMode();
}

// Temperature limit control
void ConSysGUI::TemperatureLimitButtonClicked()
{
    const int degree = ui.TemperatureLimitSpinBox->value();
    if ((degree >= 55) && (degree <= 95))
    {
        std::string cmd{ "ryzenadj.exe --tctl-temp=" + std::to_string(degree) };
        system(cmd.c_str());
    }
}

// Process table 
void ConSysGUI::RefreshProcessTable()
{
    ProcessManagerTable->Refresh();
    auto Processes = ProcessManagerTable->GetList();

    // Setting columns (one time)
    if (ui.ProcessTable->columnCount() == 0) {
        ui.ProcessTable->setColumnCount(2);
        ui.ProcessTable->setHorizontalHeaderLabels({ "PID", "Process name" });

        ui.ProcessTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
        ui.ProcessTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

        // Fixating 
        ui.ProcessTable->setColumnWidth(0, 80);
    }

    ui.ProcessTable->setRowCount(Processes.size());

    for (size_t i = 0; i < Processes.size(); ++i) {
        ui.ProcessTable->setItem(i, 0, new QTableWidgetItem(QString::number(Processes[i].pid)));
        ui.ProcessTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(Processes[i].name)));
    }
}

void ConSysGUI::StopProcessButtonClicked()
{
    int row{ ui.ProcessTable->currentRow() };
    if (row >= 0) {
        DWORD pid = ui.ProcessTable->item(row, 0)->text().toUInt();
        ProcessManagerTable->StopProcess(pid);
        RefreshProcessTable();
    }
}
// Information Update
void ConSysGUI::UpdateMonitoring() {
    hwMonitor->Update();

    ui.LabelCpuTemperature->setText(QString("%1°C").arg(hwMonitor->GetCpuTemperature()));
    ui.LabelGpuTemperature->setText(QString("%1°C").arg(hwMonitor->GetGpuTemperature()));
    ui.LabelCpuUsage->setText(QString("%1").arg(hwMonitor->GetCpuLoad()));
    ui.LabelGpuUsage->setText(QString("%1").arg(hwMonitor->GetGpuLoad()));
    ui.LabelCpuFansRPM->setText(QString("%1").arg(FanControl->GetCpuFanRPM()));
    ui.LabelGpuFansRPM->setText(QString("%1").arg(FanControl->GetGpuFanRPM()));
 }

// Fans modes

void ConSysGUI::SetCustomFansMode()
{
    FanControl->SetFanMode(1);
}

void ConSysGUI::SetMaxFansMode()
{
    FanControl->SetFanMode(2);
}

// Fans sliders
void ConSysGUI::CpuFansSpeedCustomChange()
{
    int RPM_percentage{ ui.HorizontalCpuFansSlider->value() };
    FanControl->SetCpuFanSpeed(RPM_percentage);
}

void ConSysGUI::GpuFansSpeedCustomChange()
{
    int RPM_percentage = { ui.HorizontalGpuFansSlider->value() };
    FanControl->SetGpuFanSpeed(RPM_percentage);
}