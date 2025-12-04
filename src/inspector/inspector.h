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
#include "itemtablemodel.h"

class Inspector : public QWidget {
    Q_OBJECT
public:
    Inspector(QWidget *parent = nullptr);
public slots:
    void loadItems(QString itemName, QStringList ids);
private:
    QString getBoolAttribute(QString table, QString attribute, int key) const;
    QString getTextAttribute(QString table, QString attribute, int key) const;
    QString getNumberAttribute(QString table, QString attribute, int key, QString unit) const;
    QString getItemAttribute(QString table, QString attribute, int key, QString valueItemTable) const;
    QString getItemListAttribute(QString valueItemTable, QString valueTable, int key) const;
    QString getItemSpecificNumberAttribute(QString foreignItemsTable, QString valueTable, QString unit, int key) const;
    QString getItemSpecificItemListAttribute(QString foreignItemTable, QString valueItemTable, QString valueTable, int key) const;
    QString getIntegerSpecificNumberAttribute(QString valueTable, int key, QString unit) const;
    QString getIntegerSpecificItemListAttribute(QString foreignItemTable, QString valueTable, int key) const;
    QString getItemAndIntegerSpecificNumberAttribute(QString foreignItemTable, QString valueTable, int key, QString unit) const;
    ItemTableModel* model;
    QTableView* tableView;
    QLabel* titleLabel;
    QLabel* infosLabel;
};

#endif // INSPECTOR_H
