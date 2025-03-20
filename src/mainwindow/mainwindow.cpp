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
    connect(new QShortcut(QKeySequence(Qt::Key_Return), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); kernel->terminal->clear(); }); // Enter Command (via Return key)
    connect(new QShortcut(QKeySequence(Qt::Key_Enter), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); kernel->terminal->clear(); }); // Enter Command (via Keypad Enter key)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); }); // Enter Command (via Shift + Return key)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Enter), this), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); }); // Enter Command (via Shift + Enter key)
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
    fileName = newFileName;
    kernel->terminal->printMessages = false;
    kernel->openFile(fileName, VERSION);
    kernel->terminal->printMessages = true;
    kernel->terminal->success("Opened File " + fileName);
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
    kernel->saveFile(fileName, VERSION);
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
