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

    qInfo() << "Zöglfrex " + VERSION;
    qInfo() << COPYRIGHT;
    qInfo("Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.");
    qInfo("Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.");
    qInfo("You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.");

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
        qCritical("Can't open Zöglfrex because no file name was provided.");
        return 1;
    }
    QString fileName = QFileInfo(parser.positionalArguments().first()).absoluteFilePath();
    if (!fileName.endsWith(".zfr")) {
        qCritical("Can't open files because Zöglfrex files have to end with .zfr");
        return 1;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);
    if (!db.open()) {
        qCritical("Failed to establish a database connection.");
        return 1;
    }

    const QStringList createTableQueries = {
        "CREATE TABLE IF NOT EXISTS models (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, channels TEXT DEFAULT 'D' NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS fixtures (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, model_key INTEGER REFERENCES models (key) ON DELETE SET NULL, universe INTEGER NOT NULL DEFAULT 1, address INTEGER NOT NULL DEFAULT 0, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS groups (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS group_fixtures (group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE, PRIMARY KEY (group_key, fixture_key))",
        "CREATE TABLE IF NOT EXISTS intensities (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, dimmer REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS intensity_model_dimmers (intensity_key INTEGER REFERENCES intensities (key) ON DELETE CASCADE, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE, dimmer REAL DEFAULT 0, PRIMARY KEY (intensity_key, model_key))",
        "CREATE TABLE IF NOT EXISTS colors (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, hue REAL DEFAULT 0 NOT NULL, saturation REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS positions (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, pan REAL DEFAULT 0 NOT NULL, tilt REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS raws (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS effects (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS cuelists (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, movewhiledark INTEGER NOT NULL DEFAULT 0, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS cues (key INTEGER, id INTEGER UNIQUE NOT NULL, label TEXT DEFAULT '' NOT NULL, PRIMARY KEY (key))",
    };
    for (QString createTableQuery : createTableQueries) {
        QSqlQuery query;
        if (!query.exec(createTableQuery)) {
            qCritical() << "Failed to create table." << query.lastError().text();
            return 1;
        }
    }

    MainWindow window(VERSION, COPYRIGHT);
    window.setWindowTitle("Zöglfrex - " + fileName);
    return app.exec();
}
