/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUE_H
#define CUE_H

#include <QtWidgets>

#include "item.h"
#include "group.h"
#include "color.h"
#include "position.h"
#include "raw.h"
#include "effect.h"

class Cue : public Item {
public:
    Cue(Kernel *core);
    Cue(const Cue* item);
    ~Cue();
    QMap<Group*, Intensity*> intensities;
    QMap<Group*, Color*> colors;
    QMap<Group*, Position*> positions;
    QMap<Group*, QList<Raw*>> raws;
    QMap<Group*, QList<Effect*>> effects;
    QString info() override;
    void writeAttributesToFile(QXmlStreamWriter* fileStream) override;
};

#endif // CUE_H
