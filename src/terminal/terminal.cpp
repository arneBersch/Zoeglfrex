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
    keyStrings[Cuelist] = " Cuelist ";
    keyStrings[Cue] = " Cue ";
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
    new QShortcut(Qt::Key_L, this, [this] { writeKey(Cuelist); });
    new QShortcut(Qt::Key_Q, this, [this] { writeKey(Cue); });

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
            attributeKeys.append(key);
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
    QMap<Key, QList<int>> attributes;
    if (!attributeKeys.isEmpty()) {
        attributeKeys.append(Attribute);
        QList<Key> currentItemKeys;
        for (Key key : attributeKeys) {
            if (itemKeys.contains(key) || (key == Attribute)) {
                if (!currentItemKeys.isEmpty()) {
                    Key currentItemType = currentItemKeys.first();
                    currentItemKeys.removeFirst();
                    QList<int> ids = keysToIds(currentItemKeys);
                    if (ids.isEmpty()) {
                        currentItemKeys.prepend(currentItemType);
                        error("Invalid Attribute given: " + keysToString(currentItemKeys));
                        return;
                    }
                    attributes[currentItemType] = ids;
                }
                currentItemKeys.clear();
            }
            currentItemKeys.append(key);
        }
    }
    const QList<int> ids = keysToIds(selectionIdKeys);
    if (ids.isEmpty()) {
        error("Invalid selection ID given.");
        return;
    }
    int attribute;
    if (attributes.value(Attribute).size() == 0) {
        attribute = -1;
    } else if (attributes.value(Attribute).size() == 1) {
        attribute = attributes.value(Attribute).first();
    } else {
        error("Invalid number of Attribute IDs given.");
        return;
    }

    const ItemInfos modelInfos = {"models", "Model", "Models", Model};
    const ItemInfos fixtureInfos = {"fixtures", "Fixture", "Fixtures", Fixture};
    const ItemInfos groupInfos = {"groups", "Group", "Groups", Group};
    const ItemInfos intensityInfos = {"intensities", "Intensity", "Intensities", Intensity};
    const ItemInfos colorInfos = {"colors", "Color", "Colors", Color};
    const ItemInfos positionInfos = {"positions", "Position", "Positions", Position};
    const ItemInfos rawInfos = {"raws", "Raw", "Raws", Raw};
    const ItemInfos effectInfos = {"effects", "Effect", "Effects", Effect};
    const ItemInfos cuelistInfos = {"cuelists", "Cuelist", "Cuelists", Cuelist};
    const ItemInfos cueInfos = {"cues", "Cue", "Cues", Cue};

    if (selectionType == Model) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(modelInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(modelInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(modelInfos, "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::modelChannels) || !attributes.contains(Attribute)) {
            setTextAttribute(modelInfos, "channels", "Channels", ids, "^[01DdRrGgBbWwCcMmYyPpTtZz]+$");
        } else {
            error("Unknown Model Attribute.");
        }
    } else if (selectionType == Fixture) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(fixtureInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(fixtureInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(fixtureInfos, "label", "Label", ids, "");
        } else if (attribute == AttributeIds::fixtureModel) {
            setItemAttribute(fixtureInfos, "model_key", "Model", ids, valueKeys, modelInfos);
        } else if (attribute == AttributeIds::fixtureUniverse) {
            setNumberAttribute<int>(fixtureInfos, "universe", "Universe", ids, valueKeys, "", 1, 63999, false);
        } else if ((attribute == AttributeIds::fixtureAddress) || !attributes.contains(Attribute)) {
            setNumberAttribute<int>(fixtureInfos, "address", "Address", ids, valueKeys, "", 0, 512, false);
        } else {
            error("Unknown Fixture Attribute.");
        }
    } else if (selectionType == Group) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(groupInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(groupInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(groupInfos, "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::groupFixtures) || !attributes.contains(Attribute)) {
            setItemListAttribute(groupInfos, "Fixtures", ids, valueKeys, fixtureInfos, "group_fixtures", "group_key", "fixture_key");
        } else {
            error("Unknown Group Attribute.");
        }
    } else if (selectionType == Intensity) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(intensityInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(intensityInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(intensityInfos, "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::intensityDimmer) || !attributes.contains(Attribute)) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(intensityInfos, "Dimmer Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "intensity_model_dimmers", "intensity_key", "model_key", "dimmer", "%", 0, 100, false);
            } else {
                setNumberAttribute<float>(intensityInfos, "dimmer", "Dimmer", ids, valueKeys, "%", 0, 100, false);
            }
        } else {
            error("Unknown Intensity Attribute.");
        }
    } else if (selectionType == Color) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(colorInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(colorInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(colorInfos, "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::colorHue) || attributes.contains(Attribute)) {
            setNumberAttribute<float>(colorInfos, "hue", "Hue", ids, valueKeys, "°", 0, 360, true);
        } else if (attribute == AttributeIds::colorSaturation) {
            setNumberAttribute<float>(colorInfos, "saturation", "Saturation", ids, valueKeys, "%", 0, 100, false);
        } else {
            error("Unknown Color Attribute.");
        }
    } else if (selectionType == Position) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(positionInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(positionInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(positionInfos, "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::positionPan) || attributes.contains(Attribute)) {
            setNumberAttribute<float>(positionInfos, "pan", "Pan", ids, valueKeys, "°", 0, 360, true);
        } else if (attribute == AttributeIds::positionTilt) {
            setNumberAttribute<float>(positionInfos, "tilt", "Tilt", ids, valueKeys, "°", -180, 180, false);
        } else {
            error("Unknown Position Attribute.");
        }
    } else if (selectionType == Raw) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(rawInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(rawInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(rawInfos, "label", "Label", ids, "");
        } else {
            error("Unknown Raw Attribute.");
        }
    } else if (selectionType == Effect) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(effectInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(effectInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(effectInfos, "label", "Label", ids, "");
        } else {
            error("Unknown Cue Attribute.");
        }
    } else if (selectionType == Cuelist) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(cuelistInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(cuelistInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(cuelistInfos, "label", "Label", ids, "");
        } else if (attribute == AttributeIds::cuelistMoveWhileDark) {
            setBoolAttribute(cuelistInfos, "movewhiledark", "Move while Dark", ids, valueKeys);
        } else {
            error("Unknown Cuelist Attribute.");
        }
    } else if (selectionType == Cue) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(cueInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(cueInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(cueInfos, "label", "Label", ids, "");
        } else {
            error("Unknown Cue Attribute.");
        }
    } else {
        error("Unknown Item type.");
        return;
    }
}

void Terminal::createItems(const ItemInfos item, QList<int> ids) {
    Q_ASSERT(!ids.isEmpty());
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT id FROM " + item.table + " WHERE id = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec()) {
            if (!existsQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO " + item.table + " (id) VALUES (:id)");
                insertQuery.bindValue(":id", id);
                if (insertQuery.exec()) {
                    successfulIds.append(QString::number(id));
                } else {
                    error("Failed to create " + item.singular + " " + QString::number(id) + " because the request failed: " + insertQuery.lastError().text());
                }
            }
        } else {
            error("Error executing check if " + item.singular + " exists: " + existsQuery.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Created " + item.singular + " " + successfulIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Created " + item.plural + " " + successfulIds.join(", ") + ".");
    }
}

void Terminal::deleteItems(const ItemInfos item, QList<int> ids) {
    Q_ASSERT(!ids.isEmpty());
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("DELETE FROM " + item.table + " WHERE id = :id");
        query.bindValue(":id", id);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Can't delete " + item.singular + " because the request failed: " + query.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Deleted " + item.singular + " " + successfulIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Deleted " + item.plural + " " + successfulIds.join(", ") + ".");
    }
    emit dbChanged();
}

void Terminal::moveItems(const ItemInfos item, QList<int> ids, QList<Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    bool ok;
    const int newId = keysToFloat(valueKeys, &ok);
    if (!ok) {
        error("Can't set " + item.singular + " ID because an invalid ID was given.");
        return;
    }
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT id FROM " + item.table + " WHERE id = :id");
        existsQuery.bindValue(":id", newId);
        if (existsQuery.exec()) {
            if (existsQuery.next()) {
                error("Can't set ID of " + item.singular + " to " + QString::number(newId) + " because this " + item.singular + " ID is already used.");
            } else {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE " + item.table + " SET id = :newId WHERE id = :id");
                updateQuery.bindValue(":id", id);
                updateQuery.bindValue(":newId", newId);
                if (updateQuery.exec()) {
                    successfulIds.append(QString::number(id));
                } else {
                    error("Failed to update ID of " + item.singular + " " + QString::number(id) + " because the request failed: " + updateQuery.lastError().text());
                }
            }
        } else {
            error("Error executing check if " + item.singular + " " + QString::number(newId) + " exists: " + existsQuery.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Set ID of " + item.singular + " " + successfulIds.first() + " to " + QString::number(newId) + ".");
    } else if (successfulIds.size() > 1) {
        success("Set ID of " + item.plural + " " + successfulIds.join(", ") + " to " + QString::number(newId) +".");
    }
    emit dbChanged();
}

void Terminal::setBoolAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QList<int> ids, QList<Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    int value = 0;
    QString valueText = "False";
    if ((valueKeys.size() == 1) && valueKeys.startsWith(Zero)) {
    } else if ((valueKeys.size() == 1) && valueKeys.startsWith(One)) {
        value = 1;
        valueText = "True";
    } else {
        error("Can't set " + item.singular + " " + attributeName + " because no valid value was given.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + item.table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", value);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Failed setting " + attributeName + " of " + item.singular + " " + QString::number(id) + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to " + valueText + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + valueText + ".");
    }
    emit dbChanged();
}

void Terminal::setTextAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QList<int> ids, const QString regex) {
    Q_ASSERT(!ids.isEmpty());
    QString textValue = QString();
    if (ids.length() == 1) {
        QSqlQuery query;
        query.prepare("SELECT " + attribute + " FROM " + item.table + " WHERE id = :id");
        query.bindValue(":id", ids.first());
        if (!query.exec()) {
            error("Failed to load current " + attributeName + " of " + item.singular + " " + QString::number(ids.first()) + ": " + query.lastError().text());
            return;
        }
        while (query.next()) {
            textValue = query.value(0).toString();
        }
    }
    bool ok;
    textValue = QInputDialog::getText(this, QString(), (item.singular+ " " + attributeName), QLineEdit::Normal, textValue, &ok);
    if (!ok) {
        error("Popup canceled.");
        return;
    }
    if (!regex.isEmpty() && !textValue.contains(QRegularExpression(regex))) {
        error("Can't set " + item.singular + " " + attributeName + " because the given value \"" + textValue + "\" is not valid.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + item.table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", textValue);
        if (query.exec()) {
            successfulIds.append(QString::number(id));
        } else {
            error("Failed setting " + attributeName + " of " + item.singular + " " + QString::number(id) + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to \"" + textValue + "\".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to \"" + textValue + "\".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setNumberAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QList<int> ids, QList<Key> valueKeys, const QString unit, const T minValue, const T maxValue, const bool cyclic) {
    Q_ASSERT(!ids.isEmpty());
    const bool difference = valueKeys.startsWith(Plus);
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
                error(item.singular + " " + attribute + " has to be between " + QString::number(minValue) + " and " + QString::number(maxValue) + ".");
                return;
            }
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (int id : ids) {
        T currentValue = value;
        bool valueOk = true;
        if (difference) {
            QSqlQuery currentValueQuery;
            currentValueQuery.prepare("SELECT " + attribute + " FROM " + item.table + " WHERE id = :id");
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
                            error(attributeName + " of " + item.singular + " " + QString::number(id) + " has to be between " + QString::number(minValue) + " and " + QString::number(maxValue) + ".");
                            valueOk = false;
                        }
                    }
                } else {
                    error("Failed loading the current " + attributeName + " of " + item.singular + " " + QString::number(id) + " because this " + item.singular + " doesn't exist.");
                    valueOk = false;
                }
            } else {
                error("Failed loading the current " + attributeName + " of " + item.singular + " " + QString::number(id) + ": " + currentValueQuery.lastError().text());
                valueOk = false;
            }
        }
        if (valueOk) {
            QSqlQuery query;
            query.prepare("UPDATE " + item.table + " SET " + attribute + " = :value WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":value", currentValue);
            if (query.exec()) {
                successfulIds.append(QString::number(id));
            } else {
                error("Failed setting " + attributeName + " of " + item.singular + " " + QString::number(id) + ": " + query.lastError().text());
            }
        }
    }
    if (difference) {
        if (successfulIds.length() == 1) {
            success("Changed " + attributeName + " of " + item.singular + " " + successfulIds.first() + " by " + QString::number(value) + unit + ".");
        } else if (successfulIds.length() > 1) {
            success("Changed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " by " + QString::number(value) + unit + ".");
        }
    } else {
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to " + QString::number(value) + unit + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + QString::number(value) + unit + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QList<int> ids, QList<Key> valueKeys, const ItemInfos foreignItem) {
    Q_ASSERT(!ids.isEmpty());
    if ((valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
        QStringList successfulIds;
        for (int id : ids) {
            QSqlQuery query;
            query.prepare("UPDATE " + item.table + " SET " + attribute + " = NULL WHERE id = :id");
            query.bindValue(":id", id);
            if (query.exec()) {
                successfulIds.append(QString::number(id));
            } else {
                error("Failed removing " + attributeName + " of " + item.singular + " " + QString::number(id) + ": " + query.lastError().text());
            }
        }
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.first() + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + ".");
        }
    } else {
        if (!valueKeys.startsWith(foreignItem.key)) {
            error("Can't set " + item.singular + " " + attributeName + " because no " + foreignItem.singular + " was given.");
            return;
        }
        valueKeys.removeFirst();
        bool ok;
        const int foreignItemId = keysToFloat(valueKeys, &ok);
        if (!ok) {
            error("Can't set " + item.singular + " " + attributeName + " because the given " + foreignItem.singular + " ID is invalid.");
            return;
        }
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.table + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (!foreignItemQuery.exec()) {
            error("Failed to execute check if " + foreignItem.singular + " exists: " + foreignItemQuery.lastError().text());
            return;
        }
        if (!foreignItemQuery.next()) {
            error("Can't set " + item.singular + " " + attributeName + " because " + foreignItem.singular + " " + QString::number(foreignItemId) + " doesn't exist.");
            return;
        }
        const int foreignItemKey = foreignItemQuery.value(0).toInt();
        createItems(item, ids);
        QStringList successfulIds;
        for (int id : ids) {
            QSqlQuery query;
            query.prepare("UPDATE " + item.table + " SET " + attribute + " = :item WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":item", foreignItemKey);
            if (query.exec()) {
                successfulIds.append(QString::number(id));
            } else {
                error("Failed setting " + attributeName + " of " + item.singular + " " + QString::number(id) + ": " + query.lastError().text());
            }
        }
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to " + foreignItem.singular + " " + QString::number(foreignItemId) + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + foreignItem.singular + " " + QString::number(foreignItemId) + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemListAttribute(const ItemInfos item, const QString attributeName, QList<int> ids, QList<Key> valueKeys, const ItemInfos foreignItem, const QString listTable, const QString listTableItemAttribute, const QString listTableForeignItemsAttribute) {
    Q_ASSERT(!ids.isEmpty());
    QList<int> foreignItemKeys;
    if ((valueKeys.size() != 1) || !valueKeys.startsWith(Minus)) {
        if (!valueKeys.startsWith(foreignItem.key)) {
            error("Can't set " + item.singular + " " + attributeName + " because no " + foreignItem.plural + " were given.");
            return;
        }
        valueKeys.removeFirst();
        const QList<int> foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.isEmpty()) {
            error("Can't set " + item.singular + " " + attributeName + " because the given " + foreignItem.singular + " IDs are invalid.");
            return;
        }
        for (int foreignItemId : foreignItemIds) {
            QSqlQuery foreignItemQuery;
            foreignItemQuery.prepare("SELECT key FROM " + foreignItem.table + " WHERE id = :id");
            foreignItemQuery.bindValue(":id", foreignItemId);
            if (foreignItemQuery.exec()) {
                if (foreignItemQuery.next()) {
                    foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                } else {
                    warning("Can't add " + foreignItem.singular + " " + QString::number(foreignItemId) + " to " + item.singular + " " + attributeName + " because this " + foreignItem.singular + " doesn't exist.");
                }
            } else {
                error("Failed to execute check if " + foreignItem.singular + " " + QString::number(foreignItemId) + " exists: " + foreignItemQuery.lastError().text());
            }
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.table + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + listTable + " WHERE " + listTableItemAttribute + " =  :key");
                deleteQuery.bindValue(":key", itemKey);
                if (deleteQuery.exec()) {
                    for (int foreignItemKey : foreignItemKeys) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + listTable + " (" + listTableItemAttribute + ", " + listTableForeignItemsAttribute + ") VALUES (:item, :foreign_item)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        if (insertQuery.exec()) {
                            successfulIds.append(QString::number(id));
                        } else {
                            error("Failed to insert a " + foreignItem.singular + " into " + item.singular + " " + QString::number(id) + ": " + insertQuery.lastError().text());
                        }
                    }
                } else {
                    error("Failed deleting old " + attributeName + " of " + item.singular + " " + QString::number(id) + ": " + deleteQuery.lastError().text());
                }
            } else {
                error("Failed loading " + item.singular  + QString::number(id) + " because this " + item.singular + " wasn't found.");
            }
        } else {
            error("Failed loading " + item.singular + " " + QString::number(id) + ": " + keyQuery.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to " + QString::number(foreignItemKeys.length()) + " " + foreignItem.plural + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + QString::number(foreignItemKeys.length()) + " " + foreignItem.plural + ".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setItemSpecificNumberAttribute(const ItemInfos item, const QString attributeName, QList<int> ids, QList<int> foreignItemIds, QList<Key> valueKeys, const ItemInfos foreignItem, const QString exceptionTable, const QString exceptionTableItemAttribute, const QString exceptionTableForeignItemAttribute, const QString exceptionTableValueAttribute, const QString unit, const T minValue, const T maxValue, const bool cyclic) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(!foreignItemIds.isEmpty());
    bool ok;
    T value = keysToFloat(valueKeys, &ok);
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
            error(item.singular + " " + attributeName + " has to be between " + QString::number(minValue) + " and " + QString::number(maxValue) + ".");
            return;
        }
    }
    QList<int> foreignItemKeys;
    for (int foreignItemId : foreignItemIds) {
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.table + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.singular + " " + QString::number(foreignItemId) + " because this " + foreignItem.singular + " doesn't exist.");
            }
        } else {
            error("Failed to execute check if " + foreignItem.singular + " " + QString::number(foreignItemId) + " exists: " + foreignItemQuery.lastError().text());
        }
    }
    if (foreignItemKeys.isEmpty()) {
        error("No valid " + foreignItem.plural + " were found.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (int id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.table + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allInsertsSuccessful = true;
                for (int foreignItemKey : foreignItemKeys) {
                    QSqlQuery deleteQuery;
                    deleteQuery.prepare("DELETE FROM " + exceptionTable + " WHERE " + exceptionTableItemAttribute + " =  :key AND " + exceptionTableForeignItemAttribute + " = :foreign_key");
                    deleteQuery.bindValue(":key", itemKey);
                    deleteQuery.bindValue(":foreign_key", foreignItemKey);
                    if (deleteQuery.exec()) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + exceptionTable + " (" + exceptionTableItemAttribute + ", " + exceptionTableForeignItemAttribute + ", " + exceptionTableValueAttribute + ") VALUES (:item, :foreign_item, :value)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        insertQuery.bindValue(":value", value);
                        if (!insertQuery.exec()) {
                            allInsertsSuccessful = false;
                            error("Failed to insert a " + foreignItem.singular + " into " + item.singular + " " + QString::number(id) + ": " + insertQuery.lastError().text());
                        }
                    } else {
                        error("Failed deleting old " + attributeName + " of " + item.singular + " " + QString::number(id) + ": " + deleteQuery.lastError().text());
                    }
                }
                if (allInsertsSuccessful) {
                    successfulIds.append(QString::number(id));
                }
            } else {
                error("Failed loading " + item.singular  + QString::number(id) + " because this " + item.singular + " wasn't found.");
            }
        } else {
            error("Failed loading " + item.singular + " " + QString::number(id) + ": " + keyQuery.lastError().text());
        }
    }
    QString foreignItemString = QString::number(foreignItemKeys.length()) + " " + foreignItem.plural;
    if (foreignItemKeys.length() == 1) {
        foreignItemString = foreignItem.singular + " " + QString::number(foreignItemKeys.first());
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " at " + foreignItemString + " to " + QString::number(foreignItemKeys.length()) + " " + foreignItem.plural + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + " to " + QString::number(foreignItemKeys.length()) + " " + foreignItem.plural + ".");
    }
    emit dbChanged();
}

void Terminal::updatePrompt() {
    Key itemType = Set;
    QList<Key> idKeys;
    for (const Key key : promptKeys) {
        if (itemKeys.contains(key)) {
            itemType = key;
            idKeys.clear();
        } else {
            idKeys.append(key);
        }
    }
    promptLabel->setText(keysToString(promptKeys));
    if (itemType != Set) {
        emit itemChanged(keysToString({ itemType }), keysToIds(idKeys));
    }
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
