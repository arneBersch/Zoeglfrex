/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "positiontable.h"

PositionTable::PositionTable(Kernel *core) : ItemTable(core, Keys::Position, "Position", "Positions") {
    angleAttributes[kernel->POSITIONPANATTRIBUTEID] = {"Pan", 0};
    modelSpecificAngleAttributes[kernel->POSITIONPANATTRIBUTEID] = {"Pan", 0};
    fixtureSpecificAngleAttributes[kernel->POSITIONPANATTRIBUTEID] = {"Pan", 0};
    floatAttributes[kernel->POSITIONTILTATTRIBUTEID] = {"Tilt", 0, -180, 180, "°"};
    modelSpecificFloatAttributes[kernel->POSITIONTILTATTRIBUTEID] = {"Tilt", 0, -180, 180, "°"};
    fixtureSpecificFloatAttributes[kernel->POSITIONTILTATTRIBUTEID] = {"Tilt", 0, -180, 180, "°"};
    floatAttributes[kernel->POSITIONZOOMATTRIBUTEID] = {"Zoom", 15, 0, 180, "°"};
    modelSpecificFloatAttributes[kernel->POSITIONZOOMATTRIBUTEID] = {"Zoom", 15, 0, 180, "°"};
    fixtureSpecificFloatAttributes[kernel->POSITIONZOOMATTRIBUTEID] = {"Zoom", 15, 0, 180, "°"};
}
