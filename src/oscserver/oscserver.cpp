/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "oscserver.h"

OscServer::OscServer(QWidget* parent) : QWidget(parent, Qt::Window) {
    setWindowTitle("Zöglfrex OSC Settings");

    QGridLayout* layout = new QGridLayout();
    setLayout(layout);

    layout->addWidget(new QLabel("Network Interface"), 0, 0);
    networkInterfaceComboBox = new QComboBox();
    layout->addWidget(networkInterfaceComboBox, 0, 1);

    layout->addWidget(new QLabel("Protocol"), 1, 0);
    protocolComboBox = new QComboBox();
    protocolComboBox->addItem("TCP");
    protocolComboBox->addItem("UDP");
    layout->addWidget(protocolComboBox, 1, 1);

    layout->addWidget(new QLabel("Port"), 2, 0);
    portSpinBox = new QSpinBox();
    portSpinBox->setRange(0, 65535);
    layout->addWidget(portSpinBox, 3, 1);

    QPushButton* reloadNetworkInterfaceButton = new QPushButton("Reload Network Interfaces");
    connect(reloadNetworkInterfaceButton, &QPushButton::clicked, this, &OscServer::reloadNetworkInterfaces);
    layout->addWidget(reloadNetworkInterfaceButton, 4, 0);

    reloadNetworkInterfaces();
    setInterface(networkInterfaceComboBox->currentIndex());
    connect(networkInterfaceComboBox, &QComboBox::currentIndexChanged, this, &OscServer::setInterface);

    protocolComboBox->setCurrentIndex(QSettings().value("osc/protocol", 0).toInt());
    setProtocol(protocolComboBox->currentIndex());
    connect(protocolComboBox, &QComboBox::currentIndexChanged, this, &OscServer::setProtocol);

    portSpinBox->setValue(QSettings().value("osc/port", 8000).toInt());
    setPort(portSpinBox->value());
    connect(portSpinBox, &QSpinBox::valueChanged, this, &OscServer::setPort);
}

void OscServer::reloadNetworkInterfaces() {
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
                if ((QSettings().value("osc/interface") == interface.name()) && (QSettings().value("osc/address") == address.ip().toString())) {
                    interfaceIndex = networkInterfaces.length();
                }
            }
        }
    }
    networkInterfaceComboBox->setCurrentIndex(interfaceIndex);
}

void OscServer::setInterface(int index) {
    if (index > 0) {
        QSettings().setValue("osc/interface", networkInterfaces.at(index - 1).name());
        QSettings().setValue("osc/address", networkAddresses.at(index - 1).ip().toString());
    } else {
        QSettings().setValue("osc/interface", "none");
        QSettings().setValue("osc/address", "none");
    }

    reloadSocket();
}

void OscServer::setProtocol(const int index) {
    QSettings().setValue("osc/protocol", index);

    reloadSocket();
}

void OscServer::setPort(const int port) {
    QSettings().setValue("osc/port", port);

    reloadSocket();
}

void OscServer::reloadSocket() {
    delete socket;
    socket = nullptr;

    const int interfaceIndex = networkInterfaceComboBox->currentIndex();
    if (interfaceIndex > 0) {
        const int protocolIndex = protocolComboBox->currentIndex();
        const int port = portSpinBox->value();

        if (protocolIndex == 0) { // TCP
            // TCP
        } else if (protocolIndex == 1) { // UDP
            // UDP
        } else {
            Q_ASSERT(false);
        }
    }
}
