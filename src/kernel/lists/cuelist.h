/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUELIST_H
#define CUELIST_H

#include <QtWidgets>

class Kernel;
struct Intensity;
struct Color;
struct Row;
struct Transition;

struct Cue {
    QString id;
    QString label;
    Transition* transition;
    QMap<Row*, Intensity*> intensities;
    QMap<Row*, Color*> colors;
};

namespace CueListColumns {
enum {
    id,
    label,
    transition,
};
}

class CueList : public QAbstractTableModel {
    Q_OBJECT
public:
    CueList(Kernel *core);
    Cue* getCue(QString id);
    int getCueRow(QString id);
    QString copyCue(QList<QString> ids, QString targetId);
    QString deleteCue(QList<QString> ids);
    QString deleteCueRowIntensity(QList<QString> ids, QString rowId);
    QString deleteCueRowColor(QList<QString> ids, QString rowId);
    void deleteIntensity(Intensity *intensity);
    void deleteColor(Color *color);
    void deleteTransition(Transition *transition);
    void deleteRow(Row *row);
    QString labelCue(QList<QString> ids, QString label);
    QString moveCue(QList<QString> ids, QString targetId);
    QString recordCueTransition(QList<QString> ids, QString transitionId);
    QString recordCueIntensity(QList<QString> ids, QString rowId, QString intensityId);
    QString recordCueColor(QList<QString> ids, QString rowId, QString colorId);
    QList<QString> getIds();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
private:
    QList<Cue*> cues;
    Kernel *kernel;
    Cue* recordCue(QString id, Transition *transition);
};

#include "kernel/kernel.h"

#endif // CUELIST_H
