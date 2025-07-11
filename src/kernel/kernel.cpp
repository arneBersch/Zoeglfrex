/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "kernel.h"

Kernel::Kernel(MainWindow* window) {
    mainWindow = window;
    models = new ModelTable(this);
    fixtures = new FixtureTable(this);
    groups = new GroupTable(this);
    intensities = new IntensityTable(this);
    colors = new ColorTable(this);
    positions = new PositionTable(this);
    raws = new RawTable(this);
    effects = new EffectTable(this);
    cuelists = new CuelistTable(this);
    terminal = new Terminal(this);
    inspector = new Inspector(this);
    dmxEngine = new DmxEngine(this);
    cuelistView = new CuelistView(this);
    preview2d = new Preview2d(this);
    controlPanel = new ControlPanel(this);
    playbackMonitor = new PlaybackMonitor(this);
    mutex = new QMutex();
}

void Kernel::reset() {
    QMutexLocker locker(mutex);
    models->reset();
    fixtures->reset();
    groups->reset();
    intensities->reset();
    colors->reset();
    positions->reset();
    raws->reset();
    effects->reset();
    cuelists->reset();

    dmxEngine->sacnServer->prioritySpinBox->setValue(dmxEngine->sacnServer->SACN_STANDARD_PRIORITY);
    cuelistView->cueViewModeComboBox->setCurrentIndex(CuelistViewModes::cueMode);
    cuelistView->trackingButton->setChecked(true);
    cuelistView->cueViewRowFilterComboBox->setCurrentIndex(CuelistViewRowFilters::noFilter);
    dmxEngine->blindButton->setChecked(false);
    dmxEngine->highlightButton->setChecked(false);
    dmxEngine->soloButton->setChecked(false);
    dmxEngine->skipFadeButton->setChecked(false);
    cuelistView->reload();
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
    fileStream.writeTextElement("Priority", QString::number(dmxEngine->sacnServer->prioritySpinBox->value()));
    fileStream.writeEndElement();

    models->saveItemsToFile(&fileStream);
    fixtures->saveItemsToFile(&fileStream);
    groups->saveItemsToFile(&fileStream);
    intensities->saveItemsToFile(&fileStream);
    colors->saveItemsToFile(&fileStream);
    positions->saveItemsToFile(&fileStream);
    raws->saveItemsToFile(&fileStream);
    effects->saveItemsToFile(&fileStream);
    cuelists->saveItemsToFile(&fileStream);

    fileStream.writeStartElement("Interface");
    fileStream.writeTextElement("CuelistViewMode", QString::number(cuelistView->cueViewModeComboBox->currentIndex()));
    fileStream.writeTextElement("Tracking", QString::number(cuelistView->trackingButton->isChecked()));
    fileStream.writeTextElement("CuelistViewRowFilter", QString::number(cuelistView->cueViewRowFilterComboBox->currentIndex()));
    fileStream.writeTextElement("Blind", QString::number(dmxEngine->blindButton->isChecked()));
    fileStream.writeTextElement("Highlight", QString::number(dmxEngine->highlightButton->isChecked()));
    fileStream.writeTextElement("Solo", QString::number(dmxEngine->soloButton->isChecked()));
    fileStream.writeTextElement("SkipFade", QString::number(dmxEngine->skipFadeButton->isChecked()));
    if (cuelistView->currentGroup != nullptr) {
        fileStream.writeTextElement("CurrentGroup", cuelistView->currentGroup->id);
    }
    if (cuelistView->currentFixture != nullptr) {
        fileStream.writeTextElement("CurrentFixture", cuelistView->currentFixture->id);
    }
    if (cuelistView->currentCuelist != nullptr) {
        fileStream.writeTextElement("CurrentCuelist", cuelistView->currentCuelist->id);
    }
    fileStream.writeEndElement();

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
    cuelistView->trackingButton->setChecked(false);
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
                    if (fileStream.name().toString() == "Priority") {
                        bool ok;
                        int priority = fileStream.readElementText().toInt(&ok);
                        if (!ok) {
                            terminal->error("Error reading file: Invalid sACN priority given.");
                            return;
                        }
                        dmxEngine->sacnServer->prioritySpinBox->setValue(priority);
                    } else {
                        terminal->error("Error reading file: Received unknown Output attribute " + fileStream.name().toString());
                        return;
                    }
                }
            } else if (fileStream.name().toString() == "Interface") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() == "CuelistViewMode") {
                        bool ok;
                        int index = fileStream.readElementText().toInt(&ok);
                        if (!ok) {
                            terminal->error("Error reading file: Invalid Cue View Mode given.");
                            return;
                        }
                        cuelistView->cueViewModeComboBox->setCurrentIndex(index);
                    } else if (fileStream.name().toString() == "Tracking") {
                        QString trackingValue = fileStream.readElementText();
                        if (trackingValue == "0") {
                            cuelistView->trackingButton->setChecked(false);
                        } else if (trackingValue == "1") {
                            cuelistView->trackingButton->setChecked(true);
                        } else {
                            terminal->error("Error reading file: Invalid Cuelist View Tracking value given.");
                            return;
                        }
                    } else if (fileStream.name().toString() == "CuelistViewRowFilter") {
                        bool ok;
                        int index = fileStream.readElementText().toInt(&ok);
                        if (!ok) {
                            terminal->error("Error reading file: Invalid Cue View Row Filter given.");
                            return;
                        }
                        cuelistView->cueViewRowFilterComboBox->setCurrentIndex(index);
                    } else if (fileStream.name().toString() == "Blind") {
                        QString blindValue = fileStream.readElementText();
                        if (blindValue == "0") {
                            dmxEngine->blindButton->setChecked(false);
                        } else if (blindValue == "1") {
                            dmxEngine->blindButton->setChecked(true);
                        } else {
                            terminal->error("Error reading file: Invalid Blind value given.");
                            return;
                        }
                    } else if (fileStream.name().toString() == "Highlight") {
                        QString highlightValue = fileStream.readElementText();
                        if (highlightValue == "0") {
                            dmxEngine->highlightButton->setChecked(false);
                        } else if (highlightValue == "1") {
                            dmxEngine->highlightButton->setChecked(true);
                        } else {
                            terminal->error("Error reading file: Invalid Highlight value given.");
                            return;
                        }
                    } else if (fileStream.name().toString() == "Solo") {
                        QString soloValue = fileStream.readElementText();
                        if (soloValue == "0") {
                            dmxEngine->soloButton->setChecked(false);
                        } else if (soloValue == "1") {
                            dmxEngine->soloButton->setChecked(true);
                        } else {
                            terminal->error("Error reading file: Invalid Solo value given.");
                            return;
                        }
                    } else if (fileStream.name().toString() == "SkipFade") {
                        QString skipFadeValue = fileStream.readElementText();
                        if (skipFadeValue == "0") {
                            dmxEngine->skipFadeButton->setChecked(false);
                        } else if (skipFadeValue == "1") {
                            dmxEngine->skipFadeButton->setChecked(true);
                        } else {
                            terminal->error("Error reading file: Invalid Skip Fade value given.");
                            return;
                        }
                    } else if (fileStream.name().toString() == "CurrentGroup") {
                        cuelistView->loadGroup(fileStream.readElementText());
                    } else if (fileStream.name().toString() == "CurrentFixture") {
                        cuelistView->loadFixture(fileStream.readElementText());
                    } else if (fileStream.name().toString() == "CurrentCuelist") {
                        cuelistView->loadCuelist(fileStream.readElementText());
                    } else {
                        terminal->error("Error reading file: Received unknown Interface attribute " + fileStream.name().toString());
                        return;
                    }
                }
            } else { // <Items>
                QString pluralName = fileStream.name().toString();
                while (fileStream.readNextStartElement()) { // <Item>
                    QString singularName = fileStream.name().toString();
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        terminal->error("Error reading file: No " + singularName + " ID was given.");
                        return;
                    }
                    QStringList ids = {fileStream.attributes().value("ID").toString()};
                    while (fileStream.readNextStartElement()) { // <Attribute> or <Cues>
                        if ((fileStream.name().toString() == "Attribute") && fileStream.attributes().hasAttribute("ID")) {
                            QMap<int, QString> attributes = xmlToCommandAttributes(fileStream.attributes());
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
                            } else if ((pluralName == "Positions") && (singularName == "Position")) {
                                positions->setAttribute(ids, attributes, QList<int>(), text);
                            } else if ((pluralName == "Raws") && (singularName == "Raw")) {
                                raws->setAttribute(ids, attributes, QList<int>(), text);
                            } else if ((pluralName == "Effects") && (singularName == "Effect")) {
                                effects->setAttribute(ids, attributes, QList<int>(), text);
                            } else if ((pluralName == "Cuelists") && (singularName == "Cuelist")) {
                                cuelists->setAttribute(ids, attributes, QList<int>(), text);
                            } else {
                                terminal->error("Error reading file: Expected Object Type");
                                return;
                            }
                        } else if ((fileStream.name().toString() == "Cues") && (pluralName == "Cuelists") && (singularName == "Cuelist")) {
                            cuelistView->loadCuelist(ids.first());
                            while (fileStream.readNextStartElement()) { // <Cue>
                                if (fileStream.name().toString() != "Cue") {
                                    terminal->error("Error reading file: Expected Cue.");
                                    return;
                                }
                                if (!fileStream.attributes().hasAttribute("ID")) {
                                    terminal->error("Error reading file: No Cue ID was given.");
                                    return;
                                }
                                QString cueId = fileStream.attributes().value("ID").toString();
                                while (fileStream.readNextStartElement()) { // <Attribute>
                                    if ((fileStream.name().toString() != "Attribute") || !fileStream.attributes().hasAttribute("ID")) {
                                        terminal->error("Error reading file: Expected Cue Attribute.");
                                        return;
                                    }
                                    QMap<int, QString> attributes = xmlToCommandAttributes(fileStream.attributes());
                                    cuelistView->currentCuelist->cues->setAttribute({cueId}, attributes, QList<int>(), fileStream.readElementText());
                                }
                            }
                        } else {
                            terminal->error("Error reading file: Expected " + singularName + " Attribute.");
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
    cuelistView->reload();
}

QMap<int, QString> Kernel::xmlToCommandAttributes(QXmlStreamAttributes streamAttributes) {
    QMap<int, QString> attributes;
    attributes[Keys::Attribute] = streamAttributes.value("ID").toString();
    if (streamAttributes.hasAttribute("Model")) {
        attributes[Keys::Model] = streamAttributes.value("Model").toString();
    }
    if (streamAttributes.hasAttribute("Fixture")) {
        attributes[Keys::Fixture] = streamAttributes.value("Fixture").toString();
    }
    if (streamAttributes.hasAttribute("Group")) {
        attributes[Keys::Group] = streamAttributes.value("Group").toString();
    }
    if (streamAttributes.hasAttribute("Intensity")) {
        attributes[Keys::Intensity] = streamAttributes.value("Intensity").toString();
    }
    if (streamAttributes.hasAttribute("Color")) {
        attributes[Keys::Color] = streamAttributes.value("Color").toString();
    }
    if (streamAttributes.hasAttribute("Position")) {
        attributes[Keys::Position] = streamAttributes.value("Position").toString();
    }
    if (streamAttributes.hasAttribute("Raw")) {
        attributes[Keys::Raw] = streamAttributes.value("Raw").toString();
    }
    if (streamAttributes.hasAttribute("Effect")) {
        attributes[Keys::Effect] = streamAttributes.value("Effect").toString();
    }
    if (streamAttributes.hasAttribute("Cuelist")) {
        attributes[Keys::Cuelist] = streamAttributes.value("Cuelist").toString();
    }
    if (streamAttributes.hasAttribute("Cue")) {
        attributes[Keys::Cue] = streamAttributes.value("Cue").toString();
    }
    return attributes;
};
