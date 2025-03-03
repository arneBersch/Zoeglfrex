/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QtWidgets>

#include "kernel/items/item.h"

class Kernel;

template <class T> class ItemList : public QAbstractTableModel {
public:
    ItemList(QString singularName, QString pluralName);
    T* getItem(QString id) const;
    int getItemRow(QString id) const;
    QList<QString> getIds() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    Kernel *kernel;
    QList<T*> items;
    void setAttribute(QList<QString> ids, QMap<int, QString> attribute, QList<int> values, QString text = QString());
    T* addItem(QString id);
    const QString IDATTRIBUTEID = "0";
    const QString LABELATTRIBUTEID = "1";
protected:
    virtual void setOtherAttribute(QList<QString> ids, QMap<int, QString> attribute, QList<int> values, QString text = QString()) = 0;
    QMap<QString, FloatAttribute> floatAttributes;
private:
    int findRow(QString id);
    QString singularItemName = "item";
    QString pluralItemName = "items";
};

#endif // ITEMLIST_H
