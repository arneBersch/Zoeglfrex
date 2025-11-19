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

    valueSpinBox = new QDoubleSpinBox();
    valueSpinBox->setRange(min, max);
    connect(valueSpinBox, &QDoubleSpinBox::valueChanged, this, &AttributePanel::setValue);
    layout->addWidget(valueSpinBox);

    valueSlider = new QSlider();
    valueSlider->setRange(min, max);
    connect(valueSlider, &QSlider::valueChanged, this, &AttributePanel::setValue);
    layout->addWidget(valueSlider);

    reload();
}

void AttributePanel::reload() {
    key = -1;
    QSqlQuery query;
    if (!query.exec("SELECT " + table + ".key, ROUND(" + table + "." + attribute + ", 3) FROM " + table + ", currentcue, currentitems, " + valueTable + " WHERE " + valueTable + ".item_key = currentcue.key AND " + valueTable + ".foreignitem_key = currentitems.group_key AND " + valueTable + ".valueitem_key = " + table + ".key")) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        valueSpinBox->setValue(0);
        valueSpinBox->setEnabled(false);
        valueSlider->setValue(0);
        valueSlider->setEnabled(false);
        return;
    }
    if (query.next()) {
        key = query.value(0).toInt();
        const float value = query.value(1).toFloat();
        valueSpinBox->setEnabled(true);
        if (value != valueSpinBox->value()) {
            valueSpinBox->setValue(value);
        }
        valueSlider->setEnabled(true);
        if (value != valueSlider->value()) {
            valueSlider->setValue(value);
        }
        valueSlider->setValue(value);
    } else {
        valueSpinBox->setValue(0);
        valueSpinBox->setEnabled(false);
        valueSlider->setValue(0);
        valueSlider->setEnabled(false);
    }
}

void AttributePanel::setValue(const float value) {
    QSqlQuery query;
    query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE key = :key");
    query.bindValue(":key", key);
    query.bindValue(":value", value);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    emit dbChanged();
}
