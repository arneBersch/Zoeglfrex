/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include <QtWidgets>

#include "mainwindow/mainwindow.h"

int main(int argc, char *argv[]) {
    const QString VERSION = "1.0.0";
    const QString COPYRIGHT = "Copyright (c) 2025 Arne Bersch (zoeglfrex-dmx@web.de)";

    QApplication app(argc, argv);
    app.setApplicationName("Zöglfrex");
    app.setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription("Open Source Stage Lighting Control");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The .zfr file to create or open");
    parser.process(app);
    if (parser.positionalArguments().length() != 1) {
        qFatal("Can't open Zöglfrex because no file name was provided.");
        return 1;
    }
    QString fileName = QFileInfo(parser.positionalArguments().first()).absoluteFilePath();
    if (!fileName.endsWith(".zfr")) {
        qFatal("Can't open files because Zöglfrex files have to end with .zfr");
        return 1;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);
    if (!db.open()) {
        qFatal("Failed to establish a database connection.");
        return 1;
    }

    QStringList createTableQueries = {
        "CREATE TABLE IF NOT EXISTS models (key INTEGER, id INTEGER UNIQUE, label TEXT DEFAULT '', channels TEXT DEFAULT 'D', PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS fixtures (key INTEGER, id INTEGER UNIQUE, label TEXT DEFAULT '', model INTEGER REFERENCES models (key) ON DELETE SET NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS groups (key INTEGER, id INTEGER UNIQUE, label TEXT DEFAULT '', PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS intensities (key INTEGER, id INTEGER UNIQUE, label TEXT DEFAULT '', dimmer INTEGER DEFAULT 0, PRIMARY KEY (key), CHECK (dimmer BETWEEN 0 AND 100))",
        "CREATE TABLE IF NOT EXISTS colors (key INTEGER, id INTEGER UNIQUE, label TEXT DEFAULT '', hue REAL DEFAULT 0, saturation INTEGER DEFAULT 0, PRIMARY KEY (key), CHECK (hue BETWEEN 0 AND 360), CHECK (saturation BETWEEN 0 AND 100))",
        "CREATE TABLE IF NOT EXISTS cues (key INTEGER, id INTEGER UNIQUE, label TEXT DEFAULT '', PRIMARY KEY (key))",
    };
    QSqlQuery query;
    for (QString createTableQuery : createTableQueries) {
        if (!query.exec(createTableQuery)) {
            qFatal() << "Failed to create table." << query.lastError().text();
            return 1;
        }
    }

    qInfo() << "Zöglfrex " + VERSION;
    qInfo() << COPYRIGHT;
    qInfo("Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.");
    qInfo("Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.");
    qInfo("You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.");

    MainWindow window(VERSION, COPYRIGHT);
    window.setWindowTitle("Zöglfrex - " + fileName);
    return app.exec();
}
