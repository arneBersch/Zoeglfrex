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
    transitions = new TransitionList(this);
    rows = new RowList(this);
    cues = new CueList(this);
    terminal = new Terminal(this);
    inspector = new Inspector(this);
    cuelistView = new CuelistView(this);
    mutex = new QMutex();
}

QString Kernel::execute(QList<int> command, QString text) {
    QMutexLocker locker(mutex);
    QList<int> selection;
    QList<QList<int>> operations;
    if(command.size() < 1) { // if command is empty
        return "Command is empty.";
    }
    int selectionType = command[0];
    command.removeFirst();
    if (!isItem(selectionType)) {
        return "No Item Type specified.";
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
        return "No selection given.";
    }
    QList<QString> ids = keysToSelection(selection);
    if (ids.isEmpty()) {
        return "Invalid fixture selection.";
    }
    if (operations.size() < 1) {
        return "No operator specified.";
    }
    if (!text.isNull() && operations.size() > 1) {
        return "Passing a text to executeCommand limits the amount of operations to one.";
    }
    for (QList<int> operation : operations) {
        int operationType = operation[0];
        operation.removeFirst();
        if (operationType == keys::copyItem) { // COPY
            QString targetId = keysToId(operation);
            if (targetId.isEmpty()) {
                return "Target ID not valid.";
            }
            QString result;
            if (selectionType == keys::model) { // COPY MODEL
                result = models->copyModel(ids, targetId);
            } else if (selectionType == keys::fixture) { // COPY FIXTURE
                result = fixtures->copyFixture(ids, targetId);
            } else if (selectionType == keys::group) { // COPY GROUP
                result = groups->copyGroup(ids, targetId);
            } else if (selectionType == keys::intensity) { // COPY INTENSITY
                result = intensities->copyIntensity(ids, targetId);
            } else if (selectionType == keys::color) { // COPY COLOR
                result = colors->copyColor(ids, targetId);
            } else if (selectionType == keys::transition) { // COPY TRANSITION
                result = transitions->copyTransition(ids, targetId);
            } else if (selectionType == keys::row) { // COPY ROW
                result = rows->copyRow(ids, targetId);
            } else if (selectionType == keys::cue) { // COPY CUE
                result = cues->copyCue(ids, targetId);
            } else {
                return "Unknown item type.";
            }
            if (!result.isEmpty()) { // if the operation failed
                return result;
            }
        } else if (operationType == keys::deleteItem) { // DELETE
            if (!operation.isEmpty() && !(selectionType ==  keys::cue)) {
                return "Delete doesn't take any parameters.";
            }
            QString result;
            if (selectionType == keys::model) { // DELETE MODEL
                result = models->deleteModel(ids);
            } else if (selectionType == keys::fixture) { // DELETE FIXTURE
                result = fixtures->deleteFixture(ids);
            } else if (selectionType == keys::group) { // DELETE GROUP
                result = groups->deleteGroup(ids);
            } else if (selectionType == keys::intensity) { // DELETE INTENSITY
                result = intensities->deleteIntensity(ids);
            } else if (selectionType == keys::color) { // DELETE COLOR
                result = colors->deleteColor(ids);
            } else if (selectionType == keys::transition) { // DELETE TRANSITION
                result = transitions->deleteTransition(ids);
            } else if (selectionType == keys::row) { // DELETE ROW
                result = rows->deleteRow(ids);
            } else if (selectionType == keys::cue) { // DELETE CUE
                if (operation.isEmpty()) {
                    result = cues->deleteCue(ids);
                } else {
                    if (operation[0] != keys::row) {
                        return "Delete Transition takes either no parameters or a row.";
                    }
                    operation.removeFirst();
                    bool intensityDelete = false;
                    bool colorDelete = false;
                    QList<int> rowSelection;
                    for (int key : operation) {
                        if (isNumber(key) || key == keys::period) {
                            if (intensityDelete || colorDelete) {
                                return "Can't delete Transition Row: No IDs are permitted after Intensity or Color.";
                            }
                            rowSelection.append(key);
                        } else if (key == keys::intensity) {
                            intensityDelete = true;
                        } else if (key == keys::color) {
                            colorDelete = true;
                        } else {
                            return "Can't delete Transition Row: Unknown key entered.";
                        }
                    }
                    QString rowId = keysToId(rowSelection);
                    if (rowId.isEmpty()) {
                        return "Can't delete Transition Row: No valid Row ID given.";
                    }
                    if (intensityDelete || (!(intensityDelete || colorDelete))) {
                        QString result = cues->deleteCueRowIntensity(ids, rowId);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                    if (colorDelete || (!(intensityDelete || colorDelete))) {
                        QString result = cues->deleteCueRowColor(ids, rowId);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                }
            } else {
                return "Unknown item type.";
            }
            if (!result.isEmpty()) { // if the operation failed
                return result;
            }
        } else if (operationType == keys::labelItem) { // LABEL
            if (!operation.isEmpty()) {
                return "Label doesn't take any parameters.";
            }
            QString label = text;
            if (text.isNull()) {
                bool ok = false;
                locker.unlock();
                label = QInputDialog::getText(terminal, QString(), "Label", QLineEdit::Normal, QString(), &ok);
                locker.relock();
                if (!ok) {
                    return "Popup cancelled.";
                }
            }
            label.replace("\"", "");
            QString result;
            if (selectionType == keys::model) { // LABEL MODEL
                result = models->labelModel(ids, label);
            } else if (selectionType == keys::fixture) { // LABEL FIXTURE
                result = fixtures->labelFixture(ids, label);
            } else if (selectionType == keys::group) { // LABEL GROUP
                result = groups->labelGroup(ids, label);
            } else if (selectionType == keys::intensity) { // LABEL INTENSITY
                result = intensities->labelIntensity(ids, label);
            } else if (selectionType == keys::color) { // LABEL COLOR
                result = colors->labelColor(ids, label);
            } else if (selectionType == keys::transition) { // LABEL TRANSITION
                result = transitions->labelTransition(ids, label);
            } else if (selectionType == keys::row) { // LABEL ROW
                result = rows->labelRow(ids, label);
            } else if (selectionType == keys::cue) { // LABEL CUE
                result = cues->labelCue(ids, label);
            } else {
                return "Unknown item type.";
            }
            if (!result.isEmpty()) { // if the operation failed
                return result;
            }
        } else if (operationType == keys::moveItem) { // MOVE
            QString targetId = keysToId(operation);
            if (targetId.isEmpty()) {
                return "Target ID not valid.";
            }
            QString result;
            if (selectionType == keys::model) { // MOVE MODEL
                result = models->moveModel(ids, targetId);
            } else if (selectionType == keys::fixture) { // MOVE FIXTURE
                result = fixtures->moveFixture(ids, targetId);
            } else if (selectionType == keys::group) { // MOVE GROUP
                result = groups->moveGroup(ids, targetId);
            } else if (selectionType == keys::intensity) { // MOVE INTENSITY
                result = intensities->moveIntensity(ids, targetId);
            } else if (selectionType == keys::color) { // MOVE COLOR
                result = colors->moveColor(ids, targetId);
            } else if (selectionType == keys::transition) { // MOVE TRANSITION
                result = transitions->moveTransition(ids, targetId);
            } else if (selectionType == keys::row) { // MOVE ROW
                result = rows->moveRow(ids, targetId);
            } else if (selectionType == keys::cue) { // MOVE CUE
                result = cues->moveCue(ids, targetId);
            } else {
                return "Unknown item type.";
            }
            if (!result.isEmpty()) { // if the operation failed
                return result;
            }
        } else if (operationType == keys::recordItem) { // RECORD
            if (selectionType == keys::model) { // RECORD MODEL
                if (!operation.isEmpty()) {
                    return "Record Model doesn't take any parameters.";
                }
                QString channels = text;
                if (text.isNull()) {
                    bool ok = false;
                    locker.unlock();
                    channels = QInputDialog::getText(terminal, QString(), "Channels", QLineEdit::Normal, QString(), &ok);
                    locker.relock();
                    if (!ok) {
                        return "Popup cancelled.";
                    }
                }
                QString result = models->recordModelChannels(ids, channels);
                if (!result.isEmpty()) {
                    return result;
                }
            } else if (selectionType == keys::fixture) { // RECORD FIXTURE
                if (operation.isEmpty()) {
                    return "Record Fixture requires arguments.";
                }
                int address = 0;
                QList<int> model;
                bool addressFinished = false;
                for (int key : operation) {
                    if (isNumber(key) || key == keys::period) {
                        if (!addressFinished) {
                            if (!isNumber(key)) {
                                return "DMX Address only allows numbers as input";
                            }
                            address = (address * 10) + keyToNumber(key);
                        } else {
                            model.append(key);
                        }
                    } else if (key == keys::model) {
                        if (addressFinished) {
                            return "Record Fixture only takes one Model";
                        }
                        addressFinished = true;
                    } else {
                        return "Record Fixture only takes DMX Address and Model";
                    }
                }
                if (addressFinished) {
                    QString modelId = keysToId(model);
                    if (modelId.isEmpty()) {
                        return "Model selection not valid.";
                    }
                    QString result = fixtures->recordFixtureModel(ids, modelId, address);
                    if (!result.isEmpty()) {
                        return result;
                    }
                } else if (address > 0) {
                    QString result = fixtures->recordFixtureAddress(ids, address);
                    if (!result.isEmpty()) {
                        return result;
                    }
                }
            } else if (selectionType == keys::group) { // RECORD GROUP
                QString result;
                if (operation.isEmpty()) {
                    result = groups->recordGroupFixtures(ids, QList<QString>());
                } else {
                    if (operation[0] != keys::fixture) {
                        return "Can't record Group because no Fixtures were given.";
                    }
                    operation.removeFirst();
                    QList<QString> fixtureIds = keysToSelection(operation);
                    if (fixtureIds.isEmpty()) {
                        return "Can't record Group because of invalid Fixture selection.";
                    }
                    result = groups->recordGroupFixtures(ids, fixtureIds);
                }
                if (!result.isEmpty()) {
                    return result;
                }
            } else if (selectionType == keys::intensity) { // RECORD INTENSITY
                if (operation.isEmpty()) {
                    return "Record Intensity requires arguments.";
                }
                QList<float> values = keysToValue(operation);
                if (values.empty() || values.size() > 1) {
                    return "Invalid values given to Record Intensity.";
                }
                if (values.size() >= 1) {
                    if (values[0] > -999) {
                        QString result = intensities->recordIntensityDimmer(ids, values[0]);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                }
            } else if (selectionType == keys::color) { // RECORD COLOR
                if (operation.isEmpty()) {
                    return "Record Color requires arguments.";
                }
                QList<float> values = keysToValue(operation);
                if (values.empty() || values.size() > 3) {
                    return "Invalid values given to Record Color.";
                }
                if (values.size() >= 1) {
                    if (values[0] > -999) {
                        QString result = colors->recordColorRed(ids, values[0]);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                }
                if (values.size() >= 2) {
                    if (values[1] > -999) {
                        QString result = colors->recordColorGreen(ids, values[1]);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                }
                if (values.size() >= 3) {
                    if (values[2] > -999) {
                        QString result = colors->recordColorBlue(ids, values[2]);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                }
            } else if (selectionType == keys::transition) { // RECORD TRANSITION
                if (operation.isEmpty()) {
                    return "Record Transition requires arguments.";
                }
                QList<float> values = keysToValue(operation);
                if (values.empty() || values.size() > 1) {
                    return "Invalid values given to Record Transition.";
                }
                if (values.size() >= 1) {
                    if (values[0] > -999) {
                        QString result = transitions->recordTransitionFade(ids, values[0]);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                }
            } else if (selectionType == keys::row) { // RECORD ROW
                if (operation.isEmpty()) {
                    return "Record Row requires an argument.";
                }
                if (operation[0] != keys::group) {
                    return "Record Row requires a group.";
                }
                operation.removeFirst();
                QString groupId = keysToId(operation);
                if (groupId.isEmpty()) {
                    return "Group ID not valid.";
                }
                QString result = rows->recordRowGroup(ids, groupId);
                if (!result.isEmpty()) {
                    return result;
                }
            } else if (selectionType == keys::cue) { // RECORD CUE
                if (operation.isEmpty()) {
                    return "Record Cue requires an argument.";
                }
                if (operation[0] == keys::transition) {
                    operation.removeFirst();
                    QString transitionId = keysToId(operation);
                    if (transitionId.isEmpty()) {
                        return "Transition ID not valid.";
                    }
                    QString result = cues->recordCueTransition(ids, transitionId);
                    if (!result.isEmpty()) {
                        return result;
                    }
                } else if (operation[0] == keys::row) {
                    operation.removeFirst();
                    operation.append(keys::intensity);
                    QString intensityId;
                    QString colorId;
                    QString rowId;
                    int lastItem = keys::row;
                    QList<int> selection;
                    for (int key : operation) {
                        if (isNumber(key) || key == keys::period) {
                            selection.append(key);
                        } else if (key == keys::intensity || key == keys::color) {
                            QString selectionId = keysToId(selection);
                            if (selectionId.isEmpty()) {
                                return "Invalid values given to Record Cue.";
                            }
                            selection.clear();
                            if (lastItem == keys::row) {
                                rowId = selectionId;
                            } else if (lastItem == keys::intensity) {
                                intensityId = selectionId;
                            } else if (lastItem == keys::color) {
                                colorId = selectionId;
                            } else {
                                return "Invalid values given to Record Cue.";
                            }
                            lastItem = key;
                        } else {
                            return "Invalid values given to Record Cue.";
                        }
                    }
                    if (rowId.isEmpty()) {
                        return "No row specified.";
                    }
                    if (!intensityId.isEmpty()) {
                        QString result = cues->recordCueIntensity(ids, rowId, intensityId);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                    if (!colorId.isEmpty()) {
                        QString result = cues->recordCueColor(ids, rowId, colorId);
                        if (!result.isEmpty()) {
                            return result;
                        }
                    }
                } else {
                    return "Record Cue needs either a Row or a Transition as first argument.";
                }
            } else {
                return "Unknown item type.";
            }
        } else {
            return "Unknown operator.";
        }
    }
    cuelistView->loadCue();
    return QString();
}

QString Kernel::keysToId(QList<int> keys)
{
    QString id;
    for (const int key : keys) {
        if (isNumber(key)){
            id += QString::number(keyToNumber(key));
        } else if (key == keys::period) {
            id += ".";
        } else {
            return QString();
        }
    }
    while (id.count(".") < 4) {
        id += ".0";
    }
    if (!id.contains(QRegularExpression("^[0-9]+.[0-9]+.[0-9]+.[0-9]+.[0-9]+$"))) {
        return QString();
    }
    return id;
}

QList<float> Kernel::keysToValue(QList<int> keys)
{
    keys.append(keys::comma);
    QList<float> values;
    QString value;
    for (const int key : keys) {
        if (isNumber(key)) {
            value += QString::number(keyToNumber(key));
        } else if (key == keys::period) {
            if (value.contains(".")) {
                return QList<float>();
            }
            value += ".";
        } else if (key == keys::comma) {
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
    if (!keys.endsWith(keys::comma)) {
        keys.append(keys::comma);
    }
    QList<QString> ids;
    QList<int> idKeys;
    for (int key : keys) {
        if (isNumber(key) || key == keys::period) {
            idKeys += key;
        } else if (key == keys::comma) {
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
    return ((key == keys::model) || (key == keys::fixture) || (key == keys::group) || (key == keys::intensity) || (key == keys::color) || (key == keys::transition) || (key == keys::row) || (key == keys::cue));
}

bool Kernel::isOperator(int key)
{
    return ((key == keys::copyItem) || (key == keys::deleteItem) || (key == keys::labelItem) || (key == keys::moveItem) || (key == keys::recordItem));
}

bool Kernel::isNumber(int key)
{
    return ((key == keys::zero) || (key == keys::one) || (key == keys::two) || (key == keys::three) || (key == keys::four) || (key == keys::five) || (key == keys::six) || (key == keys::seven) || (key == keys::eight) || (key == keys::nine));
}

int Kernel::keyToNumber(int key)
{
    if (key == keys::zero) {
        return 0;
    } else if (key == keys::one) {
        return 1;
    } else if (key == keys::two) {
        return 2;
    } else if (key == keys::three) {
        return 3;
    } else if (key == keys::four) {
        return 4;
    } else if (key == keys::five) {
        return 5;
    } else if (key == keys::six) {
        return 6;
    } else if (key == keys::seven) {
        return 7;
    } else if (key == keys::eight) {
        return 8;
    } else if (key == keys::nine) {
        return 9;
    } else {
        return false;
    }
}
