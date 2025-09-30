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
    reload();
}

void Inspector::reload() {
    delete model;
    model = new QSqlQueryModel();
    model->setQuery("SELECT CONCAT(id, ' ', label) FROM " + itemTable + " ORDER BY id");
    list->setModel(model);

    struct Attribute {
        QString name;
        QString unit;
        Attribute(QString nm, QString un) : name(nm), unit(un) {}
    };
    QList<Attribute> attributes;
    attributes.append(Attribute("ID", ""));
    attributes.append(Attribute("1 Label", ""));
    QSqlQuery itemQuery;
    if (itemTable == "models") {
        itemQuery.prepare("SELECT id, label, channels FROM models WHERE id = :id");
        attributes.append(Attribute("2 Channels", ""));
    } else if (itemTable == "fixtures") {
        itemQuery.prepare("SELECT fixtures.id, fixtures.label, CONCAT(models.id, ' ', models.label) FROM fixtures, models WHERE fixtures.id = :id AND model = models.key");
        attributes.append(Attribute("2 Model", ""));
    } else if (itemTable == "groups") {
        itemQuery.prepare("SELECT id, label FROM groups WHERE id = :id");
    } else if (itemTable == "intensities") {
        itemQuery.prepare("SELECT id, label, dimmer FROM intensities WHERE id = :id");
        attributes.append(Attribute("2 Dimmer", "%"));
    } else if (itemTable == "colors") {
        itemQuery.prepare("SELECT id, label, hue, saturation FROM colors WHERE id = :id");
        attributes.append(Attribute("2 Hue", "°"));
        attributes.append(Attribute("3 Saturation", "%"));
    } else if (itemTable == "cues") {
        itemQuery.prepare("SELECT id, label FROM cues WHERE id = :id");
    } else {
        Q_ASSERT(false);
    }
    itemQuery.bindValue(":id", itemId);
    itemQuery.exec();
    if (itemQuery.next()) {
        QStringList infos;
        for (int attributeIndex = 0; attributeIndex < attributes.length(); attributeIndex++) {
            infos.append(attributes.at(attributeIndex).name + ": " + itemQuery.value(attributeIndex).toString() + attributes.at(attributeIndex).unit);
        }
        infosLabel->setText(infos.join("\n"));
        infosLabel->show();
    } else {
        infosLabel->hide();
    }
}

void Inspector::loadItem(QString table, int id) {
    itemTable = table;
    itemId = id;
    titleLabel->setText(table);
    reload();
}
