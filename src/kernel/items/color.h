/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef COLOR_H
#define COLOR_H

#include <QtWidgets>

#include "item.h"

struct rgbColor {
    float red = 100;
    float green = 100;
    float blue = 100;
};

class Color : public Item {
public:
    Color(Kernel* core);
    Color(const Color* item);
    ~Color();
    QString name() override;
    QString info() override;
    rgbColor getRGB(Fixture* fixture);
};

#endif // COLOR_H
