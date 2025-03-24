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
    QMutexLocker locker(mutex);
    models->reset();
    fixtures->reset();
    groups->reset();
    intensities->reset();
    colors->reset();
    raws->reset();
    cues->reset();
    cuelistView->dmxEngine->sacnServer->universeSpinBox->setValue(cuelistView->dmxEngine->sacnServer->SACN_STANDARD_UNIVERSE); // reset sACN universe
    cuelistView->dmxEngine->sacnServer->prioritySpinBox->setValue(cuelistView->dmxEngine->sacnServer->SACN_STANDARD_PRIORITY); // reset sACN priority
    cuelistView->loadCue();
}

void Kernel::saveFile(QString fileName) {
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
    fileStream.writeTextElement("Version", VERSION);
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

void Kernel::openFile(QString fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        terminal->error("Can't open file.");
        return;
    }
    QXmlStreamReader fileStream(&file);
    reset();
    QMutexLocker locker(mutex);
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
                        if (fileStream.readElementText() != VERSION) {
                            terminal->error("Error reading file: This Zöglfrex version isn't compatible to the current version (" + VERSION + ").");
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
                    QStringList ids = {fileStream.attributes().value("ID").toString()};
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
    cuelistView->loadCue();
}
