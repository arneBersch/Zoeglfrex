/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "cuelistview.h"

CuelistView::CuelistView(QWidget *parent) : QWidget(parent) {
    settings = new QSettings("Zoeglfrex");

    model = new CuelistTableModel();

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

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

    modeComboBox = new QComboBox();
    modeComboBox->addItem("Cue Mode");
    modeComboBox->addItem("Group Mode");
    connect(modeComboBox, &QComboBox::currentIndexChanged, this, [this] (const int index) {
        if (index == 0) {
            model->setMode(CuelistTableModel::CueMode);
        } else if (index == 1) {
            model->setMode(CuelistTableModel::GroupMode);
        }
        settings->setValue("cuelistview/mode", index);
    });
    modeComboBox->setCurrentIndex(settings->value("cuelistview/mode", 0).toInt());
    buttonHeader->addWidget(modeComboBox);

    filterComboBox = new QComboBox();
    filterComboBox->addItem("All Rows");
    filterComboBox->addItem("Active Rows only");
    filterComboBox->addItem("Changed Rows only");
    connect(filterComboBox, &QComboBox::currentIndexChanged, this, [this] (const int index) {
        if (index == 0) {
            model->setRowFilter(CuelistTableModel::AllRows);
        } else if (index == 1) {
            model->setRowFilter(CuelistTableModel::ActiveRows);
        } else if (index == 2) {
            model->setRowFilter(CuelistTableModel::ChangedRows);
        }
        settings->setValue("cuelistview/filter", index);
    });
    filterComboBox->setCurrentIndex(settings->value("cuelistview/filter", 0).toInt());
    buttonHeader->addWidget(filterComboBox);

    cuelistTableView = new QTableView();
    cuelistTableView->setModel(model);
    cuelistTableView->verticalHeader()->hide();
    cuelistTableView->horizontalHeader()->setStretchLastSection(true);
    cuelistTableView->setSelectionMode(QAbstractItemView::NoSelection);
    cuelistTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cuelistTableView->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(cuelistTableView);

    new QShortcut(Qt::Key_Left, this, [this] { selectItem("currentgroup_fixtures", "SELECT currentgroup_fixtures.sortkey FROM currentitems, currentgroup_fixtures WHERE currentitems.fixture_key = currentgroup_fixtures.key", "UPDATE currentitems SET fixture_key = :key", false); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Right, this, [this] { selectItem("currentgroup_fixtures", "SELECT currentgroup_fixtures.sortkey FROM currentitems, currentgroup_fixtures WHERE currentitems.fixture_key = currentgroup_fixtures.key", "UPDATE currentitems SET fixture_key = :key", true); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Escape, this, [this] { deselectItem("fixture_key"); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Up, this, [this] { selectItem("groups", "SELECT groups.sortkey FROM currentitems, groups WHERE currentitems.group_key = groups.key", "UPDATE currentitems SET group_key = :key", false); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Down, this, [this] { selectItem("groups", "SELECT groups.sortkey FROM currentitems, groups WHERE currentitems.group_key = groups.key", "UPDATE currentitems SET group_key = :key", true); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_Space, this, [this] {
        QSqlQuery currentCueQuery;
        if (currentCueQuery.exec("SELECT cue_key FROM currentitems WHERE cue_key IS NOT NULL")) {
            if (currentCueQuery.next()) {
                selectItem("currentcuelist_cues", "SELECT sortkey FROM currentcue", "UPDATE currentitems SET cue_key = :key", false);
            } else {
                selectItem("currentcuelist_cues", "SELECT sortkey FROM currentcue", "UPDATE cuelists SET currentcue_key = :key WHERE key = (SELECT cuelist_key FROM currentitems)", false);
            }
        } else {
            qWarning() << Q_FUNC_INFO << currentCueQuery.executedQuery() << currentCueQuery.lastError().text();
        }
    }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Space, this, [this] {
        QSqlQuery currentCueQuery;
        if (currentCueQuery.exec("SELECT cue_key FROM currentitems WHERE cue_key IS NOT NULL")) {
            if (currentCueQuery.next()) {
                selectItem("currentcuelist_cues", "SELECT sortkey FROM currentcue", "UPDATE currentitems SET cue_key = :key", true);
            } else {
                selectItem("currentcuelist_cues", "SELECT sortkey FROM currentcue", "UPDATE cuelists SET currentcue_key = :key WHERE key = (SELECT cuelist_key FROM currentitems)", true);
            }
        } else {
            qWarning() << Q_FUNC_INFO << currentCueQuery.executedQuery() << currentCueQuery.lastError().text();
        }
    }, Qt::ApplicationShortcut);

    new QShortcut(Qt::SHIFT | Qt::Key_M, this, [this] {
        const int index = modeComboBox->currentIndex();
        if (index == 0) {
            modeComboBox->setCurrentIndex(1);
        } else {
            modeComboBox->setCurrentIndex(0);
        }
    }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_R, this, [this] {
        const int index = filterComboBox->currentIndex();
        if (index == 0) {
            filterComboBox->setCurrentIndex(1);
        } else if (index == 1) {
            filterComboBox->setCurrentIndex(2);
        } else {
            filterComboBox->setCurrentIndex(0);
        }
    }, Qt::ApplicationShortcut);
}

void CuelistView::reload() {
    auto setCurrentItemLabel = [](const QString queryText, const QString itemName, QLabel* label) {
        QSqlQuery query;
        if (query.exec(queryText)) {
            if (query.next()) {
                label->setText(query.value(0).toString());
            } else {
                label->setText("No " + itemName + " selected.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            label->setText(QString());
        }
    };
    setCurrentItemLabel("SELECT CONCAT('Fixture ', fixtures.id, ' ', fixtures.label) FROM fixtures, currentitems WHERE fixtures.key = currentitems.fixture_key", "Fixture", fixtureLabel);
    setCurrentItemLabel("SELECT CONCAT('Group ', groups.id, ' ', groups.label) FROM groups, currentitems WHERE groups.key = currentitems.group_key", "Group", groupLabel);
    setCurrentItemLabel("SELECT CONCAT('Cuelist ', cuelists.id, ' ', cuelists.label) FROM cuelists, currentitems WHERE cuelists.key = currentitems.cuelist_key", "Cuelist", cuelistLabel);
    setCurrentItemLabel("SELECT CONCAT('Cue ', id, ' ', label) FROM currentcue", "Cue", cueLabel);
    model->refresh();
    cuelistTableView->scrollTo(model->getCurrentRowIndex());
}

void CuelistView::selectItem(const QString table, const QString currentSortkeyQueryText, const QString updateQueryText, const bool next) {
    QSqlQuery currentSortkeyQuery;
    currentSortkeyQuery.prepare(currentSortkeyQueryText);
    if (!currentSortkeyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << currentSortkeyQuery.executedQuery() << currentSortkeyQuery.lastError().text();
        return;
    }
    QSqlQuery keyQuery;
    if (currentSortkeyQuery.next()) {
        if (next) {
            keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey = (SELECT MIN(sortkey) FROM " + table + " WHERE sortkey > :sortkey)");
        } else {
            keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey = (SELECT MAX(sortkey) FROM " + table + " WHERE sortkey < :sortkey)");
        }
        keyQuery.bindValue(":sortkey", currentSortkeyQuery.value(0).toInt());
    } else {
        keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey = (SELECT MIN(sortkey) FROM " + table + ")");
    }
    if (!keyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
        return;
    }
    if (!keyQuery.next()) {
        return;
    }
    const int key = keyQuery.value(0).toInt();
    QSqlQuery updateQuery;
    updateQuery.prepare(updateQueryText);
    updateQuery.bindValue(":key", key);
    if (!updateQuery.exec()) {
        qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
        return;
    }
    emit dbChanged();
}

void CuelistView::deselectItem(const QString currentItemsTableKey) {
    QSqlQuery query;
    if (!query.exec("UPDATE currentitems SET " + currentItemsTableKey + " = NULL")) {
        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
    }
    emit dbChanged();
}
