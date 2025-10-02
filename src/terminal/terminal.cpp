/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QWidget(parent) {
    keyStrings[Zero] = "0";
    keyStrings[One] = "1";
    keyStrings[Two] = "2";
    keyStrings[Three] = "3";
    keyStrings[Four] = "4";
    keyStrings[Five] = "5";
    keyStrings[Six] = "6";
    keyStrings[Seven] = "7";
    keyStrings[Eight] = "8";
    keyStrings[Nine] = "9";
    keyStrings[Model] = " Model ";
    keyStrings[Fixture] = " Fixture ";
    keyStrings[Group] = " Group ";
    keyStrings[Intensity] = " Intensity ";
    keyStrings[Color] = " Color ";
    keyStrings[Position] = " Position ";
    keyStrings[Raw] = " Raw ";
    keyStrings[Effect] = " Effect ";
    keyStrings[Cue] = " Cue ";
    keyStrings[Cuelist] = " Cuelist ";
    keyStrings[Set] = " Set ";
    keyStrings[Attribute] = " Attribute ";
    keyStrings[Plus] = " + ";
    keyStrings[Minus] = " - ";
    keyStrings[Period] = ".";

    QVBoxLayout *layout = new QVBoxLayout(this);

    promptLabel = new QLabel();
    layout->addWidget(promptLabel);

    messages = new QPlainTextEdit();
    messages->setReadOnly(true);
    layout->addWidget(messages);

    new QShortcut(Qt::Key_0, this, [this] { writeKey(Zero); });
    new QShortcut(Qt::Key_1, this, [this] { writeKey(One); });
    new QShortcut(Qt::Key_2, this, [this] { writeKey(Two); });
    new QShortcut(Qt::Key_3, this, [this] { writeKey(Three); });
    new QShortcut(Qt::Key_4, this, [this] { writeKey(Four); });
    new QShortcut(Qt::Key_5, this, [this] { writeKey(Five); });
    new QShortcut(Qt::Key_6, this, [this] { writeKey(Six); });
    new QShortcut(Qt::Key_7, this, [this] { writeKey(Seven); });
    new QShortcut(Qt::Key_8, this, [this] { writeKey(Eight); });
    new QShortcut(Qt::Key_9, this, [this] { writeKey(Nine); });

    new QShortcut(Qt::Key_Plus, this, [this] { writeKey(Plus); });
    new QShortcut(Qt::Key_Minus, this, [this] { writeKey(Minus); });
    new QShortcut(Qt::Key_Period, this, [this] { writeKey(Period); });
    new QShortcut(Qt::Key_Comma, this, [this] { writeKey(Period); });
    new QShortcut(Qt::Key_A, this, [this] { writeKey(Attribute); });
    new QShortcut(Qt::Key_S, this, [this] { writeKey(Set); });

    new QShortcut(Qt::Key_M, this, [this] { writeKey(Model); });
    new QShortcut(Qt::Key_F, this, [this] { writeKey(Fixture); });
    new QShortcut(Qt::Key_G, this, [this] { writeKey(Group); });
    new QShortcut(Qt::Key_I, this, [this] { writeKey(Intensity); });
    new QShortcut(Qt::Key_C, this, [this] { writeKey(Color); });
    new QShortcut(Qt::Key_P, this, [this] { writeKey(Position); });
    new QShortcut(Qt::Key_R, this, [this] { writeKey(Raw); });
    new QShortcut(Qt::Key_E, this, [this] { writeKey(Effect); });
    new QShortcut(Qt::Key_Q, this, [this] { writeKey(Cue); });
    new QShortcut(Qt::Key_L, this, [this] { writeKey(Cuelist); });

    new QShortcut(Qt::Key_Backspace, this, [this] { backspace(); });
    new QShortcut(Qt::SHIFT | Qt::Key_Backspace, this, [this] { clearPrompt(); });
    new QShortcut(Qt::Key_Enter, this, [this] { execute(); });
    new QShortcut(Qt::Key_Return, this, [this] { execute(); });
}

void Terminal::execute() {
    QList<Key> keys = promptKeys;
    if (keys.isEmpty()) {
        return;
    }
    info("> " + keysToString(keys));
    clearPrompt();

    Key selectionType = keys.first();
    keys.removeFirst();

    QList<Key> selectionIdKeys;
    QList<Key> attributeKeys;
    QList<Key> valueKeys;
    bool attributeReached = false;
    bool valueReached = false;
    for (const Key key : keys) {
        if (key == Set) {
            if (valueReached) {
                error("Can't use Set more than one time in one command.");
                return;
            }
            valueReached = true;
        } else if ((itemKeys.contains(key) || (key == Attribute)) && !valueReached) {
            //attribute.append(key);
            attributeReached = true;
        } else {
            if (valueReached) {
                valueKeys.append(key);
            } else if (attributeReached) {
                attributeKeys.append(key);
            } else {
                selectionIdKeys.append(key);
            }
        }
    }
    const QList<int> ids = keysToIds(selectionIdKeys);
    if (ids.isEmpty()) {
        emit error("Invalid selection ID given.");
        return;
    }
    bool ok;
    int attribute = keysToFloat(attributeKeys, &ok);
    if (attributeKeys.isEmpty()) {
        attribute = -1;
    } else if (!ok) {
        emit error("Invalid Attribute ID given.");
        return;
    }

    if (selectionType == Model) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("models", "Model", "Models", ids);
        } else if (attribute == 1) {
            setTextAttribute("models", "Model", "Models", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setTextAttribute("models", "Model", "Models", "channels", "Channels", ids, "^[01DdRrGgBbWwCcMmYyPpTtZz]+$");
        } else {
            error("Unknown Model Attribute.");
        }
    } else if (selectionType == Fixture) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("fixtures", "Fixture", "Fixtures", ids);
        } else if (attribute == 1) {
            setTextAttribute("fixtures", "Fixture", "Fixtures", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setItemAttribute("fixtures", "Fixture", "Fixtures", "model_key", "Model", ids, valueKeys, "models", "Model", Model);
        } else if (attribute == 3) {
            setNumberAttribute<int>("fixtures", "Fixture", "Fixtures", "universe", "Universe", ids, valueKeys, "", 1, 63999, false);
        } else if (attribute == 4) {
            setNumberAttribute<int>("fixtures", "Fixture", "Fixtures", "address", "Address", ids, valueKeys, "", 0, 512, false);
        } else {
            error("Unknown Fixture Attribute.");
        }
    } else if (selectionType == Group) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("groups", "Group", "Groups", ids);
        } else if (attribute == 1) {
            setTextAttribute("groups", "Group", "Groups", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setItemListAttribute("groups", "Group", "Groups", "Fixtures", ids, valueKeys, "fixtures", "Fixture", "Fixtures", Fixture, "group_fixtures", "group_key", "fixture_key");
        } else {
            error("Unknown Group Attribute.");
        }
    } else if (selectionType == Intensity) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("intensities", "Intensity", "Intensities", ids);
        } else if (attribute == 1) {
            setTextAttribute("intensities", "Intensity", "Intensities", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setNumberAttribute<float>("intensities", "Intensity", "Intensities", "dimmer", "Dimmer", ids, valueKeys, "%", 0, 100, false);
        } else {
            error("Unknown Intensity Attribute.");
        }
    } else if (selectionType == Color) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("colors", "Color", "Colors", ids);
        } else if (attribute == 1) {
            setTextAttribute("colors", "Color", "Colors", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setNumberAttribute<float>("colors", "Color", "Colors", "hue", "Hue", ids, valueKeys, "°", 0, 360, true);
        } else if (attribute == 3) {
            setNumberAttribute<float>("colors", "Color", "Colors", "saturation", "Saturation", ids, valueKeys, "%", 0, 100, false);
        } else {
            error("Unknown Color Attribute.");
        }
    } else if (selectionType == Position) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("positions", "Position", "Positions", ids);
        } else if (attribute == 1) {
            setTextAttribute("positions", "Position", "Positions", "label", "Label", ids, "");
        } else if (attribute == 2) {
            setNumberAttribute<float>("positions", "Position", "Positions", "pan", "Pan", ids, valueKeys, "°", 0, 360, true);
        } else if (attribute == 3) {
            setNumberAttribute<float>("positions", "Position", "Positions", "tilt", "Tilt", ids, valueKeys, "°", -180, 180, false);
        } else {
            error("Unknown Position Attribute.");
        }
    } else if (selectionType == Raw) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("raw", "Raw", "Raws", ids);
        } else if (attribute == 1) {
            setTextAttribute("raws", "Raw", "Raws", "label", "Label", ids, "");
        } else {
            error("Unknown Raw Attribute.");
        }
    } else if (selectionType == Effect) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("effects", "Effect", "Effects", ids);
        } else if (attribute == 1) {
            setTextAttribute("effects", "Effect", "Effects", "label", "Label", ids, "");
        } else {
            error("Unknown Cue Attribute.");
        }
    } else if (selectionType == Cue) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("cues", "Cue", "Cues", ids);
        } else if (attribute == 1) {
            setTextAttribute("cues", "Cue", "Cues", "label", "Label", ids, "");
        } else {
            error("Unknown Cue Attribute.");
        }
    } else if (selectionType == Cuelist) {
        if ((attribute < 0) && (valueKeys.size() == 1) && (valueKeys.first() == Minus)) {
            deleteItems("cuelists", "Cuelist", "Cuelists", ids);
        } else if (attribute == 1) {
            setTextAttribute("cuelists", "Cuelist", "Cuelists", "label", "Label", ids, "");
        } else {
            error("Unknown Cuelist Attribute.");
        }
    } else {
        error("Unknown Item type.");
        return;
    }
}

void Terminal::createItems(const QString table, const QString singularItemName, const QString pluralItemName, QList<int> ids) {
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
                    error("Failed to create " + singularItemName + " " + QString::number(id) + " because the request failed: " + insertQuery.lastError().text());
                }
            }
        } else {
            error("Error executing check if " + singularItemName + " exists: " + existsQuery.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Created " + singularItemName + " " + successfulIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Created " + pluralItemName + " " + successfulIds.join(", ") + ".");
    }
}

void Terminal::deleteItems(const QString table, const QString singularItemName, const QString pluralItemName, QList<int> ids) {
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("DELETE FROM " + table + " WHERE id = :id");
        query.bindValue(":id", id);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Can't delete " + singularItemName + " because the request failed: " + query.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Deleted " + singularItemName + " " + successfulIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Deleted " + pluralItemName + " " + successfulIds.join(", ") + ".");
    }
    emit dbChanged();
}

void Terminal::setTextAttribute(const QString table, const QString singularItemName, const QString pluralItemName, const QString attribute, const QString attributeName, QList<int> ids, const QString regex) {
    Q_ASSERT(!ids.isEmpty());
    QString textValue = QString();
    if (ids.length() == 1) {
        QSqlQuery query;
        query.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
        query.bindValue(":id", ids.first());
        if (!query.exec()) {
            error("Failed to load current " + attributeName + " of " + singularItemName + " " + QString::number(ids.first()) + ": " + query.lastError().text());
            return;
        }
        while (query.next()) {
            textValue = query.value(0).toString();
        }
    }
    bool ok;
    textValue = QInputDialog::getText(this, QString(), (singularItemName + " " + attributeName), QLineEdit::Normal, textValue, &ok);
    if (!ok) {
        error("Popup canceled.");
        return;
    }
    if (!regex.isEmpty() && !textValue.contains(QRegularExpression(regex))) {
        error("Can't set " + singularItemName + " " + attributeName + " because the given value \"" + textValue + "\" is not valid.");
        return;
    }
    createItems(table, singularItemName, pluralItemName, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", textValue);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Failed setting " + attributeName + " of " + singularItemName + " " + QString::number(id) + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + singularItemName + " " + successfulIds.first() + " to \"" + textValue + "\".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + pluralItemName + " " + successfulIds.join(", ") + " to \"" + textValue + "\".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setNumberAttribute(const QString table, const QString singularItemName, const QString pluralItemName, const QString attribute, const QString attributeName, QList<int> ids, QList<Key> valueKeys, const QString unit, const T minValue, const T maxValue, const bool cyclic) {
    Q_ASSERT(!ids.isEmpty());
    const bool difference = (!valueKeys.isEmpty() && (valueKeys.first() == Plus));
    if (difference) {
        valueKeys.removeFirst();
    }
    bool ok;
    T value = keysToFloat(valueKeys, &ok);
    if (!ok) {
        error("Invalid value given.");
        return;
    }
    if (!difference) {
        if (cyclic) {
            while (value < minValue) {
                value += (maxValue - minValue);
            }
            while (value >= maxValue) {
                value -= (maxValue - minValue);
            }
        } else {
            if ((value < minValue) || (value > maxValue)) {
                error(singularItemName + " " + attribute + " has to be between " + QString::number(minValue) + " and " + QString::number(maxValue) + ".");
                return;
            }
        }
    }
    createItems(table, singularItemName, pluralItemName, ids);
    QStringList successfulIds;
    for (int id : ids) {
        T currentValue = value;
        bool valueOk = true;
        if (difference) {
            QSqlQuery currentValueQuery;
            currentValueQuery.prepare("SELECT " + attribute + " FROM " + table + " WHERE id = :id");
            currentValueQuery.bindValue(":id", id);
            if (currentValueQuery.exec()) {
                if (currentValueQuery.next()) {
                    currentValue += currentValueQuery.value(0).toFloat();
                    if (cyclic) {
                        while (currentValue < minValue) {
                            currentValue += (maxValue - minValue);
                        }
                        while (currentValue >= maxValue) {
                            currentValue -= (maxValue - minValue);
                        }
                    } else {
                        if ((currentValue < minValue) || (currentValue > maxValue)) {
                            error(attributeName + " of " + singularItemName + " " + QString::number(id) + " has to be between " + QString::number(minValue) + " and " + QString::number(maxValue) + ".");
                            valueOk = false;
                        }
                    }
                } else {
                    error("Failed loading the current " + attributeName + " of " + singularItemName + " " + QString::number(id) + " because this " + singularItemName + " doesn't exist.");
                    valueOk = false;
                }
            } else {
                error("Failed loading the current " + attributeName + " of " + singularItemName + " " + QString::number(id) + ": " + currentValueQuery.lastError().text());
                valueOk = false;
            }
        }
        if (valueOk) {
            QSqlQuery query;
            query.prepare("UPDATE " + table + " SET " + attribute + " = :value WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":value", currentValue);
            if (query.exec()) {
                successfulIds.append(QString::number(id));
            } else {
                error("Failed setting " + attributeName + " of " + singularItemName + " " + QString::number(id) + ": " + query.lastError().text());
            }
        }
    }
    if (difference) {
        if (successfulIds.length() == 1) {
            success("Changed " + attributeName + " of " + singularItemName + " " + successfulIds.first() + " by " + QString::number(value) + unit + ".");
        } else if (successfulIds.length() > 1) {
            success("Changed " + attributeName + " of " + pluralItemName + " " + successfulIds.join(", ") + " by " + QString::number(value) + unit + ".");
        }
    } else {
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + singularItemName + " " + successfulIds.first() + " to " + QString::number(value) + unit + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + pluralItemName + " " + successfulIds.join(", ") + " to " + QString::number(value) + unit + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemAttribute(const QString table, const QString singularItemName, const QString pluralItemName, const QString attribute, const QString attributeName, QList<int> ids, QList<Key> valueKeys, const QString foreignItemTable, const QString singularForeignItemName, const Key foreignItemKey) {
    Q_ASSERT(!ids.isEmpty());
    if (!valueKeys.startsWith(foreignItemKey)) {
        error("Can't set " + singularItemName + " " + attributeName + " because no " + singularForeignItemName + " was given.");
        return;
    }
    valueKeys.removeFirst();
    bool ok;
    const int foreignItemId = keysToFloat(valueKeys, &ok);
    if (!ok) {
        error("Can't set " + singularItemName + " " + attributeName + " because the given " + singularForeignItemName + " ID is invalid.");
        return;
    }
    QSqlQuery foreignItemQuery;
    foreignItemQuery.prepare("SELECT key FROM " + foreignItemTable + " WHERE id = :id");
    foreignItemQuery.bindValue(":id", foreignItemId);
    if (!foreignItemQuery.exec()) {
        error("Failed to execute check if " + singularForeignItemName + " exists: " + foreignItemQuery.lastError().text());
        return;
    }
    if (!foreignItemQuery.next()) {
        error("Can't set " + singularItemName + " " + attributeName + " because " + singularForeignItemName + " " + QString::number(foreignItemId) + " doesn't exist.");
        return;
    }
    const int foreignItem = foreignItemQuery.value(0).toInt();
    createItems(table, singularItemName, pluralItemName, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + table + " SET " + attribute + " = :item WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":item", foreignItem);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Failed setting " + attributeName + " of " + singularItemName + " " + QString::number(id) + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + singularItemName + " " + successfulIds.first() + " to " + singularForeignItemName + " " + QString::number(foreignItem) + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + pluralItemName + " " + successfulIds.join(", ") + " to " + singularForeignItemName + " " + QString::number(foreignItem) + ".");
    }
    emit dbChanged();
}

void Terminal::setItemListAttribute(const QString table, const QString singularItemName, const QString pluralItemName, QString attributeName, QList<int> ids, QList<Key> valueKeys, const QString foreignItemTable, const QString singularForeignItemName, const QString pluralForeignItemName, const Key foreignItemKey, const QString listTable, const QString listTableItemAttribute, const QString listTableForeignItemsAttribute) {
    Q_ASSERT(!ids.isEmpty());
    QList<int> foreignItems;
    if ((valueKeys.size() != 1) || !valueKeys.startsWith(Minus)) {
        if (!valueKeys.startsWith(foreignItemKey)) {
            error("Can't set " + singularItemName + " " + attributeName + " because no " + pluralForeignItemName + " were given.");
            return;
        }
        valueKeys.removeFirst();
        const QList<int> foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.isEmpty()) {
            error("Can't set " + singularItemName + " " + attributeName + " because the given " + singularForeignItemName + " IDs are invalid.");
            return;
        }
        for (int foreignItemId : foreignItemIds) {
            QSqlQuery foreignItemQuery;
            foreignItemQuery.prepare("SELECT key FROM " + foreignItemTable + " WHERE id = :id");
            foreignItemQuery.bindValue(":id", foreignItemId);
            if (foreignItemQuery.exec()) {
                if (foreignItemQuery.next()) {
                    foreignItems.append(foreignItemQuery.value(0).toInt());
                } else {
                    warning("Can't add " + singularForeignItemName + " " + QString::number(foreignItemId) + " to " + singularItemName + " " + attributeName + " because this " + singularForeignItemName + " doesn't exist.");
                }
            } else {
                error("Failed to execute check if " + singularForeignItemName + " " + QString::number(foreignItemId) + " exists: " + foreignItemQuery.lastError().text());
            }
        }
    }
    createItems(table, singularItemName, pluralItemName, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + table + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + listTable + " WHERE " + listTableItemAttribute + " =  :key");
                deleteQuery.bindValue(":key", itemKey);
                if (deleteQuery.exec()) {
                    for (int foreignItem : foreignItems) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + listTable + " (" + listTableItemAttribute + ", " + listTableForeignItemsAttribute + ") VALUES (:item, :foreign_item)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItem);
                        if (insertQuery.exec()) {
                            successfulIds.append(QString::number(id));
                        } else {
                            error("Failed to insert a " + singularForeignItemName + " into " + singularItemName + " " + QString::number(id) + ": " + insertQuery.lastError().text());
                        }
                    }
                } else {
                    error("Failed deleting old " + attributeName + " of " + singularItemName + " " + QString::number(id) + ": " + deleteQuery.lastError().text());
                }
            } else {
                error("Failed loading " + singularItemName  + QString::number(id) + " because this " + singularItemName + " wasn't found.");
            }
        } else {
            error("Failed loading " + singularItemName + " " + QString::number(id) + ": " + keyQuery.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + singularItemName + " " + successfulIds.first() + " to " + QString::number(foreignItems.length()) + " " + pluralForeignItemName + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + pluralItemName + " " + successfulIds.join(", ") + " to " + QString::number(foreignItems.length()) + " " + pluralForeignItemName + ".");
    }
    emit dbChanged();
}

void Terminal::updatePrompt() {
    if (!promptKeys.isEmpty()) {
        QList<Key> idKeys;
        bool id = true;
        for (const Key key : promptKeys.sliced(1, (promptKeys.length() - 1))) {
            bool ok;
            keysToFloat({key}, &ok);
            if (!ok && (key != Plus)) {
                id = false;
            }
            if (id) {
                idKeys.append(key);
            }
        }
        emit itemChanged(keysToString({ promptKeys.first() }), keysToIds(idKeys));
    }
    promptLabel->setText(keysToString(promptKeys));
}

void Terminal::writeKey(Key key) {
    promptKeys.append(key);
    updatePrompt();
}

void Terminal::backspace() {
    if (promptKeys.isEmpty()) {
        return;
    }
    promptKeys.removeLast();
    updatePrompt();
}

void Terminal::clearPrompt() {
    promptKeys.clear();
    updatePrompt();
}

float Terminal::keysToFloat(QList<Key> keys, bool* ok) const {
    return keysToString(keys).replace(" ", "").toFloat(ok);
}

QList<int> Terminal::keysToIds(QList<Key> keys) const {
    keys.append(Plus);
    QList<int> ids;
    QList<Key> currentIdKeys;
    for (const Key key : keys) {
        if (key == Plus) {
            bool ok;
            const int id = keysToFloat(currentIdKeys, &ok);
            if (!ok || (id < 0)) {
                return QList<int>();
            }
            ids.append(id);
            currentIdKeys.clear();
        } else {
            currentIdKeys.append(key);
        }
    }
    return ids;
}

QString Terminal::keysToString(QList<Key> keys) const {
    QString string;
    for(const Key key: keys) {
        Q_ASSERT(keyStrings.contains(key));
        string.append(keyStrings[key]);
    }
    return string.simplified();
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
