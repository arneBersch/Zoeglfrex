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
    struct Attribute {
        QString name;
        QString beforeValue;
        QString afterValue;
    };
    QList<Attribute> attributes;
    attributes.append({"ID", "", ""});
    attributes.append({"1 Label", "\"", "\""});
    QSqlQuery itemQuery;
    if (itemName == "Model") {
        table = "models";
        itemQuery.prepare("SELECT id, label, channels FROM models WHERE id = :id");
        attributes.append({"2 Channels", "\"", "\""});
    } else if (itemName == "Fixture") {
        table = "fixtures";
        itemQuery.prepare("SELECT fixtures.id, fixtures.label, CONCAT(models.id, ' ', models.label), universe, address FROM fixtures, models WHERE fixtures.id = :id AND model = models.key");
        attributes.append({"2 Model", "", ""});
        attributes.append({"3 Universe", "", ""});
        attributes.append({"4 Address", "", ""});
    } else if (itemName == "Group") {
        table = "models";
        itemQuery.prepare("SELECT id, label FROM groups WHERE id = :id");
    } else if (itemName == "Intensity") {
        table = "intensities";
        itemQuery.prepare("SELECT id, label, ROUND(dimmer, 3) FROM intensities WHERE id = :id");
        attributes.append({"2 Dimmer", "", "%"});
    } else if (itemName == "Color") {
        table = "colors";
        itemQuery.prepare("SELECT id, label, ROUND(hue, 3), ROUND(saturation, 3) FROM colors WHERE id = :id");
        attributes.append({"2 Hue", "", "°"});
        attributes.append({"3 Saturation", "", "%"});
    } else if (itemName == "Cue") {
        table = "cues";
        itemQuery.prepare("SELECT id, label FROM cues WHERE id = :id");
    } else if (itemName == "Cuelists") {
        table = "cuelists";
        itemQuery.prepare("SELECT id, label FROM cuelists WHERE id = :id");
    }
    itemQuery.bindValue(":id", itemIds.last());
    itemQuery.exec();
    if (itemQuery.next()) {
        QStringList infos;
        for (int attributeIndex = 0; attributeIndex < attributes.length(); attributeIndex++) {
            Attribute attribute = attributes.at(attributeIndex);
            infos.append(attribute.name + ": " + attribute.beforeValue + itemQuery.value(attributeIndex).toString() + attribute.afterValue);
        }
        infosLabel->setText(infos.join("\n"));
        infosLabel->show();
    } else {
        infosLabel->hide();
    }
    if (!table.isEmpty()) {
        model->setQuery("SELECT CONCAT(id, ' ', label) FROM " + table + " ORDER BY id");
    }
}

void Inspector::loadItem(QString name, QList<int> ids) {
    itemName = name;
    itemIds = ids;
    titleLabel->setText(name);
    reload();
}
