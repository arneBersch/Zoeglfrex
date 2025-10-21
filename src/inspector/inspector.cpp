/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "inspector.h"

Inspector::Inspector(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    titleLabel = new QLabel("Cues");
    QFont titleFont = titleLabel->font();
    titleFont.setCapitalization(QFont::Capitalize);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    list = new QListView();
    list->setSelectionMode(QAbstractItemView::NoSelection);
    list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    list->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(list);

    infosLabel = new QLabel();
    infosLabel->setWordWrap(true);
    infosLabel->setStyleSheet("border: 1px solid white; padding: 10px;");
    layout->addWidget(infosLabel);
    infosLabel->hide();

    model = new QSqlQueryModel();
    list->setModel(model);
    reload();
}

void Inspector::reload() {
    QString table;
    if (itemName == "Model") {
        table = "models";
    } else if (itemName == "Fixture") {
        table = "fixtures";
    } else if (itemName == "Group") {
        table = "groups";
    } else if (itemName == "Intensity") {
        table = "intensities";
    } else if (itemName == "Color") {
        table = "colors";
    } else if (itemName == "Position") {
        table = "positions";
    } else if (itemName == "Raw") {
        table = "raws";
    } else if (itemName == "Effect") {
        table = "effects";
    } else if (itemName == "Cuelist") {
        table = "cuelists";
    } else if (itemName == "Cue") {
        table = "cues";
    }
    QStringList infos;
    if (!table.isEmpty()) {
        model->setQuery("SELECT CONCAT(id, ' ', label) FROM " + table + " ORDER BY id");
        if (!itemIds.isEmpty()) {
            const QString id = itemIds.last();
            const QString label = getTextAttribute(table, "label", id);
            if (!label.isEmpty()) {
                infos.append(QString(AttributeIds::id) + " ID: " + id);
                infos.append(QString(AttributeIds::label) + " Label: " + label);
                if (table == "models") {
                    infos.append(QString(AttributeIds::modelChannels) + " Channels: " + getTextAttribute(table, "channels", id));
                    infos.append(QString(AttributeIds::modelPanRange) + " Pan Range: " + getNumberAttribute(table, "panrange", id, "°"));
                    infos.append(QString(AttributeIds::modelTiltRange) + " Tilt Range: " + getNumberAttribute(table, "tiltrange", id, "°"));
                    infos.append(QString(AttributeIds::modelMinZoom) + " Minimal Zoom: " + getNumberAttribute(table, "minzoom", id, "°"));
                    infos.append(QString(AttributeIds::modelMaxZoom) + " Maximal Zoom: " + getNumberAttribute(table, "maxzoom", id, "°"));
                } else if (table == "fixtures") {
                    infos.append(QString(AttributeIds::fixtureModel) + " Model: " + getItemAttribute(table, "model_key", id, "models"));
                    infos.append(QString(AttributeIds::fixtureUniverse) + " Universe: " + getNumberAttribute(table, "universe", id, ""));
                    infos.append(QString(AttributeIds::fixtureAddress) + " Address: " + getNumberAttribute(table, "address", id, ""));
                    infos.append(QString(AttributeIds::fixtureRotation) + " Rotation: " + getNumberAttribute(table, "rotation", id, "°"));
                    infos.append(QString(AttributeIds::fixtureInvertPan) + " Invert Pan: " + getBoolAttribute(table, "invertPan", id));
                } else if (table == "groups") {
                    infos.append(QString(AttributeIds::groupFixtures) + " Fixtures: " + getItemListAttribute(table, "fixtures", "group_fixtures", "group_key", "fixture_key", id));
                } else if (table == "intensities") {
                    infos.append(QString(AttributeIds::intensityDimmer) + " Dimmer: " + getNumberAttribute(table, "dimmer", id, "%"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "intensity_model_dimmer", "intensity_key", "model_key", "dimmer", "%", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "intensity_fixture_dimmer", "intensity_key", "fixture_key", "dimmer", "%", id));
                } else if (table == "colors") {
                    infos.append(QString(AttributeIds::colorHue) + " Hue: " + getNumberAttribute(table, "hue", id, "°"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "color_model_hue", "color_key", "model_key", "hue", "°", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "color_fixture_hue", "color_key", "fixture_key", "hue", "°", id));
                    infos.append(QString(AttributeIds::colorSaturation) + " Saturation: " + getNumberAttribute(table, "saturation", id, "%"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "color_model_saturation", "color_key", "model_key", "saturation", "%", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "color_fixture_saturation", "color_key", "fixture_key", "saturation", "%", id));
                    infos.append(QString(AttributeIds::colorQuality) + " Quality: " + getNumberAttribute(table, "quality", id, "%"));
                } else if (table == "positions") {
                    infos.append(QString(AttributeIds::positionPan) + " Pan: " + getNumberAttribute(table, "pan", id, "°"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "position_model_pan", "position_key", "model_key", "pan", "°", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "position_fixture_pan", "position_key", "fixture_key", "pan", "°", id));
                    infos.append(QString(AttributeIds::positionTilt) + " Tilt: " + getNumberAttribute(table, "tilt", id, "°"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "position_model_tilt", "position_key", "model_key", "tilt", "°", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "position_fixture_tilt", "position_key", "fixture_key", "tilt", "°", id));
                    infos.append(QString(AttributeIds::positionZoom) + " Zoom: " + getNumberAttribute(table, "zoom", id, "°"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "position_model_zoom", "position_key", "model_key", "zoom", "°", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "position_fixture_zoom", "position_key", "fixture_key", "zoom", "°", id));
                } else if (table == "raws") {
                } else if (table == "effects") {
                    infos.append(QString(AttributeIds::effectSteps) + " Steps: " + getNumberAttribute(table, "steps", id, ""));
                } else if (table == "cuelists") {
                    infos.append(QString(AttributeIds::cuelistPriority) + " Priority: " + getNumberAttribute(table, "priority", id, ""));
                    infos.append(QString(AttributeIds::cuelistMoveWhileDark) + " Move while Dark: " + getBoolAttribute(table, "movewhiledark", id));
                } else if (table == "cues") {
                    infos.append(QString(AttributeIds::cueIntensities) + " Intensities: " + getItemSpecificItemAttribute("cues", "groups", "intensities", "cue_intensities", "cue_key", "group_key", "intensity_key", id));
                    infos.append(QString(AttributeIds::cueColors) + " Colors: " + getItemSpecificItemAttribute("cues", "groups", "colors", "cue_colors", "cue_key", "group_key", "color_key", id));
                    infos.append(QString(AttributeIds::cuePositions) + " Positions: " + getItemSpecificItemAttribute("cues", "groups", "positions", "cue_positions", "cue_key", "group_key", "position_key", id));
                } else {
                    Q_ASSERT(false);
                }
            }
        }
    }
    if (infos.isEmpty()) {
        infosLabel->hide();
    } else {
        infosLabel->setText(infos.join("\n"));
        infosLabel->show();
    }
    model->refresh();
}

void Inspector::loadItem(QString name, QStringList ids) {
    itemName = name;
    itemIds = ids;
    titleLabel->setText(name);
    reload();
}

QString Inspector::getBoolAttribute(const QString table, const QString attribute, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return QString();
    }
    const int value = query.value(0).toInt();
    if (value == 0 ) {
        return "False";
    } else if (value == 1) {
        return "True";
    }
    return QString();
}

QString Inspector::getTextAttribute(const QString table, const QString attribute, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return QString();
    }
    return "\"" + query.value(0).toString() + "\"";
}

QString Inspector::getNumberAttribute(const QString table, const QString attribute, const QString id, const QString unit) const {
    QSqlQuery query;
    query.prepare("SELECT ROUND(" + attribute + ", 3) FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return QString();
    }
    return query.value(0).toString() + unit;
}

QString Inspector::getItemAttribute(const QString table, const QString attribute, const QString id, const QString foreignItemTable) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemTable + ".id, ' ', " + foreignItemTable + ".label) FROM " + table + ", " + foreignItemTable + " WHERE " + table + "." + " id = :id AND " + table + "." + attribute + " = " + foreignItemTable + ".key");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return "(None)";
    }
    return query.value(0).toString();
}

QString Inspector::getItemListAttribute(const QString table, const QString foreignItemsTable, const QString listTable, const QString listTableItemAttribute, const QString listTableForeignItemsAttribute, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemsTable + ".id, ' ', " + foreignItemsTable + ".label) FROM " + table + ", " + listTable + ", " + foreignItemsTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + listTable + "." + listTableItemAttribute + " AND " + foreignItemsTable + ".key = " + listTable + "." + listTableForeignItemsAttribute + " ORDER BY " + foreignItemsTable + ".id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.lastError().text();
        return QString();
    }
    QStringList itemIds;
    while (query.next()) {
        itemIds.append(query.value(0).toString());
    }
    return itemIds.join(", ");
}

QString Inspector::getItemSpecificNumberAttribute(const QString table, const QString foreignItemsTable, const QString exceptionTable, const QString exceptionTableItemAttribute, const QString exceptionTableForeignItemsAttribute, const QString exceptionTableValueAttribute, const QString unit, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemsTable + ".id, ' ', " + foreignItemsTable + ".label), " + exceptionTable + "." + exceptionTableValueAttribute + " FROM " + table + ", " + exceptionTable + ", " + foreignItemsTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + exceptionTable + "." + exceptionTableItemAttribute + " AND " + foreignItemsTable + ".key = " + exceptionTable + "." + exceptionTableForeignItemsAttribute + " ORDER BY " + foreignItemsTable + ".id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.lastError().text();
        return QString();
    }
    QStringList values;
    while (query.next()) {
        values.append(query.value(0).toString() + " @ " + query.value(1).toString() + unit);
    }
    return values.join(", ");
}

QString Inspector::getItemSpecificItemAttribute(const QString table, const QString foreignItemTable, const QString valueItemTable, const QString valueTable, const QString valueTableItemAttribute, const QString valueTableForeignItemAttribute, const QString valueTableValueItemAttribute, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemTable + ".id, ' ', " + foreignItemTable + ".label), CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + table + ", " + foreignItemTable + ", " + valueItemTable + ", " + valueTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + valueTable + "." + valueTableItemAttribute + " AND " + foreignItemTable + ".key = " + valueTable + "." + valueTableForeignItemAttribute + " AND " + valueItemTable + ".key = " + valueTable + "." + valueTableValueItemAttribute + " ORDER BY " + foreignItemTable + ".id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.lastError().text();
        return QString();
    }
    QStringList values;
    while (query.next()) {
        values.append(query.value(0).toString() + " @ " + query.value(1).toString());
    }
    return values.join(", ");
}
