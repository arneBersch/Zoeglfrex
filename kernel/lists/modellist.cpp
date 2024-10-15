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

Model* ModelList::getModel(QString id)
{
    int modelRow = getModelRow(id);
    if (modelRow < 0 || modelRow >= models.size()) {
        return nullptr;
    }
    return models[modelRow];
}

int ModelList::getModelRow(QString id)
{
    for (int modelRow = 0; modelRow < models.size(); modelRow++) {
        if (models[modelRow]->id == id) {
            return modelRow;
        }
    }
    return -1;
}

QString ModelList::copyModel(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        Model* model = getModel(id);
        if (model == nullptr) {
            return "Model can't be copied because it doesn't exist.";
        }
        if (getModel(targetId) != nullptr) {
            return "Model can't be copied because Target ID is already used.";
        }
        Model *targetModel = recordModel(targetId);
        targetModel->label = model->label;
        targetModel->channels = model->channels;
    }
    return QString();
}

QString ModelList::deleteModel(QList<QString> ids)
{
    for (QString id : ids) {
        int modelRow = getModelRow(id);
        if (modelRow < 0) {
            return "Model can't be deleted because it doesn't exist.";
        }
        Model *model= models[modelRow];
        kernel->fixtures->deleteModel(model);
        models.removeAt(modelRow);
        delete model;
    }
    return QString();
}

QString ModelList::labelModel(QList<QString> ids, QString label)
{
    for (QString id : ids) {
        Model* model = getModel(id);
        if (model == nullptr) {
            return "Model can't be labeled because it doesn't exist.";
        }
        model->label = label;
    }
    return QString();
}

QString ModelList::moveModel(QList<QString> ids, QString targetId)
{
    for (QString id : ids) {
        int modelRow = getModelRow(id);
        if (modelRow < 0) {
            return "Model can't be moved because it doesn't exist.";
        }
        if (getModel(targetId) != nullptr) {
            return "Model can't be moved because Target ID is already used.";
        }
        Model* model = models[modelRow];
        models.removeAt(modelRow);
        model->id = targetId;
        QList<QString> idParts = targetId.split(".");
        int position = 0;
        for (int index=0; index < models.size(); index++) {
            QList<QString> indexIdParts = (models[index]->id).split(".");
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
        models.insert(position, model);
    }
    return QString();
}

Model* ModelList::recordModel(QString id)
{
    Model *model = new Model;
    model->id = id;
    model->label = QString();
    model->channels = "D";
    QList<QString> idParts = id.split(".");
    int position = 0;
    for (int index=0; index < models.size(); index++) {
        QList<QString> indexIdParts = (models[index]->id).split(".");
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
    models.insert(position, model);
    return model;
}

QString ModelList::recordModelChannels(QList<QString> ids, QString channels)
{
    if (!channels.contains(QRegularExpression("^[DRGB]+$"))) {
        return tr("Channels \"%1\" are not valid.").arg(channels);
    }
    for (QString id : ids) {
        Model* model = getModel(id);
        if (model == nullptr) {
            model = recordModel(id);
        } else {
            if (channels.size() > model->channels.size()) {
                for (QString fixtureId : kernel->fixtures->getIds()) {
                    Fixture *fixture = kernel->fixtures->getFixture(fixtureId);
                    if (fixture->model == model) {
                        for (int channel=(fixture->address + model->channels.size()); channel < (fixture->address + channels.size()); channel++) {
                            if (channel > 512) {
                                return "Can't record Model Channels because this would result in an address conflict.";
                            }
                            if (kernel->fixtures->usedChannels().contains(channel)) {
                                return "Can't record Model Channels because this would result in an address conflict.";
                            }
                        }
                    }
                }
            }
        }
        model->channels = channels;
    }
    return QString();
}

QList<QString> ModelList::getIds() {
    QList<QString> ids;
    for (Model *model : models) {
        ids.append(model->id);
    }
    return ids;
}

int ModelList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return models.size();
}

int ModelList::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant ModelList::data(const QModelIndex &index, const int role) const
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
        if (column == ModelListColumns::id) {
            return models[row]->id;
        } else if (column == ModelListColumns::label) {
            return models[row]->label;
        } else if (column == ModelListColumns::channels) {
            return models[row]->channels;
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

QVariant ModelList::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        if (column == ModelListColumns::id) {
            return "ID";
        } else if (column == ModelListColumns::label) {
            return "Label";
        } else if (column == ModelListColumns::channels) {
            return "Channels";
        } else {
            return QVariant();
        }
    }
    return QVariant();
}
