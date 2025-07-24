/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "kernel/kernel.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
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

    QMenu *programmingMenu = menuBar()->addMenu("Programming");
    QAction *controlPanelAction = new QAction("Control Panel");
    programmingMenu->addAction(controlPanelAction);
    connect(controlPanelAction, &QAction::triggered, kernel->controlPanel, &ControlPanel::show);
    QAction *preview2dAction = new QAction("2D Preview");
    programmingMenu->addAction(preview2dAction);
    connect(preview2dAction, &QAction::triggered, kernel->preview2d, &Preview2d::show);

    QMenu *outputMenu = menuBar()->addMenu("Output");
    QAction *outputSettingsAction = new QAction("DMX Output Settings");
    outputMenu->addAction(outputSettingsAction);
    connect(outputSettingsAction, &QAction::triggered, kernel->dmxEngine->sacnServer, &SacnServer::show);
    QAction *playbackMonitorAction = new QAction("Playback Monitor");
    outputMenu->addAction(playbackMonitorAction);
    connect(playbackMonitorAction, &QAction::triggered, kernel->playbackMonitor, &PlaybackMonitor::show);

    QMenu *helpMenu = menuBar()->addMenu("Help");
    QAction *aboutAction = new QAction("About Zöglfrex");
    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    QAction *aboutQtAction = new QAction("About Qt");
    helpMenu->addAction(aboutQtAction);
    connect(aboutQtAction, &QAction::triggered, this, &QApplication::aboutQt);
    helpMenu->addSeparator();
    QAction *openGuideAction = new QAction("Quick Start Guide");
    helpMenu->addAction(openGuideAction);
    connect(openGuideAction, &QAction::triggered, this, []{ QDesktopServices::openUrl(QUrl("https://github.com/arneBersch/Zoeglfrex/blob/main/docs/quick_start_guide.md")); });
    QAction *openReferenceAction = new QAction("Reference");
    helpMenu->addAction(openReferenceAction);
    connect(openReferenceAction, &QAction::triggered, this, []{ QDesktopServices::openUrl(QUrl("https://github.com/arneBersch/Zoeglfrex/blob/main/docs/reference.md")); });

    kernel = new Kernel(this);
    setupShortcuts(this);
    kernel->cuelistView->updateCuelistView();
    kernel->cuelistView->reload();

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

    QSplitter *mainColumns = new QSplitter();
    mainColumns->setChildrenCollapsible(false);
    mainColumns->addWidget(leftColumn);
    mainColumns->addWidget(rightColumn);
    mainColumns->setSizes(QList<int>() << 1000 << 500);
    this->setCentralWidget(mainColumns);

    qInfo() << "Zöglfrex" << kernel->VERSION;
    qInfo() << COPYRIGHT;
    qInfo() << "Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.";
    qInfo() << "Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.";
    qInfo() << "You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.";

    show();
    about(); // Open about window
}

MainWindow::~MainWindow() {}

void MainWindow::setupShortcuts(QWidget* widget) {
    QObject::connect(new QShortcut(QKeySequence(Qt::Key_Plus), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Plus); }); // +
    connect(new QShortcut(QKeySequence(Qt::Key_Minus), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Minus); }); // -
    connect(new QShortcut(QKeySequence(Qt::Key_Comma), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Period); }); // , (appends .)
    connect(new QShortcut(QKeySequence(Qt::Key_Period), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Period); }); // .
    connect(new QShortcut(QKeySequence(Qt::Key_T), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Thru); }); // Thru
    connect(new QShortcut(QKeySequence(Qt::Key_0), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Zero); }); // 0
    connect(new QShortcut(QKeySequence(Qt::Key_1), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::One); }); // 1
    connect(new QShortcut(QKeySequence(Qt::Key_2), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Two); }); // 2
    connect(new QShortcut(QKeySequence(Qt::Key_3), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Three); }); // 3
    connect(new QShortcut(QKeySequence(Qt::Key_4), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Four); }); // 4
    connect(new QShortcut(QKeySequence(Qt::Key_5), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Five); }); // 5
    connect(new QShortcut(QKeySequence(Qt::Key_6), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Six); }); // 6
    connect(new QShortcut(QKeySequence(Qt::Key_7), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Seven); }); // 7
    connect(new QShortcut(QKeySequence(Qt::Key_8), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Eight); }); // 8
    connect(new QShortcut(QKeySequence(Qt::Key_9), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Nine); }); // 9
    connect(new QShortcut(QKeySequence(Qt::Key_A), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Attribute); }); // Attribute
    connect(new QShortcut(QKeySequence(Qt::Key_C), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Color); }); // Color
    connect(new QShortcut(QKeySequence(Qt::Key_E), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Effect); }); // Effect
    connect(new QShortcut(QKeySequence(Qt::Key_F), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Fixture); }); // Fixture
    connect(new QShortcut(QKeySequence(Qt::Key_G), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Group); }); // Group
    connect(new QShortcut(QKeySequence(Qt::Key_I), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Intensity); }); // Intensity
    connect(new QShortcut(QKeySequence(Qt::Key_L), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Cuelist); }); // Cue List
    connect(new QShortcut(QKeySequence(Qt::Key_M), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Model); }); // Model
    connect(new QShortcut(QKeySequence(Qt::Key_P), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Position); }); // Model
    connect(new QShortcut(QKeySequence(Qt::Key_Q), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Cue); }); // Cue
    connect(new QShortcut(QKeySequence(Qt::Key_R), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Raw); }); // Raw
    connect(new QShortcut(QKeySequence(Qt::Key_S), widget), &QShortcut::activated, this, [this]{ kernel->terminal->write(Keys::Set); }); // Set

    connect(new QShortcut(QKeySequence(Qt::Key_Return), widget), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); kernel->terminal->clear(); }); // Enter Command (via Return key)
    connect(new QShortcut(QKeySequence(Qt::Key_Enter), widget), &QShortcut::activated, this, [this]{ kernel->terminal->execute(); kernel->terminal->clear(); }); // Enter Command (via Keypad Enter key)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return), widget), &QShortcut::activated, kernel->terminal, &Terminal::execute); // Enter Command (via Shift + Return key)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Enter), widget), &QShortcut::activated, kernel->terminal, &Terminal::execute); // Enter Command (via Shift + Enter key)
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), widget), &QShortcut::activated, kernel->terminal, &Terminal::backspace); // Backspace (Remove last keypress)
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Backspace), widget), &QShortcut::activated, kernel->terminal, &Terminal::clear); // Clear Terminal

    connect(new QShortcut(QKeySequence(Qt::Key_Space), widget), &QShortcut::activated, kernel->cuelistView, &CuelistView::nextCue); // Go to next Cue
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Space), widget), &QShortcut::activated, kernel->cuelistView, &CuelistView::previousCue); // Go to previous Cue
    connect(new QShortcut(QKeySequence(Qt::Key_Down), widget), &QShortcut::activated, kernel->cuelistView, &CuelistView::nextGroup); // Go to next Group
    connect(new QShortcut(QKeySequence(Qt::Key_Up), widget), &QShortcut::activated, kernel->cuelistView, &CuelistView::previousGroup); // Go to previous Group
    connect(new QShortcut(QKeySequence(Qt::Key_Right), widget), &QShortcut::activated, kernel->cuelistView, &CuelistView::nextFixture); // Go to next Fixture
    connect(new QShortcut(QKeySequence(Qt::Key_Left), widget), &QShortcut::activated, kernel->cuelistView, &CuelistView::previousFixture); // Go to previous Fixture
    connect(new QShortcut(QKeySequence(Qt::Key_Escape), widget), &QShortcut::activated, kernel->cuelistView, &CuelistView::noFixture); // Deselect Fixture

    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_B), widget), &QShortcut::activated, kernel->dmxEngine->blindButton, &QPushButton::click); // Blind
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F), widget), &QShortcut::activated, kernel->dmxEngine->skipFadeButton, &QPushButton::click); // Skip Fade
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_H), widget), &QShortcut::activated, kernel->dmxEngine->highlightButton, &QPushButton::click); // Highlight
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_M), widget), &QShortcut::activated, this, [this]{
        if (kernel->cuelistView->cueViewModeComboBox->currentIndex() == CuelistViewModes::cueMode) {
            kernel->cuelistView->cueViewModeComboBox->setCurrentIndex(CuelistViewModes::groupMode);
        } else if (kernel->cuelistView->cueViewModeComboBox->currentIndex() == CuelistViewModes::groupMode) {
            kernel->cuelistView->cueViewModeComboBox->setCurrentIndex(CuelistViewModes::cueMode);
        } else {
            Q_ASSERT(false);
        }
    }); // Set Cuelist View Mode
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_R), widget), &QShortcut::activated, this, [this]{
        if (kernel->cuelistView->cueViewRowFilterComboBox->currentIndex() == CuelistViewRowFilters::noFilter) {
            kernel->cuelistView->cueViewRowFilterComboBox->setCurrentIndex(CuelistViewRowFilters::activeRowsFilter);
        } else if (kernel->cuelistView->cueViewRowFilterComboBox->currentIndex() == CuelistViewRowFilters::activeRowsFilter) {
            kernel->cuelistView->cueViewRowFilterComboBox->setCurrentIndex(CuelistViewRowFilters::changedRowsFilter);
        } else if (kernel->cuelistView->cueViewRowFilterComboBox->currentIndex() == CuelistViewRowFilters::changedRowsFilter) {
            kernel->cuelistView->cueViewRowFilterComboBox->setCurrentIndex(CuelistViewRowFilters::noFilter);
        } else {
            Q_ASSERT(false);
        }
    }); // Set Cuelist View Rows Filter
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_S), widget), &QShortcut::activated, kernel->dmxEngine->soloButton, &QPushButton::click); // Solo
    connect(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_T), widget), &QShortcut::activated, kernel->cuelistView->trackingButton, &QPushButton::click); // Tracking

    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), widget), &QShortcut::activated, this, &MainWindow::newFile); // New File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), widget), &QShortcut::activated, this, &MainWindow::openFile); // Open File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), widget), &QShortcut::activated, this, &MainWindow::saveFile); // Save File
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), widget), &QShortcut::activated, this, &MainWindow::saveFileAs); // Save File As
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), widget), &QShortcut::activated, this, &MainWindow::close); // Quit Application
}

void MainWindow::openFile() {
    QString newFileName = QFileDialog::getOpenFileName(this, "Open File", QString(), filenameEnding + " Files (*." + filenameEnding + ")");
    if (newFileName.isEmpty()) {
        return;
    }
    fileName = newFileName;
    kernel->terminal->printMessages = false;
    kernel->openFile(fileName);
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
        QString filenameFilter = filenameEnding + " Files (*." + filenameEnding + ")";
        fileName = QFileDialog::getSaveFileName(this, "Save File", QString(), filenameFilter, &filenameFilter);
        if (fileName.isEmpty()) {
            return;
        }
        if (!fileName.endsWith("." + filenameEnding)) {
            fileName += "." + filenameEnding;
        }
    }
    kernel->saveFile(fileName);
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
        kernel->dmxEngine->sacnServer->close();
        kernel->preview2d->close();
        kernel->controlPanel->close();
        kernel->playbackMonitor->close();
        event->accept();
    }
}

void MainWindow::about() {
    AboutWindow about(kernel->VERSION, COPYRIGHT);
    about.exec();
}
