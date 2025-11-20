/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "attributepanel.h"

AttributePanel::AttributePanel(const QString itemTable, const QString tableAttribute, const QString value, const QString modelTable, const QString fixtureTable, const QString title, const QString attributeUnit, const int minValue, const int maxValue, const bool cyclicValue, QWidget *parent) : QWidget(parent) {
    table = itemTable;
    attribute = tableAttribute;
    valueTable = value;
    modelValueTable = modelTable;
    fixtureValueTable = fixtureTable;
    unit = attributeUnit;

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    valueButton = new QPushButton();
    valueButton->setCheckable(true);
    layout->addWidget(valueButton);

    modelValueButton = new QPushButton();
    modelValueButton->setCheckable(true);
    connect(modelValueButton, &QPushButton::clicked, this, [this] { setException(modelKey, modelValueButton->isChecked(), modelValueTable); });
    layout->addWidget(modelValueButton);

    fixtureValueButton = new QPushButton();
    fixtureValueButton->setCheckable(true);
    connect(fixtureValueButton, &QPushButton::clicked, this, [this] { setException(fixtureKey, fixtureValueButton->isChecked(), fixtureValueTable); });
    layout->addWidget(fixtureValueButton);

    valueDial = new QDial();
    valueDial->setWrapping(cyclicValue);
    valueDial->setRange(minValue, maxValue);
    connect(valueDial, &QDial::valueChanged, this, &AttributePanel::setValue);
    layout->addWidget(valueDial);
}

void AttributePanel::reload(const int item, const int model, const int fixture) {
    itemKey = item;
    modelKey = model;
    fixtureKey = fixture;

    const bool itemGiven = (itemKey >= 0);
    const bool modelGiven = (modelKey >= 0);
    const bool fixtureGiven = (fixtureKey >= 0);

    valueButton->setEnabled(itemGiven);
    valueButton->setChecked(itemGiven);
    valueDial->setEnabled(itemGiven);
    float value = 0;
    if (itemGiven) {
        QSqlQuery query;
        query.prepare("SELECT ROUND(" + table + "." + attribute + ", 3) FROM " + table + " WHERE key = :key");
        query.bindValue(":key", itemKey);
        if (query.exec()) {
            if (query.next()) {
                value = query.value(0).toFloat();
            }
        } else {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
        valueButton->setText(QString::number(value) + unit);
    } else {
        valueButton->setText("");
    }

    auto setExceptionButton = [](const int itemKey, const int exceptionItemKey, QPushButton* button, float* value, const QString table, const QString unit) {
        const bool itemGiven = (itemKey >= 0);
        const bool exceptionItemGiven = (exceptionItemKey >= 0);
        button->setEnabled(itemGiven && exceptionItemGiven);
        bool valueGiven = false;
        if (itemGiven && exceptionItemGiven) {
            QSqlQuery query;
            query.prepare("SELECT ROUND(value, 3) FROM " + table + " WHERE item_key = :key AND foreignitem_key = :exceptionkey");
            query.bindValue(":key", itemKey);
            query.bindValue(":exceptionkey", exceptionItemKey);
            if (query.exec()) {
                valueGiven = query.next();
                if (valueGiven) {
                    (*value) = query.value(0).toFloat();
                }
            } else {
                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            }
        }
        if (itemGiven) {
            button->setText(QString::number(*value) + unit);
        }
        button->setChecked(valueGiven);
    };

    setExceptionButton(itemKey, modelKey, modelValueButton, &value, modelValueTable, unit);
    setExceptionButton(itemKey, fixtureKey, fixtureValueButton, &value, fixtureValueTable, unit);

    if (valueDial->value() != (int) value) {
        valueDial->setValue(value);
    }
}

void AttributePanel::setValue(const int value) {
    Q_ASSERT(itemKey >= 0);
    QSqlQuery query;
    if (fixtureValueButton->isChecked()) {
        Q_ASSERT(fixtureKey >= 0);
        query.prepare("UPDATE " + fixtureValueTable + " SET value = :value WHERE item_key = :key AND foreignitem_key = :fixture");
        query.bindValue(":fixture", fixtureKey);
    } else if (modelValueButton->isChecked()) {
        Q_ASSERT(modelKey >= 0);
        query.prepare("UPDATE " + modelValueTable + " SET value = :value WHERE item_key = :key AND foreignitem_key = :model");
        query.bindValue(":model", modelKey);
    } else {
        query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE key = :key");
    }
    query.bindValue(":key", itemKey);
    query.bindValue(":value", value);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    emit dbChanged();
}

void AttributePanel::setException(const int exceptionItemKey, const bool exception, const QString table) {
    Q_ASSERT(itemKey >= 0);
    Q_ASSERT(exceptionItemKey >= 0);
    QSqlQuery query;
    if (exception) {
        query.prepare("INSERT OR REPLACE INTO " + table + " (item_key, foreignitem_key, value) VALUES (:key, :exceptionitem, 0)");
    } else {
        query.prepare("DELETE FROM " + table + " WHERE item_key = :key AND foreignitem_key = :exceptionitem");
    }
    query.bindValue(":key", itemKey);
    query.bindValue(":exceptionitem", exceptionItemKey);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    emit dbChanged();
}
