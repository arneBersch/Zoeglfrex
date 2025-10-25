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

    QGridLayout* labelHeader = new QGridLayout();
    layout->addLayout(labelHeader);
    fixtureLabel = new QLabel();
    labelHeader->addWidget(fixtureLabel, 0, 0);
    groupLabel = new QLabel();
    labelHeader->addWidget(groupLabel, 0, 1);
    cuelistLabel = new QLabel();
    labelHeader->addWidget(cuelistLabel, 1, 0);
    cueLabel = new QLabel();
    labelHeader->addWidget(cueLabel, 1, 1);

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

    new QShortcut(Qt::Key_Left, this, [this] { selectItem("fixtures", "fixture_key", false); });
    new QShortcut(Qt::Key_Right, this, [this] { selectItem("fixtures", "fixture_key", true); });
    new QShortcut(Qt::Key_Up, this, [this] { selectItem("groups", "group_key", false); });
    new QShortcut(Qt::Key_Down, this, [this] { selectItem("groups", "group_key", true); });

    reload();
}

void CuelistView::reload() {
    auto setCurrentItemLabel = [](const QString table, const QString currentItemsTableKey, const QString itemName, QLabel* label) {
        QSqlQuery query;
        if (query.exec("SELECT CONCAT('" + itemName + " ', " + table + ".id, ': ', " + table + ".label) FROM " + table + ", currentitems WHERE " + table + ".key = currentitems." + currentItemsTableKey)) {
            if (query.next()) {
                label->setText(query.value(0).toString());
            } else {
                label->setText("No " + itemName + " selected.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << query.lastError().text();
            label->setText(QString());
        }
    };
    setCurrentItemLabel("fixtures", "fixture_key", "Fixture", fixtureLabel);
    setCurrentItemLabel("groups", "group_key", "Group", groupLabel);
    setCurrentItemLabel("cuelists", "cuelist_key", "Cuelist", cuelistLabel);
    model->refresh();
}

void CuelistView::selectItem(const QString table, const QString currentItemsTableKey, const bool next) {
    QSqlQuery currentSortkeyQuery;
    currentSortkeyQuery.prepare("SELECT " + table + ".sortkey FROM currentitems, " + table + " WHERE currentitems." + currentItemsTableKey + " = " + table + ".key");
    if (!currentSortkeyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << currentSortkeyQuery.lastError().text();
        return;
    }
    QSqlQuery keyQuery;
    if (currentSortkeyQuery.next()) {
        const int currentSortkey = currentSortkeyQuery.value(0).toInt();
        if (next) {
            keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey > :sortkey ORDER BY sortkey ASC LIMIT 1");
        } else {
            keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey < :sortkey ORDER BY sortkey DESC LIMIT 1");
        }
        keyQuery.bindValue(":sortkey", currentSortkey);
    } else {
        keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey = 1");
    }
    if (!keyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << keyQuery.lastError().text();
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
        qWarning() << Q_FUNC_INFO << updateQuery.lastError().text();
        return;
    }
    emit dbChanged();
}
