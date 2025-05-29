/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "colortable.h"

ColorTable::ColorTable(Kernel *core) : ItemTable(core, Keys::Color, "Color", "Colors") {
    angleAttributes[HUEATTRIBUTEID] = {"Hue", 0};
    modelSpecificAngleAttributes[HUEATTRIBUTEID] = {"Hue", 0};
    fixtureSpecificAngleAttributes[HUEATTRIBUTEID] = {"Hue", 0};
    floatAttributes[SATURATIONATTRIBUTEID] = {"Saturation", 100, 0, 100, "%"};
    modelSpecificFloatAttributes[SATURATIONATTRIBUTEID] = {"Saturation", 100, 0, 100, "%"};
    fixtureSpecificFloatAttributes[SATURATIONATTRIBUTEID] = {"Saturation", 100, 0, 100, "%"};
    floatAttributes[QUALITYATTRIBUTEID] = {"Quality", 100, 0, 100, "%"};
}
