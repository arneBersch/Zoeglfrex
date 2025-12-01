/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef FIXTURECHANNELMODEL_H
#define FIXTURECHANNELMODEL_H

#include <QtWidgets>
#include <QtSql>

class FixtureChannelModel : public QAbstractTableModel {
    Q_OBJECT
public:
    FixtureChannelModel();
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void setRawKey(int key);
    void setChannelDifference(int channel, int difference);
    void updateButtons(int channel, QPushButton* valueButton, QPushButton* modelButton, QPushButton* fixtureButton);
signals:
    void dbChanged();
private:
    struct ChannelData {
        QString title = QString();
        bool valueGiven = false;
        uint8_t value = 0;
        bool modelValueGiven = false;
        uint8_t modelValue = 0;
        bool fixtureValueGiven = false;
        uint8_t fixtureValue = 0;
    };
    QList<ChannelData> channels;
    int rawKey = -1;
    int modelKey = -1;
    int fixtureKey = -1;
};

#endif // FIXTURECHANNELMODEL_H
