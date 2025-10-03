/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SACNSERVER_H
#define SACNSERVER_H

#include<QtWidgets>
#include<QtNetwork>

class SacnServer : public QWidget {
    Q_OBJECT
public:
    SacnServer(QWidget* parent = nullptr);
private:
    void reloadNetworkInterfaces();
    void sendUniverse(int universe, QByteArray channels);
    QSettings* settings;
    QByteArray packetHeader;
    QUdpSocket *socket = nullptr;
    QComboBox* networkInterfaceComboBox;
    QList<QNetworkInterface> networkInterfaces = QList<QNetworkInterface>();
    QList<QNetworkAddressEntry> networkAddresses = QList<QNetworkAddressEntry>();
    uchar sequence = 0;
};

#endif // SACNSERVER_H
