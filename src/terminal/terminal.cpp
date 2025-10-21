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
    selectionIdKeys.append(selectionType);
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
    QMap<Key, QStringList> attributes;
    if (!attributeKeys.isEmpty()) {
        attributeKeys.append(Attribute);
        QList<Key> currentItemKeys;
        for (Key key : attributeKeys) {
            if (itemKeys.contains(key) || (key == Attribute)) {
                if (!currentItemKeys.isEmpty()) {
                    Key currentItemType = currentItemKeys.first();
                    QStringList ids = keysToIds(currentItemKeys);
                    if (ids.isEmpty()) {
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
    const QStringList ids = keysToIds(selectionIdKeys);
    if (ids.isEmpty()) {
        error("Invalid selection ID given.");
        return;
    }
    QString attribute;
    if (attributes.value(Attribute, QStringList()).size() == 0) {
        attribute = QString();
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
        } else if (attribute == AttributeIds::modelPanRange) {
            setNumberAttribute<float>(modelInfos, "panrange", "Pan Range", ids, valueKeys, "°", 0, 360, true);
        } else if (attribute == AttributeIds::modelTiltRange) {
            setNumberAttribute<float>(modelInfos, "tiltrange", "Tilt Range", ids, valueKeys, "°", 0, 360, false);
        } else if (attribute == AttributeIds::modelMinZoom) {
            setNumberAttribute<float>(modelInfos, "minzoom", "Universe", ids, valueKeys, "°", 0, 180, false);
        } else if (attribute == AttributeIds::modelMaxZoom) {
            setNumberAttribute<float>(modelInfos, "maxzoom", "Universe", ids, valueKeys, "°", 0, 180, false);
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
        } else if (attribute == AttributeIds::fixtureRotation) {
            setNumberAttribute(fixtureInfos, "rotation", "Rotation", ids, valueKeys, "°", 0, 360, true);
        } else if (attribute == AttributeIds::fixtureInvertPan) {
            setBoolAttribute(fixtureInfos, "invertpan", "Invert Pan", ids, valueKeys);
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
                setItemSpecificNumberAttribute<float>(intensityInfos, "Dimmer Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "intensity_model_dimmer", "intensity_key", "model_key", "dimmer", "%", 0, 100, false);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(intensityInfos, "Dimmer Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "intensity_fixture_dimmer", "intensity_key", "fixture_key", "dimmer", "%", 0, 100, false);
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
        } else if ((attribute == AttributeIds::colorHue) || !attributes.contains(Attribute)) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Hue Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "color_model_hue", "color_key", "model_key", "hue", "°", 0, 360, true);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Hue Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "color_fixture_hue", "color_key", "fixture_key", "hue", "°", 0, 360, true);
            } else {
                setNumberAttribute<float>(colorInfos, "hue", "Hue", ids, valueKeys, "°", 0, 360, true);
            }
        } else if (attribute == AttributeIds::colorSaturation) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Saturation Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "color_model_saturation", "color_key", "model_key", "saturation", "%", 0, 100, false);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Saturation Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "color_fixture_saturation", "color_key", "fixture_key", "saturation", "%", 0, 100, false);
            } else {
                setNumberAttribute<float>(colorInfos, "saturation", "Saturation", ids, valueKeys, "%", 0, 100, false);
            }
        } else if (attribute == AttributeIds::colorQuality) {
            setNumberAttribute<float>(colorInfos, "quality", "Quality", ids, valueKeys, "%", 0, 100, false);
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
        } else if ((attribute == AttributeIds::positionPan) || !attributes.contains(Attribute)) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Pan Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "position_model_pan", "position_key", "model_key", "pan", "°", 0, 360, true);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Pan Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "position_fixture_pan", "position_key", "fixture_key", "pan", "°", 0, 360, true);
            } else {
                setNumberAttribute<float>(positionInfos, "pan", "Pan", ids, valueKeys, "°", 0, 360, true);
            }
        } else if (attribute == AttributeIds::positionTilt) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Tilt Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "position_model_tilt", "position_key", "model_key", "tilt", "°", -180, 180, false);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Tilt Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "position_fixture_tilt", "position_key", "fixture_key", "tilt", "°", -180, 180, false);
            } else {
                setNumberAttribute<float>(positionInfos, "tilt", "Tilt", ids, valueKeys, "°", -180, 180, false);
            }
        } else if (attribute == AttributeIds::positionZoom) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Zoom Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "position_model_zoom", "position_key", "model_key", "zoom", "°", 0, 180, false);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Zoom Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "position_fixture_zoom", "position_key", "fixture_key", "zoom", "°", 0, 180, false);
            } else {
                setNumberAttribute<float>(positionInfos, "zoom", "Zoom", ids, valueKeys, "°", 0, 180, false);
            }
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
        } else if ((attribute == AttributeIds::effectSteps) || !attributes.contains(Attribute)) {
            setNumberAttribute<int>(effectInfos, "steps", "Steps", ids, valueKeys, "", 2, 99, false);
        } else {
            error("Unknown Effect Attribute.");
        }
    } else if (selectionType == Cuelist) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(cuelistInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(cuelistInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(cuelistInfos, "label", "Label", ids, "");
        } else if (attribute == AttributeIds::cuelistPriority) {
            setNumberAttribute<int>(cuelistInfos, "priority", "Priority", ids, valueKeys, "", 1, 200, false);
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
        } else if (attribute == AttributeIds::cueIntensities) {
            if (attributes.contains(Group)) {
                setItemSpecificItemAttribute(cueInfos, "Intensities", ids, attributes.value(Group), valueKeys, groupInfos, intensityInfos, "cue_intensities", "cue_key", "group_key", "intensity_key");
            } else {
                error("Can't set Cue Intensities because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueColors) {
            if (attributes.contains(Group)) {
                setItemSpecificItemAttribute(cueInfos, "Colors", ids, attributes.value(Group), valueKeys, groupInfos, colorInfos, "cue_colors", "cue_key", "group_key", "color_key");
            } else {
                error("Can't set Cue Colors because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cuePositions) {
            if (attributes.contains(Group)) {
                setItemSpecificItemAttribute(cueInfos, "Positions", ids, attributes.value(Group), valueKeys, groupInfos, positionInfos, "cue_positions", "cue_key", "group_key", "position_key");
            } else {
                error("Can't set Cue Positions because no Group Attribute was provided.");
            }
        } else {
            error("Unknown Cue Attribute.");
        }
    } else {
        error("Unknown Item type.");
        return;
    }
}

void Terminal::updateSortingKeys(const ItemInfos item) {
    QSqlQuery idsQuery;
    idsQuery.prepare("SELECT id FROM " + item.table);
    if (idsQuery.exec()) {
        QStringList ids;
        while (idsQuery.next()) {
            ids.append(idsQuery.value(0).toString());
        }
        std::sort(ids.begin(), ids.end(), [](const QString a, const QString b) -> bool {
            QStringList aParts = a.split(".");
            QStringList bParts = b.split(".");
            const int minPartAmount = std::min(aParts.length(), bParts.length());
            bool smallerId = true;
            bool sameBeginning = true;
            for (int part = 0; part < minPartAmount; part++) {
                if ((aParts[part].toInt() > bParts[part].toInt()) && sameBeginning) {
                    smallerId = false;
                }
                if (aParts[part].toInt() != bParts[part].toInt()) {
                    sameBeginning = false;
                }
            }
            return (smallerId && (!sameBeginning || (aParts.length() < bParts.length())));
        });
        for (int index = 1; index <= ids.length(); index++) {
            const QString id = ids.at(index - 1);
            QSqlQuery query;
            query.prepare("UPDATE " + item.table + " SET sortkey = :sortkey WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":sortkey", index);
            if (!query.exec()) {
                error("Failed to update the sorting key of " + item.singular + " " + id + ": " + query.lastError().text());
            }
        }
    } else {
        error("Failed to update the " + item.singular + " sorting keys: " + idsQuery.lastError().text());
    }
}

void Terminal::createItems(const ItemInfos item, QStringList ids) {
    Q_ASSERT(!ids.isEmpty());
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT key FROM " + item.table + " WHERE id = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec()) {
            if (!existsQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO " + item.table + " (id, sortkey) VALUES (:id, 0)");
                insertQuery.bindValue(":id", id);
                if (insertQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    error("Failed to insert " + item.singular + " " + id + ":" + insertQuery.lastError().text());
                }
            }
        } else {
            error("Failed to check if " + item.singular + " " + id + " already exists: " + existsQuery.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Created " + item.singular + " " + successfulIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Created " + item.plural + " " + successfulIds.join(", ") + ".");
    }
    updateSortingKeys(item);
}

void Terminal::deleteItems(const ItemInfos item, QStringList ids) {
    Q_ASSERT(!ids.isEmpty());
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT sortkey FROM " + item.table + " WHERE id = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec()) {
            if (existsQuery.next()) {
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + item.table + " WHERE id = :id");
                deleteQuery.bindValue(":id", id);
                if (deleteQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    error("Can't delete " + item.singular + " because the request failed: " + deleteQuery.lastError().text());
                }
            } else {
                warning("Can't delete " + item.singular + " " + id + " because this " + item.singular + " doesn't exist.");
            }
        } else {
            error("Couldn't delete " + item.singular + " " + id + ": " + existsQuery.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Deleted " + item.singular + " " + successfulIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Deleted " + item.plural + " " + successfulIds.join(", ") + ".");
    }
    updateSortingKeys(item);
    emit dbChanged();
}

void Terminal::moveItems(const ItemInfos item, QStringList ids, QList<Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    valueKeys.prepend(item.key);
    QStringList newIds = keysToIds(valueKeys);
    if (newIds.size() != 1) {
        error("Can't set " + item.singular + " ID because an invalid ID was given.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT id FROM " + item.table + " WHERE id = :id");
        existsQuery.bindValue(":id", newIds.first());
        if (existsQuery.exec()) {
            if (existsQuery.next()) {
                warning("Can't set ID of " + item.singular + " to " + newIds.first() + " because this " + item.singular + " ID is already used.");
            } else {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE " + item.table + " SET id = :newId WHERE id = :id");
                updateQuery.bindValue(":id", id);
                updateQuery.bindValue(":newId", newIds.first());
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    error("Failed to update ID of " + item.singular + " " + id + " because the request failed: " + updateQuery.lastError().text());
                }
            }
        } else {
            error("Error executing check if " + item.singular + " " + newIds.first() + " exists: " + existsQuery.lastError().text());
        }
    }
    if (successfulIds.size() == 1) {
        success("Set ID of " + item.singular + " " + successfulIds.first() + " to " + newIds.first() + ".");
    } else if (successfulIds.size() > 1) {
        success("Set ID of " + item.plural + " " + successfulIds.join(", ") + " to " + newIds.first() + ".");
    }
    updateSortingKeys(item);
    emit dbChanged();
}

void Terminal::setBoolAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QStringList ids, QList<Key> valueKeys) {
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
    for (QString id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + item.table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", value);
        if (query.exec()) {
            successfulIds.append(id);
        } else {
            error("Failed setting " + attributeName + " of " + item.singular + " " + id + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to " + valueText + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + valueText + ".");
    }
    emit dbChanged();
}

void Terminal::setTextAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QStringList ids, const QString regex) {
    Q_ASSERT(!ids.isEmpty());
    QString textValue = QString();
    if (ids.length() == 1) {
        QSqlQuery query;
        query.prepare("SELECT " + attribute + " FROM " + item.table + " WHERE id = :id");
        query.bindValue(":id", ids.first());
        if (!query.exec()) {
            error("Failed to load current " + attributeName + " of " + item.singular + " " + ids.first() + ": " + query.lastError().text());
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
    for (QString id : ids) {
        QSqlQuery query;
        query.prepare("UPDATE " + item.table + " SET " + attribute + " = :value WHERE id = :id");
        query.bindValue(":id", id);
        query.bindValue(":value", textValue);
        if (query.exec()) {
            successfulIds.append(id);
        } else {
            error("Failed setting " + attributeName + " of " + item.singular + " " + id + ": " + query.lastError().text());
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to \"" + textValue + "\".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to \"" + textValue + "\".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setNumberAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QStringList ids, QList<Key> valueKeys, const QString unit, const T minValue, const T maxValue, const bool cyclic) {
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
    for (QString id : ids) {
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
                            error(attributeName + " of " + item.singular + " " + id + " has to be between " + QString::number(minValue) + " and " + QString::number(maxValue) + ".");
                            valueOk = false;
                        }
                    }
                } else {
                    error("Failed loading the current " + attributeName + " of " + item.singular + " " + id + " because this " + item.singular + " doesn't exist.");
                    valueOk = false;
                }
            } else {
                error("Failed loading the current " + attributeName + " of " + item.singular + " " + id + ": " + currentValueQuery.lastError().text());
                valueOk = false;
            }
        }
        if (valueOk) {
            QSqlQuery query;
            query.prepare("UPDATE " + item.table + " SET " + attribute + " = :value WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":value", currentValue);
            if (query.exec()) {
                successfulIds.append(id);
            } else {
                error("Failed setting " + attributeName + " of " + item.singular + " " + id + ": " + query.lastError().text());
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

void Terminal::setItemAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QStringList ids, QList<Key> valueKeys, const ItemInfos foreignItem) {
    Q_ASSERT(!ids.isEmpty());
    if ((valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
        QStringList successfulIds;
        for (QString id : ids) {
            QSqlQuery query;
            query.prepare("UPDATE " + item.table + " SET " + attribute + " = NULL WHERE id = :id");
            query.bindValue(":id", id);
            if (query.exec()) {
                successfulIds.append(id);
            } else {
                error("Failed removing " + attributeName + " of " + item.singular + " " + id + ": " + query.lastError().text());
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
        QStringList foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.size() != 1) {
            error("Can't set " + item.singular + " " + attributeName + " because the given " + foreignItem.singular + " ID is invalid.");
            return;
        }
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.table + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemIds.first());
        if (!foreignItemQuery.exec()) {
            error("Failed to execute check if " + foreignItem.singular + " exists: " + foreignItemQuery.lastError().text());
            return;
        }
        if (!foreignItemQuery.next()) {
            error("Can't set " + item.singular + " " + attributeName + " because " + foreignItem.singular + " " + foreignItemIds.first() + " doesn't exist.");
            return;
        }
        const int foreignItemKey = foreignItemQuery.value(0).toInt();
        createItems(item, ids);
        QStringList successfulIds;
        for (QString id : ids) {
            QSqlQuery query;
            query.prepare("UPDATE " + item.table + " SET " + attribute + " = :item WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":item", foreignItemKey);
            if (query.exec()) {
                successfulIds.append(id);
            } else {
                error("Failed setting " + attributeName + " of " + item.singular + " " + id + ": " + query.lastError().text());
            }
        }
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to " + foreignItem.singular + " " + foreignItemIds.first() + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + foreignItem.singular + " " + foreignItemIds.first() + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemListAttribute(const ItemInfos item, const QString attributeName, QStringList ids, QList<Key> valueKeys, const ItemInfos foreignItem, const QString listTable, const QString listTableItemAttribute, const QString listTableForeignItemsAttribute) {
    Q_ASSERT(!ids.isEmpty());
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    if ((valueKeys.size() != 1) || !valueKeys.startsWith(Minus)) {
        if (!valueKeys.startsWith(foreignItem.key)) {
            error("Can't set " + item.singular + " " + attributeName + " because no " + foreignItem.plural + " were given.");
            return;
        }
        const QStringList foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.isEmpty()) {
            error("Can't set " + item.singular + " " + attributeName + " because the given " + foreignItem.singular + " IDs are invalid.");
            return;
        }
        for (QString foreignItemId : foreignItemIds) {
            QSqlQuery foreignItemQuery;
            foreignItemQuery.prepare("SELECT key FROM " + foreignItem.table + " WHERE id = :id");
            foreignItemQuery.bindValue(":id", foreignItemId);
            if (foreignItemQuery.exec()) {
                if (foreignItemQuery.next()) {
                    foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                    foreignItemIdStrings.append(foreignItemId);
                } else {
                    warning("Can't add " + foreignItem.singular + " " + foreignItemId + " to " + item.singular + " " + attributeName + " because this " + foreignItem.singular + " doesn't exist.");
                }
            } else {
                error("Failed to execute check if " + foreignItem.singular + " " + foreignItemId + " exists: " + foreignItemQuery.lastError().text());
            }
        }
        Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
        if (foreignItemKeys.isEmpty()) {
            error("Can't set " + item.singular + " " + attributeName + " because no valid " + foreignItem.plural + " were given.");
            return;
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
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
                            successfulIds.append(id);
                        } else {
                            error("Failed to insert a " + foreignItem.singular + " into " + item.singular + " " + id + ": " + insertQuery.lastError().text());
                        }
                    }
                } else {
                    error("Failed deleting old " + attributeName + " of " + item.singular + " " + id + ": " + deleteQuery.lastError().text());
                }
            } else {
                error("Failed loading " + item.singular  + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            error("Failed loading " + item.singular + " " + id + ": " + keyQuery.lastError().text());
        }
    }
    QString foreignItemString = foreignItem.plural + foreignItemIdStrings.join(", ");
    if (foreignItemIdStrings.size() == 1) {
        foreignItemString = foreignItem.singular + foreignItemIdStrings.join(", ");
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " to " + foreignItemString + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + foreignItemString + ".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setItemSpecificNumberAttribute(const ItemInfos item, const QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Key> valueKeys, const ItemInfos foreignItem, const QString exceptionTable, const QString exceptionTableItemAttribute, const QString exceptionTableForeignItemAttribute, const QString exceptionTableValueAttribute, const QString unit, const T minValue, const T maxValue, const bool cyclic) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(!foreignItemIds.isEmpty());
    const bool removeValues = ((valueKeys.size() == 1) && valueKeys.startsWith(Minus));
    T value;
    if (!removeValues) {
        bool ok;
        value = keysToFloat(valueKeys, &ok);
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
    }
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    for (QString foreignItemId : foreignItemIds) {
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.table + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.singular + " " + foreignItemId + " because this " + foreignItem.singular + " doesn't exist.");
            }
        } else {
            error("Failed to execute check if " + foreignItem.singular + " " + foreignItemId + " exists: " + foreignItemQuery.lastError().text());
        }
    }
    Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
    if (foreignItemKeys.isEmpty()) {
        error("No valid " + foreignItem.plural + " were found.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.table + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                for (int foreignItemKey : foreignItemKeys) {
                    QSqlQuery query;
                    if (removeValues) {
                        query.prepare("DELETE FROM " + exceptionTable + " WHERE " + exceptionTableItemAttribute + " = :item AND " + exceptionTableForeignItemAttribute + " = :foreign_item");
                    } else {
                        query.prepare("INSERT OR REPLACE INTO " + exceptionTable + " (" + exceptionTableItemAttribute + ", " + exceptionTableForeignItemAttribute + ", " + exceptionTableValueAttribute + ") VALUES (:item, :foreign_item, :value)");
                        query.bindValue(":value", value);
                    }
                    query.bindValue(":item", itemKey);
                    query.bindValue(":foreign_item", foreignItemKey);
                    if (!query.exec()) {
                        allQueriesSuccessful = false;
                        error("Failed updating " + attributeName + " of " + item.singular + " " + id + ": " + query.lastError().text());
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.singular  + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            error("Failed loading " + item.singular + " " + id + ": " + keyQuery.lastError().text());
        }
    }
    QString foreignItemString = foreignItem.plural + " " + foreignItemIdStrings.join(", ");
    if (foreignItemIdStrings.length() == 1) {
        foreignItemString = foreignItem.singular + " " + foreignItemIdStrings.join(", ");
    }
    if (removeValues) {
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.first() + " at " + foreignItemString + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + ".");
        }
    } else {
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " at " + foreignItemString + " to " + QString::number(value) + unit + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + " to " + QString::number(value) + unit + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemSpecificItemAttribute(const ItemInfos item, const QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Key> valueKeys, const ItemInfos foreignItem, const ItemInfos valueItem, const QString valueTable, const QString valueTableItemAttribute, const QString valueTableForeignItemAttribute, const QString valueTableValueAttribute) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(!foreignItemIds.isEmpty());
    const bool removeValues = ((valueKeys.size() == 1) && valueKeys.startsWith(Minus));
    QString valueItemId;
    int valueItemKey;
    if (!removeValues) {
        if (!valueKeys.startsWith(valueItem.key)) {
            error("Can't set " + item.singular + " " + attributeName + " because no valid " + valueItem.singular + " was given.");
            return;
        }
        QStringList valueIds = keysToIds(valueKeys);
        if (valueIds.size() != 1) {
            error("Can't set " + item.singular + " " + attributeName + " because no valid " + valueItem.singular + " was given.");
            return;
        }
        valueItemId = valueIds.first();
        QSqlQuery valueItemQuery;
        valueItemQuery.prepare("SELECT key FROM " + valueItem.table + " WHERE id = :id");
        valueItemQuery.bindValue(":id", valueItemId);
        if (!valueItemQuery.exec()) {
            error("Failed to execute check if " + valueItem.singular + " exists: " + valueItemQuery.lastError().text());
            return;
        }
        if (!valueItemQuery.next()) {
            error("Can't set " + item.singular + " " + attributeName + " because " + valueItem.singular + " " + valueItemId + " doesn't exist.");
            return;
        }
        valueItemKey = valueItemQuery.value(0).toInt();
    }
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    for (QString foreignItemId : foreignItemIds) {
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.table + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.singular + " " + foreignItemId + " because this " + foreignItem.singular + " doesn't exist.");
            }
        } else {
            error("Failed to execute check if " + foreignItem.singular + " " + foreignItemId + " exists: " + foreignItemQuery.lastError().text());
        }
    }
    Q_ASSERT(foreignItemKeys.size() == foreignItemIdStrings.size());
    if (foreignItemKeys.isEmpty()) {
        error("No valid " + foreignItem.plural + " were found.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.table + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                for (int foreignItemKey : foreignItemKeys) {
                    QSqlQuery query;
                    if (removeValues) {
                        query.prepare("DELETE FROM " + valueTable + " WHERE " + valueTableItemAttribute + " =  :item AND " + valueTableForeignItemAttribute + " = :foreign_item");
                    } else {
                        query.prepare("INSERT OR REPLACE INTO " + valueTable + " (" + valueTableItemAttribute + ", " + valueTableForeignItemAttribute + ", " + valueTableValueAttribute + ") VALUES (:item, :foreign_item, :value_item)");
                        query.bindValue(":value_item", valueItemKey);
                    }
                    query.bindValue(":item", itemKey);
                    query.bindValue(":foreign_item", foreignItemKey);
                    if (!query.exec()) {
                        allQueriesSuccessful = false;
                        error("Failed updating old " + attributeName + " of " + item.singular + " " + id + ": " + query.lastError().text());
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.singular  + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            error("Failed loading " + item.singular + " " + id + ": " + keyQuery.lastError().text());
        }
    }
    QString foreignItemString = foreignItem.plural + " " + foreignItemIdStrings.join(", ");
    if (foreignItemKeys.length() == 1) {
        foreignItemString = foreignItem.singular + " " + foreignItemIdStrings.join(", ");
    }
    if (removeValues) {
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.first() + " at " + foreignItemString + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + ".");
        }
    } else {
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.first() + " at " + QString::number(foreignItemKeys.length()) + " " + foreignItemString + " to " + valueItem.singular + " " + valueItemId + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + QString::number(foreignItemKeys.length()) + " " + foreignItemString + " to " + valueItem.singular + " " + valueItemId + ".");
        }
    }
    emit dbChanged();
}

void Terminal::updatePrompt() {
    QList<Key> idKeys;
    bool append = true;
    for (const Key key : promptKeys) {
        if (itemKeys.contains(key)) {
            append = true;
            idKeys.clear();
        } else if ((key == Attribute) || (key == Set)) {
            append = false;
        }
        if (append) {
            idKeys.append(key);
        }
    }
    promptLabel->setText(keysToString(promptKeys));
    if (!idKeys.isEmpty()) {
        emit itemChanged(keysToString({ idKeys.first() }), keysToIds(idKeys));
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

QStringList Terminal::keysToIds(QList<Key> keys) const {
    if (keys.isEmpty() || !(itemKeys.contains(keys.first()) || (keys.first() == Attribute))) {
        return QStringList();
    }
    keys.removeFirst();
    keys.append(Plus);
    QStringList ids;
    QStringList idParts;
    QList<Key> currentIdPartKeys;
    for (const Key key : keys) {
        if ((key == Plus) || (key == Period)) {
            bool ok;
            const int idPart = keysToString(currentIdPartKeys).toInt(&ok);
            if (!ok || (idPart < 0)) {
                return QStringList();
            }
            idParts.append(QString::number(idPart));
            currentIdPartKeys.clear();
        } else {
            currentIdPartKeys.append(key);
        }
        if (key == Plus) {
            if (idParts.isEmpty()) {
                return QStringList();
            }
            ids.append(idParts.join("."));
            idParts.clear();
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
