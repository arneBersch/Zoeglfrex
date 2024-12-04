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
    cues = new CueList(this);
    terminal = new Terminal(this);
    inspector = new Inspector(this);
    cuelistView = new CuelistView(this);
    mutex = new QMutex();
}

void Kernel::execute(QList<int> command, QString text) {
    QMutexLocker locker(mutex);
    if(command.size() < 1) { // if command is empty
        terminal->error("Command is empty.");
        return;
    }
    int selectionType = command[0];
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
        } else if (key == Keys::Attribute) {
            if (attributeReached) {
                terminal->error("Can't give two attributes.");
                return;
            }
            if (valueReached) {
                terminal->error("Can't give attribute after value.");
                return;
            }
            attributeReached = true;
        } else {
            if (valueReached) { // if the selection is finished
                value.append(key);
            } else if (attributeReached) {
                attribute.append(key);
            } else {
                selection.append(key);
            }
        }
    }
    if (selection.isEmpty()) {
        terminal->error("No selection given.");
        return;
    }
    QList<QString> ids = keysToSelection(selection, selectionType);
    if (ids.isEmpty()) {
        terminal->error("No items selected.");
        return;
    }
    float valueFloat = keysToValue(value);
    if (attribute.isEmpty() && !value.isEmpty() && (value.first() == selectionType)) { // COPY
        value.removeFirst();
        QString sourceId = keysToId(value);
        if (sourceId.isEmpty()) {
            terminal->error("Target ID not valid.");
            return;
        }
        if (selectionType == Keys::Model) { // COPY MODEL
            models->copyItems(ids, sourceId);
        } else if (selectionType == Keys::Fixture) { // COPY FIXTURE
            fixtures->copyItems(ids, sourceId);
        } else if (selectionType == Keys::Group) { // COPY GROUP
            groups->copyItems(ids, sourceId);
        } else if (selectionType == Keys::Intensity) { // COPY INTENSITY
            intensities->copyItems(ids, sourceId);
        } else if (selectionType == Keys::Color) { // COPY COLOR
            colors->copyItems(ids, sourceId);
        } else if (selectionType == Keys::Cue) { // COPY CUE
            cues->copyItems(ids, sourceId);
        }
    } else if ((value.size() == 1) && (value.first() == Keys::Minus) && (attribute.isEmpty() || selectionType == Keys::Cue)) { // DELETE
        if (selectionType == Keys::Model) { // DELETE MODEL
            models->deleteItems(ids);
        } else if (selectionType == Keys::Fixture) { // DELETE FIXTURE
            fixtures->deleteItems(ids);
        } else if (selectionType == Keys::Group) { // DELETE GROUP
            groups->deleteItems(ids);
        } else if (selectionType == Keys::Intensity) { // DELETE INTENSITY
            intensities->deleteItems(ids);
        } else if (selectionType == Keys::Color) { // DELETE COLOR
            colors->deleteItems(ids);
        } else if (selectionType == Keys::Cue) { // DELETE CUE
            if (attribute.isEmpty()) {
                cues->deleteItems(ids);
            } else {
                int attributeId = 0;
                if (attribute[0] == Keys::Two) {
                    attributeId = 2;
                } else if (attribute[0] == Keys::Three) {
                    attributeId = 3;
                } else {
                    terminal->error("Cue Attribute Set - requires attribute 2 or 3.");
                    return;
                }
                attribute.removeFirst();
                if (attribute.isEmpty()) {
                    terminal->error("Cue Attribute " + QString::number(attributeId) + " Set - requires Group ID.");
                    return;
                }
                if (attribute[0] != Keys::Group) {
                    terminal->error("Cue Attribute " + QString::number(attributeId) + " Set - requires Group ID.");
                    return;
                }
                attribute.removeFirst();
                QString groupId = keysToId(attribute);
                if (groupId.isEmpty()) {
                    terminal->error("Can't delete Cue Attribute " + QString::number(attributeId) + " Group: No valid Group ID given.");
                    return;
                }
                if (attributeId == 2) {
                    cues->deleteCueGroupIntensity(ids, groupId);
                } else if (attributeId == 3) {
                    cues->deleteCueGroupColor(ids, groupId);
                }
            }
        }
    } else if ((attribute.size() == 1) && (attribute[0] == Keys::One)) { // LABEL
        if (!value.isEmpty()) {
            terminal->error("Attribute 1 Set doesn't take any parameters.");
            return;
        }
        QString label = text;
        if (text.isNull()) {
            bool ok = false;
            locker.unlock();
            label = QInputDialog::getText(terminal, QString(), "Label", QLineEdit::Normal, QString(), &ok);
            locker.relock();
            if (!ok) {
                terminal->warning("Popup cancelled.");
                return;
            }
        }
        label.replace("\"", "");
        if (selectionType == Keys::Model) { // LABEL MODEL
            models->labelItems(ids, label);
        } else if (selectionType == Keys::Fixture) { // LABEL FIXTURE
            fixtures->labelItems(ids, label);
        } else if (selectionType == Keys::Group) { // LABEL GROUP
            groups->labelItems(ids, label);
        } else if (selectionType == Keys::Intensity) { // LABEL INTENSITY
            intensities->labelItems(ids, label);
        } else if (selectionType == Keys::Color) { // LABEL COLOR
            colors->labelItems(ids, label);
        } else if (selectionType == Keys::Cue) { // LABEL CUE
            cues->labelItems(ids, label);
        }
    } else if ((attribute.size() == 1) && (attribute[0] == Keys::Zero)) { // MOVE
        QString targetId = keysToId(value);
        if (targetId.isEmpty()) {
            terminal->error("Target ID not valid.");
            return;
        }
        if (selectionType == Keys::Model) { // MOVE MODEL
            models->moveItems(ids, targetId);
        } else if (selectionType == Keys::Fixture) { // MOVE FIXTURE
            fixtures->moveItems(ids, targetId);
        } else if (selectionType == Keys::Group) { // MOVE GROUP
            groups->moveItems(ids, targetId);
        } else if (selectionType == Keys::Intensity) { // MOVE INTENSITY
            intensities->moveItems(ids, targetId);
        } else if (selectionType == Keys::Color) { // MOVE COLOR
            colors->moveItems(ids, targetId);
        } else if (selectionType == Keys::Cue) { // MOVE CUE
            cues->moveItems(ids, targetId);
        }
    } else { // RECORD
        if (selectionType == Keys::Model) { // RECORD MODEL
            if (attribute.isEmpty()) {
                attribute.append(Keys::Two);
            }
            if ((attribute.length() != 1) || attribute[0] != Keys::Two) {
                terminal->error("Model Attribute Set only allows Attribute 0, 1 or 2.");
                return;
            }
            if (!value.isEmpty()) {
                terminal->error("Model Attribute 2 Set doesn't take a value.");
                return;
            }
            QString channels = text;
            if (text.isNull()) {
                bool ok = false;
                locker.unlock();
                channels = QInputDialog::getText(terminal, QString(), "Channels", QLineEdit::Normal, QString(), &ok);
                locker.relock();
                if (!ok) {
                    terminal->warning("Popup cancelled.");
                    return;
                }
            }
            models->recordModelChannels(ids, channels);
        } else if (selectionType == Keys::Fixture) { // RECORD FIXTURE
            if (attribute.isEmpty()) {
                attribute.append(Keys::Three);
            }
            if (attribute.length() != 1 || ((attribute[0] != Keys::Two) && (attribute[0] != Keys::Three))) {
                terminal->error("Fixture Attribute Set only allows Attributes 0, 1, 2, 3");
                return;
            }
            if (attribute[0] == Keys::Two) {
                if (value[0] != Keys::Model) {
                    terminal->error("Fixture Attribute 2 Set requires a model");
                    return;
                }
                value.removeFirst();
                QString modelId = keysToId(value);
                if (modelId.isEmpty()) {
                    terminal->error("Can't set Model of Fixtures because Model ID is not vaild.");
                    return;
                }
                fixtures->recordFixtureModel(ids, modelId);
            } else if (attribute[0] == Keys::Three) {
                int address = 0;
                for (int key : value) {
                    if (!isNumber(key)) {
                        terminal->error("Fixture Attribute 3 Set only allows numbers as a value.");
                        return;
                    }
                    address = (address * 10) + keyToNumber(key);
                }
                fixtures->recordFixtureAddress(ids, address);
            }
        } else if (selectionType == Keys::Group) { // RECORD GROUP
            if (attribute.isEmpty()) {
                attribute.append(Keys::Two);
            }
            if (attribute.length() != 1) {
                terminal->error("Group Attribute Set only allows Attributes 0, 1 or 2.");
                return;
            }
            if (attribute[0] != Keys::Two) {
                terminal->error("Group Attribute Set only allows Attributes 0, 1 or 2.");
                return;
            }
            if (value.isEmpty()) {
                groups->recordGroupFixtures(ids, QList<QString>());
            } else {
                if (value[0] != Keys::Fixture) {
                    terminal->error("Group Attribute 2 Set requires Fixtures.");
                    return;
                }
                value.removeFirst();
                QList<QString> fixtureIds = keysToSelection(value, Keys::Fixture);
                if (fixtureIds.isEmpty()) {
                    terminal->error("Can't record Group because of invalid Fixture selection.");
                    return;
                }
                groups->recordGroupFixtures(ids, fixtureIds);
            }
        } else if (selectionType == Keys::Intensity) { // RECORD INTENSITY
            if (attribute.isEmpty()) {
                attribute.append(Keys::Two);
            }
            if (attribute.length() != 1) {
                terminal->error("Intensity Attribute Set only allows Attributes 0, 1 or 2.");
                return;
            }
            if (attribute[0] != Keys::Two) {
                terminal->error("Intensity Attribute Set only allows Attributes 0, 1 or 2.");
                return;
            }
            if (valueFloat < 1) {
                terminal->error("Invalid values given to Intensity Attribute 2 Set.");
                return;
            }
            intensities->setAttribute(ids, keysToId(attribute), valueFloat);
        } else if (selectionType == Keys::Color) { // RECORD COLOR
            if (attribute.isEmpty()) {
                attribute.append(Keys::Two);
            }
            if (valueFloat < 0) {
                terminal->error("Invalid values given to Color Attribute Set.");
                return;
            }
            colors->setAttribute(ids, keysToId(attribute), valueFloat);
        } else if (selectionType == Keys::Cue) { // RECORD CUE
            if (attribute.isEmpty()) {
                attribute.append(Keys::Four);
            }
            cues->setAttribute(ids, attribute, value);
        }
    }
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
    valueFloat = (floor((valueFloat * 100) + 0.5) / 100);
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
    for (int key : keys) {
        if (isNumber(key) || key == Keys::Period) {
            idKeys += key;
        } else if (key == Keys::Plus) {
            if (!thruBuffer.isEmpty()) {
                if (!idKeys.startsWith(Keys::Period)) {
                    return QList<QString>();
                }
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
                    ids.append(idBeginning + QString::number(counter));
                }
                thruBuffer.clear();
                idKeys.clear();
            } else if (idKeys.endsWith(Keys::Period)) {
                idKeys.removeLast();
                if (idKeys.isEmpty()) {
                    ids.append(allIds);
                } else {
                    QString idStart = keysToId(idKeys, false);
                    for (QString id : allIds) {
                        if (id.startsWith(idStart + ".")) {
                            ids.append(id);
                        } else if (idStart == id) {
                            ids.append(idStart);
                        }
                    }
                }
            } else {
                QString id = keysToId(idKeys);
                if (id.isEmpty()) {
                    return QList<QString>();
                }
                ids.append(id);
            }
            idKeys.clear();
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
