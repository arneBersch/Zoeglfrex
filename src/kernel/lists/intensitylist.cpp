/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "intensitylist.h"

IntensityList::IntensityList(Kernel *core) {
    kernel = core;
}

Intensity* IntensityList::recordIntensity(QString id) {
    Intensity *intensity = new Intensity(kernel);
    intensity->id = id;
    intensity->label = QString();
    intensity->dimmer = 100;
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    items.insert(position, intensity);
    return intensity;
}

bool IntensityList::recordIntensityDimmer(QList<QString> ids, float dimmer) {
    if (dimmer > 100 || dimmer < 0) {
        kernel->terminal->error("Record Intensity Dimmer only allows from 0% to 100%.");
        return false;
    }
    for (QString id : ids) {
        Intensity* intensity = getItem(id);
        if (intensity == nullptr) {
            intensity = recordIntensity(id);
        }
        intensity->dimmer = dimmer;
    }
    return true;
}
