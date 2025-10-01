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
    connect(prompt, &Prompt::itemChanged, this, &Terminal::promptItemChanged);
    connect(prompt, &Prompt::executed, this, &Terminal::execute);
    layout->addWidget(prompt);

    messages = new QPlainTextEdit();
    messages->setReadOnly(true);
    layout->addWidget(messages);
}

void Terminal::execute(Prompt::Key selectionType, QList<int> ids, int attribute, QList<Prompt::Key> valueKeys) {
    if (selectionType == Prompt::Model) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Prompt::Minus)) {
            deleteItems("models", "Model", ids);
        } else if (attribute == 1) {
            setTextAttribute("models", "Model", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setTextAttribute("models", "Model", "channels", "Channels", ids, "^[01DdRrGgBbWwCcMmYyPpTtZz]+$");
        } else {
            error("Unknown Model Attribute.");
        }
    } else if (selectionType == Prompt::Fixture) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Prompt::Minus)) {
            deleteItems("fixtures", "Fixture", ids);
        } else if (attribute == 1) {
            setTextAttribute("fixtures", "Fixture", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setItemAttribute("fixtures", "Fixture", "model", "Model", ids, valueKeys, "models", "Model", Prompt::Model);
        } else {
            error("Unknown Fixture Attribute.");
        }
    } else if (selectionType == Prompt::Group) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Prompt::Minus)) {
            deleteItems("groups", "Group", ids);
        } else if (attribute == 1) {
            setTextAttribute("groups", "Group", "label", "Label", ids, "");
        } else {
            error("Unknown Group Attribute.");
        }
    } else if (selectionType == Prompt::Intensity) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Prompt::Minus)) {
            deleteItems("intensities", "Intensity", ids);
        } else if (attribute == 1) {
            setTextAttribute("intensities", "Intensity", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setIntegerAttribute("intensities", "Intensity", "dimmer", "Dimmer", ids, valueKeys, 0, 100);
        } else {
            error("Unknown Intensity Attribute.");
        }
    } else if (selectionType == Prompt::Color) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Prompt::Minus)) {
            deleteItems("colors", "Color", ids);
        } else if (attribute == 1) {
            setTextAttribute("colors", "Color", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setAngleAttribute("colors", "Color", "hue", "Hue", ids, valueKeys);
        } else if (attribute == 3) {
            setIntegerAttribute("colors", "Color", "saturation", "Saturation", ids, valueKeys, 0, 100);
        } else {
            error("Unknown Color Attribute.");
        }
    } else if (selectionType == Prompt::Cue) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Prompt::Minus)) {
            deleteItems("cues", "Cue", ids);
        } else if (attribute == 1) {
            setTextAttribute("cues", "Cue", "label", "Label", ids, "");
        } else {
            error("Unknown Cue Attribute.");
        }
    } else {
        error("Unknown Item type.");
    }
}

void Terminal::createItem(QString table, QString itemName, int id) {
    QSqlQuery existsQuery;
    existsQuery.prepare("SELECT id FROM " + table + " WHERE id = :id");
    existsQuery.bindValue(":id", id);
    if (!existsQuery.exec()) {
        error("Error executing check if " + itemName + " exists: " + existsQuery.lastError().text());
        return;
    }
    if (!existsQuery.next()) {
        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO " + table + " (id) VALUES (:id)");
        insertQuery.bindValue(":id", id);
        if (insertQuery.exec()) {
            success("Created " + itemName + " with ID " + QString::number(id) + ".");
        } else {
            error("Failed to create " + itemName + " " + QString::number(id) + " because the request failed: " + insertQuery.lastError().text());
        }
    }
}

void Terminal::deleteItems(QString table, QString itemName, QList<int> ids) {
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("DELETE FROM " + table + " WHERE id = :id");
        query.bindValue(":id", id);
        if (query.exec()) {
            success("Deleted " + itemName + " " + QString::number(id) + ".");
        } else {
            error("Can't delete " + itemName + " because the request failed: " + query.lastError().text());
        }
    }
    emit dbChanged();
}

void Terminal::setTextAttribute(QString table, QString itemName, QString attribute, QString attributeName, QList<int> ids, QString regex) {
    Q_ASSERT(!ids.isEmpty());
    QString textValue = QString();
    if (ids.length() == 1) {
        QSqlQuery query;
        query.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
        query.bindValue(":id", ids.first());
        if (!query.exec()) {
            error("Failed to load current " + attributeName + " of " + itemName + " " + QString::number(ids.first()) + ": " + query.lastError().text());
            return;
        }
        while (query.next()) {
            textValue = query.value(0).toString();
        }
    }
    bool ok;
    textValue = QInputDialog::getText(this, QString(), (itemName + " " + attributeName), QLineEdit::Normal, textValue, &ok);
    if (!ok) {
        error("Popup canceled.");
        return;
    }
    if (!regex.isEmpty() && !textValue.contains(QRegularExpression(regex))) {
        error("Can't set " + itemName + " " + attributeName + " because the given value \"" + textValue + "\" is not valid.");
        return;
    }
    for (int id : ids) {
        createItem(table, itemName, id);
        QSqlQuery query;
        query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", textValue);
        if (!query.exec()) {
            error("Failed setting " + attributeName + " of " + itemName + " " + QString::number(id) + ": " + query.lastError().text());
            return;
        }
    }
    if (ids.length() == 1) {
        success("Set " + attributeName + " of " + itemName + " " + QString::number(ids.first()) + " to \"" + textValue + "\".");
    } else {
        success("Set " + attributeName + " of " + QString::number(ids.length()) + " " + itemName + "s to \"" + textValue + "\".");
    }
    emit dbChanged();
}

void Terminal::setIntegerAttribute(QString table, QString itemName, QString attribute, QString attributeName, QList<int> ids, QList<Prompt::Key> valueKeys, int minValue, int maxValue) {
    Q_ASSERT(!ids.isEmpty());
    bool ok;
    const int value = prompt->keysToFloat(valueKeys, &ok);
    if (!ok) {
        error("Invalid value given.");
        return;
    }
    if ((value < minValue) || (value > maxValue)) {
        error(itemName + " " + attribute + " has to be between " + QString::number(minValue) + " and " + QString::number(maxValue) + ".");
        return;
    }
    for (int id : ids) {
        createItem(table, itemName, id);
        QSqlQuery query;
        query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", value);
        if (!query.exec()) {
            error("Failed setting " + attributeName + " of " + itemName + " " + QString::number(id) + ": " + query.lastError().text());
            return;
        }
    }
    if (ids.length() == 1) {
        success("Set " + attributeName + " of " + itemName + " " + QString::number(ids.first()) + " to " + QString::number(value) + ".");
    } else {
        success("Set " + attributeName + " of " + QString::number(ids.length()) + " " + itemName + "s to " + QString::number(value) + ".");
    }
    emit dbChanged();
}

void Terminal::setAngleAttribute(QString table, QString itemName, QString attribute, QString attributeName, QList<int> ids, QList<Prompt::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    const bool difference = (!valueKeys.isEmpty() && (valueKeys.first() == Prompt::Plus));
    if (difference) {
        valueKeys.removeFirst();
    }
    bool ok;
    int value = prompt->keysToFloat(valueKeys, &ok);
    if (!ok) {
        error("Invalid value given.");
        return;
    }
    if (difference) {
        for (int id : ids) {
            createItem(table, itemName, id);
            QSqlQuery currentValueQuery;
            currentValueQuery.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
            currentValueQuery.bindValue(":id", id);
            if (!currentValueQuery.exec()) {
                error("Can't set " + attributeName + " of " + itemName + " " + QString::number(id) + " because the request for the current Value failed: " + currentValueQuery.lastError().text());
                return;
            }
            if (!currentValueQuery.next()) {
                error("Can't set " + attributeName + " of " + itemName + " " + QString::number(id) + " because the request for the current Value didn't return any items.");
                return;
            }
            int currentValue = currentValueQuery.value(0).toInt();
            currentValue += value;
            while (currentValue < 0) {
                currentValue += 360;
            }
            while (currentValue >= 360) {
                currentValue -= 360;
            }
            QSqlQuery query;
            query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":value", currentValue);
            if (!query.exec()) {
                error("Failed setting " + attributeName + " of " + itemName + " " + QString::number(id) + ": " + query.lastError().text());
                return;
            }
        }
    } else {
        while (value < 0) {
            value += 360;
        }
        while (value >= 360) {
            value -= 360;
        }
        for (int id : ids) {
            createItem(table, itemName, id);
            QSqlQuery query;
            query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":value", value);
            if (!query.exec()) {
                error("Failed setting " + attributeName + " of " + itemName + " " + QString::number(id) + ": " + query.lastError().text());
                return;
            }
        }
    }
    if (ids.length() == 1) {
        success("Set " + attributeName + " of " + itemName + " " + QString::number(ids.first()) + " to " + QString::number(value) + ".");
    } else {
        if (difference) {
            success("Increased " + attributeName + " of " + QString::number(ids.length()) + itemName + "s by " + QString::number(value) + ".");
        } else {
            success("Set " + attributeName + " of " + QString::number(ids.length()) + itemName + "s to " + QString::number(value) + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemAttribute(QString table, QString itemName, QString attribute, QString attributeName, QList<int> ids, QList<Prompt::Key> valueKeys, QString foreignItemTable, QString foreignItemName, Prompt::Key foreignItemKey) {
    Q_ASSERT(!ids.isEmpty());
    if (!valueKeys.startsWith(foreignItemKey)) {
        error("Can't set " + itemName + " " + attributeName + " because no " + foreignItemName + " was given.");
        return;
    }
    valueKeys.removeFirst();
    bool ok;
    const int foreignItemId = prompt->keysToFloat(valueKeys, &ok);
    if (!ok) {
        error("Can't set " + itemName + " " + attributeName + " because the given " + foreignItemName + " ID is invalid.");
        return;
    }
    QSqlQuery foreignItemQuery;
    foreignItemQuery.prepare("SELECT key FROM " + foreignItemTable + " WHERE id = :id");
    foreignItemQuery.bindValue(":id", foreignItemId);
    if (!foreignItemQuery.exec()) {
        error("Failed to execute check if " + foreignItemName + " exists: " + foreignItemQuery.lastError().text());
        return;
    }
    if (!foreignItemQuery.next()) {
        error("Can't set " + itemName + " " + attributeName + " because " + foreignItemName + " " + QString::number(foreignItemId) + " doesn't exist.");
        return;
    }
    const int foreignItem = foreignItemQuery.value(0).toInt();
    for (int id : ids) {
        createItem(table, itemName, id);
        QSqlQuery query;
        query.prepare("UPDATE " + table + " SET " + attribute + " = :item WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":item", foreignItem);
        if (!query.exec()) {
            error("Failed setting " + attributeName + " of " + itemName + " " + QString::number(id) + ": " + query.lastError().text());
            return;
        }
    }
    if (ids.length() == 1) {
        success("Set " + attributeName + " of " + itemName + " " + QString::number(ids.first()) + " to " + foreignItemName + " " + QString::number(foreignItem) + ".");
    } else {
        success("Set " + attributeName + " of " + QString::number(ids.length()) + " " + itemName + "s to " + foreignItemName + " " + QString::number(foreignItem) + ".");
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
