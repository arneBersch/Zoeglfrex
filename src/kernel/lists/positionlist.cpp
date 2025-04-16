/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "positionlist.h"

PositionList::PositionList(Kernel *core) : ItemList(core, Keys::Position, "Position", "Positions") {
    angleAttributes[PANATTRIBUTEID] = {"Pan", 0};
    modelSpecificAngleAttributes[PANATTRIBUTEID] = {"Pan", 0};
    fixtureSpecificAngleAttributes[PANATTRIBUTEID] = {"Pan", 0};
    angleAttributes[TILTATTRIBUTEID] = {"Tilt", 0};
    modelSpecificAngleAttributes[TILTATTRIBUTEID] = {"Tilt", 0};
    fixtureSpecificAngleAttributes[TILTATTRIBUTEID] = {"Tilt", 0};
}
