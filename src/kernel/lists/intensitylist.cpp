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

void IntensityList::setOtherAttribute(QList<QString> ids, QMap<int, QString> attribute, QList<int> value, QString text) {
    QString attributeString = attribute.value(Keys::Attribute);
    if (attributeString == "2") {
        float dimmer = kernel->keysToValue(value);
        if (dimmer > 100 || dimmer < 0) {
            kernel->terminal->error("Can't set Intensity Dimmer because Dimmer only allows values from 0% to 100%.");
            return;
        }
        for (QString id : ids) {
            Intensity* intensity = getItem(id);
            if (intensity == nullptr) {
                intensity = recordItem(id);
            }
            intensity->dimmer = dimmer;
            emit dataChanged(index(getItemRow(intensity->id), 0), index(getItemRow(intensity->id), 0), {Qt::DisplayRole, Qt::EditRole});
        }
        kernel->terminal->success("Set Dimmer of " + QString::number(ids.length()) + " Intensities to " + QString::number(dimmer) + "%.");
    } else {
        kernel->terminal->error("Can't set Intensity attribute " + attributeString + ".");
    }
}
