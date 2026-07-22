/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EFFECTDATA_H
#define EFFECTDATA_H

#include "valuedata.h"
#include "intensitydata.h"
#include "colordata.h"
#include "positiondata.h"
#include "rawdata.h"

class EffectData : public ValueData {
public:
    EffectData(int fixtureKey, int groupKey, QList<int> effectKeys, bool renderMwD);
    static void nextFrame();
    bool hasIntensity() const;
    IntensityData getIntensity() const;
    bool hasColor() const;
    ColorData getColor() const;
    bool hasPosition() const;
    PositionData getPosition() const;
    bool hasRaws() const;
    RawData getRaws() const;
private:
    static QHash<int, QHash<int, int>> groupFrames;
    static QHash<int, QHash<int, int>> oldGroupFrames;
    bool intensityGiven = false;
    IntensityData intensity;
    bool colorGiven = false;
    ColorData color;
    bool positionGiven = false;
    PositionData position;
    bool rawsGiven = false;
    RawData raws;
    static QHash<int, int> getStepTimeFrames(int effectKey, int stepAmount, QString table);
    static int getStepKey(int step, int effectKey, QString table);
    static QList<int> getStepRawKeys(int step, int effectKey);
};

#endif // EFFECTDATA_H
