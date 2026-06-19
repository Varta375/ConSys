#include "stdafx.h"
#include "consysgui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    QApplication app(argc, argv);
    ConSysGUI window;
    window.show();
    int result = app.exec();
    CoUninitialize();
    return result;
}
