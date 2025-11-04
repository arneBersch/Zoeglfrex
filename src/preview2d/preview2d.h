/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PREVIEW2D_H
#define PREVIEW2D_H

#include<QtWidgets>

class Preview2d : public QWidget {
    Q_OBJECT
public:
    Preview2d(QWidget* parent = nullptr);
    struct PreviewFixture {
        float x;
        float y;
        QColor color;
        float pan;
        float tilt;
        float zoom;
    };
public slots:
    void setFixtures(QList<PreviewFixture> fixtures);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QList<PreviewFixture> previewFixtures;
    const int windowWidth = 400;
    const int fixtureDiameter = 20;
    const int maxBeamLength = 50;
};

#endif // PREVIEW2D_H
