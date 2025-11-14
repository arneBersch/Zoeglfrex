/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "attributepanel.h"

AttributePanel::AttributePanel(const QString itemTable, const QString tableAttribute, const QString value, const QString title, const QString attributeUnit, QWidget *parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    table = itemTable;
    attribute = tableAttribute;
    valueTable = value;
    unit = attributeUnit;

    QLabel* label = new QLabel(title);
    layout->addWidget(label);

    valueLabel = new QLabel();
    layout->addWidget(valueLabel);

    reload();
}

void AttributePanel::reload() {
    QSqlQuery query;
    if (!query.exec("SELECT ROUND(" + table + "." + attribute + ", 3) FROM " + table + ", currentcue, currentitems, " + valueTable + " WHERE " + valueTable + ".item_key = currentcue.key AND " + valueTable + ".foreignitem_key = currentitems.group_key AND " + valueTable + ".valueitem_key = " + table + ".key")) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        valueLabel->setText(QString());
        return;
    }
    if (query.next()) {
        valueLabel->setText(query.value(0).toString() + unit);
    } else {
        valueLabel->setText("No value.");
    }
}
