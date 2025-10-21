/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef INSPECTOR_H
#define INSPECTOR_H

#include <QtWidgets>
#include <QtSql>

#include "constants.h"

class Inspector : public QWidget {
    Q_OBJECT
public:
    Inspector(QWidget *parent = nullptr);
public slots:
    void reload();
    void loadItem(QString itemName, QStringList ids);
private:
    QString getBoolAttribute(QString table, QString attribute, QString id) const;
    QString getTextAttribute(QString table, QString attribute, QString id) const;
    QString getNumberAttribute(QString table, QString attribute, QString id, QString unit) const;
    QString getItemAttribute(QString table, QString attribute, QString id, QString valueItemTable) const;
    QString getItemListAttribute(QString table, QString foreignItemsTable, QString listTable, QString listTableItemAttribute, QString listTableForeignItemsAttribute, QString id) const;
    QString getItemSpecificNumberAttribute(QString table, QString foreignItemsTable, QString exceptionTable, QString exceptionTableItemAttribute, QString exceptionTableForeignItemsAttribute, QString exceptionTableValueAttribute, QString unit, QString id) const;
    QString getItemSpecificItemAttribute(QString table, QString foreignItemTable, QString valueItemTable, QString valueTable, QString valueTableItemAttribute, QString valueTableForeignItemAttribute, QString valueTableValueItemAttribute, QString id) const;
    QSqlQueryModel* model;
    QString itemName;
    QStringList itemIds;
    QListView* list;
    QLabel* titleLabel;
    QLabel* infosLabel;
};

#endif // INSPECTOR_H
