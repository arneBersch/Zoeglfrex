/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ITEMTABLE_H
#define ITEMTABLE_H

#include <QtWidgets>

class Fixture;

struct StringAttribute {
    QString name = QString();
    QString value = QString();
    QString regex = QString();
};

struct IntAttribute {
    QString name = QString();
    int value = 0;
    int min = 0;
    int max = 100;
    QString unit = QString();
};

struct FloatAttribute {
    QString name = QString();
    float value = 0;
    float min = 0;
    float max = 100;
    QString unit = QString();
};

struct AngleAttribute {
    QString name = "Angle";
    float value = 0;
};

struct BoolAttribute {
    QString name = QString();
    bool value = false;
};

class Kernel;

template <class T> class ItemTable : public QAbstractTableModel {
public:
    ItemTable(Kernel* core, int key, QString singularName, QString pluralName);
    ~ItemTable();
    T* getItem(QString id) const;
    int getItemRow(QString id) const;
    QStringList getIds() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QList<T*> items;
    virtual void setAttribute(QStringList ids, QMap<int, QString> attribute, QList<int> values, QString text = QString());
    void saveItemsToFile(QXmlStreamWriter* fileStream);
    void reset();
    QMap<QString, StringAttribute> stringAttributes;
    QMap<QString, IntAttribute> intAttributes;
    QMap<QString, FloatAttribute> floatAttributes;
    QMap<QString, FloatAttribute> modelSpecificFloatAttributes;
    QMap<QString, FloatAttribute> fixtureSpecificFloatAttributes;
    QMap<QString, AngleAttribute> angleAttributes;
    QMap<QString, AngleAttribute> modelSpecificAngleAttributes;
    QMap<QString, AngleAttribute> fixtureSpecificAngleAttributes;
    QMap<QString, BoolAttribute> boolAttributes;
protected:
    virtual T* addItem(QString id);
    Kernel *kernel;
private:
    int findRow(QString id);
    int itemKey;
    QString singularItemName = "item";
    QString pluralItemName = "items";
};

#endif // ITEMTABLE_H
