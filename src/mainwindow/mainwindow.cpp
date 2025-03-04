/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    qInfo() << "Zöglfrex" << VERSION;
    qInfo() << COPYRIGHT;
    qInfo() << "Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.";
    qInfo() << "Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.";
    qInfo() << "You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.";

    this->resize(1500, 1000);
    this->setWindowTitle("Zöglfrex");

    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *newFileAction = new QAction("New File (CTRL+N)");
    fileMenu->addAction(newFileAction);
    connect(newFileAction, &QAction::triggered, this, &MainWindow::newFile);
    QAction *openFileAction = new QAction("Open File (CTRL+O)");
    fileMenu->addAction(openFileAction);
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openFile);
    QAction *saveFileAction = new QAction("Save File (CTRL+S)");
    fileMenu->addAction(saveFileAction);
    connect(saveFileAction, &QAction::triggered, this, &MainWindow::saveFile);
    QAction *saveFileAsAction = new QAction("Save File as (CTRL+SHIFT+S)");
    fileMenu->addAction(saveFileAsAction);
    connect(saveFileAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    fileMenu->addSeparator();
    QAction *quitAction = new QAction("Quit (CTRL+Q)");
    fileMenu->addAction(quitAction);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    QMenu *outputMenu = menuBar()->addMenu("Output");
    QAction *outputSettingsAction = new QAction("DMX Output Settings");
    outputMenu->addAction(outputSettingsAction);
    connect(outputSettingsAction, &QAction::triggered, this, [this]{ kernel->cuelistView->dmxEngine->sacnServer->show(); });
    outputMenu->addSeparator();
    QAction *goAction = new QAction("Go to next Cue (Space)");
    outputMenu->addAction(goAction);
    connect(goAction, &QAction::triggered, this, [this]{ kernel->cuelistView->nextCue(); });
    QAction *goBackAction = new QAction("Go to previous Cue (SHIFT+Space)");
    outputMenu->addAction(goBackAction);
    connect(goBackAction, &QAction::triggered, this, [this]{ kernel->cuelistView->previousCue(); });

    QMenu *helpMenu = menuBar()->addMenu("Help");
    QAction *aboutAction = new QAction("About Zöglfrex");
    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    QAction *openGuideAction = new QAction("Quick Start Guide");
    helpMenu->addAction(openGuideAction);
    connect(openGuideAction, &QAction::triggered, this, []{ QDesktopServices::openUrl(QUrl("https://github.com/arneBersch/Zoeglfrex/blob/main/docs/quick_start_guide.md")); });
    QAction *openReferenceAction = new QAction("Reference");
    helpMenu->addAction(openReferenceAction);
    connect(openReferenceAction, &QAction::triggered, this, []{ QDesktopServices::openUrl(QUrl("https://github.com/arneBersch/Zoeglfrex/blob/main/docs/reference.md")); });

    kernel = new Kernel();

    QSplitter *leftColumn = new QSplitter();
    leftColumn->setOrientation(Qt::Vertical);
    leftColumn->setChildrenCollapsible(false);
    leftColumn->addWidget(kernel->cuelistView);
    leftColumn->addWidget(kernel->terminal);
    leftColumn->setSizes(QList<int>() << 300 << 100);

    QSplitter *rightColumn = new QSplitter();
    rightColumn->setOrientation(Qt::Vertical);
    rightColumn->setChildrenCollapsible(false);
    rightColumn->addWidget(kernel->inspector);

    QSplitter *mainColumns = new QSplitter(); // The main window consists of the left and right column
    mainColumns->setChildrenCollapsible(false);
    mainColumns->addWidget(leftColumn);
    mainColumns->addWidget(rightColumn);
    mainColumns->setSizes(QList<int>() << 1000 << 500);
    this->setCentralWidget(mainColumns);

    connect(new QShortcut(QKeySequence(Qt::Key_Plus), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Plus); }); // +
    connect(new QShortcut(QKeySequence(Qt::Key_Minus), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Minus); }); // -
    connect(new QShortcut(QKeySequence(Qt::Key_Comma), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Period); }); // , (appends .)
    connect(new QShortcut(QKeySequence(Qt::Key_Period), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Period); }); // .
    connect(new QShortcut(QKeySequence(Qt::Key_T), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Thru); }); // Thru
    connect(new QShortcut(QKeySequence(Qt::Key_0), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Zero); }); // 0
    connect(new QShortcut(QKeySequence(Qt::Key_1), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::One); }); // 1
    connect(new QShortcut(QKeySequence(Qt::Key_2), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Two); }); // 2
    connect(new QShortcut(QKeySequence(Qt::Key_3), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Three); }); // 3
    connect(new QShortcut(QKeySequence(Qt::Key_4), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Four); }); // 4
    connect(new QShortcut(QKeySequence(Qt::Key_5), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Five); }); // 5
    connect(new QShortcut(QKeySequence(Qt::Key_6), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Six); }); // 6
    connect(new QShortcut(QKeySequence(Qt::Key_7), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Seven); }); // 7
    connect(new QShortcut(QKeySequence(Qt::Key_8), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Eight); }); // 8
    connect(new QShortcut(QKeySequence(Qt::Key_9), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Nine); }); // 9
    connect(new QShortcut(QKeySequence(Qt::Key_A), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Attribute); }); // Attribute
    connect(new QShortcut(QKeySequence(Qt::Key_C), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Color); }); // Color
    connect(new QShortcut(QKeySequence(Qt::Key_F), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Fixture); }); // Fixture
    connect(new QShortcut(QKeySequence(Qt::Key_G), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Group); }); // Group
    connect(new QShortcut(QKeySequence(Qt::Key_I), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Intensity); }); // Intensity
    connect(new QShortcut(QKeySequence(Qt::Key_M), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Model); }); // Model
    connect(new QShortcut(QKeySequence(Qt::Key_Q), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Cue); }); // Cue
    connect(new QShortcut(QKeySequence(Qt::Key_R), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Raw); }); // Raw
    connect(new QShortcut(QKeySequence(Qt::Key_S), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Set); }); // Set
    connect(new QShortcut(QKeySequence(Qt::Key_Return), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); }); // Enter Command (via Return key)
    connect(new QShortcut(QKeySequence(Qt::Key_Enter), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); }); // Enter Command (via Keypad Enter key)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(false); }); // Enter Command (via Shift + Return key)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Enter), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(false); }); // Enter Command (via Shift + Enter key)
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), &QShortcut::activated, this, [this]{ kernel->terminal->backspace(); }); // Backspace (Remove last keypress)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Backspace), this), &QShortcut::activated, this, [this]{ kernel->terminal->clear(); }); // Clear Terminal
    connect(new QShortcut(QKeySequence(Qt::Key_Space), this), &QShortcut::activated, this, [this]{ kernel->cuelistView->nextCue(); }); // Go to next Cue
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Space), this), &QShortcut::activated, this, [this]{ kernel->cuelistView->previousCue(); }); // Go to previous Cue
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this), &QShortcut::activated, this, [this]{ this->newFile(); }); // New File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this), &QShortcut::activated, this, [this]{ this->openFile(); }); // Open File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this), &QShortcut::activated, this, [this]{ this->saveFile(); }); // Save File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this), &QShortcut::activated, this, [this]{ this->saveFileAs(); }); // Save File As
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this), &QShortcut::activated, this, [this]{ this->close(); }); // Quit Application

    this->show();
    about(); // Open about window
}

MainWindow::~MainWindow() {
}

void MainWindow::openFile() {
    QString newFileName = QFileDialog::getOpenFileName(this, "Open File", QString(), "zfr Files (*.zfr)");
    if (newFileName.isEmpty()) {
        return;
    }
    QFile file(newFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kernel->terminal->error("Can't open file.");
        return;
    }
    QXmlStreamReader fileStream(&file);
    kernel->reset();
    if (fileStream.readNextStartElement() && (fileStream.name().toString() == "Workspace")) {
        while (fileStream.readNextStartElement()) {
            if (fileStream.name().toString() == "Creator") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() == "Name") {
                        if (fileStream.readElementText() != "Zöglfrex") {
                            kernel->terminal->error("This is not a Zöglfrex file.");
                            return;
                        }
                    } else if (fileStream.name().toString() == "Version") {
                        if (fileStream.readElementText() != VERSION) {
                            kernel->terminal->error("Error reading file: This Zöglfrex version isn't compatible to the current version (" + VERSION + ").");
                            return;
                        }
                    } else {
                        kernel->terminal->error("Error reading file: Received unknown Creator attribute " + fileStream.name().toString());
                        return;
                    }
                }
            } else if (fileStream.name().toString() == "Output") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() == "Universe") {
                        bool ok;
                        int universe = fileStream.readElementText().toInt(&ok);
                        if (!ok) {
                            kernel->terminal->error("Error reading file: Invalid sACN universe given.");
                            return;
                        }
                        kernel->cuelistView->dmxEngine->sacnServer->universeSpinBox->setValue(universe);
                    } else if (fileStream.name().toString() == "Priority") {
                        bool ok;
                        int priority = fileStream.readElementText().toInt(&ok);
                        if (!ok) {
                            kernel->terminal->error("Error reading file: Invalid sACN priority given.");
                            return;
                        }
                        kernel->cuelistView->dmxEngine->sacnServer->prioritySpinBox->setValue(priority);
                    } else {
                        kernel->terminal->error("Error reading file: Received unknown Output attribute " + fileStream.name().toString());
                        return;
                    }
                }
            } else if (fileStream.name().toString() == "Models") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() != "Model") {
                        kernel->terminal->error("Error reading file: Expected Model data.");
                        return;
                    }
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        kernel->terminal->error("Error reading file: No Model ID was given.");
                        return;
                    }
                    Model* model = kernel->models->addItem(fileStream.attributes().value("ID").toString());
                    while (fileStream.readNextStartElement()) {
                        if (fileStream.name().toString() == "Label") {
                            model->label = fileStream.readElementText();
                        } else if (fileStream.name().toString() == "Channels") {
                            model->channels = fileStream.readElementText();
                        } else {
                            kernel->terminal->error("Error reading file: Unknown Model Attribute \"" + fileStream.name().toString() + "\".");
                            return;
                        }
                    }
                }
            } else if (fileStream.name().toString() == "Fixtures") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() != "Fixture") {
                        kernel->terminal->error("Error reading file: Expected Fixture data.");
                        return;
                    }
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        kernel->terminal->error("Error reading file: No Fixture ID was given.");
                        return;
                    }
                    Fixture* fixture = kernel->fixtures->addItem(fileStream.attributes().value("ID").toString());
                    while (fileStream.readNextStartElement()) {
                        if (fileStream.name().toString() == "Label") {
                            fixture->label = fileStream.readElementText();
                        } else if (fileStream.name().toString() == "Model") {
                            Model* model = kernel->models->getItem(fileStream.readElementText());
                            if (model == nullptr) {
                                kernel->terminal->error("Error reading file: Invalid Model given for Fixture " + fixture->id);
                                return;
                            }
                            fixture->model = model;
                        } else if (fileStream.name().toString() == "Address") {
                            bool ok;
                            int address = fileStream.readElementText().toInt(&ok);
                            if (!ok) {
                                kernel->terminal->error("Error reading file: Fixture Address of Fixture " + fixture->id + " is not valid.");
                                return;
                            }
                            fixture->address = address;
                        } else {
                            kernel->terminal->error("Error reading file: Unknown Fixture Attribute \"" + fileStream.name().toString() + "\".");
                            return;
                        }
                    }
                }
            } else if (fileStream.name().toString() == "Groups") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() != "Group") {
                        kernel->terminal->error("Error reading file: Expected Group data.");
                        return;
                    }
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        kernel->terminal->error("Error reading file: No Group ID was given.");
                        return;
                    }
                    Group* group = kernel->groups->addItem(fileStream.attributes().value("ID").toString());
                    while (fileStream.readNextStartElement()) {
                        if (fileStream.name().toString() == "Label") {
                            group->label = fileStream.readElementText();
                        } else if (fileStream.name().toString() == "Fixtures") {
                            while (fileStream.readNextStartElement()) {
                                if (fileStream.name().toString() != "Fixture") {
                                    kernel->terminal->error("Error reading file: Expected Fixture for Group " + group->id);
                                    return;
                                }
                                Fixture* fixture = kernel->fixtures->getItem(fileStream.readElementText());
                                if (fixture == nullptr) {
                                    kernel->terminal->error("Error reading file: Fixture of Group " + group->id + " was not found.");
                                    return;
                                }
                                group->fixtures.append(fixture);
                            }
                        } else {
                            kernel->terminal->error("Error reading file: Unknown Group Attribute \"" + fileStream.name().toString() + "\".");
                            return;
                        }
                    }
                }
            } else if (fileStream.name().toString() == "Intensities") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() != "Intensity") {
                        kernel->terminal->error("Error reading file: Expected Intensity data.");
                        return;
                    }
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        kernel->terminal->error("Error reading file: No Intensity ID was given.");
                        return;
                    }
                    Intensity* intensity = kernel->intensities->addItem(fileStream.attributes().value("ID").toString());
                    while (fileStream.readNextStartElement()) {
                        if (fileStream.name().toString() == "Label") {
                            intensity->label = fileStream.readElementText();
                        } else if (fileStream.name().toString() == "Dimmer") {
                            bool ok;
                            float dimmer = fileStream.readElementText().toFloat(&ok);
                            if (!ok) {
                                kernel->terminal->error("Error reading file: Dimmer of Intensity " + intensity->id + " isn't valid.");
                                return;
                            }
                            intensity->dimmer = dimmer;
                        } else {
                            kernel->terminal->error("Error reading file: Unknown Intensity Attribute \"" + fileStream.name().toString() + "\".");
                            return;
                        }
                    }
                }
            } else if (fileStream.name().toString() == "Colors") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() != "Color") {
                        kernel->terminal->error("Error reading file: Expected Color data.");
                        return;
                    }
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        kernel->terminal->error("Error reading file: No Color ID was given.");
                        return;
                    }
                    Color* color = kernel->colors->addItem(fileStream.attributes().value("ID").toString());
                    while (fileStream.readNextStartElement()) {
                        if (fileStream.name().toString() == "Label") {
                            color->label = fileStream.readElementText();
                        } else if (fileStream.name().toString() == "Hue") {
                            bool ok;
                            float hue = fileStream.readElementText().toFloat(&ok);
                            if (!ok) {
                                kernel->terminal->error("Error reading file: Hue of Color " + color->id + " isn't valid.");
                                return;
                            }
                            color->hue = hue;
                        } else if (fileStream.name().toString() == "Saturation") {
                            bool ok;
                            float saturation = fileStream.readElementText().toFloat(&ok);
                            if (!ok) {
                                kernel->terminal->error("Error reading file: Saturation of Color " + color->id + " isn't valid.");
                                return;
                            }
                            color->floatAttributes[kernel->colors->SATURATIONATTRIBUTEID] = saturation;
                        } else {
                            kernel->terminal->error("Error reading file: Unknown Color Attribute \"" + fileStream.name().toString() + "\".");
                            return;
                        }
                    }
                }
            } else if (fileStream.name().toString() == "Raws") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() != "Raw") {
                        kernel->terminal->error("Error reading file: Expected Raw data.");
                        return;
                    }
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        kernel->terminal->error("Error reading file: No Raw ID was given.");
                        return;
                    }
                    Raw* raw = kernel->raws->addItem(fileStream.attributes().value("ID").toString());
                    while (fileStream.readNextStartElement()) {
                        if (fileStream.name().toString() == "Label") {
                            raw->label = fileStream.readElementText();
                        } else if (fileStream.name().toString() == "Channel") {
                            bool ok;
                            int channel = fileStream.readElementText().toInt(&ok);
                            if (!ok) {
                                kernel->terminal->error("Error reading file: Channel of Raw " + raw->id + " isn't valid.");
                                return;
                            }
                            raw->channel = channel;
                        } else if (fileStream.name().toString() == "Value") {
                            bool ok;
                            uint8_t value = fileStream.readElementText().toUInt(&ok);
                            if (!ok) {
                                kernel->terminal->error("Error reading file: Value of Raw " + raw->id + " isn't valid.");
                                return;
                            }
                            raw->value = value;
                        } else {
                            kernel->terminal->error("Error reading file: Unknown Raw Attribute \"" + fileStream.name().toString() + "\".");
                            return;
                        }
                    }
                }
            } else if (fileStream.name().toString() == "Cues") {
                while (fileStream.readNextStartElement()) {
                    if (fileStream.name().toString() != "Cue") {
                        kernel->terminal->error("Error reading file: Expected Cue data.");
                        return;
                    }
                    if (!fileStream.attributes().hasAttribute("ID")) {
                        kernel->terminal->error("Error reading file: No Cue ID was given.");
                        return;
                    }
                    Cue* cue = kernel->cues->addItem(fileStream.attributes().value("ID").toString());
                    while (fileStream.readNextStartElement()) {
                        if (fileStream.name().toString() == "Label") {
                            cue->label = fileStream.readElementText();
                        } else if (fileStream.name().toString() == "Fade") {
                            bool ok;
                            float fade = fileStream.readElementText().toFloat(&ok);
                            if (!ok) {
                                kernel->terminal->error("Error reading file: Fade of Cue " + cue->id + " isn't valid.");
                                return;
                            }
                            cue->floatAttributes[kernel->cues->FADEATTRIBUTEID] = fade;
                        } else if (fileStream.name().toString() == "Groups") {
                            while (fileStream.readNextStartElement()) {
                                if (fileStream.name().toString() != "Group") {
                                    kernel->terminal->error("Error reading file: Expected Group data for Cue " + cue->id);
                                    return;
                                }
                                if (!fileStream.attributes().hasAttribute("ID")) {
                                    kernel->terminal->error("Error reading file: No Group was given for Cue " + cue->id);
                                    return;
                                }
                                Group* group = kernel->groups->getItem(fileStream.attributes().value("ID").toString());
                                if (group == nullptr) {
                                    kernel->terminal->error("Error reading file: Group in Cue " + cue->id + " was not found.");
                                    return;
                                }
                                while (fileStream.readNextStartElement()) {
                                    if (fileStream.name().toString() == "Intensity") {
                                        Intensity* intensity = kernel->intensities->getItem(fileStream.readElementText());
                                        if (intensity == nullptr) {
                                            kernel->terminal->error("Error reading file: Intensity for Group " + group->id + " in Cue " + cue->id + " was not found.");
                                            return;
                                        }
                                        cue->intensities[group] = intensity;
                                    } else if (fileStream.name().toString() == "Color") {
                                        Color* color = kernel->colors->getItem(fileStream.readElementText());
                                        if (color == nullptr) {
                                            kernel->terminal->error("Error reading file: Color for Group " + group->id + " in Cue " + cue->id + " was not found.");
                                            return;
                                        }
                                        cue->colors[group] = color;
                                    } else if (fileStream.name().toString() == "Raws") {
                                        cue->raws[group] = QList<Raw*>();
                                        while (fileStream.readNextStartElement()) {
                                            if (fileStream.name().toString() != "Raw") {
                                                kernel->terminal->error("Error reading file: Expected Raw for Group " + group->id + " in Cue " + cue->id);
                                                return;
                                            }
                                            Raw* raw = kernel->raws->getItem(fileStream.readElementText());
                                            if (raw == nullptr) {
                                                kernel->terminal->error("Error reading file: Raw for Group " + group->id + " in Cue " + cue->id + " was not found.");
                                                return;
                                            }
                                            cue->raws[group].append(raw);
                                        }
                                    } else {
                                        kernel->terminal->error("Error reading file: Expected data for Group " + group->id + " in Cue " + cue->id);
                                        return;
                                    }
                                }
                            }
                        } else {
                            kernel->terminal->error("Error reading file: Unknown Cue Attribute \"" + fileStream.name().toString() + "\".");
                            return;
                        }
                    }
                }
            } else {
                kernel->terminal->error("Error reading file: Expected object type.");
                return;
            }
        }
    } else {
        kernel->terminal->error("Error reading file: Expected Zöglfrex workspace.");
        return;
    }
    fileName = newFileName;
    kernel->cuelistView->loadCue();
    if (fileStream.hasError()) {
        kernel->terminal->error("Can't open file because a XML parsing error occured in line " + QString::number(fileStream.lineNumber()) + ": " + fileStream.errorString() + " (" + QString::number(fileStream.error()) + ")");
        return;
    }
    kernel->terminal->success("Opened File " + newFileName);
}

void MainWindow::newFile() {
    QMessageBox messageBox;
    messageBox.setText("Are you sure you want to open a new file?");
    messageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Cancel);
    int result  = messageBox.exec();
    if (result != QMessageBox::Ok) {
        return;
    }
    fileName = QString();
    kernel->reset();
    kernel->terminal->success("Opened new file.");
}

void MainWindow::saveFile() {
    if (fileName.isEmpty()) {
        QString filenameFilter = "zfr Files (*.zfr)";
        fileName = QFileDialog::getSaveFileName(this, "Save File", QString(), filenameFilter, &filenameFilter);
        if (fileName.isEmpty()) {
            return;
        }
        if (!fileName.endsWith(".zfr")) {
            fileName += ".zfr";
        }
    }
    QMutexLocker lockelockerr(kernel->mutex);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        kernel->terminal->error("Unable to save file.");
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
    fileStream.writeTextElement("Universe", QString::number(kernel->cuelistView->dmxEngine->sacnServer->universeSpinBox->value()));
    fileStream.writeTextElement("Priority", QString::number(kernel->cuelistView->dmxEngine->sacnServer->prioritySpinBox->value()));
    fileStream.writeEndElement();

    fileStream.writeStartElement("Models");
    for (Model* model : kernel->models->items) {
        fileStream.writeStartElement("Model");
        fileStream.writeAttribute("ID", model->id);
        fileStream.writeTextElement("Label", model->label);
        fileStream.writeTextElement("Channels", model->channels);
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Fixtures");
    for (Fixture* fixture : kernel->fixtures->items) {
        fileStream.writeStartElement("Fixture");
        fileStream.writeAttribute("ID", fixture->id);
        fileStream.writeTextElement("Label", fixture->label);
        if (fixture->model != nullptr) {
            fileStream.writeTextElement("Model", fixture->model->id);
        }
        fileStream.writeTextElement("Address", QString::number(fixture->address));
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Groups");
    for (Group* group : kernel->groups->items) {
        fileStream.writeStartElement("Group");
        fileStream.writeAttribute("ID", group->id);
        fileStream.writeTextElement("Label", group->label);
        fileStream.writeStartElement("Fixtures");
        for (Fixture *fixture : group->fixtures) {
            fileStream.writeTextElement("Fixture", fixture->id);
        }
        fileStream.writeEndElement();
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Intensities");
    for (Intensity* intensity : kernel->intensities->items) {
        fileStream.writeStartElement("Intensity");
        fileStream.writeAttribute("ID", intensity->id);
        fileStream.writeTextElement("Label", intensity->label);
        fileStream.writeTextElement("Dimmer", QString::number(intensity->dimmer));
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Colors");
    for (Color* color : kernel->colors->items) {
        fileStream.writeStartElement("Color");
        fileStream.writeAttribute("ID", color->id);
        fileStream.writeTextElement("Label", color->label);
        fileStream.writeTextElement("Hue", QString::number(color->hue));
        fileStream.writeTextElement("Saturation", QString::number(color->floatAttributes.value(kernel->colors->SATURATIONATTRIBUTEID)));
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Raws");
    for (Raw* raw : kernel->raws->items) {
        fileStream.writeStartElement("Raw");
        fileStream.writeAttribute("ID", raw->id);
        fileStream.writeTextElement("Label", raw->label);
        fileStream.writeTextElement("Channel", QString::number(raw->channel));
        fileStream.writeTextElement("Value", QString::number(raw->value));
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Cues");
    for (Cue* cue : kernel->cues->items) {
        fileStream.writeStartElement("Cue");
        fileStream.writeAttribute("ID", cue->id);
        fileStream.writeTextElement("Label", cue->label);
        fileStream.writeTextElement("Fade", QString::number(cue->floatAttributes.value(kernel->cues->FADEATTRIBUTEID)));
        fileStream.writeStartElement("Groups");
        for (Group* group : kernel->groups->items) {
            fileStream.writeStartElement("Group");
            fileStream.writeAttribute("ID", group->id);
            if (cue->intensities.contains(group)) {
                fileStream.writeTextElement("Intensity", cue->intensities.value(group)->id);
            }
            if (cue->colors.contains(group)) {
                fileStream.writeTextElement("Color", cue->colors.value(group)->id);
            }
            if (cue->raws.contains(group)) {
                fileStream.writeStartElement("Raws");
                for (Raw* raw : cue->raws.value(group)) {
                    fileStream.writeTextElement("Raw", raw->id);
                }
                fileStream.writeEndElement();
            }
            fileStream.writeEndElement();
        }
        fileStream.writeEndElement();
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeEndElement();
    fileStream.writeEndDocument();
    kernel->terminal->success("Saved file as " + fileName);
}

void MainWindow::saveFileAs() {
    fileName = QString(); // reset filename
    saveFile();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    QMessageBox confirmBox;
    confirmBox.setText("Do you really want to quit Zöglfrex?");
    confirmBox.setWindowTitle("Quit Zöglfrex?");
    confirmBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    confirmBox.setDefaultButton(QMessageBox::Cancel);
    if (confirmBox.exec() == QMessageBox::Ok) {
        event->accept();
    }
}

void MainWindow::about() {
    AboutWindow about(VERSION, COPYRIGHT);
    about.exec();
}
