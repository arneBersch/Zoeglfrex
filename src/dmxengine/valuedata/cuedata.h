/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUEDATA_H
#define CUEDATA_H

#include <QtWidgets>

#include "valuedata.h"
#include "intensitydata.h"
#include "colordata.h"
#include "positiondata.h"
#include "rawdata.h"

class CueData : public ValueData {
public:
    CueData();
    CueData(int cueKey, QList<int> groupKeys, QHash<int, QSet<int>> groupFixtureKeys);
    void fade(CueData lastCue, float standardFade, QHash<int, float> fixtureFades);
    QHash<int, IntensityData> getFixtureIntensities() const;
    QHash<int, ColorData> getFixtureColors() const;
    QHash<int, PositionData> getFixturePositions() const;
    QHash<int, RawData> getFixtureRaws() const;
private:
    QHash<int, IntensityData> fixtureIntensities;
    QHash<int, ColorData> fixtureColors;
    QHash<int, PositionData> fixturePositions;
    QHash<int, RawData> fixtureRaws;
    static int getItemKey(int cueKey, int groupKey, QString table);
    static QList<int> getItemKeys(int cueKey, int groupKey, QString valueTable, QString itemTable);
    static QList<int> getItemRawKeys(const int itemKey, const QString table);
};

#endif // CUEDATA_H
