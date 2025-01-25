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

    QAction *newFileAction = new QAction("New File");
    fileMenu->addAction(newFileAction);
    connect(newFileAction, &QAction::triggered, this, &MainWindow::newFile);

    QAction *openFileAction = new QAction("Open File");
    fileMenu->addAction(openFileAction);
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openFile);

    QAction *saveFileAction = new QAction("Save File");
    fileMenu->addAction(saveFileAction);
    connect(saveFileAction, &QAction::triggered, this, &MainWindow::saveFile);

    QAction *saveFileAsAction = new QAction("Save File as");
    fileMenu->addAction(saveFileAsAction);
    connect(saveFileAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    fileMenu->addSeparator();

    QAction *quitAction = new QAction("Quit");
    fileMenu->addAction(quitAction);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

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
    connect(new QShortcut(QKeySequence(Qt::Key_C), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Color); }); // Color
    connect(new QShortcut(QKeySequence(Qt::Key_F), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Fixture); }); // Fixture
    connect(new QShortcut(QKeySequence(Qt::Key_G), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Group); }); // Group
    connect(new QShortcut(QKeySequence(Qt::Key_I), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Intensity); }); // Intensity
    connect(new QShortcut(QKeySequence(Qt::Key_M), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Model); }); // Model
    connect(new QShortcut(QKeySequence(Qt::Key_Q), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Cue); }); // Cue
    connect(new QShortcut(QKeySequence(Qt::Key_A), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Attribute); }); // At
    connect(new QShortcut(QKeySequence(Qt::Key_S), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Set); }); // Set
    connect(new QShortcut(QKeySequence(Qt::Key_Return), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); }); // Enter Command (via Return key)
    connect(new QShortcut(QKeySequence(Qt::Key_Enter), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); }); // Enter Command (via Keypad Enter key)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(false); }); // Enter Command (via Shift + Return key)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Enter), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(false); }); // Enter Command (via Shift + Enter key)
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), this), &QShortcut::activated, this, [this]{ kernel->terminal->backspace(); }); // Backspace (Remove last keypress)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Backspace), this), &QShortcut::activated, this, [this]{ kernel->terminal->clear(); }); // Clear Terminal
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this), &QShortcut::activated, this, [this]{ this->newFile(); }); // New File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this), &QShortcut::activated, this, [this]{ this->openFile(); }); // Open File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this), &QShortcut::activated, this, [this]{ this->saveFile(); }); // Save File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this), &QShortcut::activated, this, [this]{ this->saveFileAs(); }); // Save File As
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this), &QShortcut::activated, this, [this]{ this->close(); }); // Quit Application

    this->show(); // Show window
    about(); // Open about window
}

MainWindow::~MainWindow() {
}

void MainWindow::openFile() {
    QString newFileName = QFileDialog::getOpenFileName();
    if (newFileName.isEmpty()) {
        return;
    }
    if (!newFileName.endsWith(".zfr")) {
        kernel->terminal->error("Can't read file: Zöglfrex files have to end with .zfr");
        return;
    }
    QFile file(newFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kernel->terminal->error("Can't open file.");
        return;
    }
    QTextStream textStream(&file);
    QString fileVersion = textStream.readLine();
    if (fileVersion != "ZOEGLFREX_00.02.00") {
        if (fileVersion.startsWith("ZOEGLFREX_")) {
            kernel->terminal->error("Can't open file: Zöglfrex file is not compatible with this version.");
        } else {
            kernel->terminal->error("Can't open file: No Zöglfrex file.");
        }
        return;
    }
    clearKernel();
    while (!textStream.atEnd()) {
        QString line = textStream.readLine();
        if (!line.isEmpty()) {
            kernel->terminal->execute(line, "open file");
        }
    }
    fileName = newFileName;
    kernel->terminal->success("Opened File " + newFileName);
}

void MainWindow::clearKernel() {
    fileName = QString(); // reset filename
    kernel->terminal->execute("m.s-", "new file");
    kernel->terminal->execute("f.s-", "new file");
    kernel->terminal->execute("g.s-", "new file");
    kernel->terminal->execute("i.s-", "new file");
    kernel->terminal->execute("c.s-", "new file");
    kernel->terminal->execute("q.s-", "new file");
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
    clearKernel();
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

    fileStream.writeStartElement("Models");
    for (Model* model : kernel->models->items) {
        fileStream.writeStartElement("Model");
        fileStream.writeTextElement("ID", model->id);
        fileStream.writeTextElement("Label", model->label);
        fileStream.writeTextElement("Channels", model->channels);
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Fixtures");
    for (Fixture* fixture : kernel->fixtures->items) {
        fileStream.writeStartElement("Fixture");
        fileStream.writeTextElement("ID", fixture->id);
        fileStream.writeTextElement("Label", fixture->label);
        fileStream.writeTextElement("Model", fixture->model->id);
        fileStream.writeTextElement("Channels", QString::number(fixture->address));
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Groups");
    for (Group* group : kernel->groups->items) {
        fileStream.writeStartElement("Group");
        fileStream.writeTextElement("ID", group->id);
        fileStream.writeTextElement("Label", group->label);
        fileStream.writeStartElement("Fixtures");
        for (Fixture *fixture : group->fixtures) {
            fileStream.writeTextElement("Fixture", fixture->id);
        }
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Intensities");
    for (Intensity* intensity : kernel->intensities->items) {
        fileStream.writeStartElement("Intensity");
        fileStream.writeTextElement("ID", intensity->id);
        fileStream.writeTextElement("Label", intensity->label);
        fileStream.writeTextElement("Dimmer", QString::number(intensity->dimmer));
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Colors");
    for (Color* color : kernel->colors->items) {
        fileStream.writeStartElement("Color");
        fileStream.writeTextElement("ID", color->id);
        fileStream.writeTextElement("Label", color->label);
        fileStream.writeTextElement("Hue", QString::number(color->hue));
        fileStream.writeTextElement("Saturation", QString::number(color->saturation));
        fileStream.writeEndElement();
    }
    fileStream.writeEndElement();

    fileStream.writeStartElement("Cues");
    for (Cue* cue : kernel->cues->items) {
        fileStream.writeStartElement("Cue");
        fileStream.writeTextElement("ID", cue->id);
        fileStream.writeTextElement("Label", cue->label);
        fileStream.writeTextElement("Fade", QString::number(cue->fade));
        fileStream.writeStartElement("Groups");
        for (Group* group : kernel->groups->items) {
            fileStream.writeStartElement("Group");
            fileStream.writeTextElement("ID", group->id);
            if (cue->intensities.contains(group)) {
                fileStream.writeTextElement("Intensity", cue->intensities.value(group)->id);
            }
            if (cue->colors.contains(group)) {
                fileStream.writeTextElement("Color", cue->colors.value(group)->id);
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
