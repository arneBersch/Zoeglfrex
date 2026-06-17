/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef NUMBERTYPE_H
#define NUMBERTYPE_H

#include <QtWidgets>

class NumberType {
public:
    static NumberType percentage();
    static NumberType angle();
    static NumberType time();
    static NumberType priority();
    static NumberType step();
    static NumberType universe();
    static NumberType address();
    static NumberType channel();
    static NumberType dmxValue();
    static NumberType tilt();
    static NumberType zoom();
    static NumberType panRange();
    static NumberType tiltRange();
    static NumberType zoomRange();
    static NumberType coordinate();
    QString getUnit() const;
    QVariant format(bool* ok, float value) const;
private:
    enum DataType {
        INTEGER,
        FLOAT,
    };
    NumberType(float min, float max, QString unit = "", DataType type = FLOAT, bool cyclic = false);
    const float min;
    const float max;
    const QString unit;
    const DataType dataType;
    const bool cyclic;
};

#endif // NUMBERTYPE_H
