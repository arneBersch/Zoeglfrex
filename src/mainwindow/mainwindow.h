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
#include <QtSql>

#include "aboutwindow/aboutwindow.h"
#include "cuelistview/cuelistview.h"
#include "terminal/terminal.h"
#include "inspector/inspector.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QString version, QString copyright, QWidget *parent = nullptr);
public slots:
    void reload();
private:
    QString VERSION;
    QString COPYRIGHT;
    void about();
    void closeEvent(QCloseEvent *event) override;
    CuelistView* cuelistView;
    Terminal* terminal;
    Inspector* inspector;
};

#endif // MAINWINDOW_H
