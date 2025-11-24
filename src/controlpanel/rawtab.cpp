/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "rawtab.h"

RawTab::RawTab(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    rawComboBox = new QComboBox();
    layout->addWidget(rawComboBox);

    table = new QTableView();
    model = new FixtureChannelModel();
    table->setModel(model);
    table->verticalHeader()->hide();
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(table);
}

void RawTab::reload() {
    int currentRawKey = -1;
    if (rawComboBox->currentIndex() >= 0) {
        currentRawKey = rawKeys.at(rawComboBox->currentIndex());
    }
    rawKeys.clear();
    rawComboBox->clear();

    int currentRawIndex = -1;
    QSqlQuery rawsQuery;
    if (rawsQuery.exec("SELECT key, CONCAT(id, ' ', label) FROM raws")) {
        while (rawsQuery.next()) {
            const int rawKey = rawsQuery.value(0).toInt();
            const QString rawName = rawsQuery.value(1).toString();
            rawKeys.append(rawKey);
            rawComboBox->addItem(rawName);
            if (rawKey == currentRawKey) {
                currentRawIndex = rawsQuery.at();
            }
        }
    } else {
        qWarning() << Q_FUNC_INFO << rawsQuery.executedQuery() << rawsQuery.lastError().text();
    }

    if (currentRawIndex >= 0) {
        rawComboBox->setCurrentIndex(currentRawIndex);
    }

    model->refresh();
}
