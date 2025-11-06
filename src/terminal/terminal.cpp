/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "terminal.h"

Terminal::Terminal(QWidget *parent) : QWidget(parent) {
    settings = new QSettings("Zoeglfrex");

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
    keyStrings[Thru] = " Thru ";
    keyStrings[Period] = ".";

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);
    QHBoxLayout* promptLayout = new QHBoxLayout();
    layout->addLayout(promptLayout);

    promptLabel = new QLabel();
    promptLabel->setWordWrap(true);
    promptLabel->setStyleSheet("padding: 10px; background-color: #303030;");
    promptLayout->addWidget(promptLabel);

    blindButton = new QPushButton("Blind");
    blindButton->setCheckable(true);
    connect(blindButton, &QPushButton::clicked, this, [this] {
        QSqlQuery query;
        if (blindButton->isChecked()) {
            query.prepare("UPDATE currentitems SET cue_key = (SELECT cuelists.currentcue_key FROM cuelists, currentitems WHERE cuelists.key = currentitems.cuelist_key)");
        } else {
            query.prepare("UPDATE currentitems SET cue_key = NULL");
        }
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
        emit dbChanged();
    });
    promptLayout->addWidget(blindButton);

    trackingButton = new QPushButton("Tracking");
    trackingButton->setCheckable(true);
    connect(trackingButton, &QPushButton::clicked, this, [this] {
        settings->setValue("cuelistview/tracking", trackingButton->isChecked());
    });
    trackingButton->setChecked(settings->value("cuelistview/tracking", true).toBool());
    promptLayout->addWidget(trackingButton);

    messages = new QPlainTextEdit();
    messages->setReadOnly(true);
    layout->addWidget(messages);

    new QShortcut(Qt::Key_0, this, [this] { writeKey(Zero); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_1, this, [this] { writeKey(One); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_2, this, [this] { writeKey(Two); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_3, this, [this] { writeKey(Three); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_4, this, [this] { writeKey(Four); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_5, this, [this] { writeKey(Five); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_6, this, [this] { writeKey(Six); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_7, this, [this] { writeKey(Seven); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_8, this, [this] { writeKey(Eight); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_9, this, [this] { writeKey(Nine); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::Key_Plus, this, [this] { writeKey(Plus); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Minus, this, [this] { writeKey(Minus); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_T, this, [this] { writeKey(Thru); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Period, this, [this] { writeKey(Period); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Comma, this, [this] { writeKey(Period); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_A, this, [this] { writeKey(Attribute); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_S, this, [this] { writeKey(Set); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::Key_M, this, [this] { writeKey(Model); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_F, this, [this] { writeKey(Fixture); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_G, this, [this] { writeKey(Group); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_I, this, [this] { writeKey(Intensity); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_C, this, [this] { writeKey(Color); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_P, this, [this] { writeKey(Position); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_R, this, [this] { writeKey(Raw); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_E, this, [this] { writeKey(Effect); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_L, this, [this] { writeKey(Cuelist); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Q, this, [this] { writeKey(Cue); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::Key_Backspace, this, [this] { backspace(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_Backspace, this, [this] { clearPrompt(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Enter, this, [this] { execute(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Return, this, [this] { execute(); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::SHIFT | Qt::Key_B, this, [this] { blindButton->click(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_T, this, [this] { trackingButton->click(); }, Qt::ApplicationShortcut);
}

void Terminal::execute() {
    QList<Key> keys = promptKeys;
    if (keys.isEmpty()) {
        return;
    }
    info("> " + keysToString(keys));
    clearPrompt();

    const Key selectionType = keys.first();
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
    if (!attributeReached && !valueReached) {
        auto setCurrentItem = [this](const ItemInfos item, const QString itemTable, const QList<Key> idKeys, const QString updateQueryText) {
            const QStringList ids = keysToIds(idKeys);
            if (ids.size() != 1) {
                error("Invalid " + item.singular + " selection given.");
                return;
            }
            QSqlQuery keyQuery;
            keyQuery.prepare("SELECT key FROM " + itemTable + " WHERE id = :id");
            keyQuery.bindValue(":id", ids.first());
            if (!keyQuery.exec()) {
                qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                error("Can't select " + item.singular + " because the key request for " + item.singular + " " + ids.first() + " failed.");
                return;
            }
            if (!keyQuery.next()) {
                error("Can't select " + item.singular + " " + ids.first() + ".");
                return;
            }
            const int key = keyQuery.value(0).toInt();
            QSqlQuery updateQuery;
            updateQuery.prepare(updateQueryText);
            updateQuery.bindValue(":key", key);
            if (!updateQuery.exec()) {
                qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                error("Failed to select " + item.singular + ".");
            }
            emit dbChanged();
        };
        auto setCueItem = [this] (const ItemInfos item, const QString valueTable, const QList<Key> idKeys, const bool multipleItemsAllowed) {
            QList<int> itemKeys;
            if (((idKeys.size() != 2) || !idKeys.endsWith(Minus))) {
                const QStringList ids = keysToIds(idKeys);
                if (ids.isEmpty()) {
                    error("Invalid " + item.singular + " selection given.");
                    return;
                }
                if (!multipleItemsAllowed && (ids.size() > 1)) {
                    error("Can't select multiple " + item.plural + ".");
                    return;
                }
                for (QString id : ids) {
                    QSqlQuery itemKeyQuery;
                    itemKeyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
                    itemKeyQuery.bindValue(":id", id);
                    if (itemKeyQuery.exec()) {
                        if (itemKeyQuery.next()) {
                            itemKeys.append(itemKeyQuery.value(0).toInt());
                        } else {
                            warning("Can't select " + item.singular + " " + id + " because this " + item.singular + " doesn't exist.");
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << itemKeyQuery.executedQuery() << itemKeyQuery.lastError().text();
                        error("Can't set Cue " + item.plural + " because the key request for " + item.singular + " " + ids.first() + " failed.");
                    }
                }
                if (itemKeys.isEmpty()) {
                    error("No valid " + item.plural + " were given.");
                    return;
                }
            }
            QSqlQuery groupKeyQuery;
            if (!groupKeyQuery.exec("SELECT group_key FROM currentitems WHERE group_key IS NOT NULL")) {
                qWarning() << Q_FUNC_INFO << groupKeyQuery.executedQuery() << groupKeyQuery.lastError().text();
                error("Can't set Cue " + item.plural + " because request for the current Group failed.");
                return;
            }
            if (!groupKeyQuery.next()) {
                error("Can't set Cue " + item.plural + " because no Group is currently selected.");
                return;
            }
            const int groupKey = groupKeyQuery.value(0).toInt();
            QList<int> cueKeys;
            QSqlQuery cueKeyQuery;
            if (!cueKeyQuery.exec("SELECT key FROM currentcue")) {
                qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
                error("Can't set Cue " + item.plural + " because the request for the current Cue failed.");
                return;
            }
            if (!cueKeyQuery.next()) {
                error("Can't set Cue " + item.plural + " because no Cue is currently selected.");
                return;
            }
            cueKeys.append(cueKeyQuery.value(0).toInt());
            if (trackingButton->isChecked()) {
                QSqlQuery currentCueValueQuery;
                currentCueValueQuery.prepare("SELECT valueitem_key FROM " + valueTable + " WHERE item_key = :cue AND foreignitem_key = :group");
                currentCueValueQuery.bindValue(":cue", cueKeys.first());
                currentCueValueQuery.bindValue(":group", groupKey);
                if (currentCueValueQuery.exec()) {
                    QSet<int> currentCueValueKeys;
                    while (currentCueValueQuery.next()) {
                        currentCueValueKeys.insert(currentCueValueQuery.value(0).toInt());
                    }
                    QSqlQuery cueTrackingKeyQuery;
                    cueTrackingKeyQuery.prepare("SELECT key, block FROM currentcuelist_cues WHERE sortkey > :sortkey ORDER BY sortkey");
                    cueTrackingKeyQuery.bindValue(":sortkey", cueKeyQuery.value(1).toInt());
                    if (cueTrackingKeyQuery.exec()) {
                        bool sameValue = true;
                        while (cueTrackingKeyQuery.next() && (cueTrackingKeyQuery.value(1).toInt() == 0) && sameValue) {
                            const int cueKey = cueTrackingKeyQuery.value(0).toInt();
                            QSqlQuery cueValueQuery;
                            cueValueQuery.prepare("SELECT valueitem_key FROM " + valueTable + " WHERE item_key = :cue AND foreignitem_key = :group");
                            cueValueQuery.bindValue(":cue", cueKey);
                            cueValueQuery.bindValue(":group", groupKey);
                            if (cueValueQuery.exec()) {
                                QSet<int> cueValueKeys;
                                while (cueValueQuery.next()) {
                                    cueValueKeys.insert(cueValueQuery.value(0).toInt());
                                }
                                sameValue = (cueValueKeys == currentCueValueKeys);
                                if (sameValue) {
                                    cueKeys.append(cueKey);
                                }
                            } else {
                                qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
                                error("Can't set Cue " + item.plural + " because the Cue tracking request failed.");
                                return;
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
                        error("Can't set Cue " + item.plural + " because the Cue tracking request failed.");
                        return;
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
                    error("Can't set Cue " + item.plural + " because the request for the " + item.singular + " in the current Cue failed.");
                    return;
                }
            }
            for (const int cueKey : cueKeys) {
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key = :cue AND foreignitem_key = :group");
                deleteQuery.bindValue(":cue", cueKey);
                deleteQuery.bindValue(":group", groupKey);
                if (!deleteQuery.exec()) {
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    error("Failed deleting Cue " + item.plural + ".");
                }
                for (const int key : itemKeys) {
                    QSqlQuery query;
                    query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignitem_key, valueitem_key) VALUES (:cue, :group, :item)");
                    query.bindValue(":cue", cueKey);
                    query.bindValue(":group", groupKey);
                    query.bindValue(":item", key);
                    if (!query.exec()) {
                        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                        error("Failed inserting " + item.singular + ".");
                    }
                }
            }
            emit dbChanged();
        };
        if (selectionType == Fixture) {
            setCurrentItem(fixtureInfos, "currentgroup_fixtures", selectionIdKeys, "UPDATE currentitems SET fixture_key = :key");
        } else if (selectionType == Group) {
            setCurrentItem(groupInfos, groupInfos.selectTable, selectionIdKeys, "UPDATE currentitems SET group_key = :key");
        } else if (selectionType == Intensity) {
            setCueItem(intensityInfos, "cue_group_intensities", selectionIdKeys, false);
        } else if (selectionType == Color) {
            setCueItem(colorInfos, "cue_group_colors", selectionIdKeys, false);
        } else if (selectionType == Position) {
            setCueItem(positionInfos, "cue_group_positions", selectionIdKeys, false);
        } else if (selectionType == Raw) {
            setCueItem(rawInfos, "cue_group_raws", selectionIdKeys, true);
        } else if (selectionType == Effect) {
            setCueItem(effectInfos, "cue_group_effects", selectionIdKeys, true);
        } else if (selectionType == Cuelist) {
            setCurrentItem(cuelistInfos, cuelistInfos.selectTable, selectionIdKeys, "UPDATE currentitems SET cuelist_key = :key");
        } else if (selectionType == Cue) {
            QSqlQuery currentCueQuery;
            if (currentCueQuery.exec("SELECT cue_key FROM currentitems WHERE cue_key IS NOT NULL")) {
                if (currentCueQuery.next()) {
                    setCurrentItem(cueInfos, cueInfos.selectTable, selectionIdKeys, "UPDATE currentitems SET cue_key = :key");
                } else {
                    setCurrentItem(cueInfos, cueInfos.selectTable, selectionIdKeys, "UPDATE cuelists SET currentcue_key = :key WHERE key = (SELECT cuelist_key FROM currentitems)");
                }
            } else {
                qWarning() << Q_FUNC_INFO << currentCueQuery.executedQuery() << currentCueQuery.lastError().text();
            }
        } else {
            error("Can't select this Item type: " + keysToString({selectionType}));
        }
        return;
    }
    const QStringList ids = keysToIds(selectionIdKeys);
    if (ids.isEmpty()) {
        error("Invalid selection ID given.");
        return;
    }
    QMap<Key, QStringList> attributes;
    if (!attributeKeys.isEmpty()) {
        attributeKeys.append(Attribute);
        QList<Key> currentItemKeys;
        for (const Key key : attributeKeys) {
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
    QString attribute;
    if (attributes.value(Attribute, QStringList()).size() == 0) {
        attribute = QString();
    } else if (attributes.value(Attribute).size() == 1) {
        attribute = attributes.value(Attribute).first();
    } else {
        error("Invalid number of Attribute IDs given.");
        return;
    }

    if (selectionType == Model) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(modelInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(modelInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(modelInfos, "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::modelChannels) || !attributes.contains(Attribute)) {
            setTextAttribute(modelInfos, "channels", "Channels", ids, "^[01DdRrGgBbWwCcMmYyPpTtZzFf]+$");
        } else if (attribute == AttributeIds::modelPanRange) {
            setNumberAttribute<float>(modelInfos, "panrange", "Pan Range", ids, valueKeys, {0, 3600, false, "°"});
        } else if (attribute == AttributeIds::modelTiltRange) {
            setNumberAttribute<float>(modelInfos, "tiltrange", "Tilt Range", ids, valueKeys, {0, 360, false, "°"});
        } else if (attribute == AttributeIds::modelMinZoom) {
            setNumberAttribute<float>(modelInfos, "minzoom", "Universe", ids, valueKeys, {0, 180, false, "°"});
        } else if (attribute == AttributeIds::modelMaxZoom) {
            setNumberAttribute<float>(modelInfos, "maxzoom", "Universe", ids, valueKeys, {0, 180, false, "°"});
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
            setNumberAttribute<int>(fixtureInfos, "universe", "Universe", ids, valueKeys, {1, 63999});
        } else if ((attribute == AttributeIds::fixtureAddress) || !attributes.contains(Attribute)) {
            setNumberAttribute<int>(fixtureInfos, "address", "Address", ids, valueKeys, {0, 512});
        } else if (attribute == AttributeIds::fixtureXPosition) {
            setNumberAttribute<float>(fixtureInfos, "xposition", "X Position", ids, valueKeys, {-100, 100});
        } else if (attribute == AttributeIds::fixtureYPosition) {
            setNumberAttribute<float>(fixtureInfos, "yposition", "Y Position", ids, valueKeys, {-100, 100});
        } else if (attribute == AttributeIds::fixtureRotation) {
            setNumberAttribute<float>(fixtureInfos, "rotation", "Rotation", ids, valueKeys, angleInfos);
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
            setItemListAttribute(groupInfos, "Fixtures", ids, valueKeys, fixtureInfos, "group_fixtures");
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
                setItemSpecificNumberAttribute<float>(intensityInfos, "Dimmer Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "intensity_model_dimmer", percentageInfos);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(intensityInfos, "Dimmer Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "intensity_fixture_dimmer", percentageInfos);
            } else {
                setNumberAttribute<float>(intensityInfos, "dimmer", "Dimmer", ids, valueKeys, percentageInfos);
            }
        } else if (attribute == AttributeIds::intensityRaws) {
            setItemListAttribute(intensityInfos, "Raws", ids, valueKeys, rawInfos, "intensity_raws");
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
                setItemSpecificNumberAttribute<float>(colorInfos, "Hue Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "color_model_hue", angleInfos);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Hue Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "color_fixture_hue", angleInfos);
            } else {
                setNumberAttribute<float>(colorInfos, "hue", "Hue", ids, valueKeys, angleInfos);
            }
        } else if (attribute == AttributeIds::colorSaturation) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Saturation Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "color_model_saturation", percentageInfos);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Saturation Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "color_fixture_saturation", percentageInfos);
            } else {
                setNumberAttribute<float>(colorInfos, "saturation", "Saturation", ids, valueKeys, percentageInfos);
            }
        } else if (attribute == AttributeIds::colorQuality) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Quality Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "color_model_quality", percentageInfos);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(colorInfos, "Quality Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "color_fixture_quality", percentageInfos);
            } else {
                setNumberAttribute<float>(colorInfos, "quality", "Quality", ids, valueKeys, percentageInfos);
            }
        } else if (attribute == AttributeIds::colorRaws) {
            setItemListAttribute(colorInfos, "Raws", ids, valueKeys, rawInfos, "color_raws");
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
                setItemSpecificNumberAttribute<float>(positionInfos, "Pan Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "position_model_pan", angleInfos);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Pan Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "position_fixture_pan", angleInfos);
            } else {
                setNumberAttribute<float>(positionInfos, "pan", "Pan", ids, valueKeys, angleInfos);
            }
        } else if (attribute == AttributeIds::positionTilt) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Tilt Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "position_model_tilt", {-180, 180, false, "°"});
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Tilt Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "position_fixture_tilt", {-180, 180, false, "°"});
            } else {
                setNumberAttribute<float>(positionInfos, "tilt", "Tilt", ids, valueKeys, {-180, 180, false, "°"});
            }
        } else if (attribute == AttributeIds::positionZoom) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Zoom Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "position_model_zoom", {0, 180, false, "°"});
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Zoom Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "position_fixture_zoom", {0, 180, false, "°"});
            } else {
                setNumberAttribute<float>(positionInfos, "zoom", "Zoom", ids, valueKeys, {0, 180, false, "°"});
            }
        } else if (attribute == AttributeIds::positionFocus) {
            if (attributes.contains(Model)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Focus Model Exception", ids, attributes.value(Model), valueKeys, modelInfos, "position_model_focus", percentageInfos);
            } else if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(positionInfos, "Focus Fixture Exception", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "position_fixture_focus", percentageInfos);
            } else {
                setNumberAttribute<float>(positionInfos, "focus", "Focus", ids, valueKeys, percentageInfos);
            }
        } else if (attribute == AttributeIds::positionRaws) {
            setItemListAttribute(positionInfos, "Raws", ids, valueKeys, rawInfos, "position_raws");
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
        } else if (attribute.startsWith(QString(AttributeIds::rawChannelValues) + ".")) {
            if (attributes.contains(Model)) {
                setItemAndIntegerSpecificNumberAttribute<int>(rawInfos, "Channel Values", ids, attributes.value(Model), attribute, valueKeys, modelInfos, "raw_model_channel_values", {1, 512}, {0, 255});
            } else if (attributes.contains(Fixture)) {
                setItemAndIntegerSpecificNumberAttribute<int>(rawInfos, "Channel Values", ids, attributes.value(Fixture), attribute, valueKeys, fixtureInfos, "raw_fixture_channel_values", {1, 512}, {0, 255});
            } else {
                setIntegerSpecificNumberAttribute<int>(rawInfos, "Channel Values", ids, attribute, valueKeys, "raw_channel_values", {1, 512}, {0, 255});
            }
        } else if (attribute == AttributeIds::rawMoveWhileDark) {
            setBoolAttribute(rawInfos, "movewhiledark", "Move while Dark", ids, valueKeys);
        } else if (attribute == AttributeIds::rawFade) {
            setBoolAttribute(rawInfos, "fade", "Fade", ids, valueKeys);
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
            setNumberAttribute<int>(effectInfos, "steps", "Steps", ids, valueKeys, {2, 99});
        } else if (attribute.startsWith(QString(AttributeIds::effectIntensities) + ".")) {
            setIntegerSpecificItemListAttribute(effectInfos, "Intensities", ids, attribute, valueKeys, intensityInfos, "effect_step_intensities", {1, 99}, true);
        } else if (attribute.startsWith(QString(AttributeIds::effectColors) + ".")) {
            setIntegerSpecificItemListAttribute(effectInfos, "Colors", ids, attribute, valueKeys, colorInfos, "effect_step_colors", {1, 99}, true);
        } else if (attribute.startsWith(QString(AttributeIds::effectPositions) + ".")) {
            setIntegerSpecificItemListAttribute(effectInfos, "Positions", ids, attribute, valueKeys, positionInfos, "effect_step_positions", {1, 99}, true);
        } else if (attribute.startsWith(QString(AttributeIds::effectRaws) + ".")) {
            setIntegerSpecificItemListAttribute(effectInfos, "Raws", ids, attribute, valueKeys, rawInfos, "effect_step_raws", {1, 99});
        } else if (attribute == AttributeIds::effectHold) {
            setNumberAttribute<float>(effectInfos, "hold", "Hold", ids, valueKeys, {0, 600, false, "s"});
        } else if (attribute.startsWith(QString(AttributeIds::effectHold) + ".")) {
            setIntegerSpecificNumberAttribute<float>(effectInfos, "Hold", ids, attribute, valueKeys, "effect_step_hold", {1, 99}, {0, 600, false, "s"});
        } else if (attribute == AttributeIds::effectFade) {
            setNumberAttribute<float>(effectInfos, "fade", "Fade", ids, valueKeys, {0, 600, false, "s"});
        } else if (attribute.startsWith(QString(AttributeIds::effectFade) + ".")) {
            setIntegerSpecificNumberAttribute<float>(effectInfos, "Fade", ids, attribute, valueKeys, "effect_step_fade", {1, 99}, {0, 600, false, "s"});
        } else if (attribute == AttributeIds::effectPhase) {
            if (attributes.contains(Fixture)) {
                setItemSpecificNumberAttribute<float>(effectInfos, "Phase", ids, attributes.value(Fixture), valueKeys, fixtureInfos, "effect_fixture_phase", angleInfos);
            } else {
                setNumberAttribute<float>(effectInfos, "phase", "Phase", ids, valueKeys, angleInfos);
            }
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
            setNumberAttribute<int>(cuelistInfos, "priority", "Priority", ids, valueKeys, {1, 200});
        } else if (attribute == AttributeIds::cuelistMoveWhileDark) {
            setBoolAttribute(cuelistInfos, "movewhiledark", "Move while Dark", ids, valueKeys);
        } else {
            error("Unknown Cuelist Attribute.");
        }
    } else if (selectionType == Cue) {
        QSqlQuery currentCuelistQuery;
        if (currentCuelistQuery.exec("SELECT cuelists.id FROM cuelists, currentitems WHERE currentitems.cuelist_key = cuelists.key")) {
            if (!currentCuelistQuery.next()) {
                error("Can't set Cue Attribute because no Cuelist is currently selected.");
                return;
            }
        } else {
            qWarning() << Q_FUNC_INFO << currentCuelistQuery.executedQuery() << currentCuelistQuery.lastError().text();
            error("Can't set Cue Attribute because the request for the current Cuelist failed.");
            return;
        }
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Minus)) {
            deleteItems(cueInfos, ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(cueInfos, ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(cueInfos, "label", "Label", ids, "");
        } else if (attribute == AttributeIds::cueIntensities) {
            if (attributes.contains(Group)) {
                setItemSpecificItemListAttribute(cueInfos, "Intensities", ids, attributes.value(Group), valueKeys, groupInfos, intensityInfos, "cue_group_intensities", true);
            } else {
                error("Can't set Cue Intensities because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueColors) {
            if (attributes.contains(Group)) {
                setItemSpecificItemListAttribute(cueInfos, "Colors", ids, attributes.value(Group), valueKeys, groupInfos, colorInfos, "cue_group_colors", true);
            } else {
                error("Can't set Cue Colors because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cuePositions) {
            if (attributes.contains(Group)) {
                setItemSpecificItemListAttribute(cueInfos, "Positions", ids, attributes.value(Group), valueKeys, groupInfos, positionInfos, "cue_group_positions", true);
            } else {
                error("Can't set Cue Positions because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueRaws) {
            if (attributes.contains(Group)) {
                setItemSpecificItemListAttribute(cueInfos, "Raws", ids, attributes.value(Group), valueKeys, groupInfos, rawInfos, "cue_group_raws");
            } else {
                error("Can't set Cue Raws because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueEffects) {
            if (attributes.contains(Group)) {
                setItemSpecificItemListAttribute(cueInfos, "Effects", ids, attributes.value(Group), valueKeys, groupInfos, effectInfos, "cue_group_effects");
            } else {
                error("Can't set Cue Effects because no Group Attribute was provided.");
            }
        } else if ((attribute == AttributeIds::cueFade) || (!attributes.contains(Attribute))) {
            setNumberAttribute<float>(cueInfos, "fade", "Fade", ids, valueKeys, {0, 600, false, "s"});
        } else if (attribute == AttributeIds::cueBlock) {
            setBoolAttribute(cueInfos, "block", "Block", ids, valueKeys);
        } else {
            error("Unknown Cue Attribute.");
            return;
        }
    } else {
        error("Unknown Item type.");
        return;
    }
}

void Terminal::updateSortingKeys(const ItemInfos item) {
    QSqlQuery idsQuery;
    idsQuery.prepare("SELECT key, id FROM " + item.selectTable);
    if (idsQuery.exec()) {
        struct IdKey {
            int key;
            QString id;
        };
        QList<IdKey> idKeys;
        while (idsQuery.next()) {
            idKeys.append({idsQuery.value(0).toInt(), idsQuery.value(1).toString()});
        }
        std::sort(idKeys.begin(), idKeys.end(), [] (IdKey a, IdKey b) { return compareIds(a.id, b.id); });
        for (int index = 1; index <= idKeys.length(); index++) {
            const IdKey idKey = idKeys.at(index - 1);
            QSqlQuery query;
            query.prepare("UPDATE " + item.updateTable + " SET sortkey = :sortkey WHERE key = :key");
            query.bindValue(":key", idKey.key);
            query.bindValue(":sortkey", index);
            if (!query.exec()) {
                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                error("Failed to update the sorting key of " + item.singular + " " + idKey.id + ".");
            }
        }
    } else {
        qWarning() << Q_FUNC_INFO << idsQuery.executedQuery() << idsQuery.lastError().text();
        error("Failed to update the " + item.singular + " sorting keys.");
    }
}

bool Terminal::compareIds(const QString a, const QString b) {
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
}

void Terminal::createItems(const ItemInfos item, QStringList ids) {
    Q_ASSERT(!ids.isEmpty());
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec()) {
            if (!existsQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO " + item.updateTable + " (id, sortkey) VALUES (:id, 0)");
                insertQuery.bindValue(":id", id);
                if (insertQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                    error("Failed to insert " + item.singular + " " + id + ".");
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << existsQuery.executedQuery() << existsQuery.lastError().text();
            error("Failed to check if " + item.singular + " " + id + " already exists.");
        }
    }
    if (successfulIds.size() == 1) {
        success("Created " + item.singular + " " + successfulIds.join(", ") + ".");
    } else if (successfulIds.size() > 1) {
        success("Created " + item.plural + " " + successfulIds.join(", ") + ".");
    }
    updateSortingKeys(item);
}

void Terminal::deleteItems(const ItemInfos item, QStringList ids) {
    Q_ASSERT(!ids.isEmpty());
    QStringList successfulIds;
     for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + item.updateTable + " WHERE key = :key");
                deleteQuery.bindValue(":key", keyQuery.value(0).toInt());
                if (deleteQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    error("Can't delete " + item.singular + " because the request failed.");
                }
            } else {
                warning("Can't delete " + item.singular + " " + id + " because this " + item.singular + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Couldn't delete " + item.singular + " " + id + ": ");
        }
    }
    if (successfulIds.size() == 1) {
        success("Deleted " + item.singular + " " + successfulIds.join(", ") + ".");
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
        existsQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        existsQuery.bindValue(":id", newIds.first());
        if (existsQuery.exec()) {
            if (existsQuery.next()) {
                warning("Can't set ID of " + item.singular + " to " + newIds.first() + " because this " + item.singular + " ID is already used.");
            } else {
                QSqlQuery keyQuery;
                keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
                keyQuery.bindValue(":id", id);
                if (keyQuery.exec()) {
                    if (keyQuery.next()) {
                        QSqlQuery updateQuery;
                        updateQuery.prepare("UPDATE " + item.updateTable + " SET id = :newId WHERE key = :key");
                        updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                        updateQuery.bindValue(":newId", newIds.first());
                        if (updateQuery.exec()) {
                            successfulIds.append(id);
                        } else {
                            qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                            error("Failed to update ID of " + item.singular + " " + id + " because the request failed.");
                        }
                    } else {
                        warning("Can't set ID of " + item.singular + " " + id + " because this " + item.singular + " doesn't exist.");
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                    error("Failed loading " + item.singular + " " + id + ".");
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << existsQuery.executedQuery() << existsQuery.lastError().text();
            error("Error executing check if " + item.singular + " " + newIds.first() + " exists.");
        }
    }
    if (successfulIds.size() == 1) {
        success("Set ID of " + item.singular + " " + successfulIds.join(", ") + " to " + newIds.first() + ".");
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
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE " + item.updateTable + " SET " + attribute + " = :value WHERE key = :key");
                updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                updateQuery.bindValue(":value", value);
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    error("Failed setting " + attributeName + " of " + item.singular + " " + id + ".");
                }
            } else {
                warning("Failed to set " + attributeName + " of " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " to " + valueText + ".");
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
        query.prepare("SELECT " + attribute + " FROM " + item.selectTable + " WHERE id = :id");
        query.bindValue(":id", ids.first());
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            error("Failed to load current " + attributeName + " of " + item.singular + " " + ids.first() + ".");
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
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE " + item.updateTable + " SET " + attribute + " = :value WHERE key = :key");
                updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                updateQuery.bindValue(":value", textValue);
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    error("Failed setting " + attributeName + " of " + item.singular + " " + id + ".");
                }
            } else {
                warning("Failed to set " + attributeName + " of " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " to \"" + textValue + "\".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to \"" + textValue + "\".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setNumberAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QStringList ids, QList<Key> valueKeys, const NumberInfos number) {
    Q_ASSERT(!ids.isEmpty());
    const bool difference = valueKeys.startsWith(Plus);
    T value;
    if (!difference) {
        bool ok;
        value = keysToFloat(valueKeys, &ok, 0, number);
        if (!ok) {
            error("Invalid value given.");
            return;
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        bool valueOk = true;
        if (difference) {
            QSqlQuery currentValueQuery;
            currentValueQuery.prepare("SELECT " + attribute + " FROM " + item.selectTable + " WHERE id = :id");
            currentValueQuery.bindValue(":id", id);
            if (currentValueQuery.exec()) {
                if (currentValueQuery.next()) {
                    value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
                    if (!valueOk) {
                        error("Invalid value given for " + item.singular + " " + id + ".");
                    }
                } else {
                    error("Failed loading the current " + attributeName + " of " + item.singular + " " + id + " because this " + item.singular + " doesn't exist.");
                    valueOk = false;
                }
            } else {
                qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                error("Failed loading the current " + attributeName + " of " + item.singular + " " + id + ".");
                valueOk = false;
            }
        }
        if (valueOk) {
            QSqlQuery keyQuery;
            keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
            keyQuery.bindValue(":id", id);
            if (keyQuery.exec()) {
                if (keyQuery.next()) {
                    QSqlQuery updateQuery;
                    updateQuery.prepare("UPDATE " + item.updateTable + " SET " + attribute + " = :value WHERE key = :key");
                    updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                    updateQuery.bindValue(":value", value);
                    if (updateQuery.exec()) {
                        successfulIds.append(id);
                    } else {
                        qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                        error("Failed setting " + attributeName + " of " + item.singular + " " + id + ".");
                    }
                } else {
                    warning("Failed to set " + attributeName + " of " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                error("Failed loading " + item.singular + " " + id + ".");
            }
        }
    }
    if (difference) {
        if (successfulIds.length() == 1) {
            success("Changed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " by " + QString::number(value) + number.unit + ".");
        } else if (successfulIds.length() > 1) {
            success("Changed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " by " + QString::number(value) + number.unit + ".");
        }
    } else {
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " to " + QString::number(value) + number.unit + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + QString::number(value) + number.unit + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemAttribute(const ItemInfos item, const QString attribute, const QString attributeName, QStringList ids, QList<Key> valueKeys, const ItemInfos foreignItem) {
    Q_ASSERT(!ids.isEmpty());
    const bool removeItem = ((valueKeys.size() == 1) && valueKeys.startsWith(Minus));
    QString foreignItemId;
    int foreignItemKey;
    QStringList successfulIds;
    if (!removeItem) {
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
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.selectTable + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemIds.first());
        if (!foreignItemQuery.exec()) {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.singular + " exists.");
            return;
        }
        if (!foreignItemQuery.next()) {
            error("Can't set " + item.singular + " " + attributeName + " because " + foreignItem.singular + " " + foreignItemIds.first() + " doesn't exist.");
            return;
        }
        foreignItemKey = foreignItemQuery.value(0).toInt();
    }
    createItems(item, ids);
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery updateQuery;
                if (removeItem) {
                    updateQuery.prepare("UPDATE " + item.updateTable + " SET " + attribute + " = NULL WHERE key = :key");
                } else {
                    updateQuery.prepare("UPDATE " + item.updateTable + " SET " + attribute + " = :item WHERE key = :key");
                    updateQuery.bindValue(":item", foreignItemKey);
                }
                updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    error("Failed setting " + attributeName + " of " + item.singular + " " + id + ".");
                }
            } else {
                warning("Failed to set " + attributeName + " of " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    if (removeItem) {
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + ".");
        }
    } else {
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " to " + foreignItem.singular + " " + foreignItemId + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + foreignItem.singular + " " + foreignItemId + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemListAttribute(const ItemInfos item, const QString attributeName, QStringList ids, QList<Key> valueKeys, const ItemInfos foreignItem, const QString valueTable) {
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
            foreignItemQuery.prepare("SELECT key FROM " + foreignItem.selectTable + " WHERE id = :id");
            foreignItemQuery.bindValue(":id", foreignItemId);
            if (foreignItemQuery.exec()) {
                if (foreignItemQuery.next()) {
                    foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                    foreignItemIdStrings.append(foreignItemId);
                } else {
                    warning("Can't add " + foreignItem.singular + " " + foreignItemId + " to " + item.singular + " " + attributeName + " because this " + foreignItem.singular + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
                error("Failed to execute check if " + foreignItem.singular + " " + foreignItemId + " exists.");
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
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :key");
                deleteQuery.bindValue(":key", itemKey);
                if (deleteQuery.exec()) {
                    for (const int foreignItemKey : foreignItemKeys) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, valueitem_key) VALUES (:item, :foreign_item)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        if (!insertQuery.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                            error("Failed to insert a " + foreignItem.singular + " into " + item.singular + " " + id + ".");
                        }
                    }
                } else {
                    allQueriesSuccessful = false;
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    error("Failed deleting old " + attributeName + " of " + item.singular + " " + id + ".");
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    QString foreignItemString = foreignItem.plural + foreignItemIdStrings.join(", ");
    if (foreignItemIdStrings.size() == 1) {
        foreignItemString = foreignItem.singular + foreignItemIdStrings.join(", ");
    }
    if (successfulIds.length() == 1) {
        success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " to " + foreignItemString + ".");
    } else if (successfulIds.length() > 1) {
        success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " to " + foreignItemString + ".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setItemSpecificNumberAttribute(const ItemInfos item, const QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Key> valueKeys, const ItemInfos foreignItem, const QString valueTable, const NumberInfos number) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(!foreignItemIds.isEmpty());
    const bool removeValues = ((valueKeys.size() == 1) && valueKeys.startsWith(Minus));
    const bool difference = valueKeys.startsWith(Plus);
    T value;
    if (!removeValues && !difference) {
        bool ok;
        value = keysToFloat(valueKeys, &ok, 0, number);
        if (!ok) {
            error("Invalid value given.");
            return;
        }
    }
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    for (QString foreignItemId : foreignItemIds) {
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.selectTable + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.singular + " " + foreignItemId + " because this " + foreignItem.singular + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.singular + " " + foreignItemId + " exists.");
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
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                for (const int foreignItemKey : foreignItemKeys) {
                    if (removeValues) {
                        QSqlQuery query;
                        query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND foreignitem_key = :foreign_item");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":foreign_item", foreignItemKey);
                        if (!query.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            error("Failed removing the " + attributeName + " of " + item.singular + " " + id + ".");
                        }
                    } else {
                        bool valueOk = true;
                        if (difference) {
                            QSqlQuery currentValueQuery;
                            currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND foreignitem_key = :foreign_item");
                            currentValueQuery.bindValue(":item", itemKey);
                            currentValueQuery.bindValue(":foreign_item", foreignItemKey);
                            if (currentValueQuery.exec()) {
                                if (currentValueQuery.next()) {
                                    value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
                                } else {
                                    value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
                                }
                                if (!valueOk) {
                                    error("Invalid value given for " + item.singular + " " + id + ".");
                                }
                            } else {
                                qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                                error("Failed loading the current " + attributeName + " of " + item.singular + " " + id + ".");
                                valueOk = false;
                            }
                        }
                        if (valueOk) {
                            QSqlQuery query;
                            query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignitem_key, value) VALUES (:item, :foreign_item, :value)");
                            query.bindValue(":item", itemKey);
                            query.bindValue(":foreign_item", foreignItemKey);
                            query.bindValue(":value", value);
                            if (!query.exec()) {
                                allQueriesSuccessful = false;
                                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                                error("Failed removing the " + attributeName + " of " + item.singular + " " + id + ".");
                            }
                        } else {
                            allQueriesSuccessful = false;
                        }
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    QString foreignItemString = foreignItem.plural + " " + foreignItemIdStrings.join(", ");
    if (foreignItemIdStrings.length() == 1) {
        foreignItemString = foreignItem.singular + " " + foreignItemIdStrings.join(", ");
    }
    if (removeValues) {
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + foreignItemString + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + ".");
        }
    } else {
        if (difference) {
            if (successfulIds.length() == 1) {
                success("Changed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + foreignItemString + " by " + QString::number(value) + number.unit + ".");
            } else if (successfulIds.length() > 1) {
                success("Changed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + " by " + QString::number(value) + number.unit + ".");
            }
        } else {
            if (successfulIds.length() == 1) {
                success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + foreignItemString + " to " + QString::number(value) + number.unit + ".");
            } else if (successfulIds.length() > 1) {
                success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + " to " + QString::number(value) + number.unit + ".");
            }
        }
    }
    emit dbChanged();
}

void Terminal::setItemSpecificItemListAttribute(const ItemInfos item, const QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Key> valueKeys, const ItemInfos foreignItem, const ItemInfos valueItem, const QString valueTable, const bool limitToOne) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(!foreignItemIds.isEmpty());
    const bool removeValues = ((valueKeys.size() == 1) && valueKeys.startsWith(Minus));
    QList<int> valueItemKeys;
    QStringList valueItemIdStrings;
    if (!removeValues) {
        if (!valueKeys.startsWith(valueItem.key)) {
            error("Can't set " + item.singular + " " + attributeName + " because no " + valueItem.plural + " were given.");
            return;
        }
        const QStringList valueItemIds = keysToIds(valueKeys);
        if (valueItemIds.isEmpty()) {
            error("Can't set " + item.singular + " " + attributeName + " because the given " + valueItem.singular + " IDs are invalid.");
            return;
        }
        if (limitToOne && (valueItemIds.size() != 1)) {
            error("Can't set " + item.singular + " " + attributeName + " because this Attribute only accepts one " + valueItem.singular + " as a value.");
            return;
        }
        for (QString valueItemId : valueItemIds) {
            QSqlQuery valueItemQuery;
            valueItemQuery.prepare("SELECT key FROM " + valueItem.selectTable + " WHERE id = :id");
            valueItemQuery.bindValue(":id", valueItemId);
            if (valueItemQuery.exec()) {
                if (valueItemQuery.next()) {
                    valueItemKeys.append(valueItemQuery.value(0).toInt());
                    valueItemIdStrings.append(valueItemId);
                } else {
                    warning("Can't add " + valueItem.singular + " " + valueItemId + " to " + item.singular + " " + attributeName + " because this " + valueItem.singular + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << valueItemQuery.executedQuery() << valueItemQuery.lastError().text();
                error("Failed to execute check if " + valueItem.singular + " " + valueItemId + " exists.");
            }
        }
        Q_ASSERT(valueItemKeys.length() == valueItemIdStrings.length());
        if (valueItemKeys.isEmpty()) {
            error("Can't set " + item.singular + " " + attributeName + " because no valid " + valueItem.plural + " were given.");
            return;
        }
        if (valueItemKeys.isEmpty()) {
            error("No valid " + valueItem.plural + " were found.");
            return;
        }
    }
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    for (QString foreignItemId : foreignItemIds) {
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.selectTable + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.singular + " " + foreignItemId + " because this " + foreignItem.singular + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.singular + " " + foreignItemId + " exists.");
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
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                for (const int foreignItemKey : foreignItemKeys) {
                    QSqlQuery deleteQuery;
                    deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :item AND foreignitem_key = :foreign_item");
                    deleteQuery.bindValue(":item", itemKey);
                    deleteQuery.bindValue(":foreign_item", foreignItemKey);
                    if (!deleteQuery.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                        error("Failed deleting old " + attributeName + " of " + item.singular + " " + id + ".");
                    }
                    for (const int valueItemKey : valueItemKeys) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, foreignitem_key, valueitem_key) VALUES (:item, :foreign_item, :value_item)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        insertQuery.bindValue(":value_item", valueItemKey);
                        if (!insertQuery.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                            error("Failed to insert a " + foreignItem.singular + " into " + item.singular + " " + id + ".");
                        }
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    QString valueItemString = valueItem.plural + " " + valueItemIdStrings.join(", ");
    if (valueItemKeys.length() == 1) {
        valueItemString = valueItem.singular + " " + valueItemIdStrings.join(", ");
    }
    QString foreignItemString = foreignItem.plural + " " + foreignItemIdStrings.join(", ");
    if (foreignItemKeys.length() == 1) {
        foreignItemString = foreignItem.singular + " " + foreignItemIdStrings.join(", ");
    }
    if (removeValues) {
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + foreignItemString + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + ".");
        }
    } else {
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + foreignItemString + " to " + valueItemString + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + " to " + valueItemString + ".");
        }
    }
    emit dbChanged();
}

template <typename T> void Terminal::setIntegerSpecificNumberAttribute(const ItemInfos item, const QString attributeName, QStringList ids, QString numberId, QList<Key> valueKeys, const QString valueTable, const NumberInfos keyNumber, const NumberInfos valueNumber) {
    Q_ASSERT(!ids.isEmpty());
    QList<QString> numberIdParts = numberId.split(".");
    if (numberIdParts.length() != 2) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    bool ok;
    int key = numberIdParts.last().toInt(&ok);
    if (!ok) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    key = keysToFloat({Plus, Zero}, &ok, key, keyNumber);
    if (!ok) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    const bool removeValues = ((valueKeys.size() == 1) && valueKeys.startsWith(Minus));
    const bool difference = valueKeys.startsWith(Plus);
    T value;
    if (!removeValues && !difference) {
        bool ok;
        value = keysToFloat(valueKeys, &ok, 0, valueNumber);
        if (!ok) {
            error("Invalid value given.");
            return;
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                if (removeValues) {
                    QSqlQuery query;
                    query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND key = :key");
                    query.bindValue(":item", itemKey);
                    query.bindValue(":key", key);
                    if (query.exec()) {
                        successfulIds.append(id);
                    } else {
                        qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                        error("Failed removing the " + attributeName + " of " + item.singular + " " + id + ".");
                    }
                } else {
                    bool valueOk = true;
                    if (difference) {
                        QSqlQuery currentValueQuery;
                        currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND key = :key");
                        currentValueQuery.bindValue(":item", itemKey);
                        currentValueQuery.bindValue(":key", key);
                        if (currentValueQuery.exec()) {
                            if (currentValueQuery.next()) {
                                value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), valueNumber);
                            } else {
                                value = keysToFloat(valueKeys, &valueOk, 0, valueNumber);
                            }
                            if (!valueOk) {
                                error("Invalid value given for " + item.singular + " " + id + ".");
                            }
                        } else {
                            qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                            error("Failed loading the current " + attributeName + " of " + item.singular + " " + id + ".");
                            valueOk = false;
                        }
                    }
                    if (valueOk) {
                        QSqlQuery query;
                        query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, key, value) VALUES (:item, :key, :value)");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":key", key);
                        query.bindValue(":value", value);
                        if (query.exec()) {
                            successfulIds.append(id);
                        } else {
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            error("Failed removing the " + attributeName + " of " + item.singular + " " + id + ".");
                        }
                    }
                }
            } else {
                error("Failed loading " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    if (removeValues) {
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + QString::number(key) + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + QString::number(key) + ".");
        }
    } else {
        if (difference) {
            if (successfulIds.length() == 1) {
                success("Changed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + QString::number(key) + keyNumber.unit + " by " + QString::number(value) + valueNumber.unit + ".");
            } else if (successfulIds.length() > 1) {
                success("Changed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + QString::number(key) + keyNumber.unit + " by " + QString::number(value) + valueNumber.unit + ".");
            }
        } else {
            if (successfulIds.length() == 1) {
                success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + QString::number(key) + keyNumber.unit + " to " + QString::number(value) + valueNumber.unit + ".");
            } else if (successfulIds.length() > 1) {
                success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + QString::number(key) + keyNumber.unit + " to " + QString::number(value) + valueNumber.unit + ".");
            }
        }
    }
    emit dbChanged();
}

void Terminal::setIntegerSpecificItemListAttribute(ItemInfos item, QString attributeName, QStringList ids, QString integerId, QList<Key> valueKeys, ItemInfos valueItem, QString valueTable, NumberInfos keyInteger, bool limitToOne) {
    Q_ASSERT(!ids.isEmpty());
    QList<QString> numberIdParts = integerId.split(".");
    if (numberIdParts.length() != 2) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    bool ok;
    int key = numberIdParts.last().toInt(&ok);
    if (!ok) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    key = keysToFloat({Plus, Zero}, &ok, key, keyInteger);
    if (!ok) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    const bool removeValues = ((valueKeys.size() == 1) && valueKeys.startsWith(Minus));
    QList<int> valueItemKeys;
    QStringList valueItemIdStrings;
    if (!removeValues) {
        if (!valueKeys.startsWith(valueItem.key)) {
            error("Can't set " + item.singular + " " + attributeName + " because no " + valueItem.plural + " were given.");
            return;
        }
        const QStringList valueItemIds = keysToIds(valueKeys);
        if (valueItemIds.isEmpty()) {
            error("Can't set " + item.singular + " " + attributeName + " because the given " + valueItem.singular + " IDs are invalid.");
            return;
        }
        if (limitToOne && (valueItemIds.size() != 1)) {
            error("Can't set " + item.singular + " " + attributeName + " because this Attribute only accepts one " + valueItem.singular + " as a value.");
            return;
        }
        for (QString valueItemId : valueItemIds) {
            QSqlQuery valueItemQuery;
            valueItemQuery.prepare("SELECT key FROM " + valueItem.selectTable + " WHERE id = :id");
            valueItemQuery.bindValue(":id", valueItemId);
            if (valueItemQuery.exec()) {
                if (valueItemQuery.next()) {
                    valueItemKeys.append(valueItemQuery.value(0).toInt());
                    valueItemIdStrings.append(valueItemId);
                } else {
                    warning("Can't add " + valueItem.singular + " " + valueItemId + " to " + item.singular + " " + attributeName + " because this " + valueItem.singular + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << valueItemQuery.executedQuery() << valueItemQuery.lastError().text();
                error("Failed to execute check if " + valueItem.singular + " " + valueItemId + " exists.");
            }
        }
        Q_ASSERT(valueItemKeys.length() == valueItemIdStrings.length());
        if (valueItemKeys.isEmpty()) {
            error("Can't set " + item.singular + " " + attributeName + " because no valid " + valueItem.plural + " were given.");
            return;
        }
        if (valueItemKeys.isEmpty()) {
            error("No valid " + valueItem.plural + " were found.");
            return;
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :item AND key = :key");
                deleteQuery.bindValue(":item", itemKey);
                deleteQuery.bindValue(":key", key);
                if (!deleteQuery.exec()) {
                    allQueriesSuccessful = false;
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    error("Failed deleting old " + attributeName + " of " + item.singular + " " + id + ".");
                }
                for (const int valueItemKey : valueItemKeys) {
                    QSqlQuery insertQuery;
                    insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, key, valueitem_key) VALUES (:item, :key, :value_item)");
                    insertQuery.bindValue(":item", itemKey);
                    insertQuery.bindValue(":key", key);
                    insertQuery.bindValue(":value_item", valueItemKey);
                    if (!insertQuery.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                        error("Failed to insert a " + valueItem.singular + " into " + item.singular + " " + id + ".");
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    QString valueItemString = valueItem.plural + " " + valueItemIdStrings.join(", ");
    if (valueItemKeys.length() == 1) {
        valueItemString = valueItem.singular + " " + valueItemIdStrings.join(", ");
    }
    if (removeValues) {
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + QString::number(key) + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + QString::number(key) + ".");
        }
    } else {
        if (successfulIds.length() == 1) {
            success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + QString::number(key) + " to " + valueItemString + ".");
        } else if (successfulIds.length() > 1) {
            success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + QString::number(key) + " to " + valueItemString + ".");
        }
    }
    emit dbChanged();
}

template <typename T> void Terminal::setItemAndIntegerSpecificNumberAttribute(ItemInfos item, QString attributeName, QStringList ids, QStringList foreignItemIds, QString numberId, QList<Key> valueKeys, ItemInfos foreignItem, QString valueTable, NumberInfos keyNumber, NumberInfos valueNumber) {
    Q_ASSERT(!ids.isEmpty());
    QList<QString> numberIdParts = numberId.split(".");
    if (numberIdParts.length() != 2) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    bool ok;
    int key = numberIdParts.last().toInt(&ok);
    if (!ok) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    key = keysToFloat({Plus, Zero}, &ok, key, keyNumber);
    if (!ok) {
        error("Can't set " + item.singular + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    const bool removeValues = ((valueKeys.size() == 1) && valueKeys.startsWith(Minus));
    const bool difference = valueKeys.startsWith(Plus);
    T value;
    if (!removeValues && !difference) {
        bool ok;
        value = keysToFloat(valueKeys, &ok, 0, valueNumber);
        if (!ok) {
            error("Invalid value given.");
            return;
        }
    }
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    for (QString foreignItemId : foreignItemIds) {
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.selectTable + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.singular + " " + foreignItemId + " because this " + foreignItem.singular + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.singular + " " + foreignItemId + " exists.");
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
        keyQuery.prepare("SELECT key FROM " + item.selectTable + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                for (const int foreignItemKey : foreignItemKeys) {
                    if (removeValues) {
                        QSqlQuery query;
                        query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND foreignitem_key = :foreign_item AND key = :key");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":foreign_item", foreignItemKey);
                        query.bindValue(":key", key);
                        if (!query.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            error("Failed removing the " + attributeName + " of " + item.singular + " " + id + ".");
                        }
                    } else {
                        bool valueOk = true;
                        if (difference) {
                            QSqlQuery currentValueQuery;
                            currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND foreignitem_key = :foreign_item AND key = :key");
                            currentValueQuery.bindValue(":item", itemKey);
                            currentValueQuery.bindValue(":foreign_item", foreignItemKey);
                            currentValueQuery.bindValue(":key", key);
                            if (currentValueQuery.exec()) {
                                if (currentValueQuery.next()) {
                                    value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), valueNumber);
                                } else {
                                    value = keysToFloat(valueKeys, &valueOk, 0, valueNumber);
                                }
                                if (!valueOk) {
                                    error("Invalid value given for " + item.singular + " " + id + ".");
                                }
                            } else {
                                qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                                error("Failed loading the current " + attributeName + " of " + item.singular + " " + id + ".");
                                valueOk = false;
                            }
                        }
                        if (valueOk) {
                            QSqlQuery query;
                            query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignitem_key, key, value) VALUES (:item, :foreign_item, :key, :value)");
                            query.bindValue(":item", itemKey);
                            query.bindValue(":foreign_item", foreignItemKey);
                            query.bindValue(":key", key);
                            query.bindValue(":value", value);
                            if (!query.exec()) {
                                allQueriesSuccessful = false;
                                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                                error("Failed removing the " + attributeName + " of " + item.singular + " " + id + ".");
                            }
                        }
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.singular + " " + id + " because this " + item.singular + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.singular + " " + id + ".");
        }
    }
    QString foreignItemString = foreignItem.plural + " " + foreignItemIdStrings.join(", ");
    if (foreignItemKeys.length() == 1) {
        foreignItemString = foreignItem.singular + " " + foreignItemIdStrings.join(", ");
    }
    if (removeValues) {
        if (successfulIds.length() == 1) {
            success("Removed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + foreignItemString + " and " + QString::number(key) + ".");
        } else if (successfulIds.length() > 1) {
            success("Removed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString +  QString::number(key) + ".");
        }
    } else {
        if (difference) {
            if (successfulIds.length() == 1) {
                success("Changed " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + foreignItemString + " and " + QString::number(key) + keyNumber.unit + " by " + QString::number(value) + valueNumber.unit + ".");
            } else if (successfulIds.length() > 1) {
                success("Changed " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + " and " + QString::number(key) + keyNumber.unit + " by " + QString::number(value) + valueNumber.unit + ".");
            }
        } else {
            if (successfulIds.length() == 1) {
                success("Set " + attributeName + " of " + item.singular + " " + successfulIds.join(", ") + " at " + foreignItemString + " and " + QString::number(key) + keyNumber.unit + " to " + QString::number(value) + valueNumber.unit + ".");
            } else if (successfulIds.length() > 1) {
                success("Set " + attributeName + " of " + item.plural + " " + successfulIds.join(", ") + " at " + foreignItemString + " and " + QString::number(key) + keyNumber.unit + " to " + QString::number(value) + valueNumber.unit + ".");
            }
        }
    }
    emit dbChanged();
}

void Terminal::reload() {
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
    if (idKeys.isEmpty()) {
        emit itemChanged(QString(), QStringList());
    } else {
        emit itemChanged(keysToString({ idKeys.first() }), keysToIds(idKeys));
    }
}

void Terminal::writeKey(Key key) {
    promptKeys.append(key);
    reload();
}

void Terminal::backspace() {
    if (promptKeys.isEmpty()) {
        return;
    }
    promptKeys.removeLast();
    reload();
}

void Terminal::clearPrompt() {
    promptKeys.clear();
    reload();
}

float Terminal::keysToFloat(QList<Key> keys, bool* ok, const float currentValue, const NumberInfos number) const {
    const bool difference = keys.startsWith(Plus);
    if (difference) {
        keys.removeFirst();
    }
    float value = keysToString(keys).replace(" ", "").toFloat(ok);
    if (!(*ok)) {
        return value;
    }
    if (difference) {
        value += currentValue;
    }
    if (number.cyclic) {
        while (value < number.minValue) {
            value += (number.maxValue - number.minValue);
        }
        while (value >= number.maxValue) {
            value -= (number.maxValue - number.minValue);
        }
    } else {
        if ((value < number.minValue) || (value > number.maxValue)) {
            (*ok) = false;
            return value;
        }
    }
    (*ok) = true;
    return value;
}

QStringList Terminal::keysToIds(QList<Key> keys) const {
    if (keys.isEmpty()) {
        return QStringList();
    }
    const Key itemType = keys.first();
    keys.removeFirst();
    QStringList ids;
    if (keys.isEmpty()) {
        QSqlQuery query;
        if (itemType == Model) {
            query.prepare("SELECT models.id FROM models, currentfixtures WHERE currentfixtures.model_key = models.key");
        } else if (itemType == Fixture) {
            query.prepare("SELECT id FROM currentfixtures");
        } else if (itemType == Group) {
            query.prepare("SELECT groups.id FROM groups, currentitems WHERE groups.key = currentitems.group_key");
        } else if (itemType == Intensity) {
            query.prepare("SELECT intensities.id FROM intensities, currentcue, currentitems, cue_group_intensities WHERE currentitems.group_key = cue_group_intensities.foreignitem_key AND cue_group_intensities.valueitem_key = intensities.key AND cue_group_intensities.item_key = currentcue.key");
        } else if (itemType == Color) {
            query.prepare("SELECT colors.id FROM colors, currentcue, currentitems, cuelists, cue_group_colors WHERE currentitems.group_key = cue_group_colors.foreignitem_key AND cue_group_colors.valueitem_key = colors.key AND cue_group_colors.item_key = currentcue.key");
        } else if (itemType == Position) {
            query.prepare("SELECT positions.id FROM positions, currentcue, currentitems, cuelists, cue_group_positions WHERE currentitems.group_key = cue_group_positions.foreignitem_key AND cue_group_positions.valueitem_key = positions.key AND cue_group_positions.item_key = currentcue.key");
        } else if (itemType == Raw) {
            query.prepare("SELECT raws.id FROM raws, currentcue, currentitems, cuelists, cue_group_raws WHERE currentitems.group_key = cue_group_raws.foreignitem_key AND cue_group_raws.valueitem_key = raws.key AND cue_group_raws.item_key = currentcue.key");
        } else if (itemType == Effect) {
            query.prepare("SELECT effects.id FROM effects, currentcue, currentitems, cuelists, cue_group_effects WHERE currentitems.group_key = cue_group_effects.foreignitem_key AND cue_group_effects.valueitem_key = effects.key AND cue_group_effects.item_key = currentcue.key");
        } else if (itemType == Cuelist) {
            query.prepare("SELECT cuelists.id FROM cuelists, currentitems WHERE cuelists.key = currentitems.cuelist_key");
        } else if (itemType == Cue) {
            query.prepare("SELECT id FROM currentcue");
        } else {
            return QStringList();
        }
        if (query.exec()) {
            while (query.next()) {
                ids.append(query.value(0).toString());
            }
        } else {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
        }
    } else {
        QStringList allIds;
        QString table;
        if (itemType == Model) {
            table = modelInfos.selectTable;
        } else if (itemType == Fixture) {
            table = fixtureInfos.selectTable;
        } else if (itemType == Group) {
            table = groupInfos.selectTable;
        } else if (itemType == Intensity) {
            table = intensityInfos.selectTable;
        } else if (itemType == Color) {
            table = colorInfos.selectTable;
        } else if (itemType == Position) {
            table = positionInfos.selectTable;
        } else if (itemType == Raw) {
            table = rawInfos.selectTable;
        } else if (itemType == Effect) {
            table = effectInfos.selectTable;
        } else if (itemType == Cuelist) {
            table = cuelistInfos.selectTable;
        } else if (itemType == Cue) {
            table = cueInfos.selectTable;
        } else if (itemType == Attribute) {
        } else {
            return QStringList();
        }
        if (!table.isEmpty()) {
            QSqlQuery query;
            if (query.exec("SELECT id FROM " + table)) {
                while (query.next()) {
                    allIds.append(query.value(0).toString());
                }
            } else {
                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            }
        }

        keys.append(Plus);
        QStringList thruParts;
        QStringList idParts;
        QList<Key> currentIdPartKeys;
        bool idAdding = true;
        bool idStartsWithPeriod = false;
        for (const Key key : keys) {
            if (((key == Plus) || (key == Minus)) && currentIdPartKeys.isEmpty()) { // IDs which end with a period
                if (idStartsWithPeriod) {
                    if (!thruParts.isEmpty() || !idParts.isEmpty()) {
                        return QStringList();
                    }
                    for (QString existingId : allIds) {
                        if (idAdding) {
                            ids.append(existingId);
                        } else {
                            ids.removeAll(existingId);
                        }
                    }
                    idStartsWithPeriod = false;
                } else {
                    if (!thruParts.isEmpty()) {
                        return QStringList();
                    }
                    for (QString id : allIds) {
                        if (id.startsWith(idParts.join(".") + ".") || (id == idParts.join("."))) {
                            if (idAdding) {
                                ids.append(id);
                            } else {
                                ids.removeAll(id);
                            }
                        }
                    }
                    idParts.clear();
                }
            } else if ((key == Period) || (key == Plus) || (key == Minus) || (key == Thru)) {
                if ((key == Period) && idParts.isEmpty() && currentIdPartKeys.isEmpty() && !idStartsWithPeriod) {
                    idStartsWithPeriod = true;
                } else {
                    bool ok;
                    const int idPart = keysToString(currentIdPartKeys).toInt(&ok);
                    if (!ok || (idPart < 0)) {
                        return QStringList();
                    }
                    idParts.append(QString::number(idPart));
                    currentIdPartKeys.clear();
                }
                if ((key == Plus) || (key == Minus)) {
                    if (idParts.isEmpty() && !idStartsWithPeriod) {
                        return QStringList();
                    }
                    const QString id = idParts.join(".");
                    if (idStartsWithPeriod) {
                        if (thruParts.isEmpty() || (idParts.size() != 1)) {
                            return QStringList();
                        }
                        for (int ending = thruParts.last().toInt(); ending <= idParts.last().toInt(); ending++) {
                            QStringList currentIdParts = thruParts.first(thruParts.length() - 1);
                            currentIdParts.append(QString::number(ending));
                            if (idAdding) {
                                ids.append(currentIdParts.join("."));
                            } else {
                                ids.removeAll(currentIdParts.join("."));
                            }
                        }
                        thruParts.clear();
                        idStartsWithPeriod = false;
                    } else if (thruParts.isEmpty()) {
                        if (idAdding) {
                            ids.append(id);
                        } else {
                            ids.removeAll(id);
                        }
                    } else {
                        const QString thruId = thruParts.join(".");
                        QStringList allIdsExtended = allIds;
                        if (!allIdsExtended.contains(thruId)) {
                            allIdsExtended.append(thruId);
                        }
                        if (!allIdsExtended.contains(id)) {
                            allIdsExtended.append(id);
                        }
                        std::sort(allIdsExtended.begin(), allIdsExtended.end(), Terminal::compareIds);
                        for (int index = allIdsExtended.indexOf(thruId); index <= allIdsExtended.indexOf(id); index++) {
                            const QString currentId = allIdsExtended.at(index);
                            if (allIds.contains(currentId)) {
                                if (idAdding) {
                                    ids.append(currentId);
                                } else {
                                    ids.removeAll(currentId);
                                }
                            }
                        }
                    }
                    thruParts.clear();
                    idParts.clear();
                }
            } else {
                currentIdPartKeys.append(key);
            }
            if (key == Thru) {
                if (!thruParts.isEmpty() || idParts.isEmpty() || idStartsWithPeriod) {
                    return QStringList();
                }
                thruParts = idParts;
                idParts.clear();
            } else if (key == Plus) {
                idAdding = true;
            } else if (key == Minus) {
                idAdding = false;
            }
        }
    }
    ids.removeDuplicates();
    std::sort(ids.begin(), ids.end(), Terminal::compareIds);
    return ids;
}

QString Terminal::keysToString(QList<Key> keys) const {
    QString string;
    for(const Key key: keys) {
        Q_ASSERT(keyStrings.contains(key));
        string.append(keyStrings.value(key));
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
