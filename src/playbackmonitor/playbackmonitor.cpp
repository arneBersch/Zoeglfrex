/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "playbackmonitor.h"

PlaybackMonitor::PlaybackMonitor(QWidget *parent) : QWidget(parent, Qt::Window) {
    setWindowTitle("Zöglfrex Playback Monitor");
    resize(500, 300);

    model = new QSqlQueryModel();
    model->setQuery("SELECT CONCAT(id, ' ', label), key, key, key FROM cuelists ORDER BY sortkey");
    model->setHeaderData(0, Qt::Horizontal, "Cuelist");
    model->setHeaderData(1, Qt::Horizontal, "Cue");
    model->setHeaderData(2, Qt::Horizontal, "");
    model->setHeaderData(3, Qt::Horizontal, "");

    tableView = new QTableView();
    tableView->setModel(model);
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setFocusPolicy(Qt::NoFocus);
    tableView->verticalHeader()->hide();
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(tableView);
    setLayout(layout);
}

void PlaybackMonitor::reload() {
    model->refresh();
    for (int row = 0; row < model->rowCount(); row++) {
        const int cuelistKey = model->data(model->index(row, 1), Qt::DisplayRole).toInt();
        QComboBox* cueComboBox = new QComboBox();
        tableView->setIndexWidget(model->index(row, 1), cueComboBox);
        QPushButton* goButton = new QPushButton("GO");
        tableView->setIndexWidget(model->index(row, 2), goButton);
        QPushButton* goBackButton = new QPushButton("GO BACK");
        tableView->setIndexWidget(model->index(row, 3), goBackButton);
        QSqlQuery currentCueQuery;
        currentCueQuery.prepare("SELECT currentcue_key FROM cuelists WHERE key = :cuelist");
        currentCueQuery.bindValue(":cuelist", cuelistKey);
        if (currentCueQuery.exec()) {
            int currentCueKey = -1;
            if (currentCueQuery.next()) {
                currentCueKey = currentCueQuery.value(0).toInt();
            }
            QSqlQuery cueQuery;
            cueQuery.prepare("SELECT key, CONCAT(id, ' ', label) FROM cues WHERE cuelist_key = :cuelist ORDER BY sortkey");
            cueQuery.bindValue(":cuelist", cuelistKey);
            if (cueQuery.exec()) {
                QList<int> cueKeys;
                int index = -1;
                while (cueQuery.next()) {
                    const int cueKey = cueQuery.value(0).toInt();
                    if (cueKey == currentCueKey) {
                        index = cueQuery.at();
                    }
                    cueKeys.append(cueKey);
                    cueComboBox->addItem(cueQuery.value(1).toString());
                }
                if (index >= 0) {
                    cueComboBox->setCurrentIndex(index);
                    if (cueKeys.length() >= index + 1) {
                        connect(goButton, &QPushButton::clicked, this, [this, cuelistKey, cueKeys, index] { setCue(cuelistKey, cueKeys.at(index + 1)); });
                    }
                    if (index > 0) {
                        connect(goBackButton, &QPushButton::clicked, this, [this, cuelistKey, cueKeys, index] { setCue(cuelistKey, cueKeys.at(index - 1)); });
                    }
                } else if (!cueKeys.isEmpty()) {
                    setCue(cuelistKey, cueKeys.first());
                }
                connect(cueComboBox, &QComboBox::currentIndexChanged, this, [this, cuelistKey, cueKeys] (const int newIndex) { setCue(cuelistKey, cueKeys.at(newIndex)); });
            } else {
                qWarning() << Q_FUNC_INFO << cueQuery.executedQuery() << cueQuery.lastError().text();
            }
        } else {
            qWarning() << Q_FUNC_INFO << currentCueQuery.executedQuery() << currentCueQuery.lastError().text();
        }
    }
}

void PlaybackMonitor::setCue(const int cuelistKey, const int cueKey) {
    QSqlQuery query;
    query.prepare("UPDATE cuelists SET currentcue_key = :cue WHERE key = :cuelist");
    query.bindValue(":cuelist", cuelistKey);
    query.bindValue(":cue", cueKey);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    emit dbChanged();
}
