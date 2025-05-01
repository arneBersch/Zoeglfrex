/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef POSITION_H
#define POSITION_H

#include <QtWidgets>

#include "item.h"

struct positionAngles {
    float pan = 0;
    float tilt = 0;
};

class Position : public Item {
public:
    Position(Kernel* core);
    Position(const Position* item);
    ~Position();
    QString name() override;
    QString info() override;
    positionAngles getAngles(Fixture* fixture);
};

#endif // POSITION_H
