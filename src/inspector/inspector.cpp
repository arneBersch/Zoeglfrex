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

    title = new QLabel("Cues");
    QFont titleFont = title->font();
    titleFont.setCapitalization(QFont::Capitalize);
    title->setFont(titleFont);
    layout->addWidget(title);

    list = new QListView();
    list->setSelectionMode(QAbstractItemView::NoSelection);
    list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    list->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(list);

    infos = new QLabel();
    infos->setWordWrap(true);
    infos->setStyleSheet("border: 1px solid white; padding: 10px;");
    layout->addWidget(infos);
    infos->hide();

    model = new QSqlQueryModel();
    reload();
}

void Inspector::reload() {
    delete model;
    model = new QSqlQueryModel();
    model->setQuery("SELECT CONCAT(id, ' ', label) FROM " + itemTable);
    list->setModel(model);

    QSqlQuery itemQuery;
    itemQuery.prepare("SELECT * FROM " + itemTable + " WHERE id = :id");
    itemQuery.bindValue(":id", itemId);
    itemQuery.exec();
    if (itemQuery.next()) {
        infos->setText(itemQuery.value(1).toString());
        infos->show();
    } else {
        infos->hide();
    }
}

void Inspector::loadItem(QString table, int id) {
    itemTable = table;
    itemId = id;
    title->setText(table);
    reload();
}
