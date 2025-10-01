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
        } else if (attribute == 3) {
            setNumberAttribute<int>("fixtures", "Fixture", "universe", "Universe", ids, valueKeys, "", 1, 63999, false);
        } else if (attribute == 4) {
            setNumberAttribute<int>("fixtures", "Fixture", "address", "Address", ids, valueKeys, "", 0, 512, false);
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
            setNumberAttribute<float>("intensities", "Intensity", "dimmer", "Dimmer", ids, valueKeys, "%", 0, 100, false);
        } else {
            error("Unknown Intensity Attribute.");
        }
    } else if (selectionType == Prompt::Color) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Prompt::Minus)) {
            deleteItems("colors", "Color", ids);
        } else if (attribute == 1) {
            setTextAttribute("colors", "Color", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setNumberAttribute<float>("colors", "Color", "hue", "Hue", ids, valueKeys, "°", 0, 360, true);
        } else if (attribute == 3) {
            setNumberAttribute<float>("colors", "Color", "saturation", "Saturation", ids, valueKeys, "%", 0, 100, false);
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
    } else if (selectionType == Prompt::Cuelist) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Prompt::Minus)) {
            deleteItems("cuelists", "Cuelist", ids);
        } else if (attribute == 1) {
            setTextAttribute("cuelists", "Cuelist", "label", "Label", ids, "");
        } else {
            error("Unknown Cuelist Attribute.");
        }
    } else {
        error("Unknown Item type.");
    }
}

void Terminal::createItems(QString table, QString itemName, QList<int> ids) {
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT id FROM " + table + " WHERE id = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec()) {
            if (!existsQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO " + table + " (id) VALUES (:id)");
                insertQuery.bindValue(":id", id);
                if (insertQuery.exec()) {
                    successfulIds.append(QString::number(id));
                } else {
                    error("Failed to create " + itemName + " " + QString::number(id) + " because the request failed: " + insertQuery.lastError().text());
                }
            }
        } else {
            error("Error executing check if " + itemName + " exists: " + existsQuery.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Created " + itemName + " " + successfulIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Created " + itemName + "s " + successfulIds.join(", ") + ".");
    }
}

void Terminal::deleteItems(QString table, QString itemName, QList<int> ids) {
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("DELETE FROM " + table + " WHERE id = :id");
        query.bindValue(":id", id);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Can't delete " + itemName + " because the request failed: " + query.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Deleted " + itemName + " " + successfulIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Deleted " + itemName + "s " + successfulIds.join(", ") + ".");
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
    createItems(table, itemName, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", textValue);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Failed setting " + attributeName + " of " + itemName + " " + QString::number(id) + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + itemName + " " + successfulIds.first() + " to \"" + textValue + "\".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + itemName + "s " + successfulIds.join(", ") + " to \"" + textValue + "\".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setNumberAttribute(QString table, QString itemName, QString attribute, QString attributeName, QList<int> ids, QList<Prompt::Key> valueKeys, QString unit, T minValue, T maxValue, bool cyclic) {
    Q_ASSERT(!ids.isEmpty());
    bool ok;
    T value = prompt->keysToFloat(valueKeys, &ok);
    if (!ok) {
        error("Invalid value given.");
        return;
    }
    if (cyclic) {
        while (value < minValue) {
            value += (maxValue - minValue);
        }
        while (value >= maxValue) {
            value -= (maxValue - minValue);
        }
    } else {
        if ((value < minValue) || (value > maxValue)) {
            error(itemName + " " + attribute + " has to be between " + QString::number(minValue) + " and " + QString::number(maxValue) + ".");
            return;
        }
    }
    createItems(table, itemName, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", value);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Failed setting " + attributeName + " of " + itemName + " " + QString::number(id) + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + itemName + " " + successfulIds.first() + " to " + QString::number(value) + unit + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + itemName + "s " + successfulIds.join(", ") + " to " + QString::number(value) + unit + ".");
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
    createItems(table, itemName, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + table + " SET " + attribute + " = :item WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":item", foreignItem);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Failed setting " + attributeName + " of " + itemName + " " + QString::number(id) + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + itemName + " " + successfulIds.first() + " to " + foreignItemName + " " + QString::number(foreignItem) + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + itemName + "s " + successfulIds.join(", ") + " to " + foreignItemName + " " + QString::number(foreignItem) + ".");
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
