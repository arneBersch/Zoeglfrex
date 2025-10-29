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
        qFatal("Can't open Zöglfrex because no file name was provided.");
        return 1;
    }
    QString fileName = QFileInfo(parser.positionalArguments().first()).absoluteFilePath();
    if (!fileName.endsWith(".zfr")) {
        qFatal("Can't open files because Zöglfrex files have to end with .zfr");
        return 1;
    }
    const bool fileExists = QFile(fileName).exists();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);
    if (!db.open()) {
        qFatal() << "Failed to establish a database connection." << db.lastError().text();
        return 1;
    }

    if (fileExists) {
        QSqlQuery versionQuery;
        if (!versionQuery.exec("SELECT version FROM about")) {
            qWarning() << Q_FUNC_INFO << versionQuery.executedQuery() << versionQuery.lastError().text();
            qFatal() << "Failed to check the version of the Zöglfrex file.";
            return 1;
        }
        if (!versionQuery.next()) {
            qFatal("Failed because no version was found in this Zöglfrex file.");
            return 1;
        }
        if (versionQuery.value(0).toString() != VERSION) {
            qFatal() << "Can't load this Zöglfrex file because its version (" << versionQuery.value(0).toString() << ") isn't compatible with this version of Zöglfrex (" << VERSION << ")";
            return 1;
        }
    } else {
        qInfo() << "Created File" << fileName;
        const QStringList createTableQueries = {
            "PRAGMA foreign_keys = ON",
            "CREATE TABLE models (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', channels TEXT DEFAULT 'D' NOT NULL, panrange REAL DEFAULT 540 NOT NULL, tiltrange REAL DEFAULT 270 NOT NULL, minzoom REAL DEFAULT 5 NOT NULL, maxzoom REAL DEFAULT 60 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE fixtures (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', model_key INTEGER REFERENCES models (key) ON DELETE SET NULL, universe INTEGER DEFAULT 1 NOT NULL, address INTEGER DEFAULT 0 NOT NULL, rotation REAL DEFAULT 0 NOT NULL, invertpan INTEGER DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE groups (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', PRIMARY KEY (key))",
            "CREATE TABLE group_fixtures (group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE NOT NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (group_key, fixture_key))",
            "CREATE TABLE raws (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', movewhiledark INTEGER DEFAULT 0 NOT NULL, fade INTEGER DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE intensities (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', dimmer REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE intensity_model_dimmer (intensity_key INTEGER REFERENCES intensities (key) ON DELETE CASCADE NOT NULL, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE NOT NULL, dimmer REAL DEFAULT 0 NOT NULL, PRIMARY KEY (intensity_key, model_key))",
            "CREATE TABLE intensity_fixture_dimmer (intensity_key INTEGER REFERENCES intensities (key) ON DELETE CASCADE NOT NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE NOT NULL, dimmer REAL DEFAULT 0 NOT NULL, PRIMARY KEY (intensity_key, fixture_key))",
            "CREATE TABLE intensity_raws (intensity_key INTEGER REFERENCES intensities (key) ON DELETE CASCADE NOT NULL, raw_key INTEGER REFERENCES raws (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (intensity_key, raw_key))",
            "CREATE TABLE colors (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', hue REAL DEFAULT 0 NOT NULL, saturation REAL DEFAULT 0 NOT NULL, quality REAL DEFAULT 100 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE color_model_hue (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE NOT NULL, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE NOT NULL, hue REAL DEFAULT 0 NOT NULL, PRIMARY KEY (color_key, model_key))",
            "CREATE TABLE color_fixture_hue (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE NOT NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE NOT NULL, hue REAL DEFAULT 0 NOT NULL, PRIMARY KEY (color_key, fixture_key))",
            "CREATE TABLE color_model_saturation (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE NOT NULL, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE NOT NULL, saturation REAL DEFAULT 0 NOT NULL, PRIMARY KEY (color_key, model_key))",
            "CREATE TABLE color_fixture_saturation (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE NOT NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE NOT NULL, saturation REAL DEFAULT 0 NOT NULL, PRIMARY KEY (color_key, fixture_key))",
            "CREATE TABLE color_raws (color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE NOT NULL, raw_key INTEGER REFERENCES raws (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (color_key, raw_key))",
            "CREATE TABLE positions (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', pan REAL DEFAULT 0 NOT NULL, tilt REAL DEFAULT 0 NOT NULL, zoom REAL DEFAULT 15 NOT NULL, focus REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE position_model_pan  (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE NOT NULL, pan REAL DEFAULT 0 NOT NULL, PRIMARY KEY (position_key, model_key))",
            "CREATE TABLE position_fixture_pan (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE NOT NULL, pan REAL DEFAULT 0 NOT NULL, PRIMARY KEY (position_key, fixture_key))",
            "CREATE TABLE position_model_tilt (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE NOT NULL, tilt REAL DEFAULT 0 NOT NULL, PRIMARY KEY (position_key, model_key))",
            "CREATE TABLE position_fixture_tilt (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE NOT NULL, tilt REAL DEFAULT 0 NOT NULL, PRIMARY KEY (position_key, fixture_key))",
            "CREATE TABLE position_model_zoom (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE NOT NULL, zoom REAL DEFAULT 15 NOT NULL, PRIMARY KEY (position_key, model_key))",
            "CREATE TABLE position_fixture_zoom (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE NOT NULL, zoom REAL DEFAULT 15 NOT NULL, PRIMARY KEY (position_key, fixture_key))",
            "CREATE TABLE position_model_focus (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, model_key INTEGER REFERENCES models (key) ON DELETE CASCADE NOT NULL, focus REAL DEFAULT 0 NOT NULL, PRIMARY KEY (position_key, model_key))",
            "CREATE TABLE position_fixture_focus (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE CASCADE NOT NULL, focus REAL DEFAULT 0 NOT NULL, PRIMARY KEY (position_key, fixture_key))",
            "CREATE TABLE position_raws (position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, raw_key INTEGER REFERENCES raws (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (position_key, raw_key))",
            "CREATE TABLE effects (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', steps INTEGER DEFAULT 2 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE cuelists (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', priority INTEGER DEFAULT 100 NOT NULL, movewhiledark INTEGER DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE cues (key INTEGER, id TEXT NOT NULL, sortkey INTEGER NOT NULL, cuelist_key INTEGER REFERENCES cuelists (key) ON DELETE CASCADE, label TEXT DEFAULT '', PRIMARY KEY (key), UNIQUE(id, cuelist_key))",
            "CREATE TABLE cue_intensities (cue_key INTEGER REFERENCES cues (key) ON DELETE CASCADE NOT NULL, group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE NOT NULL, intensity_key INTEGER REFERENCES intensities (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (cue_key, group_key))",
            "CREATE TABLE cue_colors (cue_key INTEGER REFERENCES cues (key) ON DELETE CASCADE NOT NULL, group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE NOT NULL, color_key INTEGER REFERENCES colors (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (cue_key, group_key))",
            "CREATE TABLE cue_positions (cue_key INTEGER REFERENCES cues (key) ON DELETE CASCADE NOT NULL, group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE NOT NULL, position_key INTEGER REFERENCES positions (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (cue_key, group_key))",
            "CREATE TABLE cue_raws (cue_key INTEGER REFERENCES cues (key) ON DELETE CASCADE NOT NULL, group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE NOT NULL, raw_key INTEGER REFERENCES raws (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (cue_key, group_key, raw_key))",
            "CREATE TABLE cue_effects (cue_key INTEGER REFERENCES cues (key) ON DELETE CASCADE NOT NULL, group_key INTEGER REFERENCES groups (key) ON DELETE CASCADE NOT NULL, effect_key INTEGER REFERENCES effects (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (cue_key, group_key, effect_key))",
            "ALTER TABLE cuelists ADD COLUMN currentcue_key INTEGER REFERENCES cues (key) ON DELETE SET NULL",
            "CREATE TABLE currentitems (group_key INTEGER REFERENCES groups (key) ON DELETE SET NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE SET NULL, cuelist_key INTEGER REFERENCES cuelists (key) ON DELETE SET NULL, PRIMARY KEY (group_key, fixture_key, cuelist_key))",
            "INSERT INTO currentitems (group_key, fixture_key, cuelist_key) VALUES (NULL, NULL, NULL)",
            "CREATE VIEW currentgroup_fixtures AS SELECT fixtures.* FROM fixtures, group_fixtures, currentitems WHERE group_fixtures.group_key = currentitems.group_key AND fixtures.key = group_fixtures.fixture_key",
            "CREATE TRIGGER resetfixture_trigger AFTER UPDATE OF group_key ON currentitems BEGIN UPDATE currentitems SET fixture_key = NULL; END",
            "CREATE VIEW currentcuelist_cues AS SELECT cues.* FROM cues, currentitems WHERE cues.cuelist_key = currentitems.cuelist_key",
            "CREATE TRIGGER createcues_trigger AFTER INSERT ON cues BEGIN UPDATE cues SET cuelist_key = (SELECT cuelist_key FROM currentitems) WHERE id = NEW.id AND cuelist_key IS NULL; END",
            "CREATE TABLE about (version TEXT PRIMARY KEY)",
            "INSERT INTO about (version) VALUES ('" + VERSION + "')",
        };
        for (QString createTableQuery : createTableQueries) {
            QSqlQuery query;
            if (!query.exec(createTableQuery)) {
                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                qFatal() << "Failed to create or modify table.";
                return 1;
            }
        }
    }

    QFile file(":/resources/style.qss");
    file.open(QFile::ReadOnly);
    const QString styleSheet = QString::fromLatin1(file.readAll());
    app.setStyleSheet(styleSheet);

    MainWindow window(VERSION, COPYRIGHT);
    window.setWindowTitle("Zöglfrex - " + fileName);
    return app.exec();
}
