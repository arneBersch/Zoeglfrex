/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "sacnserver.h"

SacnServer::SacnServer() {
    // Root Layer
    // Preamble Size (Octet 0-1)
    data.append((char)0x00);
    data.append((char)0x10);

    //Post-amble Size (Octet 2-3)
    data.append((char)0x00);
    data.append((char)0x00);

    // ACN Packet Identifier (Octet 4-15)
    data.append((char)0x41);
    data.append((char)0x53);
    data.append((char)0x43);
    data.append((char)0x2d);
    data.append((char)0x45);
    data.append((char)0x31);
    data.append((char)0x2e);
    data.append((char)0x31);
    data.append((char)0x37);
    data.append((char)0x00);
    data.append((char)0x00);
    data.append((char)0x00);

    // Flags and Length (Octet 16-17)
    data.append((char)0x72);
    data.append((char)0x6e);

    // Vector (Octet 18-21)
    data.append((char)0x00);
    data.append((char)0x00);
    data.append((char)0x00);
    data.append((char)0x04);

    // CID (Octet 22-37)
    QByteArray cid = QUuid::createUuid().toRfc4122();
    data.append(cid);

    // Framing Layer
    // Flags and Length (Octet 38-39)
    data.append((char)0x72);
    data.append((char)0x58);

    // Vector (Octet 40-43)
    data.append((char)0x00);
    data.append((char)0x00);
    data.append((char)0x00);
    data.append((char)0x02);

    // Source Name (Octet 44-107)
    QByteArray source = QString("ZÖGLFREX").toUtf8();
    data.append(source);
    for (int i=0; i < 64-source.length(); i++) {
        data.append((char)0x00);
    }

    // Priority (Octet 108)
    data.append((char)100); // 0-200, Default: 100

    // Synchronization Address (Octet 109-110)
    data.append((char)0x00);
    data.append((char)0x00);

    // Sequence Number (Octet 111)
    data.append((char)0x00);

    // Options (Octet 112)
    data.append((char)0x00); // Select no options

    // Universe (Octet 113-114)
    data.append((char)0x00);
    data.append((char)0x01);

    // DMP Layer
    // Flags and Length (Octet 115-116)
    data.append((char)0x72);
    data.append((char)0x0b);

    // Vector (Octet 117)
    data.append((char)0x02);

    // Address Type & Data Type (Octet 118)
    data.append((char)0xa1);

    // First Property Address (Octet 119-120)
    data.append((char)0x00);
    data.append((char)0x00);

    // Address Increment (Octet 121-122)
    data.append((char)0x00);
    data.append((char)0x01);

    // Property Value Count (Octet 123-124)
    data.append((char)0x02);
    data.append((char)0x01);

    // Start Code (Octet 125)
    data.append((char)0x00);

    // Property Values (Octet 126-637)
    for (int channel=0; channel<512; channel++) {
        data.append((char)0x00);
    }
}

void SacnServer::setChannel(int channel, uint8_t value) {
    if (channel < 1 || channel > 512) {
        return;
    }
    data[125 + channel] = (char) value;
}

uint8_t SacnServer::getChannel(int channel) {
    if (channel < 1 || channel > 512) {
        return 0;
    }
    return data[125 + channel];
}

void SacnServer::send() {
    if (sequence == 0xff) {
        sequence = 1;
    } else {
        sequence++;
    }
    data[111] = sequence; // Update Sequence number
    qint64 result = socket->writeDatagram(data.data(), data.size(), QHostAddress("239.255.0.1"), 5568);
    if (result < 0) {
        qWarning() << Q_FUNC_INFO <<"ERROR sending sACN: " << socket->error() << " (" << socket->errorString() << ")";
    }
}

void SacnServer::connect(QString newAddress) {
    if (socket != nullptr) {
        qWarning() << "Tried to connect although socket is still connected";
        return; // socket needs to be disconnected
    }
    for (QNetworkInterface interface : QNetworkInterface::allInterfaces()) {
        for (QNetworkAddressEntry entry : interface.addressEntries()) {
            if (entry.ip().toString() == newAddress) {
                if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) {
                    qWarning() << Q_FUNC_INFO << "Tried to connect socket to invalid IP Address (not IPv4)";
                    return;
                }
                socket = new QUdpSocket(); // Create Socket
                socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false); // Disable Multicast Loopback
                socket->bind(entry.ip());
                socket->setMulticastInterface(interface); // Select the interface
                qDebug() << Q_FUNC_INFO << "Connected socket to" << entry.ip().toString();
                return;
            }
        }
    }
}

void SacnServer::disconnect() {
    delete socket;
    socket = nullptr;
    qDebug() << Q_FUNC_INFO << "Disconnected socket";
}
