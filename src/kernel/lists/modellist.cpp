/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "modellist.h"

ModelList::ModelList(Kernel *core) {
    kernel = core;
}

bool ModelList::copyItems(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        Model* model = getItem(id);
        if (model == nullptr) {
            kernel->terminal->error("Model can't be copied because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Model can't be copied because Target ID is already used.");
            return false;
        }
        Model *targetModel = recordModel(targetId);
        targetModel->label = model->label;
        targetModel->channels = model->channels;
    }
    return true;
}

bool ModelList::deleteItems(QList<QString> ids)
{
    for (QString id : ids) {
        int modelRow = getItemRow(id);
        if (modelRow < 0) {
            kernel->terminal->error("Model can't be deleted because it doesn't exist.");
            return false;
        }
        Model *model= items[modelRow];
        kernel->fixtures->deleteModel(model);
        items.removeAt(modelRow);
        delete model;
    }
    return true;
}

bool ModelList::moveItems(QList<QString> ids, QString targetId) {
    for (QString id : ids) {
        int modelRow = getItemRow(id);
        if (modelRow < 0) {
            kernel->terminal->error("Model can't be moved because it doesn't exist.");
            return false;
        }
        if (getItem(targetId) != nullptr) {
            kernel->terminal->error("Model can't be moved because Target ID is already used.");
            return false;
        }
        Model* model = items[modelRow];
        items.removeAt(modelRow);
        model->id = targetId;
        int position = 0;
        for (int index=0; index < items.size(); index++) {
            if (greaterId(items[index]->id, targetId)) {
                position++;
            }
        }
        items.insert(position, model);
    }
    return true;
}

Model* ModelList::recordModel(QString id) {
    Model *model = new Model;
    model->id = id;
    model->label = QString();
    model->channels = "D";
    int position = 0;
    for (int index=0; index < items.size(); index++) {
        if (greaterId(items[index]->id, id)) {
            position++;
        }
    }
    items.insert(position, model);
    return model;
}

bool ModelList::recordModelChannels(QList<QString> ids, QString channels) {
    if (!channels.contains(QRegularExpression("^[DRGB]+$"))) {
        kernel->terminal->error("Channels \"" + channels + "\" are not valid.");
        return false;
    }
    for (QString id : ids) {
        Model* model = getItem(id);
        if (model == nullptr) {
            model = recordModel(id);
        } else {
            if (channels.size() > model->channels.size()) {
                for (QString fixtureId : kernel->fixtures->getIds()) {
                    Fixture *fixture = kernel->fixtures->getItem(fixtureId);
                    if (fixture->model == model) {
                        for (int channel=(fixture->address + model->channels.size()); channel < (fixture->address + channels.size()); channel++) {
                            if (channel > 512) {
                                kernel->terminal->error("Can't record Model Channels because this would result in an address conflict.");
                                return false;
                            }
                            if (kernel->fixtures->usedChannels().contains(channel)) {
                                kernel->terminal->error("Can't record Model Channels because this would result in an address conflict.");
                                return false;
                            }
                        }
                    }
                }
            }
        }
        model->channels = channels;
    }
    return true;
}
