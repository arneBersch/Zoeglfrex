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

    modeComboBox = new QComboBox();
    modeComboBox->addItem("Group Mode");
    modeComboBox->addItem("Cue Mode");
    connect(modeComboBox, &QComboBox::currentIndexChanged, this, [this] (const int index) {
        model->setCueMode(index == 1);
        settings->setValue("cuelistview/mode", index);
    });
    modeComboBox->setCurrentIndex(settings->value("cuelistview/mode", 0).toInt());
    buttonHeader->addWidget(modeComboBox);

    trackingButton = new QPushButton("Tracking");
    trackingButton->setCheckable(true);
    connect(trackingButton, &QPushButton::clicked, this, [this] {
        bool checked = trackingButton->isChecked();
        emit trackingChanged(checked);
        settings->setValue("cuelistview/tracking", checked);
    });
    trackingButton->setChecked(settings->value("cuelistview/tracking", true).toBool());
    buttonHeader->addWidget(trackingButton);

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
    new QShortcut(Qt::Key_Space, this, [this] { selectItem("currentcuelist_cues", "SELECT cues.sortkey FROM cuelists, cues, currentitems WHERE currentitems.cuelist_key = cuelists.key AND cues.key = cuelists.currentcue_key", "UPDATE cuelists SET currentcue_key = :key WHERE key = (SELECT cuelist_key FROM currentitems)", true); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_Space, this, [this] { selectItem("currentcuelist_cues", "SELECT cues.sortkey FROM cuelists, cues, currentitems WHERE currentitems.cuelist_key = cuelists.key AND cues.key = cuelists.currentcue_key", "UPDATE cuelists SET currentcue_key = :key WHERE key = (SELECT cuelist_key FROM currentitems)", false); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::SHIFT | Qt::Key_M, this, [this] {
        if (modeComboBox->currentIndex() == 0) {
            modeComboBox->setCurrentIndex(1);
        } else {
            modeComboBox->setCurrentIndex(0);
        }
    }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_T, this, [this] { trackingButton->click(); }, Qt::ApplicationShortcut);
}

void CuelistView::reload() {
    emit trackingChanged(trackingButton->isChecked());
    auto setCurrentItemLabel = [](const QString keyQuery, const QString itemName, QLabel* label) {
        QSqlQuery query;
        if (query.exec(keyQuery)) {
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
    setCurrentItemLabel("SELECT CONCAT('Cue ', cues.id, ' ', cues.label) FROM cues, cuelists, currentitems WHERE cues.key = cuelists.currentcue_key AND cuelists.key = currentitems.cuelist_key", "Cue", cueLabel);
    model->refresh();
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
        const int currentSortkey = currentSortkeyQuery.value(0).toInt();
        if (next) {
            keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey > :sortkey ORDER BY sortkey ASC LIMIT 1");
        } else {
            keyQuery.prepare("SELECT key FROM " + table + " WHERE sortkey < :sortkey ORDER BY sortkey DESC LIMIT 1");
        }
        keyQuery.bindValue(":sortkey", currentSortkey);
    } else {
        keyQuery.prepare("SELECT key FROM " + table + " ORDER BY sortkey LIMIT 1");
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
