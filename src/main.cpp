/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include <QtWidgets>

#include "startscreen/startscreen.h"
#include "constants.h"

int main(int argc, char *argv[]) {
    qInfo() << "Zöglfrex " + VERSION;
    qInfo() << COPYRIGHT;
    qInfo() << LICENSE_HEADER;

    QApplication app(argc, argv);
    app.setApplicationName("Zöglfrex");
    app.setOrganizationName("Zöglfrex");
    app.setApplicationVersion(VERSION);

    QFile styleSheet(":/resources/style.qss");
    if (!styleSheet.open(QFile::ReadOnly | QFile::Text)) {
        qFatal() << "Failed to open stylesheet.";
        return 1;
    }
    QTextStream styleSheetStream(&styleSheet);
    app.setStyleSheet(styleSheetStream.readAll());

    new StartScreen();

    return app.exec();
}
