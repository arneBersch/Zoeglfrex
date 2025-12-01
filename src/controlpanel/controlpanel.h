/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QtWidgets>

#include "intensitycolorpositiontab.h"
#include "rawtab.h"

class ControlPanel : public QWidget {
    Q_OBJECT
public:
    ControlPanel(QWidget *parent = nullptr);
public slots:
    void reload();
signals:
    void dbChanged();
private:
    IntensityColorPositionTab* intensityColorPositionTab;
    RawTab* rawTab;
};

#endif // CONTROLPANEL_H
