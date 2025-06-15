/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EFFECT_H
#define EFFECT_H

#include <QtWidgets>

#include "item.h"
#include "intensity.h"
#include "color.h"
#include "position.h"
#include "raw.h"

class Effect : public Item {
public:
    Effect(Kernel* core);
    Effect(const Effect* item);
    ~Effect();
    QMap<int, Intensity*> intensitySteps;
    QMap<int, Color*> colorSteps;
    QMap<int, Position*> positionSteps;
    QMap<int, QList<Raw*>> rawSteps;
    QMap<QString, QMap<int, float>> stepSpecificFloatAttributes;
    QString info() override;
    void writeAttributesToFile(QXmlStreamWriter* fileStream) override;
    float getDimmer(Fixture* fixture, int frame) const;
    rgbColor getRGB(Fixture* fixture, int frame) const;
    positionAngles getPosition(Fixture* fixture, int frame) const;
    QMap<int, uint8_t> getRaws(Fixture* fixture, int frame, bool renderMoveWhileDarkRaws = true) const;
private:
    int getStep(Fixture* fixture, int frame, float* fade) const;
};

#endif // EFFECT_H
