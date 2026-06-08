/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include <QtWidgets>
#include <QtSql>

#include "startscreen/startscreen.h"

int main(int argc, char *argv[]) {
    const QString VERSION = "1.1.0";
    const QString FILEVERSION = "1.1.0";
    const QString COPYRIGHT = "Copyright (c) 2026 Arne Bersch (zoeglfrex-dmx@web.de)";

    qInfo() << "Zöglfrex " + VERSION;
    qInfo() << COPYRIGHT;
    qInfo("Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.");
    qInfo("Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.");
    qInfo("You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.");

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

    new StartScreen(VERSION, COPYRIGHT, FILEVERSION);

    return app.exec();
}
