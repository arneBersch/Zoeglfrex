/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"

MainWindow::MainWindow(QString version, QString copyright, QWidget *parent) : QMainWindow(parent) {
    VERSION = version;
    COPYRIGHT = copyright;

    resize(1200, 800);

    new QShortcut(Qt::CTRL | Qt::Key_Q, this, [this] { close(); });

    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *quitAction = new QAction("Quit (CTRL+Q)");
    fileMenu->addAction(quitAction);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    QMenu *helpMenu = menuBar()->addMenu("Help");
    QAction *aboutAction = new QAction("About Zöglfrex");
    helpMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    QAction *aboutQtAction = new QAction("About Qt");
    helpMenu->addAction(aboutQtAction);
    connect(aboutQtAction, &QAction::triggered, this, [] { QApplication::aboutQt(); });
    helpMenu->addSeparator();
    QAction *openGuideAction = new QAction("Quick Start Guide");
    helpMenu->addAction(openGuideAction);
    connect(openGuideAction, &QAction::triggered, this, [] { QDesktopServices::openUrl(QUrl("https://github.com/arneBersch/Zoeglfrex/blob/main/docs/quick_start_guide.md")); });
    QAction *openReferenceAction = new QAction("Reference");
    helpMenu->addAction(openReferenceAction);
    connect(openReferenceAction, &QAction::triggered, this, [] { QDesktopServices::openUrl(QUrl("https://github.com/arneBersch/Zoeglfrex/blob/main/docs/reference.md")); });

    cuelistView = new CuelistView();
    terminal = new Terminal();
    inspector = new Inspector();
    connect(terminal, &Terminal::dbChanged, this, &MainWindow::reload);
    connect(terminal, &Terminal::itemChanged, inspector, &Inspector::loadItem);

    QSplitter *leftColumn = new QSplitter();
    leftColumn->setOrientation(Qt::Vertical);
    leftColumn->setChildrenCollapsible(false);
    leftColumn->addWidget(cuelistView);
    leftColumn->addWidget(terminal);
    leftColumn->setSizes(QList<int>() << 300 << 100);

    QSplitter *mainColumns = new QSplitter();
    mainColumns->setChildrenCollapsible(false);
    mainColumns->addWidget(leftColumn);
    mainColumns->addWidget(inspector);
    mainColumns->setSizes(QList<int>() << 1000 << 500);
    setCentralWidget(mainColumns);

    show();
}

void MainWindow::reload() {
    cuelistView->reload();
    inspector->reload();
}

void MainWindow::about() {
    AboutWindow about(VERSION, COPYRIGHT);
    about.exec();
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
