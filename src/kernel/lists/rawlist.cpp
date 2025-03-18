/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "rawlist.h"

RawList::RawList(Kernel *core) : ItemList(Keys::Raw, "Raw", "Raws") {
    kernel = core;
}

void RawList::setAttribute(QList<QString> ids, QMap<int, QString> attributes, QList<int> value, QString text) {
    QString attribute = attributes.value(Keys::Attribute);
    if (attribute.startsWith(CHANNELVALUEATTRIBUTEID + ".")) {
        attribute.remove(0, QString(CHANNELVALUEATTRIBUTEID + ".").length());
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
                kernel->terminal->success("Removed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Model " + model->id + ".");
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
                kernel->terminal->success("Removed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Fixture " + fixture->id + ".");
            } else {
                for (QString id : ids) {
                    Raw* raw = getItem(id);
                    if (raw == nullptr) {
                        raw = addItem(id);
                    }
                    raw->channelValues.remove(channel);
                }
                kernel->terminal->success("Removed Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + ".");
            }
        } else {
            int newValue = kernel->terminal->keysToValue(value);
            if (text.isEmpty()) {
                if (newValue < 0) {
                    kernel->terminal->error("Can't set Raw Channel Value because no valid Value was given.");
                    return;
                }
            } else {
                bool ok = true;
                newValue = text.toInt(&ok);
                if (!ok) {
                    kernel->terminal->error("Can't set Raw Channel Value because no valid number was given.");
                    return;
                }
            }
            if ((newValue < 0) || (newValue > 255)) {
                kernel->terminal->error("Can't set Raw Channel Value because Value has to be between 0 and 255.");
                return;
            }
            if (attributes.contains(Keys::Model)) {
                Model* model = kernel->models->getItem(attributes.value(Keys::Model));
                if (model == nullptr) {
                    kernel->terminal->error("Can't set Raw Channel Value for Model " + attributes.value(Keys::Model) + " because Model does not exist.");
                    return;
                }
                for (QString id : ids) {
                    Raw* raw = getItem(id);
                    if (raw == nullptr) {
                        raw = addItem(id);
                    }
                    raw->modelSpecificChannelValues[model][channel] = (uint8_t)newValue;
                }
                kernel->terminal->success("Set Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Model " + model->id + " to " + QString::number(newValue) + ".");
            } else if (attributes.contains(Keys::Fixture)) {
                Fixture* fixture = kernel->fixtures->getItem(attributes.value(Keys::Fixture));
                if (fixture == nullptr) {
                    kernel->terminal->error("Can't set Raw Channel Value for Fixture " + attributes.value(Keys::Fixture) + " because Fixture does not exist.");
                    return;
                }
                for (QString id : ids) {
                    Raw* raw = getItem(id);
                    if (raw == nullptr) {
                        raw = addItem(id);
                    }
                    raw->fixtureSpecificChannelValues[fixture][channel] = (uint8_t)newValue;
                }
                kernel->terminal->success("Set Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " of Fixture " + fixture->id + " to " + QString::number(newValue) + ".");
            } else {
                for (QString id : ids) {
                    Raw* raw = getItem(id);
                    if (raw == nullptr) {
                        raw = addItem(id);
                    }
                    raw->channelValues[channel] = (uint8_t)newValue;
                }
                kernel->terminal->success("Set Channel Value of " + QString::number(ids.size()) + " Raws at Channel " + QString::number(channel) + " to " + QString::number(newValue) + ".");
            }
        }
    } else {
        ItemList::setAttribute(ids, attributes, value, text);
    }
}
