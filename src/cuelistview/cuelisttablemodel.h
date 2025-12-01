/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CUELISTTABLEMODEL_H
#define CUELISTTABLEMODEL_H

#include <QtWidgets>
#include <QtSql>

class CuelistTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Mode {
        CueMode,
        GroupMode,
    };
    enum RowFilter {
        AllRows,
        ActiveRows,
        ChangedRows,
    };
    CuelistTableModel();
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void refresh();
    void setMode(Mode mode);
    void setRowFilter(RowFilter filter);
    QModelIndex getCurrentRowIndex();
private:
    QStringList getCueValue(QString table, QString valueTable, int cueKey, int groupKey);
    Mode mode = CueMode;
    RowFilter filter = AllRows;
    struct RowData {
        int key;
        QString name;
        QString intensity;
        bool intensityChanged = false;
        QString color;
        bool colorChanged = false;
        QString position;
        bool positionChanged = false;
        QString raws;
        bool rawsChanged = false;
        QString effects;
        bool effectsChanged = false;
    };
    QList<RowData> rows;
    int currentKey = -1;
};

#endif // CUELISTTABLEMODEL_H
