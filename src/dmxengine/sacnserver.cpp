/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "sacnserver.h"
#include "kernel/kernel.h"

SacnServer::SacnServer(Kernel* core, QWidget* parent) : QWidget(parent, Qt::Window) {
    kernel = core;

    setWindowTitle("DMX Output Settings");
    QGridLayout *layout = new QGridLayout(this);

    QLabel* interfaceLabel = new QLabel("Network Interface");
    layout->addWidget(interfaceLabel, 0, 0);

    interfaceComboBox = new QComboBox();
    interfaceComboBox->addItem("None");
    for (QNetworkInterface interface : QNetworkInterface::allInterfaces()) {
        for (QNetworkAddressEntry address : interface.addressEntries()) {
            if (address.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                interfaceComboBox->addItem(interface.name() + " (" + address.ip().toString() + ")");
                networkInterfaces.append(interface);
                networkAddresses.append(address);
            }
        }
    }
    connect(interfaceComboBox, &QComboBox::currentIndexChanged, this, &SacnServer::setNetworkInterface);
    layout->addWidget(interfaceComboBox, 0, 1);

    QLabel* priorityLabel = new QLabel("Priority");
    layout->addWidget(priorityLabel, 1, 0);

    prioritySpinBox = new QSpinBox();
    prioritySpinBox->setRange(0, 200);
    prioritySpinBox->setValue(SACN_STANDARD_PRIORITY);
    layout->addWidget(prioritySpinBox, 1, 1);

    // Root Layer
    // Preamble Size (Octet 0-1)
    header.append((char)0x00);
    header.append((char)0x10);

    //Post-amble Size (Octet 2-3)
    header.append((char)0x00);
    header.append((char)0x00);

    // ACN Packet Identifier (Octet 4-15)
    header.append((char)0x41);
    header.append((char)0x53);
    header.append((char)0x43);
    header.append((char)0x2d);
    header.append((char)0x45);
    header.append((char)0x31);
    header.append((char)0x2e);
    header.append((char)0x31);
    header.append((char)0x37);
    header.append((char)0x00);
    header.append((char)0x00);
    header.append((char)0x00);

    // Flags and Length (Octet 16-17)
    header.append((char)0x72);
    header.append((char)0x6e);

    // Vector (Octet 18-21)
    header.append((char)0x00);
    header.append((char)0x00);
    header.append((char)0x00);
    header.append((char)0x04);

    // CID (Octet 22-37)
    QByteArray cid = QUuid::createUuid().toRfc4122();
    header.append(cid);

    // Framing Layer
    // Flags and Length (Octet 38-39)
    header.append((char)0x72);
    header.append((char)0x58);

    // Vector (Octet 40-43)
    header.append((char)0x00);
    header.append((char)0x00);
    header.append((char)0x00);
    header.append((char)0x02);

    // Source Name (Octet 44-107)
    QByteArray source = QString("ZÖGLFREX").toUtf8();
    header.append(source);
    for (int i=0; i < 64-source.length(); i++) {
        header.append((char)0x00);
    }

    // Priority (Octet 108)
    header.append((char)100); // 0-200, Default: 100

    // Synchronization Address (Octet 109-110)
    header.append((char)0x00);
    header.append((char)0x00);

    // Sequence Number (Octet 111)
    header.append((char)0x00);

    // Options (Octet 112)
    header.append((char)0x00); // Select no options

    // Universe (Octet 113-114)
    header.append((char)0x00);
    header.append((char)0x01);

    // DMP Layer
    // Flags and Length (Octet 115-116)
    header.append((char)0x72);
    header.append((char)0x0b);

    // Vector (Octet 117)
    header.append((char)0x02);

    // Address Type & Data Type (Octet 118)
    header.append((char)0xa1);

    // First Property Address (Octet 119-120)
    header.append((char)0x00);
    header.append((char)0x00);

    // Address Increment (Octet 121-122)
    header.append((char)0x00);
    header.append((char)0x01);

    // Property Value Count (Octet 123-124)
    header.append((char)0x02);
    header.append((char)0x01);

    // Start Code (Octet 125)
    header.append((char)0x00);

    // Property Values (Octet 126-637)
}

void SacnServer::send(QMap<int, QByteArray> dmxUniverses) {
    if ((socket == nullptr) || dmxUniverses.isEmpty()) {
        return;
    }
    for (const int universe : dmxUniverses.keys()) {
        Q_ASSERT(dmxUniverses.value(universe).size() == 512);
        Q_ASSERT(universe <= SACN_MAX_UNIVERSE);
        Q_ASSERT(universe >= 1);
        QByteArray dmx = header;
        dmx.append(dmxUniverses.value(universe));
        dmx[108] = (char)prioritySpinBox->value(); // Update Priority
        dmx[111] = sequence; // Update Sequence number
        dmx[113] = (char)(universe / 256); // Update Universe number
        dmx[114] = (char)(universe % 256); // Update Universe number
        QString address = "239.255.";
        address += QString::number(universe / 256);
        address += ".";
        address += QString::number(universe % 256);
        qint64 result = socket->writeDatagram(dmx.data(), dmx.size(), QHostAddress(address), SACN_PORT);
        if (result < 0) {
            qWarning() << Q_FUNC_INFO <<"ERROR sending sACN: " << socket->error() << " (" << socket->errorString() << ")";
        }
    }
    sequence++;
}

void SacnServer::setNetworkInterface() {
    int interfaceIndex = interfaceComboBox->currentIndex() - 1;
    if (interfaceIndex >= 0) {
        socket = new QUdpSocket();
        socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false); // Disable Multicast Loopback
        socket->bind(networkAddresses[interfaceIndex].ip());
        socket->setMulticastInterface(networkInterfaces[interfaceIndex]);
        kernel->terminal->success("Set sACN Output to " + networkInterfaces[interfaceIndex].name() + " (" + networkAddresses[interfaceIndex].ip().toString() + ")");
    } else {
        delete socket;
        socket = nullptr;
        kernel->terminal->success("Disabled sACN output.");
    }
}
