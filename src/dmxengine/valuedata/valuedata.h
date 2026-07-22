/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include <QtWidgets>
#include <QtSql>

#ifndef VALUEDATA_H
#define VALUEDATA_H

class ValueData {
public:
    ValueData();
protected:
    float getFixtureValue(int fixtureKey, int itemKey, QString itemTable, QString itemTableAttribute, QString modelExceptionTable, QString fixtureExceptionTable);
    float fadeValue(float oldValue, float newValue, float fade);
};

#endif // VALUEDATA_H
