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

class Fixture;
class Group;
class Intensity;

struct FixtureListAttribute {
    QString name = "Fixture";
    QList<Fixture*> value = QList<Fixture*>();
};

struct GroupSpecificIntensityAttribute {
    QString name = "Group Intensities";
    QMap<Group*, Intensity*> value = QMap<Group*, Intensity*>();
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

class Kernel;

template <class T> class ItemList : public QAbstractTableModel {
public:
    ItemList(int key, QString singularName, QString pluralName);
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
    QMap<QString, FixtureListAttribute> fixtureListAttributes;
    QMap<QString, GroupSpecificIntensityAttribute> groupSpecificIntensityAttributes;
    QMap<QString, IntAttribute> intAttributes;
    QMap<QString, FloatAttribute> floatAttributes;
    QMap<QString, FloatAttribute> fixtureSpecificFloatAttributes;
    QMap<QString, AngleAttribute> angleAttributes;
private:
    int findRow(QString id);
    int itemKey;
    QString singularItemName = "item";
    QString pluralItemName = "items";
};

#endif // ITEMLIST_H
