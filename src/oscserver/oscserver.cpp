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
    resize(500, 400);

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    layout->addWidget(new QLabel("Enable"));
    enableCheckBox = new QCheckBox();
    layout->addWidget(enableCheckBox);

    layout->addWidget(new QLabel("Port"));
    portSpinBox = new QSpinBox();
    portSpinBox->setRange(0, 65535);
    layout->addWidget(portSpinBox);

    messages = new QPlainTextEdit();
    messages->setReadOnly(true);
    layout->addWidget(messages);

    QPushButton* clearMessagesButton = new QPushButton("Clear");
    connect(clearMessagesButton, &QPushButton::clicked, messages, &QPlainTextEdit::clear);
    layout->addWidget(clearMessagesButton);

    enableCheckBox->setChecked(QSettings().value("osc/enabled", false).toBool());
    setEnabled();
    connect(enableCheckBox, &QCheckBox::checkStateChanged, this, &OscServer::setEnabled);

    portSpinBox->setValue(QSettings().value("osc/port", 8000).toInt());
    setPort(portSpinBox->value());
    connect(portSpinBox, &QSpinBox::valueChanged, this, &OscServer::setPort);
}

void OscServer::setPort(const int port) {
    QSettings().setValue("osc/port", port);
    reloadSocket();
}

void OscServer::setEnabled() {
    QSettings().setValue("osc/enabled", enableCheckBox->isChecked());
    reloadSocket();
}

void OscServer::reloadSocket() {
    socket->close();
    if (enableCheckBox->isChecked()) {
        socket->bind(QHostAddress::AnyIPv4, portSpinBox->value());
        connect(socket, &QUdpSocket::readyRead, this, &OscServer::readPendingDatagrams);
    }
}

void OscServer::readPendingDatagrams() {
    while (socket->hasPendingDatagrams()) {
        const QNetworkDatagram datagram = socket->receiveDatagram();
        QByteArray message = datagram.data();
        message.replace('\0', ' ');
        messages->appendPlainText(QTime::currentTime().toString() + " - " + datagram.senderAddress().toString() + " : " + processDatagram(datagram.data()) + " (" + message + ")");
    }
}

QString OscServer::processDatagram(const QByteArray data) {
    if (data.length() % 4 != 0) {
        return "The length of the OSC Message is no multiple of 4";
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
        return "The given OSC Address Pattern does not match any command";
    }
    const QString cuelistId = addressPatternParts[2];

    QSqlQuery cuelistKeyQuery;
    cuelistKeyQuery.prepare("SELECT key FROM cuelists WHERE id = :id");
    cuelistKeyQuery.bindValue(":id", cuelistId);
    if (!cuelistKeyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << cuelistKeyQuery.executedQuery() << cuelistKeyQuery.lastError().text();
        return "The Cuelist key query failed";
    }
    if (!cuelistKeyQuery.next()) {
        return "Cuelist " + cuelistId + " does not exist";
    }
    const int cuelistKey = cuelistKeyQuery.value(0).toInt();

    QSqlQuery cueKeyQuery;
    if (addressPatternParts[3] == "go") {
        if ((typeTag != ",\0\0\0") || !arguments.isEmpty()) {
            return "The Go command does not expect any arguments";
        }
        cueKeyQuery.prepare("SELECT key, id FROM cues WHERE cuelist_key = :cuelist AND sortkey = (SELECT MIN(sortkey) FROM cues WHERE sortkey > (SELECT sortkey FROM cues WHERE key = (SELECT currentcue_key FROM cuelists WHERE cuelist_key = :cuelist)))");
    } else if (addressPatternParts[3] == "goback") {
        if ((typeTag != ",\0\0\0") || !arguments.isEmpty()) {
            return "The Go Back command does not expect any arguments";
        }
        cueKeyQuery.prepare("SELECT key, id FROM cues WHERE cuelist_key = :cuelist AND sortkey = (SELECT MAX(sortkey) FROM cues WHERE sortkey < (SELECT sortkey FROM cues WHERE key = (SELECT currentcue_key FROM cuelists WHERE cuelist_key = :cuelist)))");
    } else if (addressPatternParts[3] == "goto") {
        if (typeTag != ",s\0\0") {
            return "The Go To command expects a Cue ID as an argument";
        }
        cueKeyQuery.prepare("SELECT key, id FROM cues WHERE cuelist_key = :cuelist AND id = :id");
        cueKeyQuery.bindValue(":id", arguments.trimmed());
    } else {
        return "The given OSC Address Pattern does not match any command";
    }
    cueKeyQuery.bindValue(":cuelist", cuelistKey);
    if (!cueKeyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
        return "The Cue key query failed";
    }
    if (!cueKeyQuery.next()) {
        return "No matching Cue was found";
    }
    const int cueKey = cueKeyQuery.value(0).toInt();
    const QString cueId = cueKeyQuery.value(1).toString();

    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE cuelists SET currentcue_key = :cue WHERE key = :cuelist");
    updateQuery.bindValue(":cuelist", cuelistKey);
    updateQuery.bindValue(":cue", cueKey);
    if (!updateQuery.exec()) {
        qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
        return "The update query failed";
    }

    emit dbChanged();
    return "Set current Cue of Cuelist " + cuelistId + " to Cue " + cueId;
}
