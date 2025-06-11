/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUELIST_H
#define CUELIST_H

#include <QtWidgets>

#include "item.h"
#include "../tables/cuetable.h"

class Cuelist : public Item {
public:
    Cuelist(Kernel* core);
    Cuelist(const Cuelist* item);
    ~Cuelist();
    QString name() override;
    QString info() override;
    void writeAttributesToFile(QXmlStreamWriter* fileStream) override;
    CueTable* cues;
    void goToCue(QString cueId);
    void go();
    void goBack();
    Cue* currentCue = nullptr;
    Cue* previousCue = nullptr;
    unsigned int remainingFadeFrames = 0;
    unsigned int totalFadeFrames = 0;
};

#endif // CUELIST_H
