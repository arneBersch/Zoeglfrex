/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "sacnserver.h"

SacnServer::SacnServer(QWidget* parent) : QWidget(parent) {
    settings = new QSettings("Zoeglfrex");
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

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &SacnServer::generateDmx);
    timer->start(25);

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
        qWarning() << Q_FUNC_INFO << socket->error() << socket->errorString();
    }
    sequence++;
}

void SacnServer::generateDmx() {
    QSqlQuery cuelistQuery;
    if (!cuelistQuery.exec("SELECT currentcue_key, priority FROM cuelists ORDER BY sortkey")) {
        qWarning() << Q_FUNC_INFO << cuelistQuery.executedQuery() << cuelistQuery.lastError().text();
        return;
    }
    QMap<int, int> fixtureIntensities;
    QMap<int, int> fixtureIntensityPriority;
    while (cuelistQuery.next()) {
        const int priority = cuelistQuery.value(1).toInt();
        QSqlQuery cueIntensityQuery;
        cueIntensityQuery.prepare("SELECT group_fixtures.valueitem_key, cue_group_intensities.valueitem_key FROM group_fixtures, groups, cue_group_intensities WHERE group_fixtures.item_key = groups.key AND cue_group_intensities.foreignitem_key = groups.key AND cue_group_intensities.item_key = :cue ORDER BY groups.sortkey");
        cueIntensityQuery.bindValue(":cue", cuelistQuery.value(0).toInt());
        if (cueIntensityQuery.exec()) {
            while (cueIntensityQuery.next()) {
                const int fixtureKey = cueIntensityQuery.value(0).toInt();
                if (priority >= fixtureIntensityPriority.value(fixtureKey, 0)) {
                    fixtureIntensities[fixtureKey] = cueIntensityQuery.value(1).toInt();
                    fixtureIntensityPriority[fixtureKey] = priority;
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << cueIntensityQuery.executedQuery() << cueIntensityQuery.lastError().text();
        }
    }
    QMap<int, QByteArray> dmxUniverses;
    QSqlQuery fixtureQuery;
    if (!fixtureQuery.exec("SELECT key, universe, address FROM fixtures")) {
        qWarning() << Q_FUNC_INFO << fixtureQuery.executedQuery() << fixtureQuery.lastError().text();
        return;
    }
    while (fixtureQuery.next()) {
        const int fixtureKey = fixtureQuery.value(0).toInt();
        const int universe = fixtureQuery.value(1).toInt();
        const int address = fixtureQuery.value(2).toInt();
        int dimmer = 0;
        if (fixtureIntensities.contains(fixtureKey)) {
            const int intensityKey = fixtureIntensities.value(fixtureKey);
            QSqlQuery fixtureExceptionQuery;
            fixtureExceptionQuery.prepare("SELECT value FROM intensity_fixture_dimmer WHERE item_key = :intensity AND foreignitem_key = :fixture");
            fixtureExceptionQuery.bindValue(":intensity", intensityKey);
            fixtureExceptionQuery.bindValue(":fixture", fixtureKey);
            if (fixtureExceptionQuery.exec()) {
                if (fixtureExceptionQuery.next()) {
                    dimmer = fixtureExceptionQuery.value(0).toFloat();
                } else {
                    QSqlQuery modelExceptionQuery;
                    modelExceptionQuery.prepare("SELECT intensity_model_dimmer.value FROM intensity_model_dimmer, fixtures WHERE intensity_model_dimmer.item_key = :intensity AND intensity_model_dimmer.foreignitem_key = fixtures.model_key AND fixtures.key = :fixture");
                    modelExceptionQuery.bindValue(":intensity", intensityKey);
                    modelExceptionQuery.bindValue(":fixture", fixtureKey);
                    if (modelExceptionQuery.exec()) {
                        if (modelExceptionQuery.next()) {
                            dimmer = modelExceptionQuery.value(0).toFloat();
                        } else {
                            QSqlQuery intensityQuery;
                            intensityQuery.prepare("SELECT dimmer FROM intensities WHERE key = :key");
                            intensityQuery.bindValue(":key", intensityKey);
                            if (intensityQuery.exec()) {
                                if (intensityQuery.next()) {
                                    dimmer = intensityQuery.value(0).toFloat();
                                } else {
                                    Q_ASSERT(false);
                                }
                            } else {
                                qWarning() << Q_FUNC_INFO << intensityQuery.executedQuery() << intensityQuery.lastError().text();
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << modelExceptionQuery.executedQuery() << modelExceptionQuery.lastError().text();
                    }
                }
            } else {
                qWarning() << Q_FUNC_INFO << fixtureExceptionQuery.executedQuery() << fixtureExceptionQuery.lastError().text();
            }
        }
        if (address > 0) {
            QSqlQuery channelsQuery;
            channelsQuery.prepare("SELECT models.channels FROM fixtures, models WHERE fixtures.key = :key AND fixtures.model_key = models.key");
            channelsQuery.bindValue(":key", fixtureKey);
            if (channelsQuery.exec()) {
                if (channelsQuery.next()) {
                    const QString channels = channelsQuery.value(0).toString();
                    if (!dmxUniverses.contains(universe)) {
                        dmxUniverses[universe] = QByteArray(512, 0);
                    }
                    for (int channel = address; channel < (address + channels.size()); channel++) {
                        QChar channelType = channels.at(channel - address);
                        const bool fine = (channelType != channelType.toUpper());
                        channelType = channelType.toUpper();
                        float value = 0;
                        if (channelType == QChar('D')) { // Dimmer
                            value = dimmer;
                        } else if (channelType == QChar('R')) { // Red
                        } else if (channelType == QChar('G')) { // Green
                        } else if (channelType == QChar('B')) { // Blue
                        } else if (channelType == QChar('W')) { // White
                        } else if (channelType == QChar('C')) { // Cyan
                        } else if (channelType == QChar('M')) { // Magenta
                        } else if (channelType == QChar('Y')) { // Yellow
                        } else if (channelType == QChar('P')) { // Pan
                        } else if (channelType == QChar('T')) { // Tilt
                        } else if (channelType == QChar('Z')) { // Zoom
                        } else if (channelType == QChar('F')) { // Focus
                        } else if (channelType == QChar('0')) { // DMX 0
                            value = 0;
                        } else if (channelType == QChar('1')) { // DMX 255
                            value = 100;
                        } else {
                            Q_ASSERT(false);
                        }
                        Q_ASSERT((value <= 100) && (value >= 0));
                        if (channel <= 512) {
                            value *= 655.35;
                            if (fine) {
                                dmxUniverses[universe][channel - 1] = (uchar)((int)value % 256);
                            } else {
                                dmxUniverses[universe][channel - 1] = (uchar)((int)value / 256);
                            }
                        }
                    }
                }
            } else {
                qWarning() << Q_FUNC_INFO << channelsQuery.executedQuery() << channelsQuery.lastError().text();
            }
        }
    }
    for (const int universe : dmxUniverses.keys()) {
        sendUniverse(universe, dmxUniverses.value(universe));
    }
}
