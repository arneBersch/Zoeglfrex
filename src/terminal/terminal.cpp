/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    prompt = new Prompt();
    connect(prompt, &Prompt::info, this, &Terminal::info);
    connect(prompt, &Prompt::error, this, &Terminal::error);
    connect(prompt, &Prompt::tableChanged, this, &Terminal::promptTableChanged);
    connect(prompt, &Prompt::executed, this, &Terminal::execute);
    layout->addWidget(prompt);

    messages = new QPlainTextEdit();
    messages->setReadOnly(true);
    layout->addWidget(messages);
}

void Terminal::execute(Prompt::Key selectionType, int id, int attribute, int value) {
    enum attributeTypes {
        text,
        number,
    };
    attributeTypes attributeType;
    QString tableName;
    QString attributeName = "";
    if (attribute == 1) {
        attributeName = "label";
        attributeType = text;
    }
    if (selectionType == Prompt::Model) {
        tableName = "models";
        if (attribute == 2) {
            attributeName = "channels";
            attributeType = text;
        }
    } else if (selectionType == Prompt::Fixture) {
        tableName = "fixtures";
    } else if (selectionType == Prompt::Group) {
        tableName = "groups";
    } else if (selectionType == Prompt::Intensity) {
        tableName = "intensities";
        if (attribute == 2) {
            attributeName = "intensity";
            attributeType = number;
        }
    } else if (selectionType == Prompt::Color) {
        tableName = "colors";
        if (attribute == 2) {
            attributeName = "hue";
            attributeType = number;
        } else if (attribute == 3) {
            attributeName = "saturation";
            attributeType = number;
        }
    } else if (selectionType == Prompt::Cue) {
        tableName = "cues";
    } else {
        error("Unknown Item type.");
        return;
    }
    if (attributeName.isEmpty()) {
        error("Unknown Attribute given.");
        return;
    }

    QSqlQuery existsQuery;
    existsQuery.prepare("SELECT id FROM " + tableName + " WHERE id = :id");
    existsQuery.bindValue(":id", id);
    if (!existsQuery.exec()) {
        error("Error executing Item check.");
        return;
    }
    if (!existsQuery.next()) {
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO " + tableName + " (id) VALUES (:id)");
        insertQuery.bindValue(":id", id);
        insertQuery.exec();
        success("Created Item with ID " + QString::number(id) + ".");
    }

    if (attributeType == text) {
        QSqlQuery query;
        query.prepare("SELECT " + attributeName + " FROM " + tableName + " WHERE id = :id");
        query.bindValue(":id", id);
        if (!query.exec()) {
            error("Failed setting load current Attribute value.");
            return;
        }
        QString textValue = QString();
        while (query.next()) {
            textValue = query.value(0).toString();
        }
        bool ok;
        textValue = QInputDialog::getText(this, QString(), "Insert Text", QLineEdit::Normal, textValue, &ok);;
        if (!ok) {
            return;
        }
        query.prepare("UPDATE " + tableName + " SET " + attributeName + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", textValue);
        if (query.exec()) {
            success("Set Attribute " + QString::number(attribute) + " of Item " + QString::number(id) + " to \"" + textValue + "\".");
        } else {
            error("Failed setting Attribute " + QString::number(attribute) + " of Item " + QString::number(id) + ".");
            return;
        }
    } else if (attributeType == number) {
        QSqlQuery query;
        query.prepare("UPDATE " + tableName + " SET " + attributeName + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", value);
        if (query.exec()) {
            success("Set Attribute " + QString::number(id) + " of Item " + QString::number(id) + " to " + QString::number(value) + ".");
        } else {
            error("Failed setting Attribute " + QString::number(id) + " of Item " + QString::number(id) + ".");
            return;
        }
    } else {
        Q_ASSERT(false);
    }
    emit dbChanged();
}

void Terminal::info(QString message) {
    messages->appendHtml("<span style=\"color: white\">" + message + "</span>");
    qInfo() << message;
}

void Terminal::success(QString message) {
    messages->appendHtml("<span style=\"color: green\">" + message + "</span>");
    qInfo() << message;
}

void Terminal::warning(QString message) {
    messages->appendHtml("<span style=\"color: yellow\">" + message + "</span>");
    qWarning() << message;
}

void Terminal::error(QString message) {
    messages->appendHtml("<span style=\"color: red\">" + message + "</span>");
    qCritical() << message;
}
