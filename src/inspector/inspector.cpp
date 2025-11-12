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
            const QString id = ids.last();
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
                    infos.append(QString(AttributeIds::fixtureXPosition) + " X Position: " + getNumberAttribute(table, "xposition", id, ""));
                    infos.append(QString(AttributeIds::fixtureYPosition) + " Y Position: " + getNumberAttribute(table, "yposition", id, ""));
                    infos.append(QString(AttributeIds::fixtureRotation) + " Rotation: " + getNumberAttribute(table, "rotation", id, "°"));
                    infos.append(QString(AttributeIds::fixtureInvertPan) + " Invert Pan: " + getBoolAttribute(table, "invertPan", id));
                } else if (table == "groups") {
                    infos.append(QString(AttributeIds::groupFixtures) + " Fixtures: " + getItemListAttribute(table, "fixtures", "group_fixtures", id));
                } else if (table == "intensities") {
                    infos.append(QString(AttributeIds::intensityDimmer) + " Dimmer: " + getNumberAttribute(table, "dimmer", id, "%"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "intensity_model_dimmer", "%", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "intensity_fixture_dimmer", "%", id));
                    infos.append(QString(AttributeIds::intensityRaws) + " Raws: " + getItemListAttribute(table, "raws", "intensity_raws", id));
                } else if (table == "colors") {
                    infos.append(QString(AttributeIds::colorHue) + " Hue: " + getNumberAttribute(table, "hue", id, "°"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "color_model_hue", "°", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "color_fixture_hue", "°", id));
                    infos.append(QString(AttributeIds::colorSaturation) + " Saturation: " + getNumberAttribute(table, "saturation", id, "%"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "color_model_saturation", "%", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "color_fixture_saturation", "%", id));
                    infos.append(QString(AttributeIds::colorQuality) + " Quality: " + getNumberAttribute(table, "quality", id, "%"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "color_model_quality", "%", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "color_fixture_quality", "%", id));
                    infos.append(QString(AttributeIds::colorRaws) + " Raws: " + getItemListAttribute(table, "raws", "color_raws", id));
                } else if (table == "positions") {
                    infos.append(QString(AttributeIds::positionPan) + " Pan: " + getNumberAttribute(table, "pan", id, "°"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "position_model_pan", "°", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "position_fixture_pan", "°", id));
                    infos.append(QString(AttributeIds::positionTilt) + " Tilt: " + getNumberAttribute(table, "tilt", id, "°"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "position_model_tilt", "°", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "position_fixture_tilt", "°", id));
                    infos.append(QString(AttributeIds::positionZoom) + " Zoom: " + getNumberAttribute(table, "zoom", id, "°"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "position_model_zoom", "°", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "position_fixture_zoom", "°", id));
                    infos.append(QString(AttributeIds::positionFocus) + " Focus: " + getNumberAttribute(table, "focus", id, "%"));
                    infos.append("   Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "position_model_focus", "%", id));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "position_fixture_focus", "%", id));
                    infos.append(QString(AttributeIds::positionRaws) + " Raws: " + getItemListAttribute(table, "raws", "position_raws", id));
                } else if (table == "raws") {
                    infos.append(QString(AttributeIds::rawChannelValues) + " Channel Values: " + getIntegerSpecificNumberAttribute(table, "raw_channel_values", id, ""));
                    infos.append("   Model Exceptions: " + getItemAndIntegerSpecificNumberAttribute(table, "models", "raw_model_channel_values", id, ""));
                    infos.append("   Fixture Exceptions: " + getItemAndIntegerSpecificNumberAttribute(table, "fixtures", "raw_fixture_channel_values", id, ""));
                    infos.append(QString(AttributeIds::rawMoveWhileDark) + " Move while Dark: " + getBoolAttribute(table, "movewhiledark", id));
                    infos.append(QString(AttributeIds::rawFade) + " Fade: " + getBoolAttribute(table, "fade", id));
                } else if (table == "effects") {
                    infos.append(QString(AttributeIds::effectSteps) + " Steps: " + getNumberAttribute(table, "steps", id, ""));
                    infos.append(QString(AttributeIds::effectIntensities) + " Intensities: " + getIntegerSpecificItemListAttribute(table, "intensities", "effect_step_intensities", id));
                    infos.append(QString(AttributeIds::effectColors) + " Colors: " + getIntegerSpecificItemListAttribute(table, "colors", "effect_step_colors", id));
                    infos.append(QString(AttributeIds::effectPositions) + " Positions: " + getIntegerSpecificItemListAttribute(table, "positions", "effect_step_positions", id));
                    infos.append(QString(AttributeIds::effectRaws) + " Raws: " + getIntegerSpecificItemListAttribute(table, "raws", "effect_step_raws", id));
                    infos.append(QString(AttributeIds::effectHold) + " Hold: " + getNumberAttribute(table, "hold", id, "s"));
                    infos.append("   Step Exceptions: " + getIntegerSpecificNumberAttribute(table, "effect_step_hold", id, "s"));
                    infos.append(QString(AttributeIds::effectFade) + " Fade: " + getNumberAttribute(table, "fade", id, "s"));
                    infos.append("   Step Exceptions: " + getIntegerSpecificNumberAttribute(table, "effect_step_fade", id, "s"));
                    infos.append(QString(AttributeIds::effectPhase) + " Phase: " + getNumberAttribute(table, "phase", id, "°"));
                    infos.append("   Fixture Exceptions: " + getItemSpecificNumberAttribute(table, "fixtures", "effect_fixture_phase", "°", id));
                    infos.append(QString(AttributeIds::effectSineFade) + " Sine Fade: " + getBoolAttribute(table, "sinefade", id));
                } else if (table == "cuelists") {
                    infos.append(QString(AttributeIds::cuelistPriority) + " Priority: " + getNumberAttribute(table, "priority", id, ""));
                    infos.append(QString(AttributeIds::cuelistMoveWhileDark) + " Move while Dark: " + getBoolAttribute(table, "movewhiledark", id));
                } else if (table == "currentcuelist_cues") {
                    infos.append(QString(AttributeIds::cueIntensities) + " Intensities: " + getItemSpecificItemListAttribute(table, "groups", "intensities", "cue_group_intensities", id));
                    infos.append(QString(AttributeIds::cueColors) + " Colors: " + getItemSpecificItemListAttribute(table, "groups", "colors", "cue_group_colors", id));
                    infos.append(QString(AttributeIds::cuePositions) + " Positions: " + getItemSpecificItemListAttribute(table, "groups", "positions", "cue_group_positions", id));
                    infos.append(QString(AttributeIds::cueRaws) + " Raws: " + getItemSpecificItemListAttribute(table, "groups", "raws", "cue_group_raws", id));
                    infos.append(QString(AttributeIds::cueEffects) + " Effects: " + getItemSpecificItemListAttribute(table, "groups", "effects", "cue_group_effects", id));
                    infos.append(QString(AttributeIds::cueBlock) + " Block: " + getBoolAttribute(table, "block", id));
                    infos.append(QString(AttributeIds::cueFade) + " Fade: " + getNumberAttribute(table, "fade", id, "s"));
                    infos.append(QString(AttributeIds::cueDelay) + " Delay: " + getNumberAttribute(table, "delay", id, "s"));
                    infos.append(QString(AttributeIds::cueFollow) + " Follow: " + getBoolAttribute(table, "follow", id));
                    infos.append(QString(AttributeIds::cueSineFade) + " Sine Fade: " + getBoolAttribute(table, "sinefade", id));
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
    model->setIds(ids);
    tableView->scrollTo(model->getLastSelectedRowIndex());
    model->refresh();
}

QString Inspector::getBoolAttribute(const QString table, const QString attribute, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);
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

QString Inspector::getTextAttribute(const QString table, const QString attribute, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
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
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return QString();
    }
    return query.value(0).toString() + unit;
}

QString Inspector::getItemAttribute(const QString table, const QString attribute, const QString id, const QString valueItemTable) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + table + ", " + valueItemTable + " WHERE " + table + "." + " id = :id AND " + table + "." + attribute + " = " + valueItemTable + ".key");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return "(None)";
    }
    return query.value(0).toString();
}

QString Inspector::getItemListAttribute(const QString table, const QString valueItemTable, const QString valueTable, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + table + ", " + valueTable + ", " + valueItemTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + valueTable + ".item_key AND " + valueItemTable + ".key = " + valueTable + ".valueitem_key ORDER BY " + valueItemTable + ".sortkey");
    query.bindValue(":id", id);
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

QString Inspector::getItemSpecificNumberAttribute(const QString table, const QString foreignItemsTable, const QString valueTable, const QString unit, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemsTable + ".id, ' ', " + foreignItemsTable + ".label), ROUND(" + valueTable + ".value, 3) FROM " + table + ", " + valueTable + ", " + foreignItemsTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + valueTable + ".item_key AND " + foreignItemsTable + ".key = " + valueTable + ".foreignitem_key ORDER BY " + foreignItemsTable + ".sortkey");
    query.bindValue(":id", id);
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

QString Inspector::getItemSpecificItemListAttribute(const QString table, const QString foreignItemTable, const QString valueItemTable, const QString valueTable, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemTable + ".id, ' ', " + foreignItemTable + ".label), CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + table + ", " + foreignItemTable + ", " + valueItemTable + ", " + valueTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + valueTable + ".item_key AND " + foreignItemTable + ".key = " + valueTable + ".foreignitem_key AND " + valueItemTable + ".key = " + valueTable + ".valueitem_key ORDER BY " + foreignItemTable + ".sortkey");
    query.bindValue(":id", id);
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

QString Inspector::getIntegerSpecificNumberAttribute(const QString table, const QString valueTable, const QString id, const QString unit) const {
    QSqlQuery query;
    query.prepare("SELECT " + valueTable + ".key, ROUND(" + valueTable + ".value, 3) FROM " + table + ", " + valueTable + " WHERE " + table + ".id = :id AND " + valueTable + ".item_key = " + table + ".key ORDER BY " + valueTable + ".key");
    query.bindValue(":id", id);
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

QString Inspector::getIntegerSpecificItemListAttribute(const QString table, const QString valueItemTable, const QString valueTable, const QString id) const {
    QSqlQuery query;
    query.prepare("SELECT " + valueTable + ".key, CONCAT(" + valueItemTable + ".id, ' ', " + valueItemTable + ".label) FROM " + table + ", " + valueItemTable + ", " + valueTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + valueTable + ".item_key AND " + valueItemTable + ".key = " + valueTable + ".valueitem_key ORDER BY " + valueTable + ".key, " + valueItemTable + ".sortkey");
    query.bindValue(":id", id);
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

QString Inspector::getItemAndIntegerSpecificNumberAttribute(const QString table, const QString foreignItemTable, const QString valueTable, const QString id, const QString unit) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemTable + ".id, ' ', " + foreignItemTable + ".label), " + valueTable + ".key, ROUND(" + valueTable + ".value, 3) FROM " + table + ", " + foreignItemTable + ", " + valueTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + valueTable + ".item_key AND " + foreignItemTable + ".key = " + valueTable + ".foreignitem_key ORDER BY " + foreignItemTable + ".sortkey, " + valueTable + ".key");
    query.bindValue(":id", id);
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
