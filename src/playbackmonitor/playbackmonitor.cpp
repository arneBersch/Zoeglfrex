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
    model->setQuery("SELECT CONCAT(id, ' ', label), key FROM cuelists ORDER BY sortkey");
    model->setHeaderData(0, Qt::Horizontal, "Cuelist");
    model->setHeaderData(1, Qt::Horizontal, "Cue");

    tableView = new QTableView();
    tableView->setModel(model);
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setFocusPolicy(Qt::NoFocus);
    tableView->verticalHeader()->hide();
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
        QSqlQuery cueQuery;
        cueQuery.prepare("SELECT CONCAT(id, ' ', label) FROM cues WHERE cuelist_key = :key ORDER BY sortkey");
        cueQuery.bindValue(":key", cuelistKey);
        if (cueQuery.exec()) {
            while (cueQuery.next()) {
                cueComboBox->addItem(cueQuery.value(0).toString());
            }
            QSqlQuery currentSortkeyQuery;
            currentSortkeyQuery.prepare("SELECT cues.sortkey FROM cues, cuelists WHERE cuelists.key = :cuelist AND cuelists.currentcue_key = cues.key");
            currentSortkeyQuery.bindValue(":cuelist", cuelistKey);
            if (currentSortkeyQuery.exec()) {
                if (currentSortkeyQuery.next()) {
                    cueComboBox->setCurrentIndex(currentSortkeyQuery.value(0).toInt() - 1);
                }
            } else {
                qWarning() << Q_FUNC_INFO << currentSortkeyQuery.executedQuery() << currentSortkeyQuery.lastError().text();
            }
            connect(cueComboBox, &QComboBox::currentIndexChanged, this, [this, cuelistKey] (const int newIndex) { updateCue(cuelistKey, newIndex + 1); });
        } else {
            qWarning() << Q_FUNC_INFO << cueQuery.executedQuery() << cueQuery.lastError().text();
        }
    }
}

void PlaybackMonitor::updateCue(const int cuelistKey, const int newCueIndex) {
    QSqlQuery cueKeyQuery;
    cueKeyQuery.prepare("SELECT key FROM cues WHERE cuelist_key = :cuelist AND sortkey = :sortkey");
    cueKeyQuery.bindValue(":cuelist", cuelistKey);
    cueKeyQuery.bindValue(":sortkey", newCueIndex);
    if (!cueKeyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
        return;
    }
    if (!cueKeyQuery.next()) {
        Q_ASSERT(false);
        return;
    }
    const int cueKey = cueKeyQuery.value(0).toInt();
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE cuelists SET currentcue_key = :cue WHERE key = :cuelist");
    updateQuery.bindValue(":cuelist", cuelistKey);
    updateQuery.bindValue(":cue", cueKey);
    if (!updateQuery.exec()) {
        qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
    }
    emit dbChanged();
}
