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
    QList<int> selection;
    QList<QList<int>> operations;
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
    for (const int key : command) {
        if (isOperator(key)) { // if a new operation is started
            QList<int> newOperation;
            newOperation.append(key);
            operations.append(newOperation);
        } else {
            if (operations.size() > 0) { // if the selection is finished
                operations[operations.size() - 1].append(key);
            } else {
                selection.append(key);
            }
        }
    }
    if (selection.isEmpty()) {
        terminal->error("No selection given.");
        return;
    }
    QList<QString> ids = keysToSelection(selection);
    if (ids.isEmpty()) {
        terminal->error("Invalid fixture selection.");
        return;
    }
    if (operations.size() < 1) {
        terminal->error("No operator specified.");
        return;
    }
    if (!text.isNull() && operations.size() > 1) {
        terminal->error("Passing a text to executeCommand limits the amount of operations to one.");
        return;
    }
    for (QList<int> operation : operations) {
        int operationType = operation[0];
        operation.removeFirst();
        if (operationType == Keys::Copy) { // COPY
            QString targetId = keysToId(operation);
            if (targetId.isEmpty()) {
                terminal->error("Target ID not valid.");
                return;
            }
            if (selectionType == Keys::Model) { // COPY MODEL
                models->copyItems(ids, targetId);
            } else if (selectionType == Keys::Fixture) { // COPY FIXTURE
                fixtures->copyItems(ids, targetId);
            } else if (selectionType == Keys::Group) { // COPY GROUP
                groups->copyItems(ids, targetId);
            } else if (selectionType == Keys::Intensity) { // COPY INTENSITY
                intensities->copyItems(ids, targetId);
            } else if (selectionType == Keys::Color) { // COPY COLOR
                colors->copyItems(ids, targetId);
            } else if (selectionType == Keys::Cue) { // COPY CUE
                cues->copyItems(ids, targetId);
            } else {
                terminal->error("Unknown item type.");
                return;
            }
        } else if (operationType == Keys::Delete) { // DELETE
            if (!operation.isEmpty() && !(selectionType ==  Keys::Cue)) {
                terminal->error("Delete doesn't take any parameters.");
                return;
            }
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
                if (operation.isEmpty()) {
                    cues->deleteItems(ids);
                } else {
                    if (operation[0] != Keys::Group) {
                        terminal->error("Delete Cue takes either no parameters or a Group.");
                        return;
                    }
                    operation.removeFirst();
                    bool intensityDelete = false;
                    bool colorDelete = false;
                    QList<int> groupSelection;
                    for (int key : operation) {
                        if (isNumber(key) || key == Keys::Period) {
                            if (intensityDelete || colorDelete) {
                                terminal->error("Can't delete Cue Group: No IDs are permitted after Intensity or Color.");
                                return;
                            }
                            groupSelection.append(key);
                        } else if (key == Keys::Intensity) {
                            intensityDelete = true;
                        } else if (key == Keys::Color) {
                            colorDelete = true;
                        } else {
                            terminal->error("Can't delete Cue Group: Invalid key entered.");
                            return;
                        }
                    }
                    QString groupId = keysToId(groupSelection);
                    if (groupId.isEmpty()) {
                        terminal->error("Can't delete Cue Group: No valid Group ID given.");
                        return;
                    }
                    if (intensityDelete || (!(intensityDelete || colorDelete))) {
                        cues->deleteCueGroupIntensity(ids, groupId);
                    }
                    if (colorDelete || (!(intensityDelete || colorDelete))) {
                        cues->deleteCueGroupColor(ids, groupId);
                    }
                }
            }
        } else if (operationType == Keys::Label) { // LABEL
            if (!operation.isEmpty()) {
                terminal->error("Label doesn't take any parameters.");
                return;
            }
            QString label = text;
            if (text.isNull()) {
                bool ok = false;
                locker.unlock();
                label = QInputDialog::getText(terminal, QString(), "Label", QLineEdit::Normal, QString(), &ok);
                locker.relock();
                if (!ok) {
                    terminal->error("Popup cancelled.");
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
        } else if (operationType == Keys::Move) { // MOVE
            QString targetId = keysToId(operation);
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
        } else if (operationType == Keys::Record) { // RECORD
            if (selectionType == Keys::Model) { // RECORD MODEL
                if (!operation.isEmpty()) {
                    terminal->error("Record Model doesn't take any parameters.");
                    return;
                }
                QString channels = text;
                if (text.isNull()) {
                    bool ok = false;
                    locker.unlock();
                    channels = QInputDialog::getText(terminal, QString(), "Channels", QLineEdit::Normal, QString(), &ok);
                    locker.relock();
                    if (!ok) {
                        terminal->error("Popup cancelled.");
                        return;
                    }
                }
                models->recordModelChannels(ids, channels);
            } else if (selectionType == Keys::Fixture) { // RECORD FIXTURE
                if (operation.isEmpty()) {
                    terminal->error("Record Fixture requires arguments.");
                    return;
                }
                if (operation[0] == Keys::Model) {
                    operation.removeFirst();
                    QString modelId = keysToId(operation);
                    if (modelId.isEmpty()) {
                        terminal->error("Can't record Fixture Model because Model ID is not vaild.");
                        return;
                    }
                    fixtures->recordFixtureModel(ids, modelId);
                } else {
                    int address = 0;
                    for (int key : operation) {
                        if (!isNumber(key)) {
                            terminal->error("Can't record Fixture address because address only allows numbers as input.");
                            return;
                        }
                        address = (address * 10) + keyToNumber(key);
                    }
                    fixtures->recordFixtureAddress(ids, address);
                }
            } else if (selectionType == Keys::Group) { // RECORD GROUP
                if (operation.isEmpty()) {
                    groups->recordGroupFixtures(ids, QList<QString>());
                } else {
                    if (operation[0] != Keys::Fixture) {
                        terminal->error("Can't record Group because no Fixtures were given.");
                        return;
                    }
                    operation.removeFirst();
                    QList<QString> fixtureIds = keysToSelection(operation);
                    if (fixtureIds.isEmpty()) {
                        terminal->error("Can't record Group because of invalid Fixture selection.");
                        return;
                    }
                    groups->recordGroupFixtures(ids, fixtureIds);
                }
            } else if (selectionType == Keys::Intensity) { // RECORD INTENSITY
                if (operation.isEmpty()) {
                    terminal->error("Record Intensity requires arguments.");
                    return;
                }
                QList<float> values = keysToValue(operation);
                if (values.empty() || values.size() > 1) {
                    terminal->error("Invalid values given to Record Intensity.");
                    return;
                }
                if (values.size() >= 1) {
                    if (values[0] > -999) {
                        intensities->recordIntensityDimmer(ids, values[0]);
                    }
                }
            } else if (selectionType == Keys::Color) { // RECORD COLOR
                if (operation.isEmpty()) {
                    terminal->error("Record Color requires arguments.");
                    return;
                }
                QList<float> values = keysToValue(operation);
                if (values.empty() || values.size() > 3) {
                    terminal->error("Invalid values given to Record Color.");
                    return;
                }
                if (values.size() >= 1) {
                    if (values[0] > -999) {
                        colors->recordColorRed(ids, values[0]);
                    }
                }
                if (values.size() >= 2) {
                    if (values[1] > -999) {
                        colors->recordColorGreen(ids, values[1]);
                    }
                }
                if (values.size() >= 3) {
                    if (values[2] > -999) {
                        colors->recordColorBlue(ids, values[2]);
                    }
                }
            } else if (selectionType == Keys::Cue) { // RECORD CUE
                if (operation.isEmpty()) {
                    terminal->error("Record Cue requires an argument.");
                    return;
                }
                if (operation[0] == Keys::Group) {
                    operation.removeFirst();
                    operation.append(Keys::Intensity);
                    QString intensityId;
                    QString colorId;
                    QString groupId;
                    int lastItem = Keys::Group;
                    QList<int> selection;
                    for (int key : operation) {
                        if (isNumber(key) || key == Keys::Period) {
                            selection.append(key);
                        } else if (key == Keys::Intensity || key == Keys::Color) {
                            QString selectionId = keysToId(selection);
                            if (selectionId.isEmpty()) {
                                terminal->error("Invalid values given to Record Cue.");
                                return;
                            }
                            selection.clear();
                            if (lastItem == Keys::Group) {
                                groupId = selectionId;
                            } else if (lastItem == Keys::Intensity) {
                                intensityId = selectionId;
                            } else if (lastItem == Keys::Color) {
                                colorId = selectionId;
                            } else {
                                terminal->error("Invalid values given to Record Cue.");
                                return;
                            }
                            lastItem = key;
                        } else {
                            terminal->error("Invalid values given to Record Cue.");
                            return;
                        }
                    }
                    if (groupId.isEmpty()) {
                        terminal->error("No Group specified.");
                        return;
                    }
                    if (!intensityId.isEmpty()) {
                        cues->recordCueIntensity(ids, groupId, intensityId);
                    }
                    if (!colorId.isEmpty()) {
                        cues->recordCueColor(ids, groupId, colorId);
                    }
                } else {
                    operation.removeFirst();
                    QList<float> values = keysToValue(operation);
                    if (values.empty() || values.size() > 1) {
                        terminal->error("Invalid values given to Record Cue Fade.");
                        return;
                    }
                    if ((values.size() >= 1) && (values[0] > 0)) {
                        cues->recordCueFade(ids, values[0]);
                    }
                }
            }
        }
    }
    cuelistView->loadCue();
}

QString Kernel::keysToId(QList<int> keys)
{
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
    while (id.endsWith(".0")) {
        id.chop(2);
    }
    return id;
}

QList<float> Kernel::keysToValue(QList<int> keys)
{
    keys.append(Keys::Plus);
    QList<float> values;
    QString value;
    for (const int key : keys) {
        if (isNumber(key)) {
            value += QString::number(keyToNumber(key));
        } else if (key == Keys::Period) {
            if (value.contains(".")) {
                return QList<float>();
            }
            value += ".";
        } else if (key == Keys::Plus) {
            if (value.isEmpty()) {
                values.append(-999);
            } else {
                bool ok;
                float newValue = value.toFloat(&ok);
                if (!ok) {
                    return QList<float>();
                }
                newValue = (floor((newValue * 100) + 0.5) / 100);
                values.append(newValue);
            }
            value = QString();
        } else {
            return QList<float>();
        }
    }
    return values;
}

QList<QString> Kernel::keysToSelection(QList<int> keys)
{
    if (keys.isEmpty()) {
        return QList<QString>();
    }
    if (!keys.endsWith(Keys::Plus)) {
        keys.append(Keys::Plus);
    }
    QList<QString> ids;
    QList<int> idKeys;
    for (int key : keys) {
        if (isNumber(key) || key == Keys::Period) {
            idKeys += key;
        } else if (key == Keys::Plus) {
            QString id = keysToId(idKeys);
            if (id.isEmpty()) {
                return QList<QString>();
            }
            ids.append(id);
            idKeys.clear();
        } else {
            return QList<QString>();
        }
    }
    return ids;
}

bool Kernel::isItem(int key)
{
    return (
        (key == Keys::Model) ||
        (key == Keys::Fixture) ||
        (key == Keys::Group) ||
        (key == Keys::Intensity) ||
        (key == Keys::Color) ||
        (key == Keys::Cue)
    );
}

bool Kernel::isOperator(int key)
{
    return (
        (key == Keys::Copy) ||
        (key == Keys::Delete) ||
        (key == Keys::Label) ||
        (key == Keys::Move) ||
        (key == Keys::Record)
        );
}

bool Kernel::isNumber(int key)
{
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

int Kernel::keyToNumber(int key)
{
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
