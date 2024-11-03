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

Intensity* IntensityList::getItem(QString id)
{
    int intensityRow = getItemRow(id);
    if (intensityRow < 0 || intensityRow >= items.size()) {
        return nullptr;
    }
    return items[intensityRow];
}

int IntensityList::getItemRow(QString id)
{
    for (int intensityRow = 0; intensityRow < items.size(); intensityRow++) {
        if (items[intensityRow]->id == id) {
            return intensityRow;
        }
    }
    return -1;
}

bool IntensityList::copyItems(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Intensity* intensity = getItem(id);
        if (intensity == nullptr) {
            kernel->terminal->error("Intensity can't be copied because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Intensity can't be copied because Target ID is already used.");
            return false;
        }
        Intensity *targetIntensity = recordIntensity(targetId);
        targetIntensity->label = intensity->label;
        targetIntensity->dimmer = intensity->dimmer;
    }
    return true;
}

bool IntensityList::deleteItems(QList<QString> ids)
{
    for (QString id : ids) {
        int intensityRow = getItemRow(id);
        if (intensityRow < 0) {
            kernel->terminal->error("Intensity can't be deleted because it doesn't exist.");
            return false;
        }
        Intensity *intensity = items[intensityRow];
        kernel->cues->deleteIntensity(intensity);
        items.removeAt(intensityRow);
        delete intensity;
    }
    return true;
}

bool IntensityList::moveItems(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int intensityRow = getItemRow(id);
        if (intensityRow < 0) {
            kernel->terminal->error("Intensity can't be moved because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Intenity can't be moved because Target ID is already used.");
            return false;
        }
        Intensity* intensity = items[intensityRow];
        items.removeAt(intensityRow);
        intensity->id = targetId;
        int position = 0;
        for (int index=0; index < items.size(); index++) {
            if (greaterId(items[index]->id, targetId)) {
                position++;
            }
        }
        items.insert(position, intensity);
    }
    return true;
}

Intensity* IntensityList::recordIntensity(QString id)
{
    Intensity *intensity = new Intensity;
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

bool IntensityList::recordIntensityDimmer(QList<QString> ids, float dimmer)
{
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

QList<QString> IntensityList::getIds() {
    QList<QString> ids;
    for (Intensity *intensity : items) {
        ids.append(intensity->id);
    }
    return ids;
}

int IntensityList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return items.size();
}

QVariant IntensityList::data(const QModelIndex &index, const int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }
    if (column >= columnCount() || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        return items[row]->name();
    }
    return QVariant();
}
