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
#include <QtSql>

#include "prompt.h"

class Terminal : public QWidget {
    Q_OBJECT
public:
    Terminal(QWidget *parent = nullptr);
    void info(QString message);
    void success(QString message);
    void warning(QString message);
    void error(QString message);
signals:
    void dbChanged();
    void promptTableChanged(QString table);
private:
    void executePrompt();
    QPlainTextEdit *messages;
    Prompt* prompt;
};

#endif // TERMINAL_H
