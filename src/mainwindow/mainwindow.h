/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

#include "kernel/kernel.h"
#include "aboutwindow/aboutwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    Kernel *kernel;
    void openFile();
    void newFile();
    void saveFile();
    void saveFileAs();
    void closeEvent(QCloseEvent *event) override;
    void clearKernel();
    void about();
    QString fileName;
    const QString VERSION = "Zöglfrex 0.1.0";
    const QString COPYRIGHT = "Copyright (c) 2024 Arne Bersch (zoeglfrex-dmx@web.de)";
};
#endif // MAINWINDOW_H
