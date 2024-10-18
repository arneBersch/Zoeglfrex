/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "aboutwindow.h"

AboutWindow::AboutWindow(QString version, QString copyright, QWidget *parent) : QDialog{parent} {
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *header = new QLabel(version);
    header->setStyleSheet("font-size: 50px");
    layout->addWidget(header);

    QLabel *copyrightNotice = new QLabel(copyright);
    layout->addWidget(copyrightNotice);

    QLabel *licenseNotice = new QLabel("This application is licensed under the terms of the GNU General Public License.");
    layout->addWidget(licenseNotice);

    QTextEdit *license = new QTextEdit();
    license->setReadOnly(true);
    QFile licenseFile(":/resources/license.txt");
    licenseFile.open(QFile::ReadOnly);
    license->setText(licenseFile.readAll());
    layout->addWidget(license);

    setLayout(layout);
    setWindowTitle("About Zöglfrex");
}
