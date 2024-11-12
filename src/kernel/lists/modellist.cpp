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

Model* ModelList::recordModel(QString id) {
    Model *model = new Model(kernel);
    model->id = id;
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    beginInsertRows(QModelIndex(), position, position);
    items.insert(position, model);
    endInsertRows();
    return model;
}

void ModelList::recordModelChannels(QList<QString> ids, QString channels) {
    if (!channels.contains(QRegularExpression("^[DRGB]+$"))) {
        kernel->terminal->error("Didn't record Models because channels \"" + channels + "\" are not valid.");
        return;
    }
    int modelCounter = 0;
    for (QString id : ids) {
        Model* model = getItem(id);
        bool addressConflict = false;
        if (model == nullptr) {
            model = recordModel(id);
        } else {
            if (channels.size() > model->channels.size()) {
                for (Fixture* fixture : kernel->fixtures->items) {
                    if (fixture->model == model) {
                        for (int channel=(fixture->address + model->channels.size()); channel < (fixture->address + channels.size()); channel++) {
                            if (channel > 512) {
                                kernel->terminal->warning("Can't record Model Channels because this would result in an address conflict.");
                                addressConflict = true;
                            }
                            if (kernel->fixtures->usedChannels().contains(channel)) {
                                kernel->terminal->warning("Can't record Model Channels because this would result in an address conflict.");
                                addressConflict = true;
                            }
                        }
                    }
                }
            }
        }
        if (!addressConflict) {
            model->channels = channels;
            modelCounter++;
        }
    }
    kernel->terminal->success("Recorded " + QString::number(modelCounter) + " Models with channels \"" + channels + "\".");

}
