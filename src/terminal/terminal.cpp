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
    connect(prompt, &Prompt::tableChanged, this, &Terminal::promptTableChanged);
    layout->addWidget(prompt);

    messages = new QPlainTextEdit();
    messages->setReadOnly(true);
    layout->addWidget(messages);

    new QShortcut(Qt::Key_Enter, this, [this] { executePrompt(); });
    new QShortcut(Qt::Key_Return, this, [this] { executePrompt(); });
}

void Terminal::executePrompt() {
    QList<Prompt::Key> keys = prompt->getPromptKeys();
    if (keys.isEmpty()) {
        return;
    }
    info("> " + prompt->promptToString());
    prompt->clearPrompt();

    Prompt::Key selectionType = keys.first();
    keys.removeFirst();
    if (!prompt->isItemKey(selectionType)) {
        error("No item type specified.");
        return;
    }

    QList<Prompt::Key> selectionIdKeys;
    QList<Prompt::Key> attributeKeys;
    QList<Prompt::Key> value;
    bool attributeReached = false;
    bool valueReached = false;
    for (const Prompt::Key key : keys) {
        if (key == Prompt::Set) {
            if (valueReached) {
                error("Can't use Set more than one time in one command.");
                return;
            }
            valueReached = true;
        } else if ((prompt->isItemKey(key) || (key == Prompt::Attribute)) && !valueReached) {
            //attribute.append(key);
            attributeReached = true;
        } else {
            if (valueReached) {
                value.append(key);
            } else if (attributeReached) {
                attributeKeys.append(key);
            } else {
                selectionIdKeys.append(key);
            }
        }
    }
    const int selectionId = prompt->keysToNumber(selectionIdKeys);
    if (selectionId < 0) {
        error("Invalid ID selection.");
        return;
    }
    const int attributeId = prompt->keysToNumber(attributeKeys);
    if (attributeId < 0) {
        error("Invalid Attribute ID.");
        return;
    }

    enum attributeTypes {
        text,
        number,
    };
    attributeTypes attributeType;
    QString tableName;
    QString attributeName = "";
    if (attributeId == 1) {
        attributeName = "label";
        attributeType = text;
    }
    if (selectionType == Prompt::Model) {
        tableName = "models";
        if (attributeId == 2) {
            attributeName = "channels";
            attributeType = text;
        }
    } else if (selectionType == Prompt::Fixture) {
        tableName = "fixtures";
    } else if (selectionType == Prompt::Group) {
        tableName = "groups";
    } else if (selectionType == Prompt::Intensity) {
        tableName = "intensities";
        if (attributeId == 2) {
            attributeName = "intensity";
            attributeType = number;
        }
    } else if (selectionType == Prompt::Color) {
        tableName = "colors";
        if (attributeId == 2) {
            attributeName = "hue";
            attributeType = number;
        } else if (attributeId == 3) {
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
    existsQuery.bindValue(":id", selectionId);
    if (!existsQuery.exec()) {
        error("Error executing Item check.");
        return;
    }
    if (!existsQuery.next()) {
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO " + tableName + " (id) VALUES (:id)");
        insertQuery.bindValue(":id", selectionId);
        insertQuery.exec();
        success("Created Item with ID " + QString::number(selectionId) + ".");
    }

    if (attributeType == text) {
        QSqlQuery query;
        query.prepare("SELECT " + attributeName + " FROM " + tableName + " WHERE id = :id");
        query.bindValue(":id", selectionId);
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
        query.bindValue(":id", selectionId);
        query.bindValue(":value", textValue);
        if (query.exec()) {
            success("Set Attribute " + QString::number(attributeId) + " of Item " + QString::number(selectionId) + " to \"" + textValue + "\".");
        } else {
            error("Failed setting Attribute " + QString::number(attributeId) + " of Item " + QString::number(selectionId) + ".");
            return;
        }
    } else if (attributeType == number) {
        int numberValue = prompt->keysToNumber(value);
        QSqlQuery query;
        query.prepare("UPDATE " + tableName + " SET " + attributeName + " = :value WHERE id = :id");
        query.bindValue(":id", selectionId);
        query.bindValue(":value", numberValue);
        if (query.exec()) {
            success("Set Attribute " + QString::number(attributeId) + " of Item " + QString::number(selectionId) + " to " + QString::number(numberValue) + ".");
        } else {
            error("Failed setting Attribute " + QString::number(attributeId) + " of Item " + QString::number(selectionId) + ".");
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
