/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "oscserver.h"
#include <QtSql>

OscServer::OscServer(QWidget* parent) : QWidget(parent, Qt::Window) {
    setWindowTitle("Zöglfrex OSC Settings");

    QGridLayout* layout = new QGridLayout();
    setLayout(layout);

    layout->addWidget(new QLabel("Network Interface"), 0, 0);
    networkInterfaceComboBox = new QComboBox();
    layout->addWidget(networkInterfaceComboBox, 0, 1);

    layout->addWidget(new QLabel("Port"), 1, 0);
    portSpinBox = new QSpinBox();
    portSpinBox->setRange(0, 65535);
    layout->addWidget(portSpinBox, 1, 1);

    QPushButton* reloadNetworkInterfaceButton = new QPushButton("Reload Network Interfaces");
    connect(reloadNetworkInterfaceButton, &QPushButton::clicked, this, &OscServer::reloadNetworkInterfaces);
    layout->addWidget(reloadNetworkInterfaceButton, 2, 0);

    messages = new QPlainTextEdit();
    messages->setReadOnly(true);
    layout->addWidget(messages, 3, 0);

    reloadNetworkInterfaces();
    setInterface(networkInterfaceComboBox->currentIndex());
    connect(networkInterfaceComboBox, &QComboBox::currentIndexChanged, this, &OscServer::setInterface);

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

void OscServer::setPort(const int port) {
    QSettings().setValue("osc/port", port);

    reloadSocket();
}

void OscServer::reloadSocket() {
    delete socket;
    socket = nullptr;

    const int interfaceIndex = networkInterfaceComboBox->currentIndex();
    if (interfaceIndex > 0) {
        socket = new QUdpSocket();
        socket->bind(QHostAddress::AnyIPv4, portSpinBox->value());
        connect(socket, &QUdpSocket::readyRead, this, &OscServer::readPendingDatagrams);
    }
}

void OscServer::readPendingDatagrams() {
    while (socket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = socket->receiveDatagram();
        messages->appendPlainText(QTime::currentTime().toString() + " - " + datagram.senderAddress().toString() + " : " + processDatagram(datagram.data()) + " (" + datagram.data() + ")");
    }
}

QString OscServer::processDatagram(const QByteArray data) {
    if (data.length() % 4 != 0) {
        return "The length of the OSC Message is no multiple of 4.";
    }

    QByteArray addressPattern;
    QByteArray typeTag;
    QByteArray arguments;
    bool typeTagReached = false;
    bool argumentsReached = false;
    for (int i = 0; i < (data.length() / 4); i++) {
        QByteArray part = data.sliced(4 * i, 4);
        if (argumentsReached) {
            arguments.append(part);
        } else if (typeTagReached) {
            typeTag.append(part);
            if (part.back() == 0x00) {
                argumentsReached = true;
            }
        } else {
            addressPattern.append(part);
            if (part.back() == 0x00) {
                typeTagReached = true;
            }
        }
    }

    QList<QByteArray> addressPatternParts = addressPattern.split('/');
    if ((addressPatternParts.length() != 4) || (addressPatternParts[0] != "") || (addressPatternParts[1] != "zfr")) {
        return "The given OSC Address Pattern does not match any command.";
    }
    const QString cuelistId = addressPatternParts[2];

    QSqlQuery cuelistKeyQuery;
    cuelistKeyQuery.prepare("SELECT key FROM cuelists WHERE id = :id");
    cuelistKeyQuery.bindValue(":id", cuelistId);
    if (!cuelistKeyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << cuelistKeyQuery.executedQuery() << cuelistKeyQuery.lastError().text();
        return "The Cuelist key query failed.";
    }
    if (!cuelistKeyQuery.next()) {
        return "Cuelist " + cuelistId + " does not exist.";
    }
    const int cuelistKey = cuelistKeyQuery.value(0).toInt();

    QSqlQuery cueKeyQuery;
    if (addressPatternParts[3] == "go") {
        if ((typeTag != ",\0\0\0") || !arguments.isEmpty()) {
            return "The Go command does not expect any arguments.";
        }
        cueKeyQuery.prepare("SELECT key, id FROM cues WHERE cuelist_key = :cuelist AND sortkey = (SELECT MIN(sortkey) FROM cues WHERE sortkey > (SELECT sortkey FROM cues WHERE key = (SELECT currentcue_key FROM cuelists WHERE cuelist_key = :cuelist)))");
    } else if (addressPatternParts[3] == "goback") {
        if ((typeTag != ",\0\0\0") || !arguments.isEmpty()) {
            return "The Go Back command does not expect any arguments.";
        }
        cueKeyQuery.prepare("SELECT key, id FROM cues WHERE cuelist_key = :cuelist AND sortkey = (SELECT MAX(sortkey) FROM cues WHERE sortkey < (SELECT sortkey FROM cues WHERE key = (SELECT currentcue_key FROM cuelists WHERE cuelist_key = :cuelist)))");
    } else if (addressPatternParts[3] == "goto") {
        if (typeTag != ",s\0\0") {
            return "The Go To command expects a Cue ID as an argument.";
        }
        cueKeyQuery.prepare("SELECT key, id FROM cues WHERE cuelist_key = :cuelist AND id = :id");
        cueKeyQuery.bindValue(":id", arguments.trimmed());
    } else {
        return "The given OSC Address Pattern does not match any command.";
    }
    cueKeyQuery.bindValue(":cuelist", cuelistKey);
    if (!cueKeyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
        return "The Cue key query failed.";
    }
    if (!cueKeyQuery.next()) {
        return "No matching Cue was found.";
    }
    const int cueKey = cueKeyQuery.value(0).toInt();
    const QString cueId = cueKeyQuery.value(1).toString();

    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE cuelists SET currentcue_key = :cue WHERE key = :cuelist");
    updateQuery.bindValue(":cuelist", cuelistKey);
    updateQuery.bindValue(":cue", cueKey);
    if (!updateQuery.exec()) {
        qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
        return "The update query failed.";
    }

    emit dbChanged();
    return "Set current Cue of Cuelist " + cuelistId + " to Cue " + cueId + ".";
}
