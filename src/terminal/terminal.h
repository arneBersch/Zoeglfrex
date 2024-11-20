/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TERMINAL_H
#define TERMINAL_H

#include <QtWidgets>

class Kernel;

class Terminal : public QWidget
{
    Q_OBJECT
public:
    Terminal(Kernel *core, QWidget *parent = nullptr);
    void write(int key);
    void backspace();
    void clear();
    void execute(bool clear=true);
    void execute(QString command, QString action);
    void info(QString message);
    void success(QString message);
    void warning(QString message);
    void error(QString message);
private:
    Kernel *kernel;
    QString promptText(QList<int> keys);
    QList<int> command;
    QVBoxLayout *grid;
    QVBoxLayout *messages;
    QScrollArea *scrollArea;
    QLabel *prompt;
private slots:
    void scrollToLastMessage(int min, int max);
};

#include "kernel/kernel.h"

#endif // TERMINAL_H
