/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "attributepanel.h"

AttributePanel::AttributePanel(const QString itemTable, const QString tableAttribute, const QString value, const QString modelTable, const QString fixtureTable, const QString title, const QString attributeUnit, const int min, const int max, const bool cyclicValue, QWidget *parent) : QWidget(parent) {
    table = itemTable;
    attribute = tableAttribute;
    valueTable = value;
    modelValueTable = modelTable;
    fixtureValueTable = fixtureTable;
    unit = attributeUnit;
    minValue = min;
    maxValue = max;
    cyclic = cyclicValue;

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
    connect(modelValueButton, &QPushButton::clicked, this, [this] {
        Q_ASSERT(key >= 0);
        Q_ASSERT(modelKey >= 0);
        QSqlQuery query;
        if (modelValueButton->isChecked()) {
            query.prepare("INSERT OR REPLACE INTO " + modelValueTable + " (item_key, foreignitem_key, value) VALUES (:key, :model, 0)");
        } else {
            query.prepare("DELETE FROM " + modelValueTable + " WHERE item_key = :key AND foreignitem_key = :model");
        }
        query.bindValue(":key", key);
        query.bindValue(":model", modelKey);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
        emit dbChanged();
    });
    layout->addWidget(modelValueButton);

    fixtureValueButton = new QPushButton();
    fixtureValueButton->setCheckable(true);
    connect(fixtureValueButton, &QPushButton::clicked, this, [this] {
        Q_ASSERT(key >= 0);
        Q_ASSERT(fixtureKey >= 0);
        QSqlQuery query;
        if (fixtureValueButton->isChecked()) {
            query.prepare("INSERT OR REPLACE INTO " + fixtureValueTable + " (item_key, foreignitem_key, value) VALUES (:key, :fixture, 0)");
        } else {
            query.prepare("DELETE FROM " + fixtureValueTable + " WHERE item_key = :key AND foreignitem_key = :fixture");
        }
        query.bindValue(":key", key);
        query.bindValue(":fixture", fixtureKey);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
        emit dbChanged();
    });
    layout->addWidget(fixtureValueButton);

    valueDial = new QDial();
    valueDial->setWrapping(cyclic);
    valueDial->setRange(min, max);
    connect(valueDial, &QDial::valueChanged, this, &AttributePanel::setValue);
    layout->addWidget(valueDial);

    reload();
}

void AttributePanel::reload() {
    valueButton->setEnabled(false);
    modelValueButton->setEnabled(false);
    fixtureValueButton->setEnabled(false);
    key = -1;
    modelKey = -1;
    fixtureKey = -1;
    QSqlQuery query;
    if (!query.exec("SELECT " + table + ".key, ROUND(" + table + "." + attribute + ", 3) FROM " + table + ", currentcue, currentitems, " + valueTable + " WHERE " + valueTable + ".item_key = currentcue.key AND " + valueTable + ".foreignitem_key = currentitems.group_key AND " + valueTable + ".valueitem_key = " + table + ".key")) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        valueButton->setText("");
        valueButton->setChecked(false);
        modelValueButton->setText("");
        modelValueButton->setChecked(false);
        fixtureValueButton->setText("");
        fixtureValueButton->setChecked(false);
        valueDial->setValue(0);
        valueDial->setEnabled(false);
        return;
    }
    if (query.next()) {
        key = query.value(0).toInt();
        float value = query.value(1).toFloat();
        valueButton->setEnabled(true);
        valueButton->setChecked(true);
        valueButton->setText(QString::number(value) + unit);
        valueDial->setEnabled(true);
        QSqlQuery modelKeyQuery;
        if (modelKeyQuery.exec("SELECT fixtures.model_key FROM fixtures, currentitems WHERE currentitems.fixture_key = fixtures.key AND fixtures.model_key IS NOT NULL")) {
            if (modelKeyQuery.next()) {
                modelKey = modelKeyQuery.value(0).toInt();
                modelValueButton->setEnabled(true);
                modelValueButton->setText("No value");
                QSqlQuery modelValueQuery;
                modelValueQuery.prepare("SELECT ROUND(value, 3) FROM " + modelValueTable + " WHERE item_key = :key AND foreignitem_key = :model");
                modelValueQuery.bindValue(":key", key);
                modelValueQuery.bindValue(":model", modelKey);
                if (modelValueQuery.exec()) {
                    if (modelValueQuery.next()) {
                        modelValueButton->setChecked(true);
                        value = modelValueQuery.value(0).toFloat();
                        modelValueButton->setText(QString::number(value) + unit);
                    } else {
                        modelValueButton->setChecked(false);
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << modelValueQuery.executedQuery() << modelValueQuery.lastError().text();
                }
            } else {
                modelValueButton->setText("No Model");
            }
        } else {
            qWarning() << Q_FUNC_INFO << modelKeyQuery.executedQuery() << modelKeyQuery.lastError().text();
            modelValueButton->setText("");
        }
        QSqlQuery fixtureKeyQuery;
        if (fixtureKeyQuery.exec("SELECT fixture_key FROM currentitems WHERE fixture_key IS NOT NULL")) {
            if (fixtureKeyQuery.next()) {
                fixtureKey = fixtureKeyQuery.value(0).toInt();
                fixtureValueButton->setEnabled(true);
                fixtureValueButton->setText("No value");
                QSqlQuery fixtureValueQuery;
                fixtureValueQuery.prepare("SELECT ROUND(value, 3) FROM " + fixtureValueTable + " WHERE item_key = :key AND foreignitem_key = :fixture");
                fixtureValueQuery.bindValue(":key", key);
                fixtureValueQuery.bindValue(":fixture", fixtureKey);
                if (fixtureValueQuery.exec()) {
                    if (fixtureValueQuery.next()) {
                        fixtureValueButton->setChecked(true);
                        value = fixtureValueQuery.value(0).toFloat();
                        fixtureValueButton->setText(QString::number(value) + unit);
                    } else {
                        fixtureValueButton->setChecked(false);
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << fixtureValueQuery.executedQuery() << fixtureValueQuery.lastError().text();
                }
            } else {
                fixtureValueButton->setText("No Fixture");
            }
        } else {
            qWarning() << Q_FUNC_INFO << fixtureKeyQuery.executedQuery() << fixtureKeyQuery.lastError().text();
            fixtureValueButton->setText("");
        }
        if (valueDial->value() != (int) value) {
            valueDial->setValue(value);
        }
    } else {
        valueButton->setText("");
        modelValueButton->setText("");
        fixtureValueButton->setText("");
        valueDial->setValue(0);
        valueDial->setEnabled(false);
    }
}

void AttributePanel::setValue(const int value) {
    QSqlQuery query;
    if ((fixtureKey >= 0) && fixtureValueButton->isChecked()) {
        query.prepare("UPDATE " + fixtureValueTable + " SET value = :value WHERE item_key = :key AND foreignitem_key = :fixture");
        query.bindValue(":fixture", fixtureKey);
    } else if ((modelKey >= 0) && modelValueButton->isChecked()) {
        query.prepare("UPDATE " + modelValueTable + " SET value = :value WHERE item_key = :key AND foreignitem_key = :model");
        query.bindValue(":model", modelKey);
    } else {
        query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE key = :key");
    }
    query.bindValue(":key", key);
    query.bindValue(":value", value);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    emit dbChanged();
}
