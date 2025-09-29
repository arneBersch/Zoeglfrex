/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "aboutwindow.h"

AboutWindow::AboutWindow(QString version, QString copyright, QWidget *parent) : QDialog(parent) {
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *headerLayout = new QHBoxLayout();
    layout->addLayout(headerLayout);

    QLabel *icon = new QLabel();
    QPixmap iconPixmap = QPixmap();
    iconPixmap.load(":/resources/icon.png");
    icon->setPixmap(iconPixmap.scaled(200, 200));
    headerLayout->addWidget(icon);

    QLabel *headerLabel = new QLabel("Zöglfrex " + version);
    headerLabel->setStyleSheet("font-size: 50px");
    headerLayout->addWidget(headerLabel);

    QLabel *copyrightNotice = new QLabel(copyright);
    layout->addWidget(copyrightNotice);

    QLabel *licenseNotice = new QLabel("This application is licensed under the terms of the GNU General Public License.");
    layout->addWidget(licenseNotice);

    QPlainTextEdit *license = new QPlainTextEdit();
    license->setReadOnly(true);
    QFile licenseFile(":/resources/license.txt");
    licenseFile.open(QFile::ReadOnly);
    license->setPlainText(licenseFile.readAll());
    layout->addWidget(license);

    setLayout(layout);
    setWindowTitle("About Zöglfrex");
}
