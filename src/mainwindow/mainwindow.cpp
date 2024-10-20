/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    qInfo() << VERSION;
    qInfo() << COPYRIGHT;
    qInfo() << "Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.";
    qInfo() << "Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.";
    qInfo() << "You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.";

    this->resize(1500, 1000);
    this->setWindowTitle("Zöglfrex");

    QMenu *fileMenu = menuBar()->addMenu("File");

    QAction *newFileAction = new QAction("New File", this);
    fileMenu->addAction(newFileAction);
    connect(newFileAction, &QAction::triggered, this, &MainWindow::newFile);

    QAction *openFileAction = new QAction("Open File", this);
    fileMenu->addAction(openFileAction);
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openFile);

    QAction *saveFileAction = new QAction("Save File", this);
    fileMenu->addAction(saveFileAction);
    connect(saveFileAction, &QAction::triggered, this, &MainWindow::saveFile);

    QAction *saveFileAsAction = new QAction("Save File as", this);
    fileMenu->addAction(saveFileAsAction);
    connect(saveFileAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    fileMenu->addSeparator();

    QAction *quitAction = new QAction("Quit", this);
    fileMenu->addAction(quitAction);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    QMenu *helpMenu = menuBar()->addMenu("Help");

    QAction *aboutAction = new QAction("About Zöglfrex", this);
    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

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

    connect(new QShortcut(QKeySequence(Qt::Key_Plus), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::plus); }); // +
    connect(new QShortcut(QKeySequence(Qt::Key_Minus), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::minus); }); // -
    connect(new QShortcut(QKeySequence(Qt::Key_Comma), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::comma); }); // ,
    connect(new QShortcut(QKeySequence(Qt::Key_Period), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::period); }); // .
    connect(new QShortcut(QKeySequence(Qt::Key_Slash), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::thru); }); // Thru
    connect(new QShortcut(QKeySequence(Qt::Key_0), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::zero); }); // 0
    connect(new QShortcut(QKeySequence(Qt::Key_1), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::one); }); // 1
    connect(new QShortcut(QKeySequence(Qt::Key_2), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::two); }); // 2
    connect(new QShortcut(QKeySequence(Qt::Key_3), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::three); }); // 3
    connect(new QShortcut(QKeySequence(Qt::Key_4), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::four); }); // 4
    connect(new QShortcut(QKeySequence(Qt::Key_5), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::five); }); // 5
    connect(new QShortcut(QKeySequence(Qt::Key_6), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::six); }); // 6
    connect(new QShortcut(QKeySequence(Qt::Key_7), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::seven); }); // 7
    connect(new QShortcut(QKeySequence(Qt::Key_8), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::eight); }); // 8
    connect(new QShortcut(QKeySequence(Qt::Key_9), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::nine); }); // 9
    connect(new QShortcut(QKeySequence(Qt::Key_C), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::color); }); // Color
    connect(new QShortcut(QKeySequence(Qt::Key_F), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::fixture); }); // Fixture
    connect(new QShortcut(QKeySequence(Qt::Key_G), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::group); }); // Group
    connect(new QShortcut(QKeySequence(Qt::Key_I), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::intensity); }); // Intensity
    connect(new QShortcut(QKeySequence(Qt::Key_M), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::model); }); // Model
    connect(new QShortcut(QKeySequence(Qt::Key_R), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::row); }); // Row
    connect(new QShortcut(QKeySequence(Qt::Key_T), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::transition); }); // Transition
    connect(new QShortcut(QKeySequence(Qt::Key_Q), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::cue); }); // Cue
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_C), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::copyItem); }); // Copy
    connect(new QShortcut(QKeySequence(Qt::SHIFT| Qt::Key_D), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::deleteItem); }); // Delete
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_L), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::labelItem); }); // Label
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_M), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::moveItem); }); // Move
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_R), this), &QShortcut::activated, this, [this]{ kernel->terminal->write(keys::recordItem); }); // Record
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
    if (textStream.readLine() != "ZOEGLFREX_00.00.00") {
        kernel->terminal->error("Can't open file.");
        return;
    }
    clearKernel();
    while (!textStream.atEnd()) {
        QString line = textStream.readLine();
        if (!line.isEmpty()) {
            bool result = kernel->terminal->execute(line);
            if (!result) {
                kernel->terminal->error("Can't open file.");
                clearKernel();
                return;
            }
        }
    }
    fileName = newFileName;
    kernel->terminal->success("Opened File " + newFileName);
}

void MainWindow::clearKernel() {
    fileName = QString();
    kernel->models->deleteModel(kernel->models->getIds());
    kernel->fixtures->deleteFixture(kernel->fixtures->getIds());
    kernel->groups->deleteGroup(kernel->groups->getIds());
    kernel->intensities->deleteIntensity(kernel->intensities->getIds());
    kernel->colors->deleteColor(kernel->colors->getIds());
    kernel->transitions->deleteTransition(kernel->transitions->getIds());
    kernel->rows->deleteRow(kernel->rows->getIds());
    kernel->cues->deleteCue(kernel->cues->getIds());
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
    QMutexLocker locker(kernel->mutex);
    clearKernel();
    kernel->terminal->success("Opened new file.");
}

void MainWindow::saveFile() {
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName();
        if (fileName.isEmpty()) {
            return;
        }
        if (!fileName.endsWith(".zfr")) {
            fileName += ".zfr";
        }
    }
    QMutexLocker locker(kernel->mutex);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        kernel->terminal->error("Unable to save file.");
        return;
    }
    QTextStream fileStream(&file);
    fileStream << "ZOEGLFREX_00.00.00\n";

    for (QString modelId : kernel->models->getIds()) {
        Model *model = kernel->models->getModel(modelId);
        fileStream << "m" << model->id << "R\"" << model->channels << "\"\n";
        fileStream << "m" << model->id << "L\"" << model->label << "\"\n";
    }

    for (QString fixtureId : kernel->fixtures->getIds()) {
        Fixture *fixture = kernel->fixtures->getFixture(fixtureId);
        fileStream << "f" << fixture->id << "R" << fixture->address << "m" << fixture->model->id << "\n";
        fileStream << "f" << fixture->id << "L\"" << fixture->label << "\"\n";
    }

    for (QString groupId : kernel->groups->getIds()) {
        Group* group = kernel->groups->getGroup(groupId);
        QString fixtures;
        if (!group->fixtures.isEmpty()) {
            fixtures = "f";
            for (Fixture *fixture : group->fixtures) {
                fixtures += fixture->id;
                fixtures += ",";
            }
        } else {
            fixtures = "";
        }
        fileStream << "g" << group->id << "R" << fixtures << "\n";
        fileStream << "g" << group->id << "L\"" << group->label << "\"\n";
    }

    for (QString intensityId : kernel->intensities->getIds()) {
        Intensity *intensity = kernel->intensities->getIntensity(intensityId);
        fileStream << "i" << intensity->id << "R" << intensity->dimmer << "\n";
        fileStream << "i" << intensity->id << "L\"" << intensity->label << "\"\n";
    }

    for (QString colorId : kernel->colors->getIds()) {
        Color *color = kernel->colors->getColor(colorId);
        fileStream << "c" << color->id << "R" << color->red << "," << color->green << "," << color->blue << "\n";
        fileStream << "c" << color->id << "L\"" << color->label << "\"\n";
    }

    for (QString transitionId : kernel->transitions->getIds()) {
        Transition *transition = kernel->transitions->getTransition(transitionId);
        fileStream << "t" << transition->id << "R" << transition->fade << "\n";
        fileStream << "t" << transition->id << "L\"" << transition->label << "\"\n";
    }

    for (QString rowId : kernel->rows->getIds()) {
        Row *row = kernel->rows->getRow(rowId);
        fileStream << "r" << row->id << "Rg" << row->group->id << "\n";
        fileStream << "r" << row->id << "L\"" << row->label << "\"\n";
    }

    for (QString cueId : kernel->cues->getIds()) {
        Cue *cue = kernel->cues->getCue(cueId);
        fileStream << "q" << cue->id << "Rt" << cue->transition->id << "\n";
        fileStream << "q" << cue->id << "L\"" << cue->label << "\"\n";
        for (QString rowId : kernel->rows->getIds()) {
            Row *row = kernel->rows->getRow(rowId);
            if (cue->intensities.contains(row)) {
                fileStream << "q" << cue->id << "Rr" << row->id << "i" << cue->intensities.value(row)->id << "\n";
            }
            if (cue->colors.contains(row)) {
                fileStream << "q" << cue->id << "Rr" << row->id << "c" << cue->colors.value(row)->id << "\n";
            }
        }
    }
    fileStream << Qt::endl;
    kernel->terminal->success("Saved file " + fileName);
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
