/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ITEM_H
#define ITEM_H

#include <QtWidgets>

class Kernel;
class Model;
class Fixture;
class Group;
class Intensity;

class Item {
public:
    Item(Kernel* core);
    Item(const Item* item);
    virtual ~Item();
    QString id;
    virtual QString name();
    virtual QString info();
    virtual void writeAttributesToFile(QXmlStreamWriter *fileStream);
    QMap<QString, QString> stringAttributes;
    QMap<QString, int> intAttributes;
    QMap<QString, float> floatAttributes;
    QMap<QString, QMap<Model*, float>> modelSpecificFloatAttributes;
    QMap<QString, QMap<Fixture*, float>> fixtureSpecificFloatAttributes;
    QMap<QString, float> angleAttributes;
    QMap<QString, QMap<Model*, float>> modelSpecificAngleAttributes;
    QMap<QString, QMap<Fixture*, float>> fixtureSpecificAngleAttributes;
protected:
    Kernel *kernel;
};

#endif // ITEM_H
