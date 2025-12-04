/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "inspector.h"

Inspector::Inspector(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    titleLabel = new QLabel("Cues");
    layout->addWidget(titleLabel);

    tableView = new QTableView();
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setFocusPolicy(Qt::NoFocus);
    tableView->verticalHeader()->hide();
    tableView->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(tableView);

    infosLabel = new QLabel();
    infosLabel->setWordWrap(true);
    infosLabel->setStyleSheet("border: 1px solid white; padding: 10px;");
    layout->addWidget(infosLabel);
    infosLabel->hide();

    model = new ItemTableModel();
    tableView->setModel(model);
}

void Inspector::loadItems(const QString itemName, const QStringList ids) {
    QString table;
    QString title;
    if (itemName == "Model") {
        table = "models";
        title = "Models";
    } else if (itemName == "Fixture") {
        table = "fixtures";
        title = "Fixtures";
    } else if (itemName == "Group") {
        table = "groups";
        title = "Groups";
    } else if (itemName == "Intensity") {
        table = "intensities";
        title = "Intensities";
    } else if (itemName == "Color") {
        table = "colors";
        title = "Colors";
    } else if (itemName == "Position") {
        table = "positions";
        title = "Position";
    } else if (itemName == "Raw") {
        table = "raws";
        title = "Raws";
    } else if (itemName == "Effect") {
        table = "effects";
        title = "Effects";
    } else if (itemName == "Cuelist") {
        table = "cuelists";
        title = "Cuelists";
    } else if (itemName == "Cue") {
        table = "currentcuelist_cues";
        title = "Cues";
        QSqlQuery currentCuelistQuery;
        if (currentCuelistQuery.exec("SELECT CONCAT(cuelists.id, ' ', cuelists.label) FROM cuelists, currentitems WHERE currentitems.cuelist_key = cuelists.key")) {
            if (currentCuelistQuery.next()) {
                title.append(" (Cuelist " + currentCuelistQuery.value(0).toString() + ")");
            } else {
                title.append(" (No cuelist selected.)");
            }
        } else {
            qWarning() << Q_FUNC_INFO << currentCuelistQuery.executedQuery() << currentCuelistQuery.lastError().text();
        }
    }
    QStringList infos;
    if (!table.isEmpty()) {
        titleLabel->setText(title);
        model->setTable(table);
        if (!ids.isEmpty()) {
            QSqlQuery keyQuery;
            keyQuery.prepare("SELECT key FROM " + table + " WHERE id = :id");
            keyQuery.bindValue(":id", ids.last());
            if (keyQuery.exec()) {
                if (keyQuery.next()) {
                    const int key = keyQuery.value(0).toInt();
                    infos.append(QString(AttributeIds::id) + " ID: " + ids.last());
                    infos.append(QString(AttributeIds::label) + " Label: " + getTextAttribute(table, "label", key));
                    if (table == "models") {
                        infos.append(QString(AttributeIds::modelChannels) + " Channels: " + getTextAttribute(table, "channels", key));
                        infos.append(QString(AttributeIds::modelPanRange) + " Pan Range: " + getNumberAttribute(table, "panrange", key, "°"));
                        infos.append(QString(AttributeIds::modelTiltRange) + " Tilt Range: " + getNumberAttribute(table, "tiltrange", key, "°"));
                        infos.append(QString(AttributeIds::modelMinZoom) + " Minimal Zoom: " + getNumberAttribute(table, "minzoom", key, "°"));
                        infos.append(QString(AttributeIds::modelMaxZoom) + " Maximal Zoom: " + getNumberAttribute(table, "maxzoom", key, "°"));
                    } else if (table == "fixtures") {
                        infos.append(QString(AttributeIds::fixtureModel) + " Model: " + getItemAttribute(table, "model_key", key, "models"));
                        infos.append(QString(AttributeIds::fixtureUniverse) + " Universe: " + getNumberAttribute(table, "universe", key, ""));
                        infos.append(QString(AttributeIds::fixtureAddress) + " Address: " + getNumberAttribute(table, "address", key, ""));
                        infos.append(QString(AttributeIds::fixtureXPosition) + " X Position: " + getNumberAttribute(table, "xposition", key, ""));
                        infos.append(QString(AttributeIds::fixtureYPosition) + " Y Position: " + getNumberAttribute(table, "yposition", key, ""));
                        infos.append(QString(AttributeIds::fixtureRotation) + " Rotation: " + getNumberAttribute(table, "rotation", key, "°"));
                        infos.append(QString(AttributeIds::fixtureInvertPan) + " Invert Pan: " + getBoolAttribute(table, "invertPan", key));
                    } else if (table == "groups") {
                        infos.append(QString(AttributeIds::groupFixtures) + " Fixtures: " + getItemListAttribute("fixtures", "group_fixtures", key));
                    } else if (table == "intensities") {
                        infos.append(QString(AttributeIds::intensityDimmer) + " Dimmer: " + getNumberAttribute(table, "dimmer", key, "%"));
                        infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute("models", "intensity_model_dimmer", "%", key));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "intensity_fixture_dimmer", "%", key));
                        infos.append(QString(AttributeIds::intensityRaws) + " Raws: " + getItemListAttribute("raws", "intensity_raws", key));
                    } else if (table == "colors") {
                        infos.append(QString(AttributeIds::colorHue) + " Hue: " + getNumberAttribute(table, "hue", key, "°"));
                        infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute("models", "color_model_hue", "°", key));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "color_fixture_hue", "°", key));
                        infos.append(QString(AttributeIds::colorSaturation) + " Saturation: " + getNumberAttribute(table, "saturation", key, "%"));
                        infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute("models", "color_model_saturation", "%", key));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "color_fixture_saturation", "%", key));
                        infos.append(QString(AttributeIds::colorQuality) + " Quality: " + getNumberAttribute(table, "quality", key, "%"));
                        infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute("models", "color_model_quality", "%", key));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "color_fixture_quality", "%", key));
                        infos.append(QString(AttributeIds::colorRaws) + " Raws: " + getItemListAttribute("raws", "color_raws", key));
                    } else if (table == "positions") {
                        infos.append(QString(AttributeIds::positionPan) + " Pan: " + getNumberAttribute(table, "pan", key, "°"));
                        infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute("models", "position_model_pan", "°", key));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "position_fixture_pan", "°", key));
                        infos.append(QString(AttributeIds::positionTilt) + " Tilt: " + getNumberAttribute(table, "tilt", key, "°"));
                        infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute("models", "position_model_tilt", "°", key));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "position_fixture_tilt", "°", key));
                        infos.append(QString(AttributeIds::positionZoom) + " Zoom: " + getNumberAttribute(table, "zoom", key, "°"));
                        infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute("models", "position_model_zoom", "°", key));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "position_fixture_zoom", "°", key));
                        infos.append(QString(AttributeIds::positionFocus) + " Focus: " + getNumberAttribute(table, "focus", key, "%"));
                        infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute("models", "position_model_focus", "%", key));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "position_fixture_focus", "%", key));
                        infos.append(QString(AttributeIds::positionRaws) + " Raws: " + getItemListAttribute("raws", "position_raws", key));
                    } else if (table == "raws") {
                        infos.append(QString(AttributeIds::rawChannelValues) + " Channel Values: " + getIntegerSpecificNumberAttribute("raw_channel_values", key, ""));
                        infos.append("   Model Exceptions: " + getItemAndIntegerSpecificNumberAttribute("models", "raw_model_channel_values", key, ""));
                        infos.append("   Fixture Exceptions: " + getItemAndIntegerSpecificNumberAttribute("fixtures", "raw_fixture_channel_values", key, ""));
                        infos.append(QString(AttributeIds::rawMoveWhileDark) + " Move while Dark: " + getBoolAttribute(table, "movewhiledark", key));
                        infos.append(QString(AttributeIds::rawFade) + " Fade: " + getBoolAttribute(table, "fade", key));
                    } else if (table == "effects") {
                        infos.append(QString(AttributeIds::effectSteps) + " Steps: " + getNumberAttribute(table, "steps", key, ""));
                        infos.append(QString(AttributeIds::effectIntensities) + " Intensities: " + getIntegerSpecificItemListAttribute("intensities", "effect_step_intensities", key));
                        infos.append(QString(AttributeIds::effectColors) + " Colors: " + getIntegerSpecificItemListAttribute("colors", "effect_step_colors", key));
                        infos.append(QString(AttributeIds::effectPositions) + " Positions: " + getIntegerSpecificItemListAttribute("positions", "effect_step_positions", key));
                        infos.append(QString(AttributeIds::effectRaws) + " Raws: " + getIntegerSpecificItemListAttribute("raws", "effect_step_raws", key));
                        infos.append(QString(AttributeIds::effectHold) + " Hold: " + getNumberAttribute(table, "hold", key, "s"));
                        infos.append("   Step Exceptions: " + getIntegerSpecificNumberAttribute("effect_step_hold", key, "s"));
                        infos.append(QString(AttributeIds::effectFade) + " Fade: " + getNumberAttribute(table, "fade", key, "s"));
                        infos.append("   Step Exceptions: " + getIntegerSpecificNumberAttribute("effect_step_fade", key, "s"));
                        infos.append(QString(AttributeIds::effectPhase) + " Phase: " + getNumberAttribute(table, "phase", key, "°"));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "effect_fixture_phase", "°", key));
                        infos.append(QString(AttributeIds::effectSineFade) + " Sine Fade: " + getBoolAttribute(table, "sinefade", key));
                    } else if (table == "cuelists") {
                        infos.append(QString(AttributeIds::cuelistPriority) + " Priority: " + getNumberAttribute(table, "priority", key, ""));
                        infos.append(QString(AttributeIds::cuelistMoveWhileDark) + " Move while Dark: " + getBoolAttribute(table, "movewhiledark", key));
                    } else if (table == "currentcuelist_cues") {
                        infos.append(QString(AttributeIds::cueIntensities) + " Intensities: " + getItemSpecificItemListAttribute("groups", "intensities", "cue_group_intensities", key));
                        infos.append(QString(AttributeIds::cueColors) + " Colors: " + getItemSpecificItemListAttribute("groups", "colors", "cue_group_colors", key));
                        infos.append(QString(AttributeIds::cuePositions) + " Positions: " + getItemSpecificItemListAttribute("groups", "positions", "cue_group_positions", key));
                        infos.append(QString(AttributeIds::cueRaws) + " Raws: " + getItemSpecificItemListAttribute("groups", "raws", "cue_group_raws", key));
                        infos.append(QString(AttributeIds::cueEffects) + " Effects: " + getItemSpecificItemListAttribute("groups", "effects", "cue_group_effects", key));
                        infos.append(QString(AttributeIds::cueBlock) + " Block: " + getBoolAttribute(table, "block", key));
                        infos.append(QString(AttributeIds::cueFade) + " Fade: " + getNumberAttribute(table, "fade", key, "s"));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "cue_fixture_fade", "s", key));
                        infos.append(QString(AttributeIds::cueDelay) + " Delay: " + getNumberAttribute(table, "delay", key, "s"));
                        infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute("fixtures", "cue_fixture_delay", "s", key));
                        infos.append(QString(AttributeIds::cueFollow) + " Follow: " + getBoolAttribute(table, "follow", key));
                        infos.append(QString(AttributeIds::cueSineFade) + " Sine Fade: " + getBoolAttribute(table, "sinefade", key));
                    } else {
                        Q_ASSERT(false);
                    }
                }
            } else {
                qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            }
        }
    }
    if (infos.isEmpty()) {
        infosLabel->hide();
    } else {
        infosLabel->setText(infos.join("\n"));
        infosLabel->show();
    }
    model->setIds(ids);
    tableView->scrollTo(model->getLastSelectedRowIndex());
    model->refresh();
}

QString Inspector::getBoolAttribute(const QString table, const QString attribute, const int key) const {
    QSqlQuery query;
    query.prepare("SELECT " + attribute + " FROM " + table + " WHERE key = :item");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
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

QString Inspector::getTextAttribute(const QString table, const QString attribute, const int key) const {
    QSqlQuery query;
    query.prepare("SELECT " + attribute + " FROM " + table + " WHERE key = :item");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return QString();
    }
    return "\"" + query.value(0).toString() + "\"";
}

QString Inspector::getNumberAttribute(const QString table, const QString attribute, const int key, const QString unit) const {
    QSqlQuery query;
    query.prepare("SELECT ROUND(" + attribute + ", 3) FROM " + table + " WHERE key = :item");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return QString();
    }
    return query.value(0).toString() + unit;
}

QString Inspector::getItemAttribute(const QString table, const QString attribute, const int key, const QString valueItemTable) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + table + ", " + valueItemTable + " WHERE " + table + ".key = :item AND " + table + "." + attribute + " = " + valueItemTable + ".key");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return "(None)";
    }
    return query.value(0).toString();
}

QString Inspector::getItemListAttribute(const QString valueItemTable, const QString valueTable, const int key) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + valueTable + ", " + valueItemTable + " WHERE " + valueTable + ".item_key = :item AND " + valueItemTable + ".key = " + valueTable + ".valueitem_key ORDER BY " + valueItemTable + ".sortkey");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    QStringList itemIds;
    while (query.next()) {
        itemIds.append(query.value(0).toString());
    }
    return itemIds.join(", ");
}

QString Inspector::getItemSpecificNumberAttribute(const QString foreignItemsTable, const QString valueTable, const QString unit, const int key) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemsTable + ".id, ' ', " + foreignItemsTable + ".label), ROUND(" + valueTable + ".value, 3) FROM " + valueTable + ", " + foreignItemsTable + " WHERE " + valueTable + ".item_key = :item AND " + foreignItemsTable + ".key = " + valueTable + ".foreignitem_key ORDER BY " + foreignItemsTable + ".sortkey");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    QStringList values;
    while (query.next()) {
        values.append(query.value(0).toString() + " @ " + query.value(1).toString() + unit);
    }
    return values.join(", ");
}

QString Inspector::getItemSpecificItemListAttribute(const QString foreignItemTable, const QString valueItemTable, const QString valueTable, const int key) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemTable + ".id, ' ', " + foreignItemTable + ".label), CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + foreignItemTable + ", " + valueItemTable + ", " + valueTable + " WHERE " + valueTable + ".item_key = :item AND " + foreignItemTable + ".key = " + valueTable + ".foreignitem_key AND " + valueItemTable + ".key = " + valueTable + ".valueitem_key ORDER BY " + foreignItemTable + ".sortkey");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    QMap<QString, QStringList> foreignItemValues;
    while (query.next()) {
        const QString foreignItem = query.value(0).toString();
        if (!foreignItemValues.contains(foreignItem)) {
            foreignItemValues[foreignItem] = QStringList();
        }
        foreignItemValues[foreignItem].append(query.value(1).toString());
    }
    QStringList values;
    for (QString foreignItem : foreignItemValues.keys()) {
        values.append(foreignItem + " @ " + foreignItemValues.value(foreignItem).join(", "));
    }
    return values.join("; ");
}

QString Inspector::getIntegerSpecificNumberAttribute(const QString valueTable, const int key, const QString unit) const {
    QSqlQuery query;
    query.prepare("SELECT key, ROUND(value, 3) FROM " + valueTable + " WHERE item_key = :item ORDER BY key");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    QStringList values;
    while (query.next()) {
        values.append(query.value(0).toString() + " @ " + query.value(1).toString() + unit);
    }
    return values.join(", ");
}

QString Inspector::getIntegerSpecificItemListAttribute(const QString valueItemTable, const QString valueTable, const int key) const {
    QSqlQuery query;
    query.prepare("SELECT " + valueTable + ".key, CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + valueItemTable + ", " + valueTable + " WHERE " + valueTable + ".item_key = :item AND " + valueItemTable + ".key = " + valueTable + ".valueitem_key ORDER BY " + valueTable + ".key, " + valueItemTable + ".sortkey");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    QMap<QString, QStringList> foreignItemValues;
    while (query.next()) {
        const QString foreignItem = query.value(0).toString();
        if (!foreignItemValues.contains(foreignItem)) {
            foreignItemValues[foreignItem] = QStringList();
        }
        foreignItemValues[foreignItem].append(query.value(1).toString());
    }
    QStringList values;
    for (QString foreignItem : foreignItemValues.keys()) {
        values.append(foreignItem + " @ " + foreignItemValues.value(foreignItem).join(", "));
    }
    return values.join("; ");
}

QString Inspector::getItemAndIntegerSpecificNumberAttribute(const QString foreignItemTable, const QString valueTable, const int key, const QString unit) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemTable + ".id, ' ', " + foreignItemTable + ".label), " + valueTable + ".key, ROUND(" + valueTable + ".value, 3) FROM " + foreignItemTable + ", " + valueTable + " WHERE " + valueTable + ".item_key = :item AND " + foreignItemTable + ".key = " + valueTable + ".foreignitem_key ORDER BY " + foreignItemTable + ".sortkey, " + valueTable + ".key");
    query.bindValue(":item", key);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    QMap<QString, QStringList> foreignItemValues;
    while (query.next()) {
        const QString foreignItem = query.value(0).toString();
        if (!foreignItemValues.contains(foreignItem)) {
            foreignItemValues[foreignItem] = QStringList();
        }
        foreignItemValues[foreignItem].append(query.value(1).toString() + " @ " + query.value(2).toString() + unit);
    }
    QStringList values;
    for (QString foreignItem : foreignItemValues.keys()) {
        values.append(foreignItem + ": " + foreignItemValues.value(foreignItem).join(", "));
    }
    return values.join("; ");
}
