/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "modellist.h"

ModelList::ModelList(Kernel *core) : ItemList("Model", "Models") {
    kernel = core;
}

void ModelList::recordModelChannels(QList<QString> ids, QString channels) {
    if (!channels.contains(QRegularExpression("^[01DRGB]+$"))) {
        kernel->terminal->error("Didn't record Models because channels \"" + channels + "\" are not valid.");
        return;
    }
    int modelCounter = 0;
    for (QString id : ids) {
        Model* model = getItem(id);
        if (model == nullptr) {
            model = recordItem(id);
        }
        QString oldChannels = model->channels;
        model->channels = channels;
        if (kernel->fixtures->channelsOkay()) {
            emit dataChanged(index(getItemRow(model->id), 0), index(getItemRow(model->id), 0), {Qt::DisplayRole, Qt::EditRole});
            modelCounter++;
        } else {
            model->channels = oldChannels; // do not change channels if this would result in DMX address conflicts
        }
    }
    kernel->terminal->success("Recorded " + QString::number(modelCounter) + " Models with channels \"" + channels + "\".");
}
