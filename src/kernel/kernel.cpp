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

void Kernel::saveFile(QString fileName, QString version) {
    QMutexLocker locker(mutex);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        terminal->error("Unable to save file.");
        return;
    }
    QXmlStreamWriter fileStream(&file);
    fileStream.setAutoFormatting(true);
    fileStream.writeStartDocument();
    fileStream.writeStartElement("Workspace");

    fileStream.writeStartElement("Creator");
    fileStream.writeTextElement("Name", "Zöglfrex");
    fileStream.writeTextElement("Version", version);
    fileStream.writeEndElement();

    fileStream.writeStartElement("Output");
    fileStream.writeTextElement("Universe", QString::number(cuelistView->dmxEngine->sacnServer->universeSpinBox->value()));
    fileStream.writeTextElement("Priority", QString::number(cuelistView->dmxEngine->sacnServer->prioritySpinBox->value()));
    fileStream.writeEndElement();

    models->saveItemsToFile(&fileStream);
    fixtures->saveItemsToFile(&fileStream);
    groups->saveItemsToFile(&fileStream);
    intensities->saveItemsToFile(&fileStream);
    colors->saveItemsToFile(&fileStream);
    raws->saveItemsToFile(&fileStream);
    cues->saveItemsToFile(&fileStream);

    fileStream.writeEndElement();
    fileStream.writeEndDocument();

    terminal->success("Saved file as " + fileName);
}

void Kernel::openFile(QString fileName, QString version) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        terminal->error("Can't open file.");
        return;
    }
    QXmlStreamReader fileStream(&file);
    reset();
    if (fileStream.readNextStartElement() && (fileStream.name().toString() == "Workspace")) {
        while (fileStream.readNextStartElement()) {
            if (fileStream.name().toString() == "Creator") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() == "Name") {
                        if (fileStream.readElementText() != "Zöglfrex") {
                            terminal->error("This is not a Zöglfrex file.");
                            return;
                        }
                    } else if (fileStream.name().toString() == "Version") {
                        if (fileStream.readElementText() != version) {
                            terminal->error("Error reading file: This Zöglfrex version isn't compatible to the current version (" + version + ").");
                            return;
                        }
                    } else {
                        terminal->error("Error reading file: Received unknown Creator attribute " + fileStream.name().toString());
                        return;
                    }
                }
            } else if (fileStream.name().toString() == "Output") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() == "Universe") {
                        bool ok;
                        int universe = fileStream.readElementText().toInt(&ok);
                        if (!ok) {
                            terminal->error("Error reading file: Invalid sACN universe given.");
                            return;
                        }
                        cuelistView->dmxEngine->sacnServer->universeSpinBox->setValue(universe);
                    } else if (fileStream.name().toString() == "Priority") {
                        bool ok;
                        int priority = fileStream.readElementText().toInt(&ok);
                        if (!ok) {
                            terminal->error("Error reading file: Invalid sACN priority given.");
                            return;
                        }
                        cuelistView->dmxEngine->sacnServer->prioritySpinBox->setValue(priority);
                    } else {
                        terminal->error("Error reading file: Received unknown Output attribute " + fileStream.name().toString());
                        return;
                    }
                }
            } else {
                QString pluralName = fileStream.name().toString();
                while (fileStream.readNextStartElement()) {
                    QString singularName = fileStream.name().toString();
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        terminal->error("Error reading file: No ID for " + singularName + " was given.");
                        return;
                    }
                    QList<QString> ids = {fileStream.attributes().value("ID").toString()};
                    while (fileStream.readNextStartElement()) {
                        if ((fileStream.name().toString() != "Attribute") || !fileStream.attributes().hasAttribute("ID")) {
                            terminal->error("Error reading file: Expected " + singularName + " Attribute.");
                            return;
                        }
                        QMap<int, QString> attributes;
                        attributes[Keys::Attribute] = fileStream.attributes().value("ID").toString();
                        if (fileStream.attributes().hasAttribute("Model")) {
                            attributes[Keys::Model] = fileStream.attributes().value("Model").toString();
                        }
                        if (fileStream.attributes().hasAttribute("Fixture")) {
                            attributes[Keys::Fixture] = fileStream.attributes().value("Fixture").toString();
                        }
                        if (fileStream.attributes().hasAttribute("Group")) {
                            attributes[Keys::Group] = fileStream.attributes().value("Group").toString();
                        }
                        if (fileStream.attributes().hasAttribute("Intensity")) {
                            attributes[Keys::Intensity] = fileStream.attributes().value("Intensity").toString();
                        }
                        if (fileStream.attributes().hasAttribute("Color")) {
                            attributes[Keys::Color] = fileStream.attributes().value("Color").toString();
                        }
                        if (fileStream.attributes().hasAttribute("Raw")) {
                            attributes[Keys::Raw] = fileStream.attributes().value("Raw").toString();
                        }
                        if (fileStream.attributes().hasAttribute("Cue")) {
                            attributes[Keys::Cue] = fileStream.attributes().value("Cue").toString();
                        }
                        QString text = fileStream.readElementText();
                        if ((pluralName == "Models") && (singularName == "Model")) {
                            models->setAttribute(ids, attributes, QList<int>(), text);
                        } else if ((pluralName == "Fixtures") && (singularName == "Fixture")) {
                            fixtures->setAttribute(ids, attributes, QList<int>(), text);
                        } else if ((pluralName == "Groups") && (singularName == "Group")) {
                            groups->setAttribute(ids, attributes, QList<int>(), text);
                        } else if ((pluralName == "Intensities") && (singularName == "Intensity")) {
                            intensities->setAttribute(ids, attributes, QList<int>(), text);
                        } else if ((pluralName == "Colors") && (singularName == "Color")) {
                            colors->setAttribute(ids, attributes, QList<int>(), text);
                        } else if ((pluralName == "Raws") && (singularName == "Raw")) {
                            raws->setAttribute(ids, attributes, QList<int>(), text);
                        } else if ((pluralName == "Cues") && (singularName == "Cue")) {
                            cues->setAttribute(ids, attributes, QList<int>(), text);
                        } else {
                            terminal->error("Error reading file: Expected Object Type");
                            return;
                        }
                    }
                }
            }
        }
    } else {
        terminal->error("Error reading file: Expected Zöglfrex workspace.");
        return;
    }
    if (fileStream.hasError()) {
        terminal->error("Can't open file because a XML parsing error occured in line " + QString::number(fileStream.lineNumber()) + ": " + fileStream.errorString() + " (" + QString::number(fileStream.error()) + ")");
        return;
    }
    terminal->success("Opened File " + fileName);
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
