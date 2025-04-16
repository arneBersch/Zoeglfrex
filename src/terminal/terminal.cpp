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
    QVBoxLayout *grid = new QVBoxLayout(this);
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    connect(scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this, SLOT(scrollToLastMessage(int, int)));
    messages = new QVBoxLayout();
    QFrame *messagesFrame = new QFrame();
    messagesFrame->setLayout(messages);
    scrollArea->setWidget(messagesFrame);
    messages->addStretch();
    grid->addWidget(scrollArea);
    prompt = new QLabel();
    prompt->setWordWrap(true);
    grid->addWidget(prompt);
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
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't update Fixtures of current Group because no Group is currently selected.");
                return;
            }
            if (ids.size() != 1) {
                error("Can't select Fixture because Fixture only allows one Fixture ID.");
                return;
            }
            Fixture* fixture = kernel->fixtures->getItem(ids.first());
            if (fixture == nullptr) {
                error("Can't select Fixture because Fixture " + ids.first() + " doesn't exist.");
                return;
            }
            if (!kernel->cuelistView->currentGroup->fixtures.contains(fixture)) {
                success("Can't select Fixture " + fixture->name() + " because the current Group " + kernel->cuelistView->currentGroup->name() + " doesn't contain this Fixture.");
                return;
            }
            kernel->cuelistView->currentFixture = fixture;
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
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Intensity because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->cues->INTENSITIESATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->currentCue->id);
            int cueRow = kernel->cues->getItemRow(kernel->cuelistView->currentCue->id) + 1;
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                if (kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
                    Intensity* oldIntensity = kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                        && kernel->cues->items[cueRow]->intensities.contains(kernel->cuelistView->currentGroup)
                        && (kernel->cues->items[cueRow]->intensities.value(kernel->cuelistView->currentGroup) == oldIntensity)
                        && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                    ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                if (kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
                    Intensity* oldIntensity = kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                        && kernel->cues->items[cueRow]->intensities.contains(kernel->cuelistView->currentGroup)
                        && (kernel->cues->items[cueRow]->intensities.value(kernel->cuelistView->currentGroup) == oldIntensity)
                        && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                    ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                } else {
                    while ((cueRow < kernel->cues->items.size())
                        && !kernel->cues->items[cueRow]->intensities.contains(kernel->cuelistView->currentGroup)
                        && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                    ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Color) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Color because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Color because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->cues->COLORSATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->currentCue->id);
            int cueRow = kernel->cues->getItemRow(kernel->cuelistView->currentCue->id) + 1;
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                if (kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
                    Color* oldColor = kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                        && kernel->cues->items[cueRow]->colors.contains(kernel->cuelistView->currentGroup)
                        && (kernel->cues->items[cueRow]->colors.value(kernel->cuelistView->currentGroup) == oldColor)
                        && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                    ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                if (kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
                    Color* oldColor = kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                           && kernel->cues->items[cueRow]->colors.contains(kernel->cuelistView->currentGroup)
                           && (kernel->cues->items[cueRow]->colors.value(kernel->cuelistView->currentGroup) == oldColor)
                           && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                           ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                } else {
                    while ((cueRow < kernel->cues->items.size())
                           && !kernel->cues->items[cueRow]->colors.contains(kernel->cuelistView->currentGroup)
                           && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                           ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Position) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Position because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Position because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->cues->POSITIONSATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->currentCue->id);
            int cueRow = kernel->cues->getItemRow(kernel->cuelistView->currentCue->id) + 1;
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                if (kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
                    Position* oldPosition = kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                           && kernel->cues->items[cueRow]->positions.contains(kernel->cuelistView->currentGroup)
                           && (kernel->cues->items[cueRow]->positions.value(kernel->cuelistView->currentGroup) == oldPosition)
                           && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                           ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                if (kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
                    Position* oldPosition = kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                           && kernel->cues->items[cueRow]->positions.contains(kernel->cuelistView->currentGroup)
                           && (kernel->cues->items[cueRow]->positions.value(kernel->cuelistView->currentGroup) == oldPosition)
                           && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                           ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                } else {
                    while ((cueRow < kernel->cues->items.size())
                           && !kernel->cues->items[cueRow]->positions.contains(kernel->cuelistView->currentGroup)
                           && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                           ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Raw) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Raws because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Raws because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->cues->RAWSATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->currentCue->id);
            int cueRow = kernel->cues->getItemRow(kernel->cuelistView->currentCue->id) + 1;
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                if (kernel->cuelistView->currentCue->raws.contains(kernel->cuelistView->currentGroup)) {
                    QList<Raw*> oldRaws = kernel->cuelistView->currentCue->raws.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                        && kernel->cues->items[cueRow]->raws.contains(kernel->cuelistView->currentGroup)
                        && (kernel->cues->items[cueRow]->raws.value(kernel->cuelistView->currentGroup) == oldRaws)
                        && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                    ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                if (kernel->cuelistView->currentCue->raws.contains(kernel->cuelistView->currentGroup)) {
                    QList<Raw*> oldRaws = kernel->cuelistView->currentCue->raws.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                        && kernel->cues->items[cueRow]->raws.contains(kernel->cuelistView->currentGroup)
                        && (kernel->cues->items[cueRow]->raws.value(kernel->cuelistView->currentGroup) == oldRaws)
                        && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                    ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                } else {
                    while ((cueRow < kernel->cues->items.size())
                        && !kernel->cues->items[cueRow]->raws.contains(kernel->cuelistView->currentGroup)
                        && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                    ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, command);
            }
        } else if (selectionType == Keys::Effect) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't set Effects because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Effects because no Cue is currently selected.");
                return;
            }
            attributeMap[Keys::Attribute] = kernel->cues->EFFECTSATTRIBUTEID;
            attributeMap[Keys::Group] = kernel->cuelistView->currentGroup->id;
            QStringList cueIds;
            cueIds.append(kernel->cuelistView->currentCue->id);
            int cueRow = kernel->cues->getItemRow(kernel->cuelistView->currentCue->id) + 1;
            if ((commandWithoutSelectionType.size() == 1) && (commandWithoutSelectionType.first() == Keys::Minus)) {
                if (kernel->cuelistView->currentCue->effects.contains(kernel->cuelistView->currentGroup)) {
                    QList<Effect*> oldEffects = kernel->cuelistView->currentCue->effects.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                           && kernel->cues->items[cueRow]->effects.contains(kernel->cuelistView->currentGroup)
                           && (kernel->cues->items[cueRow]->effects.value(kernel->cuelistView->currentGroup) == oldEffects)
                           && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                           ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, commandWithoutSelectionType);
            } else {
                if (kernel->cuelistView->currentCue->effects.contains(kernel->cuelistView->currentGroup)) {
                    QList<Effect*> oldEffects = kernel->cuelistView->currentCue->effects.value(kernel->cuelistView->currentGroup);
                    while ((cueRow < kernel->cues->items.size())
                           && kernel->cues->items[cueRow]->effects.contains(kernel->cuelistView->currentGroup)
                           && (kernel->cues->items[cueRow]->effects.value(kernel->cuelistView->currentGroup) == oldEffects)
                           && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                           ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                } else {
                    while ((cueRow < kernel->cues->items.size())
                           && !kernel->cues->items[cueRow]->effects.contains(kernel->cuelistView->currentGroup)
                           && !kernel->cues->items[cueRow]->boolAttributes.value(kernel->cues->BLOCKATTRIBUTEID)
                           ) {
                        cueIds.append(kernel->cues->items[cueRow]->id);
                        cueRow++;
                    }
                }
                kernel->cues->setAttribute(cueIds, attributeMap, command);
            }
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
        kernel->cuelistView->loadView();
        return;
    }
    if (selection.isEmpty()) {
        if (selectionType == Keys::Model) {
            if (kernel->cuelistView->currentFixture == nullptr) {
                error("Can't load the Model of the current Fixture because no Fixture is selected.");
                return;
            }
            if (kernel->cuelistView->currentFixture->model == nullptr) {
                error("Can't load the Model of the current Fixture because the current Fixture holds no Model.");
                return;
            }
            ids.append(kernel->cuelistView->currentFixture->model->id);
        } else if (selectionType == Keys::Fixture) {
            if (kernel->cuelistView->currentFixture == nullptr) {
                if (kernel->cuelistView->currentGroup == nullptr) {
                    error("Can't load the Fixtures of the current Group because no Group is selected.");
                    return;
                }
                for (Fixture* fixture : kernel->cuelistView->currentGroup->fixtures) {
                    ids.append(fixture->id);
                }
            } else {
                ids.append(kernel->cuelistView->currentFixture->id);
            }
        } else if (selectionType == Keys::Group) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't load the current Group because no Group is selected.");
                return;
            }
            ids.append(kernel->cuelistView->currentGroup->id);
        } else if (selectionType == Keys::Intensity) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't load the Intensity because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Intensity because no Cue is currently selected.");
                return;
            }
            if (!kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
                error("Can't load the Intensity because the selected Cue contains no Intensity for this Group.");
                return;
            }
            ids.append(kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->id);
        } else if (selectionType == Keys::Color) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't load the Color because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Color because no Cue is currently selected.");
                return;
            }
            if (!kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
                error("Can't load the Color because the selected Cue contains no Color for this Group.");
                return;
            }
            ids.append(kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->id);
        } else if (selectionType == Keys::Position) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't load the Position because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Position because no Cue is currently selected.");
                return;
            }
            if (!kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
                error("Can't load the Position because the selected Cue contains no Position for this Group.");
                return;
            }
            ids.append(kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->id);
        } else if (selectionType == Keys::Raw) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't load the Raws because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Raws because no Cue is currently selected.");
                return;
            }
            if (!kernel->cuelistView->currentCue->raws.contains(kernel->cuelistView->currentGroup)) {
                error("Can't load the Raws because the selected Cue contains no Raws for this Group.");
                return;
            }
            for (Raw* raw : kernel->cuelistView->currentCue->raws.value(kernel->cuelistView->currentGroup)) {
                ids.append(raw->id);
            }
        } else if (selectionType == Keys::Effect) {
            if (kernel->cuelistView->currentGroup == nullptr) {
                error("Can't load the Effects because no Group is currently selected.");
                return;
            }
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't set Effects because no Cue is currently selected.");
                return;
            }
            if (!kernel->cuelistView->currentCue->effects.contains(kernel->cuelistView->currentGroup)) {
                error("Can't load the Effects because the selected Cue contains no Effects for this Group.");
                return;
            }
            for (Effect* effect : kernel->cuelistView->currentCue->effects.value(kernel->cuelistView->currentGroup)) {
                ids.append(effect->id);
            }
        } else if (selectionType == Keys::Cue) {
            if (kernel->cuelistView->currentCue == nullptr) {
                error("Can't load the Cue because no Cue is selected.");
                return;
            }
            ids.append(kernel->cuelistView->currentCue->id);
        } else {
            error("No standard selection for this Item type.");
            return;
        }
    }
    if (ids.isEmpty()) {
        error("No items selected.");
        return;
    }
    attribute.append(Keys::Attribute);
    int currentItemType;
    QList<int> currentId;
    bool firstItem = true;
    for (int attributeKey : attribute) {
        if (isItem(attributeKey) || (attributeKey == Keys::Attribute)) {
            if (!firstItem) {
                QString currentIdString = keysToId(currentId);
                if (currentId.isEmpty()) {
                    if (currentItemType == Keys::Model) {
                        if (kernel->cuelistView->currentFixture == nullptr) {
                            error("Can't load the Model of the current Fixture because no Fixture is selected.");
                            return;
                        }
                        if (kernel->cuelistView->currentFixture->model == nullptr) {
                            error("Can't load the Model of the current Fixture because the current Fixture holds no Model.");
                            return;
                        }
                        currentIdString = kernel->cuelistView->currentFixture->model->id;
                    } else if (currentItemType == Keys::Fixture) {
                        if (kernel->cuelistView->currentFixture == nullptr) {
                            error("Can't load the current Fixture because no Fixture is selected.");
                            return;
                        }
                        currentIdString = kernel->cuelistView->currentFixture->id;
                    } else if (currentItemType == Keys::Group) {
                        if (kernel->cuelistView->currentGroup == nullptr) {
                            error("Can't load the current Group because no Group is selected.");
                            return;
                        }
                        currentIdString = kernel->cuelistView->currentGroup->id;
                    } else if (currentItemType == Keys::Intensity) {
                        if (kernel->cuelistView->currentGroup == nullptr) {
                            error("Can't load the current Intensity because no Group is currently selected.");
                            return;
                        }
                        if (kernel->cuelistView->currentCue == nullptr) {
                            error("Can't load the current Intensity because no Cue is currently selected.");
                            return;
                        }
                        if (!kernel->cuelistView->currentCue->intensities.contains(kernel->cuelistView->currentGroup)) {
                            error("Can't load the current Intensity because the current Cue contains no Intensity for the current Group.");
                            return;
                        }
                        currentIdString = kernel->cuelistView->currentCue->intensities.value(kernel->cuelistView->currentGroup)->id;
                    } else if (currentItemType == Keys::Color) {
                        if (kernel->cuelistView->currentGroup == nullptr) {
                            error("Can't load the current Color because no Group is currently selected.");
                            return;
                        }
                        if (kernel->cuelistView->currentCue == nullptr) {
                            error("Can't load the current Color because no Cue is currently selected.");
                            return;
                        }
                        if (!kernel->cuelistView->currentCue->colors.contains(kernel->cuelistView->currentGroup)) {
                            error("Can't load the current Color because the current Cue contains no Color for the current Group.");
                            return;
                        }
                        currentIdString = kernel->cuelistView->currentCue->colors.value(kernel->cuelistView->currentGroup)->id;
                    } else if (currentItemType == Keys::Position) {
                        if (kernel->cuelistView->currentGroup == nullptr) {
                            error("Can't load the current Position because no Group is currently selected.");
                            return;
                        }
                        if (kernel->cuelistView->currentCue == nullptr) {
                            error("Can't load the current Position because no Cue is currently selected.");
                            return;
                        }
                        if (!kernel->cuelistView->currentCue->positions.contains(kernel->cuelistView->currentGroup)) {
                            error("Can't load the current Position because the current Cue contains no Color for the current Group.");
                            return;
                        }
                        currentIdString = kernel->cuelistView->currentCue->positions.value(kernel->cuelistView->currentGroup)->id;
                    } else if (currentItemType == Keys::Cue) {
                        if (kernel->cuelistView->currentCue == nullptr) {
                            error("Can't load the Cue because no Cue is selected.");
                            return;
                        }
                        currentIdString = kernel->cuelistView->currentCue->id;
                    } else {
                        error("Invalid Attributes given.");
                        return;
                    }
                }
                if (currentIdString.isEmpty()) {
                    error("Invalid ID given in Attributes.");
                    return;
                }
                attributeMap[currentItemType] = currentIdString;
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
            attributeMap[Keys::Attribute] = kernel->models->CHANNELSATTRIBUTEID;
        } else if (selectionType == Keys::Fixture) {
            attributeMap[Keys::Attribute] = kernel->fixtures->ADDRESSATTRIBUTEID;
        } else if (selectionType == Keys::Group) {
            attributeMap[Keys::Attribute] = kernel->groups->FIXTURESATTRIBUTEID;
        } else if (selectionType == Keys::Intensity) {
            attributeMap[Keys::Attribute] = kernel->intensities->DIMMERATTRIBUTEID;
        } else if (selectionType == Keys::Color) {
            attributeMap[Keys::Attribute] = kernel->colors->HUEATTRIBUTEID;
        } else if (selectionType == Keys::Position) {
            attributeMap[Keys::Attribute] = kernel->positions->PANATTRIBUTEID;
        } else if (selectionType == Keys::Raw) {
            attributeMap[Keys::Attribute] = kernel->raws->CHANNELVALUEATTRIBUTEID;
        } else if (selectionType == Keys::Effect) {
            attributeMap[Keys::Attribute] = kernel->effects->STEPSATTRIBUTEID;
        } else if (selectionType == Keys::Cue) {
            attributeMap[Keys::Attribute] = kernel->cues->FADEATTRIBUTEID;
        }
    }
    QString text;
    if (((selectionType == Keys::Model) && kernel->models->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Fixture) && kernel->fixtures->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Group) && kernel->groups->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Intensity) && kernel->intensities->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Color) && kernel->colors->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Position) && kernel->positions->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Raw) && kernel->raws->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Effect) && kernel->effects->stringAttributes.contains(attributeMap.value(Keys::Attribute)))
        || ((selectionType == Keys::Cue) && kernel->cues->stringAttributes.contains(attributeMap.value(Keys::Attribute)))) {
        bool ok = false;
        locker.unlock();
        text = QInputDialog::getText(this, QString(), "Insert Text", QLineEdit::Normal, QString(), &ok);
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
    } else if (selectionType == Keys::Cue) {
        kernel->cues->setAttribute(ids, attributeMap, value, text);
    }
    QMap<int, QSet<int>> channels;
    for (Fixture* fixture : kernel->fixtures->items) {
        const int address = fixture->intAttributes.value(kernel->fixtures->ADDRESSATTRIBUTEID);
        const int universe = fixture->intAttributes.value(kernel->fixtures->UNIVERSEATTRIBUTEID);
        if (!channels.contains(universe)) {
            channels[universe] = QSet<int>();
        }
        if (address > 0) {
            int fixtureChannels = 0;
            if (fixture->model != nullptr) {
                fixtureChannels = fixture->model->stringAttributes.value(kernel->models->CHANNELSATTRIBUTEID).size();
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
    kernel->cuelistView->loadView();
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
    } else if (itemType == Keys::Cue) {
        allIds = kernel->cues->getIds();
    } else {
        return QStringList();
    }
    if (keys.isEmpty()) {
        return QStringList();
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

void Terminal::scrollToLastMessage(int min, int max) {
    Q_UNUSED(min);
    scrollArea->verticalScrollBar()->setValue(max);
}
