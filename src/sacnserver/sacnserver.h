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

public slots:
    void sendUniverses(QHash<int, QByteArray> universeData);

private slots:
    void loadSocket(int socketIndex);

private:
    const int MIN_PRIORITY = 0;
    const int MAX_PRIORITY = 200;
    const int DEFAULT_PRIORITY = 100;
    const int MIN_UNIVERSE = 1;
    const int MAX_UNIVERSE = 63999;
    const int PORT = 5568;
    const QString DATA_ADDRESS_FORMAT =  "239.255.%1.%2";
    const QHostAddress DISCOVERY_ADDRESS = QHostAddress("239.255.250.214");
    const QByteArray CID = QUuid::createUuid().toRfc4122();
    const int DISCOVERY_INTERVAL = 10 * 1000; // send the Discovery Universe List every 10 seconds

    void sendUniverseList();
    void reloadNetworkInterfaces();
    void updateFlagsAndLength(QByteArray* data, int index);
    void addRootLayerData(QByteArray* data, char vectorSuffix);
    QSettings* settings;
    QUdpSocket* socket = nullptr;
    QComboBox* networkInterfaceComboBox;
    QList<QNetworkInterface> networkInterfaces = QList<QNetworkInterface>();
    QList<QNetworkAddressEntry> networkAddresses = QList<QNetworkAddressEntry>();
    uint8_t sequence = 0;
    QList<int> universes;
};

#endif // SACNSERVER_H
