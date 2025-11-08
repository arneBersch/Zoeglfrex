/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DMXENGINE_H
#define DMXENGINE_H

#include <QtWidgets>
#include <QtSql>

#include "preview2d/preview2d.h"

class DmxEngine : public QWidget {
    Q_OBJECT
public:
    DmxEngine(QWidget* parent = nullptr);
signals:
    void sendUniverse(int universe, QByteArray data);
    void updatePreviewFixtures(QList<Preview2d::PreviewFixture> fixtures);
private:
    void generateDmx();
    QMap<int, int> getCurrentCueItems(const int cueId, const QString table);
    float getFixtureValue(int fixtureKey, int itemKey, const QString itemTable, const QString itemTableAttribute, const QString modelExceptionTable, const QString fixtureExceptionTable);
    QProgressBar* fadeProgressBar;
    QMap<int, float> fixturePan;
    QMap<int, int> cuelistCurrentCueKeys;
    QMap<int, int> cuelistRemainingFadeFrames;
    QMap<int, int> cuelistTotalFadeFrames;
    const int FRAMEDURATION = 25;
    struct ColorData {
        float red = 100;
        float green = 100;
        float blue = 100;
        float quality = 100;
    };
    struct PositionData {
        float pan = 0;
        float tilt = 0;
        float zoom = 15;
        float focus = 0;
    };
};

#endif // DMXENGINE_H
