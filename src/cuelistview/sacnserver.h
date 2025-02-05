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

class SacnServer {
public:
    SacnServer();
    void setChannel(int channel, uint8_t value);
    void send();
    void connect(QNetworkInterface networkInterface, QNetworkAddressEntry networkAddress);
    void disconnect();
private:
    QByteArray data;
    uchar sequence = 1;
    QUdpSocket *socket = nullptr;
};


#endif // SACNSERVER_H
