/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "attributepanel.h"

AttributePanel::AttributePanel(const QString itemTable, const QString tableAttribute, const QString value, const QString title, const QString attributeUnit, const float min, const float max, const bool cyclicValue, QWidget *parent) : QWidget(parent) {
    table = itemTable;
    attribute = tableAttribute;
    valueTable = value;
    unit = attributeUnit;
    minValue = min;
    maxValue = max;
    cyclic = cyclicValue;

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    plus10Button = new QPushButton("+ 10" + unit);
    connect(plus10Button, &QPushButton::clicked, this, [this] { changeValue(10); });
    layout->addWidget(plus10Button);

    plus1Button = new QPushButton("+ 1" + unit);
    connect(plus1Button, &QPushButton::clicked, this, [this] { changeValue(1); });
    layout->addWidget(plus1Button);

    valueLabel = new QLabel();
    valueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(valueLabel);

    minus1Button = new QPushButton("- 1" + unit);
    connect(minus1Button, &QPushButton::clicked, this, [this] { changeValue(-1); });
    layout->addWidget(minus1Button);

    minus10Button = new QPushButton("- 10" + unit);
    connect(minus10Button, &QPushButton::clicked, this, [this] { changeValue(-10); });
    layout->addWidget(minus10Button);

    reload();
}

void AttributePanel::reload() {
    key = -1;
    value = 0;
    valueLabel->setText("No value.");
    plus10Button->setEnabled(false);
    plus1Button->setEnabled(false);
    minus1Button->setEnabled(false);
    minus10Button->setEnabled(false);
    QSqlQuery query;
    if (!query.exec("SELECT " + table + ".key, ROUND(" + table + "." + attribute + ", 3) FROM " + table + ", currentcue, currentitems, " + valueTable + " WHERE " + valueTable + ".item_key = currentcue.key AND " + valueTable + ".foreignitem_key = currentitems.group_key AND " + valueTable + ".valueitem_key = " + table + ".key")) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        valueLabel->setText(QString());
        return;
    }
    if (query.next()) {
        key = query.value(0).toInt();
        value = query.value(1).toFloat();
        valueLabel->setText(QString::number(value) + unit);
        plus10Button->setEnabled(true);
        plus1Button->setEnabled(true);
        minus1Button->setEnabled(true);
        minus10Button->setEnabled(true);
    }
}

void AttributePanel::changeValue(const float difference) {
    float newValue = value + difference;
    if (cyclic) {
        while (value >= maxValue) {
            value -= (maxValue - minValue);
        }
        while (value < minValue) {
            value += (maxValue - minValue);
        }
    } else {
        if (newValue > maxValue) {
            newValue = maxValue;
        } else if (newValue < minValue) {
            newValue = minValue;
        }
    }
    QSqlQuery query;
    query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE key = :key");
    query.bindValue(":key", key);
    query.bindValue(":value", newValue);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    emit dbChanged();
}
