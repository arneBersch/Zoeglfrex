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

Intensity* IntensityList::getIntensity(QString id)
{
    int intensityRow = getIntensityRow(id);
    if (intensityRow < 0 || intensityRow >= intensities.size()) {
        return nullptr;
    }
    return intensities[intensityRow];
}

int IntensityList::getIntensityRow(QString id)
{
    for (int intensityRow = 0; intensityRow < intensities.size(); intensityRow++) {
        if (intensities[intensityRow]->id == id) {
            return intensityRow;
        }
    }
    return -1;
}

QString IntensityList::copyIntensity(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Intensity* intensity = getIntensity(id);
        if (intensity == nullptr) {
            return "Intensity can't be copied because it doesn't exist.";
        }
        if (getIntensity(targetId) != nullptr) {
            return "Intensity can't be copied because Target ID is already used.";
        }
        Intensity *targetIntensity = recordIntensity(targetId);
        targetIntensity->label = intensity->label;
        targetIntensity->dimmer = intensity->dimmer;
    }
    return QString();
}

QString IntensityList::deleteIntensity(QList<QString> ids)
{
    for (QString id : ids) {
        int intensityRow = getIntensityRow(id);
        if (intensityRow < 0) {
            return "Intensity can't be deleted because it doesn't exist.";
        }
        Intensity *intensity = intensities[intensityRow];
        kernel->cues->deleteIntensity(intensity);
        intensities.removeAt(intensityRow);
        delete intensity;
    }
    return QString();
}

QString IntensityList::labelIntensity(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Intensity* intensity = getIntensity(id);
        if (intensity == nullptr) {
            return "Intensity can't be labeled because it doesn't exist.";
        }
        intensity->label = label;
    }
    return QString();
}

QString IntensityList::moveIntensity(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int intensityRow = getIntensityRow(id);
        if (intensityRow < 0) {
            return "Intensity can't be moved because it doesn't exist.";
        }
        if (getIntensity(targetId) != nullptr) {
            return "Intensity can't be moved because Target ID is already used.";
        }
        Intensity* intensity = intensities[intensityRow];
        intensities.removeAt(intensityRow);
        intensity->id = targetId;
        QList<QString> idParts = targetId.split(".");
        int position = 0;
        for (int index=0; index < intensities.size(); index++) {
            QList<QString> indexIdParts = (intensities[index]->id).split(".");
            if (indexIdParts[0].toInt() < idParts[0].toInt()) {
                position++;
            } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
                if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                    position++;
                } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                    if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                        position++;
                    } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                        if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                            position++;
                        } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                            if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                                position++;
                            }
                        }
                    }
                }
            }
        }
        intensities.insert(position, intensity);
    }
    return QString();
}

Intensity* IntensityList::recordIntensity(QString id)
{
    Intensity *intensity = new Intensity;
    intensity->id = id;
    intensity->label = QString();
    intensity->dimmer = 100;
    QList<QString> idParts = id.split(".");
    int position = 0;
    for (int index=0; index < intensities.size(); index++) {
        QList<QString> indexIdParts = (intensities[index]->id).split(".");
        if (indexIdParts[0].toInt() < idParts[0].toInt()) {
            position++;
        } else if (indexIdParts[0].toInt() == idParts[0].toInt()) {
            if (indexIdParts[1].toInt() < idParts[1].toInt()) {
                position++;
            } else if (indexIdParts[1].toInt() == idParts[1].toInt()) {
                if (indexIdParts[2].toInt() < idParts[2].toInt()) {
                    position++;
                } else if (indexIdParts[2].toInt() == idParts[2].toInt()) {
                    if (indexIdParts[3].toInt() < idParts[3].toInt()) {
                        position++;
                    } else if (indexIdParts[3].toInt() == idParts[3].toInt()) {
                        if (indexIdParts[4].toInt() < idParts[4].toInt()) {
                            position++;
                        }
                    }
                }
            }
        }
    }
    intensities.insert(position, intensity);
    return intensity;
}

QString IntensityList::recordIntensityDimmer(QList<QString> ids, float dimmer)
{
    if (dimmer > 100 || dimmer < 0) {
        return "Record Intensity Dimmer only allows from 0% to 100%.";
    }
    for (QString id : ids) {
        Intensity* intensity = getIntensity(id);
        if (intensity == nullptr) {
            intensity = recordIntensity(id);
        }
        intensity->dimmer = dimmer;
    }
    return QString();
}

QList<QString> IntensityList::getIds() {
    QList<QString> ids;
    for (Intensity *intensity : intensities) {
        ids.append(intensity->id);
    }
    return ids;
}

int IntensityList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return intensities.size();
}

int IntensityList::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant IntensityList::data(const QModelIndex &index, const int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (row >= (this->rowCount()) || row < 0) {
        return QVariant();
    }
    if (column >= (this->columnCount()) || column < 0) {
        return QVariant();
    }
    if (index.isValid() && role == Qt::DisplayRole) {
        if (column == IntensityListColumns::id) {
            return intensities[row]->id;
        } else if (column == IntensityListColumns::label) {
            return intensities[row]->label;
        } else if (column == IntensityListColumns::dimmer) {
            return intensities[row]->dimmer;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant IntensityList::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == IntensityListColumns::id) {
            return "ID";
        } else if (column == IntensityListColumns::label) {
            return "Label";
        } else if (column == IntensityListColumns::dimmer) {
            return "Dimmer (%)";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
