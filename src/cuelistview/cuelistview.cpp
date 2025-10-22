/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelistview.h"

CuelistView::CuelistView(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout* labelHeader = new QHBoxLayout();
    layout->addLayout(labelHeader);
    cueLabel = new QLabel("No Cue selected.");
    labelHeader->addWidget(cueLabel);
    groupLabel = new QLabel("No Group selected.");
    labelHeader->addWidget(groupLabel);

    QHBoxLayout *buttonHeader = new QHBoxLayout();
    layout->addLayout(buttonHeader);

    cuelistTableView = new QTableView();
    cuelistTableView->verticalHeader()->hide();
    cuelistTableView->horizontalHeader()->setStretchLastSection(true);
    cuelistTableView->setSelectionMode(QAbstractItemView::NoSelection);
    cuelistTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cuelistTableView->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(cuelistTableView);

    model = new QSqlQueryModel();
    model->setQuery("SELECT id, label FROM cues ORDER BY sortkey");
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Label");
    cuelistTableView->setModel(model);

    new QShortcut(Qt::Key_Up, this, [this] { selectItem("groups", "group_key", false); });
    new QShortcut(Qt::Key_Down, this, [this] { selectItem("groups", "group_key", true); });
    new QShortcut(Qt::SHIFT | Qt::Key_Space, this, [this] { selectItem("cues", "cue_key", false); });
    new QShortcut(Qt::Key_Space, this, [this] { selectItem("cues", "cue_key", true); });
}

void CuelistView::reload() {
    model->refresh();
}

void CuelistView::selectItem(const QString table, const QString currentItemsTableKey, const bool next) {
    QSqlQuery currentSortkeyQuery;
    currentSortkeyQuery.prepare("SELECT " + table + ".sortkey FROM currentitems, " + table + " WHERE currentitems." + currentItemsTableKey + " = " + table + ".key");
    if (!currentSortkeyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << currentSortkeyQuery.lastError().text();
        return;
    }
    int sortkey = 1;
    if (next) {
        if (currentSortkeyQuery.next()) {
            sortkey = currentSortkeyQuery.value(0).toInt() + 1;
        }
    } else {
        if (currentSortkeyQuery.next()) {
            sortkey = currentSortkeyQuery.value(0).toInt() - 1;
        }
    }
    QSqlQuery keyQuery;
    keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey = :sortkey");
    keyQuery.bindValue(":sortkey", sortkey);
    if (!keyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << currentSortkeyQuery.lastError().text();
        return;
    }
    if (!keyQuery.next()) {
        return;
    }
    const int key = keyQuery.value(0).toInt();
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE currentitems SET " + currentItemsTableKey + " = :key");
    updateQuery.bindValue(":key", key);
    if (!updateQuery.exec()) {
        qWarning() << Q_FUNC_INFO << currentSortkeyQuery.lastError().text();
        return;
    }
}
