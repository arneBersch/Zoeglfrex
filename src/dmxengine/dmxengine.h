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
    QPushButton* highlightButton;
    QPushButton* soloButton;
    QProgressBar* fadeProgressBar;
    QPushButton* skipFadeButton;

    QHash<int, float> fixturePan;

    QHash<int, int> cuelistCurrentCueKeys;
    QHash<int, int> cuelistFadeFrames;
    QHash<int, QHash<int, int>> cuelistFixtureFadeFrames;
    QHash<int, int> cuelistDelayFrames;
    QHash<int, QHash<int, int>> cuelistFixtureDelayFrames;
    QHash<int, int> cuelistTransitionFrames;
    QHash<int, int> cuelistRemainingTransitionFrames;
    QHash<int, bool> cuelistSineFade;

    const int FRAMEDURATION = 25;
};

#endif // DMXENGINE_H
