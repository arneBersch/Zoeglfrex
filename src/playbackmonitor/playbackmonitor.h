/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PLAYBACKMONITOR_H
#define PLAYBACKMONITOR_H

#include <QtWidgets>
#include <QtSql>

class PlaybackMonitor : public QWidget {
    Q_OBJECT
public:
    PlaybackMonitor(QWidget *parent = nullptr);
public slots:
    void reload();
signals:
    void dbChanged();
private:
    void updateCue(const int cuelistKey, const int newCueIndex);
    QSqlQueryModel* model;
    QTableView* tableView;
};

#endif // PLAYBACKMONITOR_H
