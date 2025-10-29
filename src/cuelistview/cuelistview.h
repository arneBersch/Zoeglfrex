/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUELISTVIEW_H
#define CUELISTVIEW_H

#include <QtWidgets>
#include <QtSql>

#include "cuelisttablemodel.h"

class CuelistView : public QWidget {
    Q_OBJECT
public:
    CuelistView(QWidget *parent = nullptr);
signals:
    void dbChanged();
public slots:
    void reload();
private:
    void selectItem(QString table, QString currentSortkeyQueryText, QString updateQueryText, bool next);
    void deselectItem(QString currentItemsTableKey);
    CuelistTableModel* model;
    QTableView *cuelistTableView;
    QLabel* fixtureLabel;
    QLabel* groupLabel;
    QLabel* cueLabel;
    QLabel* cuelistLabel;
    QComboBox* modeComboBox;
};

#endif // CUELISTVIEW_H
