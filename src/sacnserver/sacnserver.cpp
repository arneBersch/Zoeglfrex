/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "sacnserver.h"

SacnServer::SacnServer(QWidget* parent) : QWidget(parent) {
    settings = new QSettings("zoeglfrex.conf", QSettings::NativeFormat);
    QGridLayout* layout = new QGridLayout(this);
    setWindowTitle("Zöglfrex sACN Settings");

    QLabel* networkInterfaceLabel = new QLabel("Network Interface");
    layout->addWidget(networkInterfaceLabel, 0, 0);
    networkInterfaceComboBox = new QComboBox();
    reloadNetworkInterfaces();
    connect(networkInterfaceComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        index--;
        delete socket;
        socket = nullptr;
        if (index >= 0) {
            socket = new QUdpSocket();
            socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false); // Disable Multicast Loopback
            socket->bind(networkAddresses.at(index).ip());
            socket->setMulticastInterface(networkInterfaces.at(index));
            settings->setValue("sacn/interface", networkInterfaces.at(index).name());
            settings->setValue("sacn/address", networkAddresses.at(index).ip().toString());
        } else {
            settings->setValue("sacn/interface", "none");
            settings->setValue("sacn/address", "none");
        }
    });
    layout->addWidget(networkInterfaceComboBox, 0, 1);

    QPushButton* reloadNetworkInterfaceButton = new QPushButton("Reload Network Interfaces");
    connect(reloadNetworkInterfaceButton, &QPushButton::clicked, this, &SacnServer::reloadNetworkInterfaces);
    layout->addWidget(reloadNetworkInterfaceButton, 1, 0);

    QLabel* priorityLabel = new QLabel("Priority");
    layout->addWidget(priorityLabel, 2, 0);
    QSpinBox* prioritySpinBox = new QSpinBox();
    prioritySpinBox->setMinimum(0);
    prioritySpinBox->setMaximum(200);
    prioritySpinBox->setValue(settings->value("sacn/priority", 100).toInt());
    connect(prioritySpinBox, &QSpinBox::valueChanged, this, [this](int port) {
        settings->setValue("sacn/priority", port);
    });
    layout->addWidget(prioritySpinBox, 2, 1);

    // Root Layer
    // Preamble Size (Octet 0-1)
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x10);

    //Post-amble Size (Octet 2-3)
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);

    // ACN Packet Identifier (Octet 4-15)
    packetHeader.append((char)0x41);
    packetHeader.append((char)0x53);
    packetHeader.append((char)0x43);
    packetHeader.append((char)0x2d);
    packetHeader.append((char)0x45);
    packetHeader.append((char)0x31);
    packetHeader.append((char)0x2e);
    packetHeader.append((char)0x31);
    packetHeader.append((char)0x37);
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);

    // Flags and Length (Octet 16-17)
    packetHeader.append((char)0x72);
    packetHeader.append((char)0x6e);

    // Vector (Octet 18-21)
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x04);

    // CID (Octet 22-37)
    QByteArray cid = QUuid::createUuid().toRfc4122();
    packetHeader.append(cid);

    // Framing Layer
    // Flags and Length (Octet 38-39)
    packetHeader.append((char)0x72);
    packetHeader.append((char)0x58);

    // Vector (Octet 40-43)
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x02);

    // Source Name (Octet 44-107)
    QByteArray source = QString("ZÖGLFREX").toUtf8();
    packetHeader.append(source);
    for (int i=0; i < 64-source.length(); i++) {
        packetHeader.append((char)0x00);
    }

    // Priority (Octet 108)
    packetHeader.append((char)100); // 0-200, Default: 100

    // Synchronization Address (Octet 109-110)
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);

    // Sequence Number (Octet 111)
    packetHeader.append((char)0x00);

    // Options (Octet 112)
    packetHeader.append((char)0x00); // Select no options

    // Universe (Octet 113-114)
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x01);

    // DMP Layer
    // Flags and Length (Octet 115-116)
    packetHeader.append((char)0x72);
    packetHeader.append((char)0x0b);

    // Vector (Octet 117)
    packetHeader.append((char)0x02);

    // Address Type & Data Type (Octet 118)
    packetHeader.append((char)0xa1);

    // First Property Address (Octet 119-120)
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x00);

    // Address Increment (Octet 121-122)
    packetHeader.append((char)0x00);
    packetHeader.append((char)0x01);

    // Property Value Count (Octet 123-124)
    packetHeader.append((char)0x02);
    packetHeader.append((char)0x01);

    // Start Code (Octet 125)
    packetHeader.append((char)0x00);

    // Property Values (Octet 126-637)
}

void SacnServer::reloadNetworkInterfaces() {
    networkInterfaceComboBox->clear();
    networkInterfaces.clear();
    networkAddresses.clear();
    networkInterfaceComboBox->addItem("None");
    int interfaceIndex = 0;
    for (QNetworkInterface interface : QNetworkInterface::allInterfaces()) {
        for (QNetworkAddressEntry address : interface.addressEntries()) {
            if (address.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                networkInterfaceComboBox->addItem(interface.name() + " (" + address.ip().toString() + ")");
                networkInterfaces.append(interface);
                networkAddresses.append(address);
                if ((settings->value("sacn/interface") == interface.name()) && (settings->value("sacn/address") == address.ip().toString())) {
                    interfaceIndex = networkInterfaces.length();
                }
            }
        }
    }
    qInfo() << networkInterfaces;
    networkInterfaceComboBox->setCurrentIndex(interfaceIndex);
}

void SacnServer::sendUniverse(const int universe, QByteArray data) {
    if (socket == nullptr) {
        return;
    }
    Q_ASSERT(data.size() == 512);
    Q_ASSERT(universe <= 63999);
    Q_ASSERT(universe >= 1);
    QByteArray dmx = packetHeader;
    dmx.append(data);
    dmx[108] = (char)settings->value("sacn/priority", 100).toInt();
    dmx[111] = sequence;
    dmx[113] = (char)(universe / 256);
    dmx[114] = (char)(universe % 256);
    const QString address = "239.255." + QString::number(universe / 256) + "." + QString::number(universe % 256);
    qint64 result = socket->writeDatagram(dmx.data(), dmx.size(), QHostAddress(address), 5568);
    if (result < 0) {
        qCritical() << "ERROR sending sACN: " << socket->error() << " (" << socket->errorString() << ")";
    }
    sequence++;
}
