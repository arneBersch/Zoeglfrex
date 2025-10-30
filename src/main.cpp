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
        auto createItemListAttributeTable = [] (const QString tableName, const QString itemTable, const QString foreignItemTable) {
            return "CREATE TABLE " + tableName + " (item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL, foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (item_key, foreignitem_key))";
        };
        auto createItemSpecificNumberAttributeTable = [] (const QString tableName, const QString itemTable, const QString foreignItemTable, const QString valueType) {
            return "CREATE TABLE " + tableName + " (item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL, foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL, value " + valueType + " DEFAULT 0 NOT NULL, PRIMARY KEY (item_key, foreignitem_key))";
        };
        auto createItemSpecificItemAttributeTable = [] (const QString tableName, const QString itemTable, const QString foreignItemTable, const QString valueItemTable) {
            return "CREATE TABLE " + tableName + " (item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL, foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL, valueitem_key INTEGER REFERENCES " + valueItemTable + " (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (item_key, foreignitem_key))";
        };
        auto createItemSpecificItemListAttributeTable = [] (const QString tableName, const QString itemTable, const QString foreignItemTable, const QString valueItemTable) {
            return "CREATE TABLE " + tableName + " (item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL, foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL, valueitem_key INTEGER REFERENCES " + valueItemTable + " (key) ON DELETE CASCADE NOT NULL, PRIMARY KEY (item_key, foreignitem_key, valueitem_key))";
        };
        auto createIntegerSpecificNumberAttribute = [] (const QString tableName, const QString itemTable, const QString valueType) {
            return "CREATE TABLE " + tableName + " (item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL, key INTEGER NOT NULL, value " + valueType + " DEFAULT 0 NOT NULL, PRIMARY KEY (item_key, key))";
        };
        auto createItemAndIntegerSpecificNumberAttribute = [] (const QString tableName, const QString itemTable, const QString foreignItemTable, const QString valueType) {
            return "CREATE TABLE " + tableName + " (item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL, foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL, key INTEGER NOT NULL, value " + valueType + " DEFAULT 0 NOT NULL, PRIMARY KEY (item_key, foreignitem_key, key))";
        };
        const QStringList createTableQueries = {
            "PRAGMA foreign_keys = ON",
            "CREATE TABLE models (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', channels TEXT DEFAULT 'D' NOT NULL, panrange REAL DEFAULT 540 NOT NULL, tiltrange REAL DEFAULT 270 NOT NULL, minzoom REAL DEFAULT 5 NOT NULL, maxzoom REAL DEFAULT 60 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE fixtures (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', model_key INTEGER REFERENCES models (key) ON DELETE SET NULL, universe INTEGER DEFAULT 1 NOT NULL, address INTEGER DEFAULT 0 NOT NULL, rotation REAL DEFAULT 0 NOT NULL, invertpan INTEGER DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE groups (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', PRIMARY KEY (key))",
            createItemListAttributeTable("group_fixtures", "groups", "fixtures"),
            "CREATE TABLE raws (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', movewhiledark INTEGER DEFAULT 0 NOT NULL, fade INTEGER DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            createIntegerSpecificNumberAttribute("raw_channel_values", "raws", "INTEGER"),
            createItemAndIntegerSpecificNumberAttribute("raw_model_channel_values", "raws", "models", "INTEGER"),
            createItemAndIntegerSpecificNumberAttribute("raw_fixture_channel_values", "raws", "fixtures", "INTEGER"),
            "CREATE TABLE intensities (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', dimmer REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            createItemSpecificNumberAttributeTable("intensity_model_dimmer", "intensities", "models", "REAL"),
            createItemSpecificNumberAttributeTable("intensity_fixture_dimmer", "intensities", "fixtures", "REAL"),
            createItemListAttributeTable("intensity_raws", "intensities", "raws"),
            "CREATE TABLE colors (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', hue REAL DEFAULT 0 NOT NULL, saturation REAL DEFAULT 0 NOT NULL, quality REAL DEFAULT 100 NOT NULL, PRIMARY KEY (key))",
            createItemSpecificNumberAttributeTable("color_model_hue", "colors", "models", "REAL"),
            createItemSpecificNumberAttributeTable("color_fixture_hue", "colors", "fixtures", "REAL"),
            createItemSpecificNumberAttributeTable("color_model_saturation", "colors", "models", "REAL"),
            createItemSpecificNumberAttributeTable("color_fixture_saturation", "colors", "fixtures", "REAL"),
            createItemListAttributeTable("color_raws", "colors", "raws"),
            "CREATE TABLE positions (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', pan REAL DEFAULT 0 NOT NULL, tilt REAL DEFAULT 0 NOT NULL, zoom REAL DEFAULT 15 NOT NULL, focus REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            createItemSpecificNumberAttributeTable("position_model_pan", "positions", "models", "REAL"),
            createItemSpecificNumberAttributeTable("position_fixture_pan", "positions", "fixtures", "REAL"),
            createItemSpecificNumberAttributeTable("position_model_tilt", "positions", "models", "REAL"),
            createItemSpecificNumberAttributeTable("position_fixture_tilt", "positions", "fixtures", "REAL"),
            createItemSpecificNumberAttributeTable("position_model_zoom", "positions", "models", "REAL"),
            createItemSpecificNumberAttributeTable("position_fixture_zoom", "positions", "fixtures", "REAL"),
            createItemSpecificNumberAttributeTable("position_model_focus", "positions", "models", "REAL"),
            createItemSpecificNumberAttributeTable("position_fixture_focus", "positions", "fixtures", "REAL"),
            createItemListAttributeTable("position_raws", "positions", "raws"),
            "CREATE TABLE effects (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', steps INTEGER DEFAULT 2 NOT NULL, hold REAL DEFAULT 0 NOT NULL, fade REAL DEFAULT 0 NOT NULL, phase REAL DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            createIntegerSpecificNumberAttribute("effect_step_hold", "effects", "REAL"),
            createIntegerSpecificNumberAttribute("effect_step_fade", "effects", "REAL"),
            createItemSpecificNumberAttributeTable("effect_fixture_phase", "effects", "fixtures", "REAL"),
            "CREATE TABLE cuelists (key INTEGER, id TEXT UNIQUE NOT NULL, sortkey INTEGER NOT NULL, label TEXT DEFAULT '', priority INTEGER DEFAULT 100 NOT NULL, movewhiledark INTEGER DEFAULT 0 NOT NULL, PRIMARY KEY (key))",
            "CREATE TABLE cues (key INTEGER, id TEXT NOT NULL, sortkey INTEGER NOT NULL, cuelist_key INTEGER REFERENCES cuelists (key) ON DELETE CASCADE, label TEXT DEFAULT '', fade REAL DEFAULT 0 NOT NULL, block INTEGER DEFAULT 0 NOT NULL, PRIMARY KEY (key), UNIQUE(id, cuelist_key))",
            createItemSpecificItemAttributeTable("cue_group_intensities", "cues", "groups", "intensities"),
            createItemSpecificItemAttributeTable("cue_group_colors", "cues", "groups", "colors"),
            createItemSpecificItemAttributeTable("cue_group_positions", "cues", "groups", "positions"),
            createItemSpecificItemListAttributeTable("cue_group_raws", "cues", "groups", "raws"),
            createItemSpecificItemListAttributeTable("cue_group_effects", "cues", "groups", "effects"),
            "ALTER TABLE cuelists ADD COLUMN currentcue_key INTEGER REFERENCES cues (key) ON DELETE SET NULL",
            "CREATE TABLE currentitems (group_key INTEGER REFERENCES groups (key) ON DELETE SET NULL, fixture_key INTEGER REFERENCES fixtures (key) ON DELETE SET NULL, cuelist_key INTEGER REFERENCES cuelists (key) ON DELETE SET NULL, PRIMARY KEY (group_key, fixture_key, cuelist_key))",
            "INSERT INTO currentitems (group_key, fixture_key, cuelist_key) VALUES (NULL, NULL, NULL)",
            "CREATE VIEW currentgroup_fixtures AS SELECT fixtures.* FROM fixtures, group_fixtures, currentitems WHERE group_fixtures.item_key = currentitems.group_key AND fixtures.key = group_fixtures.foreignitem_key",
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
