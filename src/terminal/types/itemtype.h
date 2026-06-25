/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ITEMTYPE_H
#define ITEMTYPE_H

#include <QtWidgets>
#include <QtSql>

#include "../keys.h"

class ItemType {
public:
    QString getSelectTable() const;
    QString getUpdateTable() const;
    QString getSingular() const;
    QString getPlural() const;
    Keys::Key getKey() const;
    QString format(QStringList ids) const;
    QStringList selectItems(QStringList ids) const;
    QStringList deselectItems() const;
    QStringList createItems(QStringList ids) const;
    QStringList deleteItems(QStringList ids) const;
    QStringList updateSortingKeys() const;
    QList<int> getItemKeys(QStringList ids, QStringList* output) const;
    bool operator==(ItemType item) const;

    static ItemType model();
    static ItemType fixture();
    static ItemType group();
    static ItemType intensity();
    static ItemType color();
    static ItemType position();
    static ItemType raw();
    static ItemType effect();
    static ItemType cuelist();
    static ItemType cue();
    static QList<ItemType> allTypes();
private:
    ItemType(QString table, QString singular, QString plural, Keys::Key key);
    ItemType(QString selectTable, QString updateTable, QString singular, QString plural, Keys::Key key);
    QString selectTable;
    QString updateTable;
    QString singular;
    QString plural;
    Keys::Key key;
    static bool compareIds(QString idA, QString idB);
    QStringList setCurrentItem(QStringList ids, QString itemTable, QString updateQueryText) const;
    QStringList setCueItem(QStringList ids, QString valueTable, bool multipleItemsAllowed) const;
};

#endif // ITEMTYPE_H
