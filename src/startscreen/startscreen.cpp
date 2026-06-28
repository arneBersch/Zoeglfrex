/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "startscreen.h"
#include "mainwindow/mainwindow.h"

StartScreen::StartScreen(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    QLabel* icon = new QLabel();
    QPixmap iconPixmap = QPixmap();
    iconPixmap.load(":/resources/icon.png");
    icon->setPixmap(iconPixmap.scaled(200, 200));
    icon->setAlignment(Qt::AlignCenter);
    layout->addWidget(icon);

    QLabel* headerLabel = new QLabel("Zöglfrex " + VERSION);
    headerLabel->setStyleSheet("font-size: 50px");
    layout->addWidget(headerLabel);

    QLabel* copyrightLabel = new QLabel(COPYRIGHT);
    layout->addWidget(copyrightLabel);

    QLabel* licenseLabel = new QLabel(LICENSE_HEADER);
    licenseLabel->setWordWrap(true);
    layout->addWidget(licenseLabel);

    QPushButton* newFileButton = new QPushButton("New File");
    connect(newFileButton, &QPushButton::clicked, this, [this] {
        QString fileName = QFileDialog::getSaveFileName(this, "New File", QString(), FILENAME_FILTER);
        if (!fileName.isEmpty()) {
            if (!fileName.endsWith(".zfr")) {
                fileName += ".zfr";
            }
            openFile(fileName);
        }
    });
    layout->addWidget(newFileButton);

    QPushButton* openFileButton = new QPushButton("Open File");
    connect(openFileButton, &QPushButton::clicked, this, [this]{
        const QString fileName = QFileDialog::getOpenFileName(this, "Open File", QString(), FILENAME_FILTER);
        if (!fileName.isEmpty()) {
            openFile(fileName);
        }
    });
    layout->addWidget(openFileButton);

    QPushButton* lastFileButton = new QPushButton("Open Last File");
    QString lastFile = QSettings("zoeglfrex").value("lastfile", QString()).toString();
    if (lastFile.isEmpty()) {
        lastFileButton->setDisabled(true);
    } else {
        lastFileButton->setText("Open Last File:\n" + lastFile);
        connect(lastFileButton, &QPushButton::clicked, this, [this, lastFile]{
            openFile(lastFile);
        });
    }
    layout->addWidget(lastFileButton);

    resize(800, 600);
    show();
}

void StartScreen::openFile(const QString fileName) {
    const bool fileExists = QFile(fileName).exists();

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(fileName);
    if (!db.open()) {
        QMessageBox messageBox;
        messageBox.setText("Failed to establish a database connection.");
        messageBox.setInformativeText("This should not have happened!");
        messageBox.setDetailedText(db.lastError().text());
        messageBox.exec();
        return;
    }

    QList<QString> queries;
    queries.append("PRAGMA foreign_keys = ON");

    if (fileExists) {
        const QString fileVersion = getFileSetting("fileversion", QString()).toString();
        if (fileVersion != FILEVERSION) {
            QMessageBox messageBox;
            messageBox.setText("Can't load this Zöglfrex file because its file version isn't compatible with this version of Zöglfrex.");
            messageBox.setDetailedText("Expected \"" + FILEVERSION + "\" but got \"" + fileVersion + "\".");
            messageBox.exec();
            return;
        }
    } else {
        queries.append(getCreateFileQueries());
    }

    for (QString queryText : queries) {
        QSqlQuery query;
        if (!query.exec(queryText)) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            QMessageBox messageBox;
            messageBox.setText("Failed to create or modify table.");
            messageBox.setInformativeText("This should not have happened!");
            messageBox.setDetailedText(query.lastError().text());
            messageBox.exec();
            return;
        }
    }
    setFileSetting("fileversion", FILEVERSION);
    QSettings("zoeglfrex").setValue("lastfile", fileName);

    close();    
    new MainWindow();
}

QList<QString> StartScreen::getCreateFileQueries() {
    QList<QString> queries;

    queries.append(getCreateTableQuery(
        "models",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
            "channels TEXT DEFAULT 'D' NOT NULL",
            "panrange REAL DEFAULT 540 NOT NULL",
            "tiltrange REAL DEFAULT 270 NOT NULL",
            "minzoom REAL DEFAULT 5 NOT NULL",
            "maxzoom REAL DEFAULT 60 NOT NULL",
        },
        {"key"}
    ));

    queries.append(getCreateTableQuery(
        "fixtures",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
            "model_key INTEGER REFERENCES models (key) ON DELETE SET NULL",
            "universe INTEGER DEFAULT 1 NOT NULL",
            "address INTEGER DEFAULT 0 NOT NULL",
            "xposition REAL DEFAULT 0 NOT NULL",
            "yposition REAL DEFAULT 0 NOT NULL",
            "rotation REAL DEFAULT 0 NOT NULL",
            "invertpan INTEGER DEFAULT 0 NOT NULL",
        },
        {"key"}
    ));

    queries.append(getCreateTableQuery(
        "groups",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
        },
        {"key"}
    ));
    queries.append(getCreateItemListTableQuery("group_fixtures", "groups", "fixtures"));

    queries.append(getCreateTableQuery(
        "raws",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
            "movewhiledark INTEGER DEFAULT 0 NOT NULL",
            "fade INTEGER DEFAULT 0 NOT NULL",
        },
        {"key"}
    ));
    queries.append(getCreateIntegerSpecificNumberTableQuery("raw_channel_values", "raws", "INTEGER"));
    queries.append(getCreateItemAndIntegerSpecificNumberTableQuery("raw_model_channel_values", "raws", "models", "INTEGER"));
    queries.append(getCreateItemAndIntegerSpecificNumberTableQuery("raw_fixture_channel_values", "raws", "fixtures", "INTEGER"));

    queries.append(getCreateTableQuery(
        "intensities",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
            "dimmer REAL DEFAULT 0 NOT NULL",
        },
        {"key"}
    ));
    queries.append(getCreateItemSpecificNumberTableQuery("intensity_model_dimmer", "intensities", "models", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("intensity_fixture_dimmer", "intensities", "fixtures", "REAL"));
    queries.append(getCreateItemListTableQuery("intensity_raws", "intensities", "raws"));

    queries.append(getCreateTableQuery(
        "colors",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
            "hue REAL DEFAULT 0 NOT NULL",
            "saturation REAL DEFAULT 0 NOT NULL",
            "quality REAL DEFAULT 100 NOT NULL",
        },
        {"key"}
    ));
    queries.append(getCreateItemSpecificNumberTableQuery("color_model_hue", "colors", "models", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("color_fixture_hue", "colors", "fixtures", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("color_model_saturation", "colors", "models", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("color_fixture_saturation", "colors", "fixtures", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("color_model_quality", "colors", "models", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("color_fixture_quality", "colors", "fixtures", "REAL"));
    queries.append(getCreateItemListTableQuery("color_raws", "colors", "raws"));

    queries.append(getCreateTableQuery(
        "positions",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
            "pan REAL DEFAULT 0 NOT NULL",
            "tilt REAL DEFAULT 0 NOT NULL",
            "zoom REAL DEFAULT 15 NOT NULL",
            "focus REAL DEFAULT 0 NOT NULL",
        },
        {"key"}
    ));
    queries.append(getCreateItemSpecificNumberTableQuery("position_model_pan", "positions", "models", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("position_fixture_pan", "positions", "fixtures", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("position_model_tilt", "positions", "models", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("position_fixture_tilt", "positions", "fixtures", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("position_model_zoom", "positions", "models", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("position_fixture_zoom", "positions", "fixtures", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("position_model_focus", "positions", "models", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("position_fixture_focus", "positions", "fixtures", "REAL"));
    queries.append(getCreateItemListTableQuery("position_raws", "positions", "raws"));

    queries.append(getCreateTableQuery(
        "effects",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
            "steps INTEGER DEFAULT 2 NOT NULL",
            "hold REAL DEFAULT 0 NOT NULL",
            "fade REAL DEFAULT 0 NOT NULL",
            "phase REAL DEFAULT 0 NOT NULL",
            "sinefade INTEGER DEFAULT 0 NOT NULL",
        },
        {"key"}
    ));
    queries.append(getCreateIntegerSpecificItemTableQuery("effect_step_intensities", "effects", "intensities"));
    queries.append(getCreateIntegerSpecificItemTableQuery("effect_step_colors", "effects", "colors"));
    queries.append(getCreateIntegerSpecificItemTableQuery("effect_step_positions", "effects", "positions"));
    queries.append(getCreateIntegerSpecificItemListTableQuery("effect_step_raws", "effects", "raws"));
    queries.append(getCreateIntegerSpecificNumberTableQuery("effect_step_hold", "effects", "REAL"));
    queries.append(getCreateIntegerSpecificNumberTableQuery("effect_step_fade", "effects", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("effect_fixture_phase", "effects", "fixtures", "REAL"));

    queries.append(getCreateTableQuery(
        "cuelists",
        {
            "key INTEGER",
            "id TEXT UNIQUE NOT NULL",
            "sortkey INTEGER NOT NULL",
            "label TEXT DEFAULT ''",
            "priority INTEGER DEFAULT 100 NOT NULL",
            "movewhiledark INTEGER DEFAULT 0 NOT NULL",
        },
        {"key"}
    ));
    queries.append(getCreateTableQuery(
        "cues",
        {
            "key INTEGER",
            "id TEXT NOT NULL",
            "sortkey INTEGER NOT NULL",
            "cuelist_key INTEGER REFERENCES cuelists (key) ON DELETE CASCADE",
            "label TEXT DEFAULT ''",
            "block INTEGER DEFAULT 0 NOT NULL",
            "fade REAL DEFAULT 0 NOT NULL",
            "delay REAL DEFAULT 0 NOT NULL",
            "follow INTEGER DEFAULT 0 NOT NULL",
            "sinefade INTEGER DEFAULT 0 NOT NULL",
            "UNIQUE(id, cuelist_key)",
        },
        {"key"}
    ));
    queries.append(getCreateItemSpecificItemTableQuery("cue_group_intensities", "cues", "groups", "intensities"));
    queries.append(getCreateItemSpecificItemTableQuery("cue_group_colors", "cues", "groups", "colors"));
    queries.append(getCreateItemSpecificItemTableQuery("cue_group_positions", "cues", "groups", "positions"));
    queries.append(getCreateItemSpecificItemListTableQuery("cue_group_raws", "cues", "groups", "raws"));
    queries.append(getCreateItemSpecificItemListTableQuery("cue_group_effects", "cues", "groups", "effects"));
    queries.append(getCreateItemSpecificNumberTableQuery("cue_fixture_fade", "cues", "fixtures", "REAL"));
    queries.append(getCreateItemSpecificNumberTableQuery("cue_fixture_delay", "cues", "fixtures", "REAL"));

    queries.append("ALTER TABLE cuelists ADD COLUMN currentcue_key INTEGER REFERENCES cues (key) ON DELETE SET NULL");
    queries.append("ALTER TABLE cuelists ADD COLUMN lastcue_key INTEGER REFERENCES cues (key) ON DELETE SET NULL");

    queries.append(getCreateTableQuery(
        "currentitems",
        {
            "group_key INTEGER REFERENCES groups (key) ON DELETE SET NULL",
            "fixture_key INTEGER REFERENCES fixtures (key) ON DELETE SET NULL",
            "cuelist_key INTEGER REFERENCES cuelists (key) ON DELETE SET NULL",
            "cue_key INTEGER REFERENCES cues (key) ON DELETE SET NULL",
        },
        {"group_key", "fixture_key", "cuelist_key", "cue_key"}
    ));
    queries.append("INSERT INTO currentitems (group_key, fixture_key, cuelist_key, cue_key) VALUES (NULL, NULL, NULL, NULL)");

    queries.append("CREATE TABLE settings (key TEXT PRIMARY KEY, value TEXT NOT NULL)");

    queries.append("CREATE VIEW currentgroup_fixtures AS SELECT fixtures.* FROM fixtures, group_fixtures, currentitems WHERE group_fixtures.item_key = currentitems.group_key AND fixtures.key = group_fixtures.valueitem_key");
    queries.append("CREATE VIEW currentfixtures AS SELECT fixtures.* FROM fixtures, currentitems, group_fixtures WHERE currentitems.fixture_key = fixtures.key OR (currentitems.fixture_key IS NULL AND group_fixtures.item_key = currentitems.group_key AND fixtures.key = group_fixtures.valueitem_key)");
    queries.append("CREATE VIEW currentcuelist_cues AS SELECT cues.* FROM cues, currentitems WHERE cues.cuelist_key = currentitems.cuelist_key");
    queries.append("CREATE VIEW currentcue AS SELECT cues.* FROM cues, currentitems, cuelists WHERE currentitems.cue_key = cues.key OR (currentitems.cue_key IS NULL AND currentitems.cuelist_key = cuelists.key AND cuelists.currentcue_key = cues.key)");

    queries.append("CREATE TRIGGER resetfixture_trigger AFTER UPDATE OF group_key ON currentitems BEGIN UPDATE currentitems SET fixture_key = NULL; END");
    queries.append("CREATE TRIGGER creategroup_trigger AFTER INSERT ON groups BEGIN UPDATE currentitems SET group_key = NEW.key WHERE group_key IS NULL; END");
    queries.append("CREATE TRIGGER createcuelist_trigger AFTER INSERT ON cuelists BEGIN UPDATE currentitems SET cuelist_key = NEW.key WHERE cuelist_key IS NULL; END");
    queries.append("CREATE TRIGGER resetcue_trigger AFTER UPDATE OF cuelist_key ON currentitems BEGIN UPDATE currentitems SET cue_key = NULL; END");
    queries.append("CREATE TRIGGER createcues_trigger AFTER INSERT ON cues BEGIN UPDATE cues SET cuelist_key = (SELECT cuelist_key FROM currentitems) WHERE id = NEW.id AND cuelist_key IS NULL; END");
    queries.append("CREATE TRIGGER updatelastcue_trigger AFTER UPDATE OF currentcue_key ON cuelists BEGIN UPDATE cuelists SET lastcue_key = OLD.currentcue_key WHERE key = OLD.key AND OLD.currentcue_key != NEW.currentcue_key; END");

    queries.append("CREATE INDEX models_sortkey_index ON models (sortkey)");
    queries.append("CREATE INDEX fixtures_sortkey_index ON fixtures (sortkey)");
    queries.append("CREATE INDEX groups_sortkey_index ON groups (sortkey)");
    queries.append("CREATE INDEX intensities_sortkey_index ON intensities (sortkey)");
    queries.append("CREATE INDEX colors_sortkey_index ON colors (sortkey)");
    queries.append("CREATE INDEX positions_sortkey_index ON positions (sortkey)");
    queries.append("CREATE INDEX raws_sortkey_index ON raws (sortkey)");
    queries.append("CREATE INDEX effects_sortkey_index ON effects (sortkey)");
    queries.append("CREATE INDEX cuelists_sortkey_index ON cuelists (sortkey)");
    queries.append("CREATE INDEX cues_sortkey_index ON cues (cuelist_key, sortkey)");

    return queries;
}

QString StartScreen::getCreateTableQuery(QString tableName, QStringList fields, QStringList primaryKeys) {
    return "CREATE TABLE " + tableName + " (" + fields.join(", ") + ", PRIMARY KEY (" + primaryKeys.join(", ") + "))";
}

QString StartScreen::getCreateItemListTableQuery(QString tableName, QString itemTable, QString foreignItemTable) {
    return getCreateTableQuery(
        tableName,
        {
            "item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL",
            "valueitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL",
        },
        {"item_key", "valueitem_key"}
    );
}

QString StartScreen::getCreateItemSpecificNumberTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueType) {
    return getCreateTableQuery(
        tableName,
        {
            "item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL",
            "foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL",
            "value " + valueType + " DEFAULT 0 NOT NULL",
        },
        {"item_key", "foreignitem_key"}
    );
}

QString StartScreen::getCreateItemSpecificItemTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueItemTable) {
    return getCreateTableQuery(
        tableName,
        {
            "item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL",
            "foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL",
            "valueitem_key INTEGER REFERENCES " + valueItemTable + " (key) ON DELETE CASCADE NOT NULL",
        },
        {"item_key", "foreignitem_key"}
    );
}

QString StartScreen::getCreateItemSpecificItemListTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueItemTable) {
    return getCreateTableQuery(
        tableName,
        {
            "item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL",
            "foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL",
            "valueitem_key INTEGER REFERENCES " + valueItemTable + " (key) ON DELETE CASCADE NOT NULL",
        },
        {"item_key", "foreignitem_key", "valueitem_key"}
    );
}

QString StartScreen::getCreateIntegerSpecificItemTableQuery(QString tableName, QString itemTable, QString valueItemTable) {
    return getCreateTableQuery(
        tableName,
        {
            "item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL",
            "key INTEGER NOT NULL",
            "valueitem_key INTEGER REFERENCES " + valueItemTable + " (key) ON DELETE CASCADE NOT NULL",
        },
        {"item_key", "key"}
    );
}

QString StartScreen::getCreateIntegerSpecificItemListTableQuery(QString tableName, QString itemTable, QString valueItemTable) {
    return getCreateTableQuery(
        tableName,
        {
            "item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL",
            "key INTEGER NOT NULL",
            "valueitem_key INTEGER REFERENCES " + valueItemTable + " (key) ON DELETE CASCADE NOT NULL",
        },
        {"item_key", "key", "valueitem_key"}
    );
}

QString StartScreen::getCreateIntegerSpecificNumberTableQuery(QString tableName, QString itemTable, QString valueType) {
    return getCreateTableQuery(
        tableName,
        {
            "item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL",
            "key INTEGER NOT NULL",
            "value " + valueType + " DEFAULT 0 NOT NULL",
        },
        {"item_key", "key"}
    );
}

QString StartScreen::getCreateItemAndIntegerSpecificNumberTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueType) {
    return getCreateTableQuery(
        tableName,
        {
            "item_key INTEGER REFERENCES " + itemTable + " (key) ON DELETE CASCADE NOT NULL",
            "foreignitem_key INTEGER REFERENCES " + foreignItemTable + " (key) ON DELETE CASCADE NOT NULL",
            "key INTEGER NOT NULL",
            "value " + valueType + " DEFAULT 0 NOT NULL",
        },
        {"item_key", "foreignitem_key", "key"}
    );
}

void StartScreen::setFileSetting(const QString key, const QVariant value) {
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (:key, :value)");
    query.bindValue(":key", key);
    query.bindValue(":value", value);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
}

QVariant StartScreen::getFileSetting(const QString key, const QVariant defaultValue) {
    QSqlQuery query;
    query.prepare("SELECT value FROM settings WHERE key = :key");
    query.bindValue(":key", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return defaultValue;
    }
    if (!query.next()) {
        return defaultValue;
    }
    return query.value(0);
}
