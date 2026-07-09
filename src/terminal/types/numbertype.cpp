/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "numbertype.h"

NumberType::NumberType(const float minValue, const float maxValue, const QString numberUnit, const DataType type, const bool numberCyclic)
    : min(minValue), max(maxValue), unit(numberUnit), dataType(type), cyclic(numberCyclic) {
    Q_ASSERT(minValue <= maxValue);
}

QString NumberType::getUnit() const {
    return unit;
}

NumberType NumberType::percentage() {
    return NumberType(0, 100, "%");
}

NumberType NumberType::angle() {
    return NumberType(0, 360, "°", FLOAT, true);
}

NumberType NumberType::time() {
    return NumberType(0, 3600, "s");
}

NumberType NumberType::priority() {
    return NumberType(0, 200, "", INTEGER);
}

NumberType NumberType::step() {
    return NumberType(1, 99, "", INTEGER);
}

NumberType NumberType::universe() {
    return NumberType(1, 63999, "", INTEGER);
}

NumberType NumberType::address() {
    return NumberType(0, 512, "", INTEGER);
}

NumberType NumberType::channel() {
    return NumberType(1, 512, "", INTEGER);
}

NumberType NumberType::dmxValue() {
    return NumberType(0, 255, "", INTEGER);
}

NumberType NumberType::tilt() {
    return NumberType(-180, 180, "°");
}

NumberType NumberType::zoom() {
    return NumberType(0, 180, "°");
}

NumberType NumberType::panRange() {
    return NumberType(0, 3600, "°");
}

NumberType NumberType::tiltRange() {
    return NumberType(0, 360, "°");
}

NumberType NumberType::zoomRange() {
    return NumberType(0, 180, "°");
}

NumberType NumberType::coordinate() {
    return NumberType(-1000, 1000);
}

QVariant NumberType::format(float value) const {
    if (cyclic) {
        while (value < min) {
            value += max - min;
        }
        while (value >= max) {
            value -= max - min;
        }
    } else {
        if ((value < min) || (value > max)) {
            return QVariant();
        }
    }

    if (dataType == INTEGER) {
        return QVariant((int)value);
    }
    return QVariant(value);
}
