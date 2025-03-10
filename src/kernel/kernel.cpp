/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "kernel.h"

Kernel::Kernel() {
    models = new ModelList(this);
    fixtures = new FixtureList(this);
    groups = new GroupList(this);
    intensities = new IntensityList(this);
    colors = new ColorList(this);
    raws = new RawList(this);
    cues = new CueList(this);
    terminal = new Terminal(this);
    inspector = new Inspector(this);
    cuelistView = new CuelistView(this);
    mutex = new QMutex();
}

void Kernel::execute(QList<int> command, QString text) {
    QMutexLocker locker(mutex);
    if(command.isEmpty()) {
        terminal->error("Command is empty.");
        return;
    }
    int selectionType = command.first();
    command.removeFirst();
    if (!isItem(selectionType)) {
        terminal->error("No Item Type specified.");
        return;
    }
    QList<int> selection;
    QList<int> attribute;
    QList<int> value;
    bool attributeReached = false;
    bool valueReached = false;
    for (const int key : command) {
        if (key == Keys::Set) {
            if (valueReached) {
                terminal->error("Can't use Set more than one time in one command.");
                return;
            }
            valueReached = true;
        } else if ((isItem(key) || (key == Keys::Attribute)) && !valueReached) {
            attribute.append(key);
            attributeReached = true;
        } else {
            if (valueReached) {
                value.append(key);
            } else if (attributeReached) {
                attribute.append(key);
            } else {
                selection.append(key);
            }
        }
    }
    QList<QString> ids = keysToSelection(selection, selectionType);
    QMap<int, QString> attributeMap = QMap<int, QString>();
    if (!valueReached && !attributeReached) {
        if (selectionType == Keys::Fixture) {
            if (cuelistView->currentGroup == nullptr) {
                terminal->error("Can't update Fixtures of Group because no Group is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = groups->FIXTURESATTRIBUTEID;
            QList<QString> groupIds;
            groupIds.append(cuelistView->currentGroup->id);
            command.insert(0, selectionType);
            groups->setAttribute(groupIds, attributeMap, command);
        } else if (selectionType == Keys::Group) {
            if (ids.size() != 1) {
                terminal->error("Can't select Group because Group only allows one Group ID.");
                return;
            }
            Group* group = groups->getItem(ids.first());
            if (group == nullptr) {
                group = groups->addItem(ids.first());
                terminal->success("Added and selected Group " + group->id);
            } else {
                terminal->success("Selected Group " + group->id);
            }
            cuelistView->currentGroup = group;
        } else if (selectionType == Keys::Intensity) {
            if (!cuelistView->validGroupAndCue()) {
                return;
            }
            attributeMap[Keys::Attribute] = cues->INTENSITIESATTRIBUTEID;
            attributeMap[Keys::Group] = cuelistView->currentGroup->id;
            QList<QString> cueIds;
            cueIds.append(cuelistView->currentCue->id);
            command.insert(0, selectionType);
            cues->setAttribute(cueIds, attributeMap, command);
        } else if (selectionType == Keys::Color) {
            if (!cuelistView->validGroupAndCue()) {
                return;
            }
            attributeMap[Keys::Attribute] = cues->COLORSATTRIBUTEID;
            attributeMap[Keys::Group] = cuelistView->currentGroup->id;
            QList<QString> cueIds;
            cueIds.append(cuelistView->currentCue->id);
            command.insert(0, selectionType);
            cues->setAttribute(cueIds, attributeMap, command);
        } else if (selectionType == Keys::Raw) {
            if (!cuelistView->validGroupAndCue()) {
                return;
            }
            attributeMap[Keys::Attribute] = cues->RAWSATTRIBUTEID;
            attributeMap[Keys::Group] = cuelistView->currentGroup->id;
            QList<QString> cueIds;
            cueIds.append(cuelistView->currentCue->id);
            command.insert(0, selectionType);
            cues->setAttribute(cueIds, attributeMap, command);
        } else if (selectionType == Keys::Cue) {
            if (ids.size() != 1) {
                terminal->error("Can't select Cue because Cue only allows one Cue ID.");
                return;
            }
            Cue* cue = cues->getItem(ids.first());
            if (cue == nullptr) {
                cue = cues->addItem(ids.first());
                terminal->success("Added and selected Cue " + cue->id);
            } else {
                terminal->success("Selected Cue " + cue->id);
            }
            cuelistView->currentCue = cue;
        } else {
            terminal->error("No Attribute and Value were given.");
            return;
        }
        cuelistView->loadCue();
        return;
    }
    if (selection.isEmpty()) {
        if (selectionType == Keys::Fixture) {
            if (cuelistView->currentGroup == nullptr) {
                terminal->error("Can't load the Fixtures of the current because no Group is selected.");
                return;
            }
            for (Fixture* fixture : fixtures->items) {
                ids.append(fixture->id);
            }
        } else if (selectionType == Keys::Group) {
            if (cuelistView->currentGroup == nullptr) {
                terminal->error("Can't load the Group because no Group is selected.");
                return;
            }
            ids.append(cuelistView->currentGroup->id);
        } else if (selectionType == Keys::Intensity) {
            if (!cuelistView->validGroupAndCue()) {
                return;
            }
            if (!cuelistView->currentCue->groupSpecificIntensityAttributes.value(cues->INTENSITIESATTRIBUTEID).contains(cuelistView->currentGroup)) {
                terminal->error("Can't load the Intensity as the selected Cue contains no Intensity for this Group.");
                return;
            }
            ids.append(cuelistView->currentCue->groupSpecificIntensityAttributes.value(cues->INTENSITIESATTRIBUTEID).value(cuelistView->currentGroup)->id);
        } else if (selectionType == Keys::Color) {
            if (!cuelistView->validGroupAndCue()) {
                return;
            }
            if (!cuelistView->currentCue->colors.contains(cuelistView->currentGroup)) {
                terminal->error("Can't load the Color as the selected Cue contains no Color for this Group.");
                return;
            }
            ids.append(cuelistView->currentCue->colors.value(cuelistView->currentGroup)->id);
        } else if (selectionType == Keys::Raw) {
            if (!cuelistView->validGroupAndCue()) {
                return;
            }
            if (!cuelistView->currentCue->raws.contains(cuelistView->currentGroup) || cuelistView->currentCue->raws[cuelistView->currentGroup].isEmpty()) {
                terminal->error("Can't load the Raws as the selected Cue contains no Raws for this Group.");
                return;
            }
            for (Raw* raw :  cuelistView->currentCue->raws[cuelistView->currentGroup]) {
                ids.append(raw->id);
            }
        } else if (selectionType == Keys::Cue) {
            if (cuelistView->currentCue == nullptr) {
                terminal->error("Can't load the Cue because no Cue is selected.");
                return;
            }
            ids.append(cuelistView->currentCue->id);
        } else {
            terminal->error("No selection given.");
            return;
        }
    }
    if (ids.isEmpty()) {
        terminal->error("No items selected.");
        return;
    }
    int currentItemType = Keys::Attribute;
    QList<int> currentId;
    for (int attributeKey : attribute) {
        if (isItem(attributeKey) || (attributeKey == Keys::Attribute)) {
            if (!currentId.isEmpty()) {
                QString currentIdString = keysToId(currentId);
                attributeMap[currentItemType] = currentIdString;
            }
            currentId.clear();
            currentItemType = attributeKey;
        } else if (isNumber(attributeKey) || (attributeKey == Keys::Period)) {
            currentId.append(attributeKey);
        } else {
            terminal->error("Invalid key in Attribute.");
            return;
        }
    }
    if (!currentId.isEmpty()) {
        QString currentIdString = keysToId(currentId);
        attributeMap[currentItemType] = currentIdString;
    }
    if ((attributeMap.value(Keys::Attribute) == models->LABELATTRIBUTEID) && text.isNull()) {
        bool ok = false;
        locker.unlock();
        text = QInputDialog::getText(terminal, QString(), "Label", QLineEdit::Normal, QString(), &ok);
        locker.relock();
        if (!ok) {
            terminal->warning("Popup cancelled.");
            return;
        }
    }
    bool standardAttribute = (!attributeMap.contains(Keys::Attribute) && !(attributeMap.isEmpty() && !value.isEmpty() && (((value.size() == 1) && (value.first() == Keys::Minus)) || (value.first() == selectionType))));
    if (selectionType == Keys::Model) {
        if (standardAttribute) {
            attributeMap[Keys::Attribute] = models->CHANNELSATTRIBUTEID;
        }
        if ((attributeMap.value(Keys::Attribute) == models->CHANNELSATTRIBUTEID) && text.isNull()) {
            bool ok = false;
            locker.unlock();
            text = QInputDialog::getText(terminal, QString(), "Channels", QLineEdit::Normal, QString(), &ok);
            locker.relock();
            if (!ok) {
                terminal->warning("Popup cancelled.");
                return;
            }
        }
        models->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Fixture) {
        if (standardAttribute) {
            attributeMap[Keys::Attribute] = fixtures->ADDRESSATTRIBUTEID;
        }
        fixtures->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Group) {
        if (standardAttribute) {
            attributeMap[Keys::Attribute] = groups->FIXTURESATTRIBUTEID;
        }
        groups->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Intensity) {
        if (standardAttribute) {
            attributeMap[Keys::Attribute] = intensities->DIMMERATTRIBUTEID;
        }
        intensities->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Color) {
        if (standardAttribute) {
            attributeMap[Keys::Attribute] = colors->HUEATTRIBUTEID;
        }
        colors->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Raw) {
        if (standardAttribute) {
            attributeMap[Keys::Attribute] = raws->VALUEATTRIBUTEID;
        }
        raws->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Cue) {
        if (standardAttribute) {
            attributeMap[Keys::Attribute] = cues->FADEATTRIBUTEID;
        }
        cues->setAttribute(ids, attributeMap, value, text);
    }
    cuelistView->loadCue();
}

void Kernel::reset() {
    for (Model* model : models->items) {
        delete model;
    }
    models->items.clear();
    for (Fixture* fixture : fixtures->items) {
        delete fixture;
    }
    fixtures->items.clear();
    for (Group* group : groups->items) {
        delete group;
    }
    groups->items.clear();
    for (Intensity* intensity : intensities->items) {
        delete intensity;
    }
    intensities->items.clear();
    for (Color* color : colors->items) {
        delete color;
    }
    colors->items.clear();
    for (Raw* raw : raws->items) {
        delete raw;
    }
    raws->items.clear();
    QList<Cue*> cueItems = cues->items;
    cues->items.clear();
    for (Cue* cue : cueItems) {
        delete cue;
    }

    cuelistView->dmxEngine->sacnServer->universeSpinBox->setValue(cuelistView->dmxEngine->sacnServer->SACN_STANDARD_UNIVERSE); // reset sACN universe
    cuelistView->dmxEngine->sacnServer->prioritySpinBox->setValue(cuelistView->dmxEngine->sacnServer->SACN_STANDARD_PRIORITY); // reset sACN priority
    cuelistView->loadCue();
}

QString Kernel::keysToId(QList<int> keys, bool removeTrailingZeros) {
    QString id;
    int number = 0;
    for (const int key : keys) {
        if (isNumber(key)) {
            number = (number * 10) + keyToNumber(key);
        } else if (key == Keys::Period) {
            id += QString::number(number) + ".";
            number = 0;
        } else {
            return QString();
        }
    }
    id += QString::number(number);
    if (removeTrailingZeros) {
        while (id.endsWith(".0")) {
            id.chop(2);
        }
    }
    return id;
}

float Kernel::keysToValue(QList<int> keys) {
    QString value;
    for (const int key : keys) {
        if (isNumber(key)) {
            value += QString::number(keyToNumber(key));
        } else if (key == Keys::Period) {
            value += ".";
        } else {
            return -1;
        }
    }
    bool ok;
    float valueFloat = value.toFloat(&ok);
    if (!ok) {
        return -1;
    }
    return valueFloat;
}

QList<QString> Kernel::keysToSelection(QList<int> keys, int itemType) {
    QList<QString> allIds;
    if (itemType == Keys::Model) {
        allIds = models->getIds();
    } else if (itemType == Keys::Fixture) {
        allIds = fixtures->getIds();
    } else if (itemType == Keys::Group) {
        allIds = groups->getIds();
    } else if (itemType == Keys::Intensity) {
        allIds = intensities->getIds();
    } else if (itemType == Keys::Color) {
        allIds = colors->getIds();
    } else if (itemType == Keys::Raw) {
        allIds = raws->getIds();
    } else if (itemType == Keys::Cue) {
        allIds = cues->getIds();
    } else {
        return QList<QString>();
    }
    if (keys.isEmpty()) {
        return QList<QString>();
    }
    if (!keys.endsWith(Keys::Plus)) {
        keys.append(Keys::Plus);
    }
    QList<QString> ids;
    QList<int> idKeys;
    QList<int> thruBuffer;
    bool idAdding = true;
    for (int key : keys) {
        if (isNumber(key) || key == Keys::Period) {
            idKeys += key;
        } else if ((key == Keys::Plus) || (key == Keys::Minus)) {
            if (!thruBuffer.isEmpty()) {
                if (idKeys.startsWith(Keys::Period)) {
                    idKeys.removeFirst();
                    if (idKeys.contains(Keys::Period)) {
                        return QList<QString>();
                    }
                    int maxId = keysToId(idKeys).toInt();
                    QString idBeginning = keysToId(thruBuffer, false);
                    int minId = idBeginning.split('.').last().toInt();
                    if (idBeginning.contains('.')) {
                        idBeginning = idBeginning.left(idBeginning.lastIndexOf(QChar('.'))) + ".";
                    } else {
                        idBeginning = QString();
                    }
                    if (minId >= maxId) {
                        return QList<QString>();
                    }
                    for (int counter = minId; counter <= maxId; counter++) {
                        QString id = idBeginning + QString::number(counter);
                        if (idAdding) {
                            ids.append(id);
                        } else {
                            ids.removeAll(id);
                        }
                    }
                } else {
                    QString firstId = keysToId(thruBuffer);
                    if (firstId.isEmpty() || !allIds.contains(firstId)) {
                        return QList<QString>();
                    }
                    QString lastId = keysToId(idKeys);
                    if (lastId.isEmpty() || !allIds.contains(lastId)) {
                        return QList<QString>();
                    }
                    int firstIdRow = allIds.indexOf(firstId);
                    int lastIdRow = allIds.indexOf(lastId);
                    if (firstIdRow >= lastIdRow) {
                        return QList<QString>();
                    }
                    for (int idRow = firstIdRow; idRow <= lastIdRow; idRow++) {
                        if (idAdding) {
                            ids.append(allIds[idRow]);
                        } else {
                            ids.removeAll(allIds[idRow]);
                        }
                    }
                }
                thruBuffer.clear();
                idKeys.clear();
            } else if (idKeys.endsWith(Keys::Period)) {
                idKeys.removeLast();
                if (idKeys.isEmpty()) {
                    if (idAdding) {
                        ids.append(allIds);
                    } else {
                        for (QString id : allIds) {
                            ids.removeAll(id);
                        }
                    }
                } else {
                    QString idStart = keysToId(idKeys, false);
                    for (QString id : allIds) {
                        if ((id.startsWith(idStart + ".")) || (idStart == id)) {
                            if (idAdding) {
                                ids.append(id);
                            } else {
                                ids.removeAll(id);
                            }
                        }
                    }
                }
            } else {
                QString id = keysToId(idKeys);
                if (id.isEmpty()) {
                    return QList<QString>();
                }
                if (idAdding) {
                    ids.append(id);
                } else {
                    ids.removeAll(id);
                }
            }
            idKeys.clear();
            idAdding = (key == Keys::Plus);
        } else if (key == Keys::Thru) {
            if (!thruBuffer.isEmpty()) {
                return QList<QString>();
            }
            thruBuffer = idKeys;
            idKeys.clear();
        } else {
            return QList<QString>();
        }
    }
    return ids;
}

bool Kernel::isItem(int key) {
    return (
        (key == Keys::Model) ||
        (key == Keys::Fixture) ||
        (key == Keys::Group) ||
        (key == Keys::Intensity) ||
        (key == Keys::Color) ||
        (key == Keys::Raw) ||
        (key == Keys::Cue)
    );
}

bool Kernel::isNumber(int key) {
    return (
        (key == Keys::Zero) ||
        (key == Keys::One) ||
        (key == Keys::Two) ||
        (key == Keys::Three) ||
        (key == Keys::Four) ||
        (key == Keys::Five) ||
        (key == Keys::Six) ||
        (key == Keys::Seven) ||
        (key == Keys::Eight) ||
        (key == Keys::Nine)
        );
}

int Kernel::keyToNumber(int key) {
    if (key == Keys::Zero) {
        return 0;
    } else if (key == Keys::One) {
        return 1;
    } else if (key == Keys::Two) {
        return 2;
    } else if (key == Keys::Three) {
        return 3;
    } else if (key == Keys::Four) {
        return 4;
    } else if (key == Keys::Five) {
        return 5;
    } else if (key == Keys::Six) {
        return 6;
    } else if (key == Keys::Seven) {
        return 7;
    } else if (key == Keys::Eight) {
        return 8;
    } else if (key == Keys::Nine) {
        return 9;
    } else {
        return false;
    }
}

bool Kernel::patchOkay() {
    QSet<int> channels;
    for (Fixture* fixture : fixtures->items) {
        if (fixture->address > 0) {
            int fixtureChannels = 1;
            if (fixture->model != nullptr) {
                fixtureChannels = fixture->model->channels.size();
            }
            for (int channel = fixture->address; channel < (fixture->address + fixtureChannels); channel++) {
                if (channel > 512) {
                    terminal->warning("Patch Conflict detected: Fixture " + fixture->name() + " would have channels greater than 512.");
                    return false;
                }
                if (channels.contains(channel)) {
                    terminal->warning("Patch Conflict detected: Channel " + QString::number(channel) + " would be used twice.");
                    return false;
                }
                channels.insert(channel);
            }
        }
    }
    return true;
}
