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
        "PRAGMA foreign_keys = ON",
        "CREATE TABLE IF NOT EXISTS models (key INTEGER, id TEST UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', channels TEXT DEFAULT 'D' NOT NULL, panrange REAL DEFAULT 540 NOT NULL, tiltrange REAL DEFAULT 270 NOT NULL, minzoom REAL DEFAULT 5 NOT NULL, maxzoom REAL DEFAULT 60 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS fixtures (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', model_key INTEGER REFERENCES models (key) ON DELETE SET NULL, universe INTEGER DEFAULT 1 NOT NULL, address INTEGER DEFAULT 0 NOT NULL, rotation REAL DEFAULT 0 NOT NULL, invertpan INTEGER DEFAULT 1 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS groups (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS group_fixtures (group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE, PRIMARY KEY (group_key, fixture_key))",
        "CREATE TABLE IF NOT EXISTS intensities (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', dimmer REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS intensity_model_dimmer (intensity_key INTEGER REFERENCES intensities (key) ON DELETE CASCADE, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE, dimmer REAL DEFAULT 0, PRIMARY KEY (intensity_key, model_key))",
        "CREATE TABLE IF NOT EXISTS intensity_fixture_dimmer (intensity_key INTEGER REFERENCES intensities (key) ON DELETE CASCADE, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE, dimmer REAL DEFAULT 0, PRIMARY KEY (intensity_key, fixture_key))",
        "CREATE TABLE IF NOT EXISTS colors (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', hue REAL DEFAULT 0 NOT NULL, saturation REAL DEFAULT 0 NOT NULL, quality REAL DEFAULT 100 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS color_model_hue (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE, hue REAL DEFAULT 0, PRIMARY KEY (color_key, model_key))",
        "CREATE TABLE IF NOT EXISTS color_fixture_hue (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE, hue REAL DEFAULT 0, PRIMARY KEY (color_key, fixture_key))",
        "CREATE TABLE IF NOT EXISTS color_model_saturation (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE, saturation REAL DEFAULT 0, PRIMARY KEY (color_key, model_key))",
        "CREATE TABLE IF NOT EXISTS color_fixture_saturation (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE, saturation REAL DEFAULT 0, PRIMARY KEY (color_key, fixture_key))",
        "CREATE TABLE IF NOT EXISTS positions (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', pan REAL DEFAULT 0 NOT NULL, tilt REAL DEFAULT 0 NOT NULL, zoom REAL DEFAULT 15 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS position_model_pan  (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE, pan REAL DEFAULT 0, PRIMARY KEY (position_key, model_key))",
        "CREATE TABLE IF NOT EXISTS position_fixture_pan (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE, pan REAL DEFAULT 0, PRIMARY KEY (position_key, fixture_key))",
        "CREATE TABLE IF NOT EXISTS position_model_tilt (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE, tilt REAL DEFAULT 0, PRIMARY KEY (position_key, model_key))",
        "CREATE TABLE IF NOT EXISTS position_fixture_tilt (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE, tilt REAL DEFAULT 0, PRIMARY KEY (position_key, fixture_key))",
        "CREATE TABLE IF NOT EXISTS position_model_zoom (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE, zoom REAL DEFAULT 15, PRIMARY KEY (position_key, model_key))",
        "CREATE TABLE IF NOT EXISTS position_fixture_zoom (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE, zoom REAL DEFAULT 15, PRIMARY KEY (position_key, fixture_key))",
        "CREATE TABLE IF NOT EXISTS raws (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS effects (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', steps INTEGER DEFAULT 2 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS cuelists (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', priority INTEGER DEFAULT 100 NOT NULL, movewhiledark INTEGER DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS cues (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', PRIMARY KEY (key))",
        "CREATE TABLE IF NOT EXISTS cue_intensities (cue_key INTEGER REFERENCES cues (key) ON DELETE CASCADE, group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE, intensity_key INTEGER REFERENCES intensities (key) ON DELETE CASCADE, PRIMARY KEY (cue_key, group_key))",
        "CREATE TABLE IF NOT EXISTS cue_colors (cue_key INTEGER REFERENCES cues (key) ON DELETE CASCADE, group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE, color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE, PRIMARY KEY (cue_key, group_key))",
        "CREATE TABLE IF NOT EXISTS cue_positions (cue_key INTEGER REFERENCES cues (key) ON DELETE CASCADE, group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE, position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE, PRIMARY KEY (cue_key, group_key))",
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
