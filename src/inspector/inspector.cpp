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
        QString attribute;
        QString name;
        QString unit;
    };
    QList<Attribute> attributes;
    attributes.append({"id", "ID", ""});
    attributes.append({"label", "1 Label", ""});
    if (itemTable == "models") {
        attributes.append({"channels", "2 Channels", ""});
    } else if (itemTable == "fixtures") {
        attributes.append({"model", "2 Model", ""});
    } else if (itemTable == "groups") {
    } else if (itemTable == "intensities") {
        attributes.append({"dimmer", "2 Dimmer", "%"});
    } else if (itemTable == "colors") {
        attributes.append({"hue", "2 Hue", "°"});
        attributes.append({"saturation", "3 Saturation", "%"});
    } else if (itemTable == "cues") {
    } else {
        Q_ASSERT(false);
    }
    QSqlQuery itemQuery;
    QStringList itemAttributes;
    for (Attribute attribute : attributes) {
        itemAttributes.append(attribute.attribute);
    }
    itemQuery.prepare("SELECT " + itemAttributes.join(", ") + " FROM " + itemTable + " WHERE id = :id");
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
