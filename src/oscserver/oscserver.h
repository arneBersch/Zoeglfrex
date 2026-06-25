/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef OSCSERVER_H
#define OSCSERVER_H

#include <QtWidgets>
#include <QtNetwork>

class OscServer : public QWidget {
    Q_OBJECT
public:
    OscServer(QWidget* parent = nullptr);
signals:
    void dbChanged();
private slots:
    void setInterface(int index);
    void setPort(int port);
private:
    void reloadSocket();
    void reloadNetworkInterfaces();
    void readPendingDatagrams();
    QString processDatagram(QByteArray data);
    QUdpSocket* socket = nullptr;
    QComboBox* networkInterfaceComboBox;
    QSpinBox* portSpinBox;
    QPlainTextEdit *messages;
    QList<QNetworkInterface> networkInterfaces = QList<QNetworkInterface>();
    QList<QNetworkAddressEntry> networkAddresses = QList<QNetworkAddressEntry>();
};

#endif // OSCSERVER_H
