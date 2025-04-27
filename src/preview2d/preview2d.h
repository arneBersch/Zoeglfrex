/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PREVIEW2D_H
#define PREVIEW2D_H

#include <QtWidgets>

#include "fixturegraphicsitem.h"
class Kernel;
class Fixture;

class Preview2d : public QWidget {
public:
    Preview2d(Kernel* core);
    void updateImage();
    QMap<Fixture*, FixtureGraphicsItem*> fixtureCircles;
private:
    int fixtureCirclesAmount = 0;
    QGraphicsView* view;
    QGraphicsScene* scene;
    Kernel* kernel;
};

#endif // PREVIEW2D_H
