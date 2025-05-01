/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SACNSERVER_H
#define SACNSERVER_H

#include <QtWidgets>
#include <QtNetwork>
class Kernel;

class SacnServer : public QWidget {
    Q_OBJECT
public:
    SacnServer(Kernel* core, QWidget *parent = nullptr);
    void send(QMap<int, QByteArray> dmxUniverses);
    QSpinBox *prioritySpinBox;
    const int SACN_STANDARD_PRIORITY = 100;
private:
    void setNetworkInterface();
    QByteArray header;
    uchar sequence = 0;
    QUdpSocket *socket = nullptr;
    QComboBox *interfaceComboBox;
    QList<QNetworkInterface> networkInterfaces = QList<QNetworkInterface>();
    QList<QNetworkAddressEntry> networkAddresses = QList<QNetworkAddressEntry>();
    Kernel *kernel;
    const int SACN_PORT = 5568;
    const int SACN_MAX_UNIVERSE = 63999;
};

#endif // SACNSERVER_H
