/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "intensitytable.h"

IntensityTable::IntensityTable(Kernel *core) : ItemTable(core, Keys::Intensity, "Intensity", "Intensities") {
    floatAttributes[DIMMERATTRIBUTEID] = {"Dimmer", 0, 0, 100, "%"};
    modelSpecificFloatAttributes[DIMMERATTRIBUTEID] = {"Dimmer", 0, 0, 100, "%"};
    fixtureSpecificFloatAttributes[DIMMERATTRIBUTEID] = {"Dimmer", 0, 0, 100, "%"};
}
