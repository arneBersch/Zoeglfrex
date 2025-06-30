/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "terminal.h"

Terminal::Terminal(Kernel *core, QWidget *parent) : QWidget(parent) {
    kernel = core;
    QVBoxLayout *layout = new QVBoxLayout(this);
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    connect(scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this, SLOT(scrollToLastMessage(int, int)));
    messages = new QVBoxLayout();
    QFrame *messagesFrame = new QFrame();
    messagesFrame->setLayout(messages);
    scrollArea->setWidget(messagesFrame);
    messages->addStretch();
    layout->addWidget(scrollArea);
    prompt = new QLabel();
    prompt->setWordWrap(true);
    prompt->setStyleSheet("padding: 10px; background-color: #303030;");
    layout->addWidget(prompt);
}

void Terminal::write(int key) {
    command.append(key);
    prompt->setText(promptText(command));
    QMutexLocker locker(kernel->mutex);
    kernel->inspector->load(command);
}

void Terminal::backspace() {
    if (command.isEmpty()) {
        return;
    }
    command.removeLast();
    prompt->setText(promptText(command));
    QMutexLocker locker(kernel->mutex);
    kernel->inspector->load(command);
}

void Terminal::clear() {
    command.clear();
    prompt->setText(promptText(command));
    kernel->inspector->load(command);
}

void Terminal::execute() {
    if(command.isEmpty()) {
        return;
    }
    info(">>> " + promptText(command));
    QMutexLocker locker(kernel->mutex);
    int selectionType = command.first();
    QList<int> commandWithoutSelectionType = command;
    commandWithoutSelectionType.removeFirst();
    if (!isItem(selectionType)) {
        error("No Item Type specified.");
        return;
    }
    QList<int> selection;
    QList<int> attribute;
    QList<int> value;
    bool attributeReached = false;
    bool valueReached = false;
    for (const int key : commandWithoutSelectionType) {
        if (key == Keys::Set) {
            if (valueReached) {
                error("Can't use Set more than one time in one command.");
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
    QStringList ids = keysToSelection(selection, selectionType);
    QMap<int, QString> attributeMap = QMap<int, QString>();
    if (!valueReached && !attributeReached) {
        if (selectionType == Keys::Fixture) {
            if (ids.size() != 1) {
                error("Can't select Fixture because Fixture only allows one Fixture ID.");
                return;
            }
            kernel->cuelistView->loadFixture(ids.first());
        } else if (selectionType == Keys::Group) {
            if (ids.size() != 1) {
                error("Can't select Group because Group only allows one Group ID.");
                return;
            }
            kernel->cuelistView->loadGroup(ids.first());
        } else if (selectionType == Keys::Intensity) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Intensity because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->selectedCue() == nullptr) {
                error("Can't set Intensity because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->CUEINTENSITIESATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->selectedCue()->id);
            int cueRow = kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->selectedCue()->id) + 1;
            Intensity* oldIntensity = kernel->cuelistView->selectedCue()->intensities.value(kernel->cuelistView->currentGroup, nullptr);
            while ((cueRow < kernel->cuelistView->currentCuelist->cues->items.size())
                && (kernel->cuelistView->currentCuelist->cues->items[cueRow]->intensities.value(kernel->cuelistView->currentGroup, nullptr) == oldIntensity)
                && !kernel->cuelistView->currentCuelist->cues->items[cueRow]->boolAttributes.value(kernel->CUEBLOCKATTRIBUTEID)
                && kernel->cuelistView->trackingButton->isChecked()
            ) {
                cueIds.append(kernel->cuelistView->currentCuelist->cues->items[cueRow]->id);
                cueRow++;
            }
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Color) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Color because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->selectedCue() == nullptr) {
                error("Can't set Color because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->CUECOLORSATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->selectedCue()->id);
            int cueRow = kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->selectedCue()->id) + 1;
            Color* oldColor = kernel->cuelistView->selectedCue()->colors.value(kernel->cuelistView->currentGroup, nullptr);
            while ((cueRow < kernel->cuelistView->currentCuelist->cues->items.size())
                   && (kernel->cuelistView->currentCuelist->cues->items[cueRow]->colors.value(kernel->cuelistView->currentGroup, nullptr) == oldColor)
                   && !kernel->cuelistView->currentCuelist->cues->items[cueRow]->boolAttributes.value(kernel->CUEBLOCKATTRIBUTEID)
                   && kernel->cuelistView->trackingButton->isChecked()
                   ) {
                cueIds.append(kernel->cuelistView->currentCuelist->cues->items[cueRow]->id);
                cueRow++;
            }
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Position) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Position because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->selectedCue() == nullptr) {
                error("Can't set Position because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->CUEPOSITIONSATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->selectedCue()->id);
            int cueRow = kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->selectedCue()->id) + 1;
            Position* oldPosition = kernel->cuelistView->selectedCue()->positions.value(kernel->cuelistView->currentGroup, nullptr);
            while ((cueRow < kernel->cuelistView->currentCuelist->cues->items.size())
                   && (kernel->cuelistView->currentCuelist->cues->items[cueRow]->positions.value(kernel->cuelistView->currentGroup, nullptr) == oldPosition)
                   && !kernel->cuelistView->currentCuelist->cues->items[cueRow]->boolAttributes.value(kernel->CUEBLOCKATTRIBUTEID)
                   && kernel->cuelistView->trackingButton->isChecked()
                   ) {
                cueIds.append(kernel->cuelistView->currentCuelist->cues->items[cueRow]->id);
                cueRow++;
            }
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Raw) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Raws because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->selectedCue() == nullptr) {
                error("Can't set Raws because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->CUERAWSATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->selectedCue()->id);
            int cueRow = kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->selectedCue()->id) + 1;
            QList<Raw*> oldRaws = kernel->cuelistView->selectedCue()->raws.value(kernel->cuelistView->currentGroup, QList<Raw*>());
            while ((cueRow < kernel->cuelistView->currentCuelist->cues->items.size())
                   && (kernel->cuelistView->currentCuelist->cues->items[cueRow]->raws.value(kernel->cuelistView->currentGroup, QList<Raw*>()) == oldRaws)
                   && !kernel->cuelistView->currentCuelist->cues->items[cueRow]->boolAttributes.value(kernel->CUEBLOCKATTRIBUTEID)
                   && kernel->cuelistView->trackingButton->isChecked()
                   ) {
                cueIds.append(kernel->cuelistView->currentCuelist->cues->items[cueRow]->id);
                cueRow++;
            }
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Effect) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Effects because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->selectedCue() == nullptr) {
                error("Can't set Effects because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->CUEEFFECTSATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->selectedCue()->id);
            int cueRow = kernel->cuelistView->currentCuelist->cues->getItemRow(kernel->cuelistView->selectedCue()->id) + 1;
            QList<Effect*> oldEffects = kernel->cuelistView->selectedCue()->effects.value(kernel->cuelistView->currentGroup, QList<Effect*>());
            while ((cueRow < kernel->cuelistView->currentCuelist->cues->items.size())
                   && (kernel->cuelistView->currentCuelist->cues->items[cueRow]->effects.value(kernel->cuelistView->currentGroup, QList<Effect*>()) == oldEffects)
                   && !kernel->cuelistView->currentCuelist->cues->items[cueRow]->boolAttributes.value(kernel->CUEBLOCKATTRIBUTEID)
                   && kernel->cuelistView->trackingButton->isChecked()
                   ) {
                cueIds.append(kernel->cuelistView->currentCuelist->cues->items[cueRow]->id);
                cueRow++;
            }
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                kernel->cuelistView->currentCuelist->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Cuelist) {
            if (ids.size() != 1) {
                error("Can't select Cuelist because Cuelist only allows one Cuelist ID.");
                return;
            }
            kernel->cuelistView->loadCuelist(ids.first());
        } else if (selectionType == Keys::Cue) {
            if (ids.size() != 1) {
                error("Can't select Cue because Cue only allows one Cue ID.");
                return;
            }
            kernel->cuelistView->loadCue(ids.first());
        } else {
            error("No Attribute and Value were given.");
            return;
        }
        kernel->cuelistView->reload();
        return;
    }
    if (ids.isEmpty()) {
        error("Can't execute command because of an invalid Item selection.");
        return;
    }
    attribute.append(Keys::Attribute);
    int currentItemType;
    QList<int> currentId;
    bool firstItem = true;
    for (int attributeKey : attribute) {
        if (isItem(attributeKey) || (attributeKey == Keys::Attribute)) {
            if (!firstItem) {
                if (currentItemType == Keys::Attribute) {
                    attributeMap[currentItemType] = keysToId(currentId);
                    if (attributeMap[currentItemType].isEmpty()) {
                        kernel->terminal->error("Invalid Attribute ID given.");
                        return;
                    }
                } else {
                    QStringList currentIdStrings = keysToSelection(currentId, currentItemType);
                    if (currentIdStrings.size() != 1) {
                        error("Invalid ID given in Attributes.");
                        return;
                    }
                    attributeMap[currentItemType] = currentIdStrings.first();
                }
            }
            currentId.clear();
            currentItemType = attributeKey;
            firstItem = false;
        } else if (isNumber(attributeKey) || (attributeKey == Keys::Period)) {
            currentId.append(attributeKey);
        } else {
            error("Invalid key in Attribute.");
            return;
        }
    }
    if (!attributeMap.contains(Keys::Attribute) && !(attributeMap.isEmpty() && !value.isEmpty() && (((value.size() == 1) && (value.first() == Keys::Minus)) || (value.first() == selectionType)))) {
        if (selectionType == Keys::Model) {
            attributeMap[Keys::Attribute] = kernel->MODELCHANNELSATTRIBUTEID;
        } else if (selectionType == Keys::Fixture) {
            attributeMap[Keys::Attribute] = kernel->FIXTUREADDRESSATTRIBUTEID;
        } else if (selectionType == Keys::Group) {
            attributeMap[Keys::Attribute] = kernel->GROUPFIXTURESATTRIBUTEID;
        } else if (selectionType == Keys::Intensity) {
            attributeMap[Keys::Attribute] = kernel->INTENSITYDIMMERATTRIBUTEID;
        } else if (selectionType == Keys::Color) {
            attributeMap[Keys::Attribute] = kernel->COLORHUEATTRIBUTEID;
        } else if (selectionType == Keys::Position) {
            attributeMap[Keys::Attribute] = kernel->POSITIONPANATTRIBUTEID;
        } else if (selectionType == Keys::Raw) {
            attributeMap[Keys::Attribute] = kernel->RAWCHANNELVALUEATTRIBUTEID;
        } else if (selectionType == Keys::Effect) {
            attributeMap[Keys::Attribute] = kernel->EFFECTSTEPSATTRIBUTEID;
        } else if (selectionType == Keys::Cuelist) {
            attributeMap[Keys::Attribute] = kernel->CUELISTCUEATTRIBUTEID;
        } else if (selectionType == Keys::Cue) {
            attributeMap[Keys::Attribute] = kernel->CUEFADEATTRIBUTEID;
        }
    }
    QString text = QString();
    if (((selectionType == Keys::Model) && kernel->models->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Fixture) && kernel->fixtures->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Group) && kernel->groups->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Intensity) && kernel->intensities->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Color) && kernel->colors->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Position) && kernel->positions->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Raw) && kernel->raws->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Effect) && kernel->effects->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Cuelist) && kernel->cuelists->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Cue) && (kernel->cuelistView->currentCuelist != nullptr) && kernel->cuelistView->currentCuelist->cues->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
    ) {
        if (ids.size() == 1) {
            Item* item = nullptr;
            if (selectionType == Keys::Model) {
                item = kernel->models->getItem(ids.first());
            } else if (selectionType == Keys::Fixture) {
                item = kernel->fixtures->getItem(ids.first());
            } else if (selectionType == Keys::Group) {
                item = kernel->groups->getItem(ids.first());
            } else if (selectionType == Keys::Intensity) {
                item = kernel->intensities->getItem(ids.first());
            } else if (selectionType == Keys::Color) {
                item = kernel->colors->getItem(ids.first());
            } else if (selectionType == Keys::Position) {
                item = kernel->positions->getItem(ids.first());
            } else if (selectionType == Keys::Raw) {
                item = kernel->raws->getItem(ids.first());
            } else if (selectionType == Keys::Effect) {
                item = kernel->effects->getItem(ids.first());
            } else if (selectionType == Keys::Cuelist) {
                item = kernel->cuelists->getItem(ids.first());
            } else if ((selectionType == Keys::Cue) && (kernel->cuelistView->currentCuelist != nullptr)) {
                item = kernel->cuelistView->currentCuelist->cues->getItem(ids.first());
            }
            if (item != nullptr) {
                text = item->stringAttributes.value(attributeMap.value(Keys::Attribute));
            }
        }
        bool ok = false;
        locker.unlock();
        text = QInputDialog::getText(this, QString(), "Insert Text", QLineEdit::Normal, text, &ok);
        locker.relock();
        if (!ok) {
            error("Popup cancelled.");
            return;
        }
    }
    if (selectionType == Keys::Model) {
        kernel->models->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Fixture) {
        kernel->fixtures->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Group) {
        kernel->groups->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Intensity) {
        kernel->intensities->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Color) {
        kernel->colors->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Position) {
        kernel->positions->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Raw) {
        kernel->raws->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Effect) {
        kernel->effects->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Cuelist) {
        kernel->cuelists->setAttribute(ids, attributeMap, value, text);
    } else if (selectionType == Keys::Cue) {
        if (kernel->cuelistView->currentCuelist == nullptr) {
            error("Can't set Cue Attribute because no Cuelist is currently selected.");
            return;
        }
        kernel->cuelistView->currentCuelist->cues->setAttribute(ids, attributeMap, value, text);
    }
    QMap<int, QSet<int>> channels;
    for (Fixture* fixture : kernel->fixtures->items) {
        const int address = fixture->intAttributes.value(kernel->FIXTUREADDRESSATTRIBUTEID);
        const int universe = fixture->intAttributes.value(kernel->FIXTUREUNIVERSEATTRIBUTEID);
        if (!channels.contains(universe)) {
            channels[universe] = QSet<int>();
        }
        if (address > 0) {
            int fixtureChannels = 0;
            if (fixture->model != nullptr) {
                fixtureChannels = fixture->model->stringAttributes.value(kernel->MODELCHANNELSATTRIBUTEID).size();
            }
            for (int channel = address; channel < (address + fixtureChannels); channel++) {
                if (channel > 512) {
                    warning("Patch Conflict detected: Fixture " + fixture->name() + " would have channels greater than 512.");
                }
                if (channels[universe].contains(channel)) {
                    warning("Patch Conflict detected: Channel " + QString::number(channel) + " in Universe " + QString::number(universe) + " would be used twice.");
                }
                channels[universe].insert(channel);
            }
        }
    }
    kernel->cuelistView->reload();
    kernel->inspector->load({selectionType});
}

void Terminal::info(QString message) {
    if (printMessages) {
        QLabel *label = new QLabel(message);
        label->setWordWrap(true);
        messages->addWidget(label);
    }
    qInfo() << message;
}

void Terminal::success(QString message) {
    if (printMessages) {
        QLabel *label = new QLabel(message);
        label->setWordWrap(true);
        label->setStyleSheet("color: green;");
        messages->addWidget(label);
    }
    qInfo() << message;
}

void Terminal::warning(QString message) {
    QLabel *label = new QLabel(message);
    label->setWordWrap(true);
    label->setStyleSheet("color: yellow;");
    messages->addWidget(label);
    qWarning() << message;
}

void Terminal::error(QString message) {
    QLabel *label = new QLabel(message);
    label->setWordWrap(true);
    label->setStyleSheet("color: red;");
    messages->addWidget(label);
    qWarning() << message;
}

QString Terminal::promptText(QList<int> keys) {
    QString commandString = "";
    for(const int key: keys) {
        if (key == Keys::Plus) {
            commandString += " + ";
        } else if (key == Keys::Minus) {
            commandString += " - ";
        } else if (key == Keys::Period) {
            commandString += ".";
        } else if (key == Keys::Thru) {
            commandString += " Thru ";
        } else if (key == Keys::Zero) {
            commandString += "0";
        } else if (key == Keys::One) {
            commandString += "1";
        } else if (key == Keys::Two) {
            commandString += "2";
        } else if (key == Keys::Three) {
            commandString += "3";
        } else if (key == Keys::Four) {
            commandString += "4";
        } else if (key == Keys::Five) {
            commandString += "5";
        } else if (key == Keys::Six) {
            commandString += "6";
        } else if (key == Keys::Seven) {
            commandString += "7";
        } else if (key == Keys::Eight) {
            commandString += "8";
        } else if (key == Keys::Nine) {
            commandString += "9";
        } else if (key == Keys::Model) {
            commandString += " Model ";
        } else if (key == Keys::Fixture) {
            commandString += " Fixture ";
        } else if (key == Keys::Group) {
            commandString += " Group ";
        } else if (key == Keys::Intensity) {
            commandString += " Intensity ";
        } else if (key == Keys::Color) {
            commandString += " Color ";
        } else if (key == Keys::Position) {
            commandString += " Position ";
        } else if (key == Keys::Raw) {
            commandString += " Raw ";
        } else if (key == Keys::Effect) {
            commandString += " Effect ";
        } else if (key == Keys::Cuelist) {
            commandString += " Cuelist ";
        } else if (key == Keys::Cue) {
            commandString += " Cue ";
        } else if (key == Keys::Set) {
            commandString += " Set ";
        } else if (key == Keys::Attribute) {
            commandString += " Attribute ";
        } else {
            Q_ASSERT(false);
        }
    }
    commandString.replace("  ", " "); // Remove double whitespaces
    if (commandString.startsWith(" ")) { // if first character is a whitespace
        commandString.remove(0, 1); // remove first character
    }
    if (commandString.endsWith(" ")) { // if last character is a whitespace
        commandString.remove(commandString.size(), 1); // remove last character
    }
    return commandString;
}

QString Terminal::keysToId(QList<int> keys, bool removeTrailingZeros) {
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

float Terminal::keysToValue(QList<int> keys) {
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

QStringList Terminal::keysToSelection(QList<int> keys, int itemType) {
    QStringList allIds;
    if (itemType == Keys::Model) {
        allIds = kernel->models->getIds();
    } else if (itemType == Keys::Fixture) {
        allIds = kernel->fixtures->getIds();
    } else if (itemType == Keys::Group) {
        allIds = kernel->groups->getIds();
    } else if (itemType == Keys::Intensity) {
        allIds = kernel->intensities->getIds();
    } else if (itemType == Keys::Color) {
        allIds = kernel->colors->getIds();
    } else if (itemType == Keys::Position) {
        allIds = kernel->positions->getIds();
    } else if (itemType == Keys::Raw) {
        allIds = kernel->raws->getIds();
    } else if (itemType == Keys::Effect) {
        allIds = kernel->effects->getIds();
    } else if (itemType == Keys::Cuelist) {
        allIds = kernel->cuelists->getIds();
    } else if (itemType == Keys::Cue) {
        if (kernel->cuelistView->currentCuelist != nullptr) {
            allIds = kernel->cuelistView->currentCuelist->cues->getIds();
        }
    } else {
        return QStringList();
    }
    if (keys.isEmpty()) {
        if ((itemType == Keys::Model) && (kernel->cuelistView->currentFixture != nullptr) && (kernel->cuelistView->currentFixture->model != nullptr)) {
            return {kernel->cuelistView->currentFixture->model->id};
        } else if ((itemType == Keys::Model) && (kernel->cuelistView->currentFixture == nullptr) && (kernel->cuelistView->currentGroup != nullptr)) {
            QStringList ids;
            for (Fixture* fixture : kernel->cuelistView->currentGroup->fixtures) {
                if (fixture->model != nullptr) {
                    ids.append(fixture->model->id);
                }
            }
            return ids;
        } else if ((itemType == Keys::Fixture) && (kernel->cuelistView->currentFixture != nullptr)) {
            return {kernel->cuelistView->currentFixture->id};
        } else if ((itemType == Keys::Fixture) && (kernel->cuelistView->currentFixture == nullptr) && (kernel->cuelistView->currentGroup != nullptr)) {
            QStringList ids;
            for (Fixture* fixture : kernel->cuelistView->currentGroup->fixtures) {
                ids.append(fixture->id);
            }
            return ids;
        } else if ((itemType == Keys::Group) && (kernel->cuelistView->currentGroup != nullptr)) {
            return {kernel->cuelistView->currentGroup->id};
        } else if ((itemType == Keys::Intensity) && (kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->intensities.contains(kernel->cuelistView->currentGroup)) {
            return {kernel->cuelistView->selectedCue()->intensities.value(kernel->cuelistView->currentGroup)->id};
        } else if ((itemType == Keys::Color) && (kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->colors.contains(kernel->cuelistView->currentGroup)) {
            return {kernel->cuelistView->selectedCue()->colors.value(kernel->cuelistView->currentGroup)->id};
        } else if ((itemType == Keys::Position) && (kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->positions.contains(kernel->cuelistView->currentGroup)) {
            return {kernel->cuelistView->selectedCue()->positions.value(kernel->cuelistView->currentGroup)->id};
        } else if ((itemType == Keys::Raw) && (kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->raws.contains(kernel->cuelistView->currentGroup)) {
            QStringList ids;
            for (Raw* raw : kernel->cuelistView->selectedCue()->raws.value(kernel->cuelistView->currentGroup)) {
                ids.append(raw->id);
            }
            return ids;
        } else if ((itemType == Keys::Effect) && (kernel->cuelistView->selectedCue() != nullptr) && kernel->cuelistView->selectedCue()->effects.contains(kernel->cuelistView->currentGroup)) {
            QStringList ids;
            for (Effect* effect : kernel->cuelistView->selectedCue()->effects.value(kernel->cuelistView->currentGroup)) {
                ids.append(effect->id);
            }
            return ids;
        } else if ((itemType == Keys::Cuelist) && (kernel->cuelistView->currentCuelist != nullptr)) {
            return {kernel->cuelistView->currentCuelist->id};
        } else if ((itemType == Keys::Cue) && (kernel->cuelistView->selectedCue() != nullptr)) {
            return {kernel->cuelistView->selectedCue()->id};
        } else {
            return QStringList();
        }
    }
    if (!keys.endsWith(Keys::Plus)) {
        keys.append(Keys::Plus);
    }
    QStringList ids;
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
                        return QStringList();
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
                        return QStringList();
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
                        return QStringList();
                    }
                    QString lastId = keysToId(idKeys);
                    if (lastId.isEmpty() || !allIds.contains(lastId)) {
                        return QStringList();
                    }
                    int firstIdRow = allIds.indexOf(firstId);
                    int lastIdRow = allIds.indexOf(lastId);
                    if (firstIdRow >= lastIdRow) {
                        return QStringList();
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
                    return QStringList();
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
                return QStringList();
            }
            thruBuffer = idKeys;
            idKeys.clear();
        } else {
            return QStringList();
        }
    }
    return ids;
}

bool Terminal::isItem(int key) {
    return (
        (key == Keys::Model) ||
        (key == Keys::Fixture) ||
        (key == Keys::Group) ||
        (key == Keys::Intensity) ||
        (key == Keys::Color) ||
        (key == Keys::Position) ||
        (key == Keys::Raw) ||
        (key == Keys::Effect) ||
        (key == Keys::Cuelist) ||
        (key == Keys::Cue)
    );
}

bool Terminal::isNumber(int key) {
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

int Terminal::keyToNumber(int key) {
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

void Terminal::scrollToLastMessage(int, int max) {
    scrollArea->verticalScrollBar()->setValue(max);
}
