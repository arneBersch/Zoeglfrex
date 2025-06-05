/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "rawtable.h"

RawTable::RawTable(Kernel *core) : ItemTable(core, Keys::Raw, "Raw", "Raws") {
    boolAttributes[kernel->RAWMOVEINBLACKATTRIBUTEID] = {"MiB", true};
    boolAttributes[kernel->RAWFADEATTRIBUTEID] = {"Fade", false};
}

void RawTable::setAttribute(QStringList ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute.startsWith(kernel->RAWCHANNELVALUEATTRIBUTEID + ".")) {
        attribute.remove(0, QString(kernel->RAWCHANNELVALUEATTRIBUTEID + ".").length());
        bool ok = true;
        int channel = attribute.toInt(&ok);
        if (!ok) {
            kernel->terminal->error("Can't set Raw Channel Value because no valid Attribute was given.");
            return;
        }
        if ((channel < 1) || (channel > 512)) {
            kernel->terminal->error("Can't set Raw Channel Value because Channel has to be between 1 and 512.");
            return;
        }
        if ((value.size() == 1) && (value.first() == Keys::Minus)) {
            if (attributes.contains(Keys::Model)) {
                Model* model = kernel->models->getItem(attributes.value(Keys::Model));
                if (model == nullptr) {
                    kernel->terminal->error("Can't remove Raw Channel Value for Model " + attributes.value(Keys::Model) + " because Model does not exist.");
                    return;
                }
                for (QString id : ids) {
                    Raw* raw = getItem(id);
                    if (raw == nullptr) {
                        raw = addItem(id);
                    }
                    if (raw->modelSpecificChannelValues.contains(model)) {
                        raw->modelSpecificChannelValues[model].remove(channel);
                    }
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Removed Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + " of Model " + model->name() + ".");
                } else {
                    kernel->terminal->success("Removed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Model " + model->name() + ".");
                }
            } else if (attributes.contains(Keys::Fixture)) {
                Fixture* fixture = kernel->fixtures->getItem(attributes.value(Keys::Fixture));
                if (fixture == nullptr) {
                    kernel->terminal->error("Can't remove Raw Channel Value for Fixture " + attributes.value(Keys::Fixture) + " because Fixture does not exist.");
                    return;
                }
                for (QString id : ids) {
                    Raw* raw = getItem(id);
                    if (raw == nullptr) {
                        raw = addItem(id);
                    }
                    if (raw->fixtureSpecificChannelValues.contains(fixture)) {
                        raw->fixtureSpecificChannelValues[fixture].remove(channel);
                    }
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Removed Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + " of Fixture " + fixture->name() + ".");
                } else {
                    kernel->terminal->success("Removed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Fixture " + fixture->name() + ".");
                }
            } else {
                for (QString id : ids) {
                    Raw* raw = getItem(id);
                    if (raw == nullptr) {
                        raw = addItem(id);
                    }
                    raw->channelValues.remove(channel);
                }
                if (ids.size() == 1) {
                    kernel->terminal->success("Removed Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + ".");
                } else {
                    kernel->terminal->success("Removed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + ".");
                }
            }
        } else {
            bool difference = (!value.isEmpty() && (value.first() == Keys::Plus));
            if (difference) {
                value.removeFirst();
            }
            bool negativeValue = (!value.isEmpty() && (value.first() == Keys::Minus));
            if (negativeValue) {
                value.removeFirst();
            }
            int newValue = kernel->terminal->keysToValue(value);
            if (text.isEmpty()) {
                if (newValue < 0) {
                    kernel->terminal->error("Can't set Raw Channel Value because no valid Value was given.");
                    return;
                }
                if (negativeValue) {
                    newValue *= -1;
                }
            } else {
                bool ok = true;
                newValue = text.toInt(&ok);
                if (!ok) {
                    kernel->terminal->error("Can't set Raw Channel Value because no valid number was given.");
                    return;
                }
            }
            if (!difference && ((newValue < 0) || (newValue > 255))) {
                kernel->terminal->error("Can't set Raw Channel Value because Value has to be between 0 and 255.");
                return;
            }
            if (attributes.contains(Keys::Model)) {
                Model* model = kernel->models->getItem(attributes.value(Keys::Model));
                if (model == nullptr) {
                    kernel->terminal->error("Can't set Raw Channel Value for Model " + attributes.value(Keys::Model) + " because Model does not exist.");
                    return;
                }
                if (difference) {
                    for (QString id : ids) {
                        Raw* raw = getItem(id);
                        if (raw == nullptr) {
                            raw = addItem(id);
                        }
                        if (!raw->modelSpecificChannelValues.contains(model) || !raw->modelSpecificChannelValues.value(model).contains(channel)) {
                            raw->modelSpecificChannelValues[model][channel] = 0;
                            if (raw->channelValues.contains(channel)) {
                                raw->modelSpecificChannelValues[model][channel] = raw->channelValues.value(channel);
                            }
                        }
                        if ((raw->modelSpecificChannelValues.value(model).value(channel) + newValue) < 0) {
                            raw->modelSpecificChannelValues[model][channel] = 0;
                            kernel->terminal->warning("Can't decrease Channel Value of Raw " + raw->name() + " because Value must be at least 0.");
                        } else if ((raw->modelSpecificChannelValues.value(model).value(channel) + newValue) > 255) {
                            raw->modelSpecificChannelValues[model][channel] = 255;
                            kernel->terminal->warning("Can't increase Channel Values of Raw " + raw->name() + " because Value must not exceed 255.");
                        } else {
                            raw->modelSpecificChannelValues[model][channel] += newValue;
                        }
                        emit dataChanged(index(getItemRow(raw->id), 0), index(getItemRow(raw->id), 0), {Qt::DisplayRole, Qt::EditRole});
                    }
                    if (ids.size() == 1) {
                        kernel->terminal->success("Set Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + " of Model " + model->name() + " to " + QString::number(getItem(ids.first())->modelSpecificChannelValues.value(model).value(channel)) + ".");
                    } else {
                        kernel->terminal->success("Changed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Model " + model->name() + ".");
                    }
                } else {
                    for (QString id : ids) {
                        Raw* raw = getItem(id);
                        if (raw == nullptr) {
                            raw = addItem(id);
                        }
                        raw->modelSpecificChannelValues[model][channel] = (uint8_t)newValue;
                    }
                    if (ids.size() == 1) {
                        kernel->terminal->success("Set Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + " of Model " + model->name() + " to " + QString::number(newValue) + ".");
                    } else {
                        kernel->terminal->success("Set Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Model " + model->name() + " to " + QString::number(newValue) + ".");
                    }
                }
            } else if (attributes.contains(Keys::Fixture)) {
                Fixture* fixture = kernel->fixtures->getItem(attributes.value(Keys::Fixture));
                if (fixture == nullptr) {
                    kernel->terminal->error("Can't set Raw Channel Value for Fixture " + attributes.value(Keys::Fixture) + " because Fixture does not exist.");
                    return;
                }
                if (difference) {
                    for (QString id : ids) {
                        Raw* raw = getItem(id);
                        if (raw == nullptr) {
                            raw = addItem(id);
                        }
                        if (!raw->fixtureSpecificChannelValues.contains(fixture) || !raw->fixtureSpecificChannelValues.value(fixture).contains(channel)) {
                            raw->fixtureSpecificChannelValues[fixture][channel] = 0;
                            if (raw->modelSpecificChannelValues.contains(fixture->model) && raw->modelSpecificChannelValues.value(fixture->model).contains(channel)) {
                                raw->fixtureSpecificChannelValues[fixture][channel] = raw->modelSpecificChannelValues.value(fixture->model).value(channel);
                            } else if (raw->channelValues.contains(channel)) {
                                raw->fixtureSpecificChannelValues[fixture][channel] = raw->channelValues.value(channel);
                            }
                        }
                        raw->fixtureSpecificChannelValues[fixture][channel] += newValue;
                        if ((raw->fixtureSpecificChannelValues.value(fixture).value(channel) + newValue) < 0) {
                            raw->fixtureSpecificChannelValues[fixture][channel] = 0;
                            kernel->terminal->warning("Can't decrease Channel Value of Raw " + raw->name() + " because Value must be at least 0.");
                        } else if ((raw->fixtureSpecificChannelValues.value(fixture).value(channel) + newValue) > 255) {
                            raw->fixtureSpecificChannelValues[fixture][channel] = 255;
                            kernel->terminal->warning("Can't increase Channel Values of Raw " + raw->name() + " because Value must not exceed 255.");
                        } else {
                            raw->fixtureSpecificChannelValues[fixture][channel] += newValue;
                        }
                        emit dataChanged(index(getItemRow(raw->id), 0), index(getItemRow(raw->id), 0), {Qt::DisplayRole, Qt::EditRole});
                    }
                    if (ids.size() == 1) {
                        kernel->terminal->success("Set Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + " of Fixture " + fixture->name() + " to " + QString::number(getItem(ids.first())->fixtureSpecificChannelValues.value(fixture).value(channel)) + ".");
                    } else {
                        kernel->terminal->success("Changed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Fixture " + fixture->name() + ".");
                    }
                } else {
                    for (QString id : ids) {
                        Raw* raw = getItem(id);
                        if (raw == nullptr) {
                            raw = addItem(id);
                        }
                        raw->fixtureSpecificChannelValues[fixture][channel] = (uint8_t)newValue;
                    }
                    if (ids.size() == 1) {
                        kernel->terminal->success("Set Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + " of Fixture " + fixture->name() + " to " + QString::number(newValue) + ".");
                    } else {
                        kernel->terminal->success("Set Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Fixture " + fixture->name() + " to " + QString::number(newValue) + ".");
                    }
                }
            } else {
                if (difference) {
                    for (QString id : ids) {
                        Raw* raw = getItem(id);
                        if (raw == nullptr) {
                            raw = addItem(id);
                        }
                        if (!raw->channelValues.contains(channel)) {
                            raw->channelValues[channel] = 0;
                        }
                        if ((raw->channelValues.value(channel) + newValue) < 0) {
                            raw->channelValues[channel] = 0;
                            kernel->terminal->warning("Can't decrease Channel Value of Raw " + raw->name() + " because Value must be at least 0.");
                        } else if ((raw->channelValues.value(channel) + newValue) > 255) {
                            raw->channelValues[channel] = 255;
                            kernel->terminal->warning("Can't increase Channel Values of Raw " + raw->name() + " because Value must not exceed 255.");
                        } else {
                            raw->channelValues[channel] += newValue;
                        }
                        emit dataChanged(index(getItemRow(raw->id), 0), index(getItemRow(raw->id), 0), {Qt::DisplayRole, Qt::EditRole});
                    }
                    if (ids.size() == 1) {
                        kernel->terminal->success("Set Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + " to " + QString::number(getItem(ids.first())->channelValues.value(channel)) + ".");
                    } else {
                        kernel->terminal->success("Changed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + ".");
                    }
                } else {
                    for (QString id : ids) {
                        Raw* raw = getItem(id);
                        if (raw == nullptr) {
                            raw = addItem(id);
                        }
                        raw->channelValues[channel] = (uint8_t)newValue;
                    }
                    if (ids.size() == 1) {
                        kernel->terminal->success("Set Channel Value of Raw " + getItem(ids.first())->name() + " at Channel " + QString::number(channel) + " to " + QString::number(newValue) + ".");
                    } else {
                        kernel->terminal->success("Set Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " to " + QString::number(newValue) + ".");
                    }
                }
            }
        }
    } else if (attribute == kernel->RAWCHANNELVALUEATTRIBUTEID) {
        if ((value.size() != 1) || (value.first() != Keys::Minus)) {
            kernel->terminal->error("Can't set Raw Channel Value because no Channel was given.");
            return;
        }
        if (attributes.contains(Keys::Model)) {
            Model* model = kernel->models->getItem(attributes.value(Keys::Model));
            if (model == nullptr) {
                kernel->terminal->error("Can't remove Raw Channels for Model " + attributes.value(Keys::Model) + " because Model does not exist.");
                return;
            }
            for (QString id : ids) {
                Raw* raw = getItem(id);
                if (raw != nullptr) {
                    raw->modelSpecificChannelValues.remove(model);
                }
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed Channel Values of Raw " + getItem(ids.first())->name() + " at Model " + model->name() + ".");
            } else {
                kernel->terminal->success("Removed Channel Values of " + QString::number(ids.size()) + " Raws at Model " + model->name() + ".");
            }
        } else if (attributes.contains(Keys::Fixture)) {
            Fixture* fixture = kernel->fixtures->getItem(attributes.value(Keys::Fixture));
            if (fixture == nullptr) {
                kernel->terminal->error("Can't remove Raw Channels for Fixture " + attributes.value(Keys::Fixture) + " because Fixture does not exist.");
                return;
            }
            for (QString id : ids) {
                Raw* raw = getItem(id);
                if (raw != nullptr) {
                    raw->fixtureSpecificChannelValues.remove(fixture);
                }
            }
            if (ids.size() == 1) {
                kernel->terminal->success("Removed Channel Values of Raw " + getItem(ids.first())->name() + " at Fixture " + fixture->name() + ".");
            } else {
                kernel->terminal->success("Removed Channel Values of " + QString::number(ids.size()) + " Raws at Fixture " + fixture->name() + ".");
            }
        } else {
            kernel->terminal->error("Can't remove Raw Channel Values because no Model or Fixture was given.");
            return;
        }
    } else {
        ItemTable::setAttribute(ids, attributes, value, text);
    }
}

bool RawTable::isCurrentItem(Raw* item) const {
    if ((kernel->cuelistView->currentCuelist != nullptr) && (kernel->cuelistView->currentCuelist->currentCue != nullptr)) {
        return (kernel->cuelistView->currentCuelist->currentCue->raws.value(kernel->cuelistView->currentGroup, QList<Raw*>()).contains(item));
    }
    return false;
}

