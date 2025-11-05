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
    model->setQuery("SELECT CONCAT(id, ' ', label) FROM cuelists");

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
}
