/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef INTENSITYCOLORPOSITIONTAB_H
#define INTENSITYCOLORPOSITIONTAB_H

#include <QtWidgets>
#include <QtSql>

#include "attributepanel.h"

class IntensityColorPositionTab : public QWidget {
    Q_OBJECT
public:
    IntensityColorPositionTab(QWidget* parent = nullptr);
public slots:
    void reload();
signals:
    void dbChanged();
private:
    int updateItemLabel(QString table, QString valueTable, QString itemName, QLabel* label);
    QLabel* intensityLabel;
    QLabel* colorLabel;
    QLabel* positionLabel;
    AttributePanel* intensityDimmerPanel;
    AttributePanel* colorHuePanel;
    AttributePanel* colorSaturationPanel;
    AttributePanel* positionPanPanel;
    AttributePanel* positionTiltPanel;
    AttributePanel* positionZoomPanel;
    AttributePanel* positionFocusPanel;
};

#endif // INTENSITYCOLORPOSITIONTAB_H
