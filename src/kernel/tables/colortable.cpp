/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "colortable.h"

ColorTable::ColorTable(Kernel *core) : ItemTable(core, Keys::Color, "Color", "Colors") {
    angleAttributes[kernel->COLORHUEATTRIBUTEID] = {"Hue", 0};
    modelSpecificAngleAttributes[kernel->COLORHUEATTRIBUTEID] = {"Hue", 0};
    fixtureSpecificAngleAttributes[kernel->COLORHUEATTRIBUTEID] = {"Hue", 0};
    floatAttributes[kernel->COLORSATURATIONATTRIBUTEID] = {"Saturation", 100, 0, 100, "%"};
    modelSpecificFloatAttributes[kernel->COLORSATURATIONATTRIBUTEID] = {"Saturation", 100, 0, 100, "%"};
    fixtureSpecificFloatAttributes[kernel->COLORSATURATIONATTRIBUTEID] = {"Saturation", 100, 0, 100, "%"};
    floatAttributes[kernel->COLORQUALITYATTRIBUTEID] = {"Quality", 100, 0, 100, "%"};
}

bool ColorTable::isCurrentItem(Color* item) const {
    if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr)) {
        return (kernel->cuelistView->currentCuelist->currentCue->colors.value(kernel->cuelistView->currentGroup, nullptr) == item);
    }
    return false;
}
