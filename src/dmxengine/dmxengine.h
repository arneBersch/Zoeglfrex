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
    void sendUniverses(QHash<int, QByteArray> universes);
    void updatePreviewFixtures(QHash<int, Preview2d::PreviewData> fixtures);
    void dbChanged();
private:
    void generateDmx();
    float getFixtureValue(int fixtureKey, int itemKey, QString itemTable, QString itemTableAttribute, QString modelExceptionTable, QString fixtureExceptionTable);
    QSettings* settings;
    QPushButton* highlightButton;
    QPushButton* soloButton;
    QProgressBar* fadeProgressBar;
    QPushButton* skipFadeButton;
    QList<int> groupKeys;
    QHash<int, QSet<int>> groupFixtureKeys;
    QHash<int, float> fixturePan;
    QHash<int, QHash<int, int>> groupEffectFrames;
    QHash<int, int> cuelistCurrentCueKeys;
    QHash<int, int> cuelistFadeFrames;
    QHash<int, QHash<int, int>> cuelistFixtureFadeFrames;
    QHash<int, int> cuelistDelayFrames;
    QHash<int, QHash<int, int>> cuelistFixtureDelayFrames;
    QHash<int, int> cuelistTransitionFrames;
    QHash<int, int> cuelistRemainingTransitionFrames;
    QHash<int, bool> cuelistSineFade;
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
    struct RawChannelData {
        uint8_t value = 0;
        bool fading = false;
        bool moveWhileDark = false;
    };
    void renderCue(int cueKey, QHash<int, QHash<int, int>> oldGroupEffectFrames, QHash<int, float>* fixtureIntensities, QHash<int, ColorData>* fixtureColors, QHash<int, PositionData>* fixturePositions, QHash<int, QHash<int, RawChannelData>>* fixtureRaws);
    float getFixtureIntensity(int fixtureKey, int intensityKey);
    ColorData getFixtureColor(int fixtureKey, int colorKey);
    PositionData getFixturePosition(int fixtureKey, int positionKey);
    QHash<int, RawChannelData> getFixtureRaws(int fixtureKey, QList<int> rawKeys);
    void getFixtureEffects(int fixtureKey, QList<int> effectKeys, QHash<int, int> effectFrames, bool* intensityInformation, float* intensity, bool* colorInformation, ColorData* color, bool* PositionInformation, PositionData* position, QHash<int, RawChannelData>* raws);
};

#endif // DMXENGINE_H
