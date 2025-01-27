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
    if ((selectionType == Keys::Cue) && !valueReached && !attributeReached && (ids.size() == 1)) {
        Cue* cue = cues->getItem(ids.first());
        if (cue == nullptr) {
            cue = cues->addItem(ids.first());
            terminal->success("Added and loaded Cue " + cue->id);
        } else {
            terminal->success("Loaded Cue " + cue->id);
        }
        cuelistView->currentCue = cue;
        cuelistView->loadCue();
        return;
    }
    if ((attribute.size() == 1) && (attribute[0] == Keys::One)) { // if label text input is required (Attribute 1 / Label)
        if (text.isNull()) {
            bool ok = false;
            locker.unlock();
            text = QInputDialog::getText(terminal, QString(), "Label", QLineEdit::Normal, QString(), &ok);
            locker.relock();
            if (!ok) {
                terminal->warning("Popup cancelled.");
                return;
            }
        }
    }
    QMap<int, QString> attributeMap = QMap<int, QString>();
    int currentItemType = Keys::Attribute;
    QList<int> currentId;
    for (int attributeKey : attribute) {
        if (isItem(attributeKey)) {
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
    if (selectionType == Keys::Model) {
        if (!attributeMap.contains(Keys::Attribute) && ((value.size() == 1) || (value.first() != Keys::Minus))) {
            attributeMap[Keys::Attribute] = "2";
        }
        QString channels = text;
        if ((attributeMap.value(Keys::Attribute) == "2") && text.isNull()) {
            bool ok = false;
            locker.unlock();
            channels = QInputDialog::getText(terminal, QString(), "Channels", QLineEdit::Normal, QString(), &ok);
            locker.relock();
            if (!ok) {
                terminal->warning("Popup cancelled.");
                return;
            }
        }
        models->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Fixture) {
        if (!attributeMap.contains(Keys::Attribute) && !value.isEmpty() && (value.first() != Keys::Minus)) {
            attributeMap[Keys::Attribute] = "3";
        }
        fixtures->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Group) {
        if (!attributeMap.contains(Keys::Attribute) && !value.isEmpty() && (value.first() != Keys::Minus)) {
            attributeMap[Keys::Attribute] = "2";
        }
        groups->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Intensity) {
        if (!attributeMap.contains(Keys::Attribute) && !value.isEmpty() && (value.first() != Keys::Minus)) {
            attributeMap[Keys::Attribute] = "2";
        }
        intensities->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Color) {
        if (!attributeMap.contains(Keys::Attribute) && !value.isEmpty() && (value.first() != Keys::Minus)) {
            attributeMap[Keys::Attribute] = "2";
        }
        colors->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Cue) {
        if (!attributeMap.contains(Keys::Attribute) && !value.isEmpty() && (value.first() != Keys::Minus)) {
            attributeMap[Keys::Attribute] = "4";
        }
        cues->setAttribute(ids, attributeMap, value, text);
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
