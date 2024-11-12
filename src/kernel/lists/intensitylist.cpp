/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "intensitylist.h"

IntensityList::IntensityList(Kernel *core) : ItemList("Intensity", "Intensities") {
    kernel = core;
}

Intensity* IntensityList::recordItem(QString id) {
    Intensity *intensity = new Intensity(kernel);
    intensity->id = id;
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    beginInsertRows(QModelIndex(), position, position);
    items.insert(position, intensity);
    endInsertRows();
    return intensity;
}

void IntensityList::recordIntensityDimmer(QList<QString> ids, float dimmer) {
    if (dimmer > 100 || dimmer < 0) {
        kernel->terminal->error("Didn't record Intensities because Record Intensity Dimmer only allows values from 0% to 100%.");
        return;
    }
    for (QString id : ids) {
        Intensity* intensity = getItem(id);
        if (intensity == nullptr) {
            intensity = recordItem(id);
        }
        intensity->dimmer = dimmer;
    }
    kernel->terminal->success("Recorded " + QString::number(ids.length()) + " Intensities with dimmer " + QString::number(dimmer) + "%.");
}
