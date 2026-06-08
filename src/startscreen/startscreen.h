/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include<QtWidgets>
#include<QtSql>

class StartScreen : public QWidget {
public:
    StartScreen(QString version, QString copyright, QString fileVersion, QWidget *parent = nullptr);
    void openFile(QString fileName, QString version, QString copyright, QString fileVersion);
private:
    const QString FILENAME_FILTER = "zfr Files (*.zfr)";
    static QList<QString> getCreateFileQueries(QString fileVersion);
    static QString getCreateTableQuery(QString tableName, QStringList fields, QStringList primaryKeys);
    static QString getCreateItemListTableQuery(QString tableName, QString itemTable, QString foreignItemTable);
    static QString getCreateItemSpecificNumberTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueType);
    static QString getCreateItemSpecificItemTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueItemTable);
    static QString getCreateItemSpecificItemListTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueItemTable);
    static QString getCreateIntegerSpecificItemTableQuery(QString tableName, QString itemTable, QString valueItemTable);
    static QString getCreateIntegerSpecificItemListTableQuery(QString tableName, QString itemTable, QString valueItemTable);
    static QString getCreateIntegerSpecificNumberTableQuery(QString tableName, QString itemTable, QString valueType);
    static QString getCreateItemAndIntegerSpecificNumberTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueType);
};

#endif // STARTSCREEN_H
