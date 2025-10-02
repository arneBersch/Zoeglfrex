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
    } else if (itemName == "Cue") {
        table = "cues";
    } else if (itemName == "Cuelist") {
        table = "cuelists";
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
    infos.append("1 Label: " + label);
    if (table == "models") {
        infos.append("2 Channels: " + getTextAttribute(table, "channels", id));
    } else if (table == "fixtures") {
        infos.append("2 Model: " + getItemAttribute(table, "model", id, "models"));
        infos.append("3 Universe: " + getNumberAttribute(table, "universe", id, ""));
        infos.append("4 Address: " + getNumberAttribute(table, "address", id, ""));
    } else if (table == "groups") {
    } else if (table == "intensities") {
        infos.append("2 Dimmer: " + getNumberAttribute(table, "dimmer", id, "%"));
    } else if (table == "colors") {
        infos.append("2 Hue: " + getNumberAttribute(table, "hue", id, "°"));
        infos.append("3 Saturation: " + getNumberAttribute(table, "saturation", id, "%"));
    } else if (table == "cues") {
    } else if (table == "cuelists") {
    } else {
        Q_ASSERT(false);
    }
    return infos;
}

QString Inspector::getTextAttribute(const QString table, const QString attribute, const int id) const {
    QSqlQuery query;
    query.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) {
        return QString();
    }
    return "\"" + query.value(0).toString() + "\"";
}

QString Inspector::getNumberAttribute(const QString table, const QString attribute, const int id, const QString unit) const {
    QSqlQuery query;
    query.prepare("SELECT ROUND(" + attribute + ", 3) FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) {
        return QString();
    }
    return query.value(0).toString() + unit;
}

QString Inspector::getItemAttribute(const QString table, const QString attribute, const int id, const QString foreignItemTable) const {
    QSqlQuery query;
    query.prepare("SELECT CONCAT(" + foreignItemTable + ".id, ' ', " + foreignItemTable + ".label) FROM " + table + ", " + foreignItemTable + " WHERE " + table + "." + " id = :id AND " + table + "." + attribute + " = " + foreignItemTable + ".key");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) {
        return QString();
    }
    return query.value(0).toString();
}
