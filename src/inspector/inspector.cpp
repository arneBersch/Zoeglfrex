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
    QStringList infos;
    QSqlQuery itemQuery;
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
    if (!table.isEmpty()) {
        model->setQuery("SELECT CONCAT(id, ' ', label) FROM " + table + " ORDER BY id");
        if (!itemIds.isEmpty()) {
            infos = getItemInfos(table, itemIds.last());
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

void Inspector::loadItem(QString name, QList<int> ids) {
    itemName = name;
    itemIds = ids;
    titleLabel->setText(name);
    reload();
}

QStringList Inspector::getItemInfos(const QString table, const int id) const {
    const QString label = getTextAttribute(table, "label", id);
    if (label.isEmpty()) {
        return QStringList();
    }
    QStringList infos;
    infos.append(QString::number(AttributeIds::id) + " ID: " + QString::number(id));
    infos.append(QString::number(AttributeIds::label) + " Label: " + label);
    if (table == "models") {
        infos.append(QString::number(AttributeIds::modelChannels) + " Channels: " + getTextAttribute(table, "channels", id));
    } else if (table == "fixtures") {
        infos.append(QString::number(AttributeIds::fixtureModel) + " Model: " + getItemAttribute(table, "model_key", id, "models"));
        infos.append(QString::number(AttributeIds::fixtureUniverse) + " Universe: " + getNumberAttribute(table, "universe", id, ""));
        infos.append(QString::number(AttributeIds::fixtureAddress) + " Address: " + getNumberAttribute(table, "address", id, ""));
    } else if (table == "groups") {
        infos.append(QString::number(AttributeIds::groupFixtures) + " Fixtures: " + getItemListAttribute(table, "fixtures", "group_fixtures", "group_key", "fixture_key", id));
    } else if (table == "intensities") {
        infos.append(QString::number(AttributeIds::intensityDimmer) + " Dimmer: " + getNumberAttribute(table, "dimmer", id, "%"));
        infos.append("  Model Exceptions: " + getItemSpecificNumberAttribute(table, "models", "intensity_model_dimmers", "intensity_key", "model_key", "dimmer", "%", id));
    } else if (table == "colors") {
        infos.append(QString::number(AttributeIds::colorHue) + " Hue: " + getNumberAttribute(table, "hue", id, "°"));
        infos.append(QString::number(AttributeIds::colorSaturation) + " Saturation: " + getNumberAttribute(table, "saturation", id, "%"));
    } else if (table == "positions") {
        infos.append(QString::number(AttributeIds::positionPan) + " Pan: " + getNumberAttribute(table, "pan", id, "°"));
        infos.append(QString::number(AttributeIds::positionTilt) + " Tilt: " + getNumberAttribute(table, "tilt", id, "°"));
    } else if (table == "raws") {
    } else if (table == "effects") {
    } else if (table == "cuelists") {
        infos.append(QString::number(AttributeIds::cuelistMoveWhileDark) + " Move while Dark: " + getBoolAttribute(table, "movewhiledark", id));
    } else if (table == "cues") {
    } else {
        Q_ASSERT(false);
    }
    return infos;
}

QString Inspector::getBoolAttribute(QString table, QString attribute, int id) const {
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

QString Inspector::getTextAttribute(const QString table, const QString attribute, const int id) const {
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

QString Inspector::getNumberAttribute(const QString table, const QString attribute, const int id, const QString unit) const {
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

QString Inspector::getItemAttribute(const QString table, const QString attribute, const int id, const QString foreignItemTable) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemTable + ".id, ' ', " + foreignItemTable + ".label) FROM " + table + ", " + foreignItemTable + " WHERE " + table + "." + " id = :id AND " + table + "." + attribute + " = " + foreignItemTable + ".key");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.lastError().text();
        return QString();
    }
    if (!query.next()) {
        return QString();
    }
    return query.value(0).toString();
}

QString Inspector::getItemListAttribute(const QString table, const QString foreignItemsTable, const QString listTable, const QString listTableItemAttribute, const QString listTableForeignItemsAttribute, const int id) const {
    QSqlQuery query;
    query.prepare("SELECT " + foreignItemsTable + ".id FROM " + table + ", " + listTable + ", " + foreignItemsTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + listTable + "." + listTableItemAttribute + " AND " + foreignItemsTable + ".key = " + listTable + "." + listTableForeignItemsAttribute);
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

QString Inspector::getItemSpecificNumberAttribute(const QString table, const QString foreignItemsTable, const QString exceptionTable, const QString exceptionTableItemAttribute, const QString exceptionTableForeignItemsAttribute, const QString exceptionTableValueAttribute, const QString unit, const int id) const {
    QSqlQuery query;
    query.prepare("SELECT " + foreignItemsTable + ".id, " + exceptionTable + "." + exceptionTableValueAttribute + " FROM " + table + ", " + exceptionTable + ", " + foreignItemsTable + " WHERE " + table + ".id = :id AND " + table + ".key = " + exceptionTable + "." + exceptionTableItemAttribute + " AND " + foreignItemsTable + ".key = " + exceptionTable + "." + exceptionTableForeignItemsAttribute);
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
