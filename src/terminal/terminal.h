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
    void promptItemChanged(QString table, int id);
private slots:
    void execute(Prompt::Key selectionType, QList<int> ids, int attribute, QList<Prompt::Key> value);
private:
    void createItems(QString table, QString itemSingularName, QString itemPluralName, QList<int> ids);
    void deleteItems(QString table, QString itemSingularName, QString itemPluralName, QList<int> ids);
    void setTextAttribute(QString table, QString itemSingularName, QString itemPluralName, QString attribute, QString attributeName, QList<int> ids, QString regex);
    template <typename T> void setNumberAttribute(QString table, QString itemSingularName, QString itemPluralName, QString attribute, QString attributeName, QList<int> ids, QList<Prompt::Key> valueKeys, QString unit, T minValue, T maxValue, bool cyclic);
    void setItemAttribute(QString table, QString itemSingularName, QString itemPluralName, QString attribute, QString attributeName, QList<int> ids, QList<Prompt::Key> valueKeys, QString foreignItemTable, QString foreignItemName, Prompt::Key foreignItemKey);
    QPlainTextEdit *messages;
    Prompt* prompt;
};

#endif // TERMINAL_H
