/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "terminal.h"
#include "constants.h"

Terminal::Terminal(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);
    QHBoxLayout* promptLayout = new QHBoxLayout();
    layout->addLayout(promptLayout);

    promptLabel = new QLabel();
    promptLabel->setWordWrap(true);
    promptLabel->setStyleSheet("padding: 10px; background-color: #303030;");
    promptLayout->addWidget(promptLabel, 1);

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
        QSettings().setValue("terminal/tracking", trackingButton->isChecked());
    });
    trackingButton->setChecked(QSettings().value("terminal/tracking", true).toBool());
    promptLayout->addWidget(trackingButton);

    messages = new QPlainTextEdit();
    messages->setReadOnly(true);
    layout->addWidget(messages);

    new QShortcut(Qt::Key_0, this, [this] { writeKey(Keys::Zero); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_1, this, [this] { writeKey(Keys::One); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_2, this, [this] { writeKey(Keys::Two); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_3, this, [this] { writeKey(Keys::Three); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_4, this, [this] { writeKey(Keys::Four); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_5, this, [this] { writeKey(Keys::Five); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_6, this, [this] { writeKey(Keys::Six); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_7, this, [this] { writeKey(Keys::Seven); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_8, this, [this] { writeKey(Keys::Eight); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_9, this, [this] { writeKey(Keys::Nine); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::Key_Plus, this, [this] { writeKey(Keys::Plus); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Minus, this, [this] { writeKey(Keys::Minus); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_T, this, [this] { writeKey(Keys::Thru); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Period, this, [this] { writeKey(Keys::Period); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Comma, this, [this] { writeKey(Keys::Period); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_A, this, [this] { writeKey(Keys::Attribute); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_S, this, [this] { writeKey(Keys::Set); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::Key_M, this, [this] { writeKey(Keys::Model); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_F, this, [this] { writeKey(Keys::Fixture); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_G, this, [this] { writeKey(Keys::Group); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_I, this, [this] { writeKey(Keys::Intensity); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_C, this, [this] { writeKey(Keys::Color); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_P, this, [this] { writeKey(Keys::Position); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_R, this, [this] { writeKey(Keys::Raw); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_E, this, [this] { writeKey(Keys::Effect); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_L, this, [this] { writeKey(Keys::Cuelist); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Q, this, [this] { writeKey(Keys::Cue); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::Key_Backspace, this, [this] { backspace(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_Backspace, this, [this] { clearPrompt(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Enter, this, [this] { execute(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::Key_Return, this, [this] { execute(); }, Qt::ApplicationShortcut);

    new QShortcut(Qt::SHIFT | Qt::Key_B, this, [this] { blindButton->click(); }, Qt::ApplicationShortcut);
    new QShortcut(Qt::SHIFT | Qt::Key_T, this, [this] { trackingButton->click(); }, Qt::ApplicationShortcut);
}

void Terminal::execute() {
    QList<Keys::Key> keys = promptKeys;
    if (keys.isEmpty()) {
        return;
    }
    info("> " + keysToString(keys));
    clearPrompt();

    const Keys::Key selectionType = keys.first();
    keys.removeFirst();

    QList<Keys::Key> selectionIdKeys;
    selectionIdKeys.append(selectionType);
    QList<Keys::Key> attributeKeys;
    QList<Keys::Key> valueKeys;
    bool attributeReached = false;
    bool valueReached = false;
    for (const Keys::Key key : keys) {
        if (key == Keys::Set) {
            if (valueReached) {
                error("Can't use Set more than one time in one command.");
                return;
            }
            valueReached = true;
        } else if ((Keys::isItemKey(key) || (key == Keys::Attribute)) && !valueReached) {
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
        if (selectionType == ItemType::fixture().getKey()) {
            setCurrentItem(ItemType::fixture(), "currentgroup_fixtures", selectionIdKeys, "UPDATE currentitems SET fixture_key = :key");
        } else if (selectionType == ItemType::group().getKey()) {
            setCurrentItem(ItemType::group(), ItemType::group().getSelectTable(), selectionIdKeys, "UPDATE currentitems SET group_key = :key");
        } else if (selectionType == ItemType::intensity().getKey()) {
            setCueItem(ItemType::intensity(), "cue_group_intensities", selectionIdKeys, false);
        } else if (selectionType == ItemType::color().getKey()) {
            setCueItem(ItemType::color(), "cue_group_colors", selectionIdKeys, false);
        } else if (selectionType == ItemType::position().getKey()) {
            setCueItem(ItemType::position(), "cue_group_positions", selectionIdKeys, false);
        } else if (selectionType == ItemType::raw().getKey()) {
            setCueItem(ItemType::raw(), "cue_group_raws", selectionIdKeys, true);
        } else if (selectionType == ItemType::effect().getKey()) {
            setCueItem(ItemType::effect(), "cue_group_effects", selectionIdKeys, true);
        } else if (selectionType == ItemType::cuelist().getKey()) {
            setCurrentItem(ItemType::cuelist(), ItemType::cuelist().getSelectTable(), selectionIdKeys, "UPDATE currentitems SET cuelist_key = :key");
        } else if (selectionType == ItemType::cue().getKey()) {
            QSqlQuery currentCueQuery;
            if (currentCueQuery.exec("SELECT cue_key FROM currentitems WHERE cue_key IS NOT NULL")) {
                if (currentCueQuery.next()) {
                    setCurrentItem(ItemType::cue(), ItemType::cue().getSelectTable(), selectionIdKeys, "UPDATE currentitems SET cue_key = :key");
                } else {
                    setCurrentItem(ItemType::cue(), ItemType::cue().getSelectTable(), selectionIdKeys, "UPDATE cuelists SET currentcue_key = :key WHERE key = (SELECT cuelist_key FROM currentitems)");
                }
            } else {
                qWarning() << Q_FUNC_INFO << currentCueQuery.executedQuery() << currentCueQuery.lastError().text();
            }
        } else {
            error("Can't select this Item type: " + Keys::keysToString({selectionType}));
        }
        return;
    }
    const QStringList ids = keysToIds(selectionIdKeys);
    if (ids.isEmpty()) {
        error("Invalid selection ID given.");
        return;
    }
    QHash<Keys::Key, QStringList> attributes;
    if (!attributeKeys.isEmpty()) {
        attributeKeys.append(Keys::Attribute);
        QList<Keys::Key> currentItemKeys;
        for (const Keys::Key key : attributeKeys) {
            if (Keys::isItemKey(key) || (key == Keys::Attribute)) {
                if (!currentItemKeys.isEmpty()) {
                    Keys::Key currentItemType = currentItemKeys.first();
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
    if (attributes.value(Keys::Attribute, QStringList()).size() == 0) {
        attribute = QString();
    } else if (attributes.value(Keys::Attribute).size() == 1) {
        attribute = attributes.value(Keys::Attribute).first();
    } else {
        error("Invalid number of Attribute IDs given.");
        return;
    }

    if (selectionType == ItemType::model().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::model(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::model(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::model(), "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::modelChannels) || !attributes.contains(Keys::Attribute)) {
            setTextAttribute(ItemType::model(), "channels", "Channels", ids, "^[01DdRrGgBbWwCcMmYyHhSsPpTtZzFf]+$");
        } else if (attribute == AttributeIds::modelPanRange) {
            setNumberAttribute<float>(ItemType::model(), "panrange", "Pan Range", ids, valueKeys, {0, 3600, false, "°"});
        } else if (attribute == AttributeIds::modelTiltRange) {
            setNumberAttribute<float>(ItemType::model(), "tiltrange", "Tilt Range", ids, valueKeys, {0, 360, false, "°"});
        } else if (attribute == AttributeIds::modelMinZoom) {
            setNumberAttribute<float>(ItemType::model(), "minzoom", "Minimal Zoom", ids, valueKeys, {0, 180, false, "°"});
        } else if (attribute == AttributeIds::modelMaxZoom) {
            setNumberAttribute<float>(ItemType::model(), "maxzoom", "Maximal Zoom", ids, valueKeys, {0, 180, false, "°"});
        } else {
            error("Unknown Model Attribute.");
        }
    } else if (selectionType == ItemType::fixture().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::fixture(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::fixture(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::fixture(), "label", "Label", ids, "");
        } else if (attribute == AttributeIds::fixtureModel) {
            setItemAttribute(ItemType::fixture(), "model_key", "Model", ids, valueKeys, ItemType::model());
        } else if (attribute == AttributeIds::fixtureUniverse) {
            setNumberAttribute<int>(ItemType::fixture(), "universe", "Universe", ids, valueKeys, {1, 63999});
        } else if ((attribute == AttributeIds::fixtureAddress) || !attributes.contains(Keys::Attribute)) {
            setNumberAttribute<int>(ItemType::fixture(), "address", "Address", ids, valueKeys, {0, 512});
        } else if (attribute == AttributeIds::fixtureXPosition) {
            setNumberAttribute<float>(ItemType::fixture(), "xposition", "X Position", ids, valueKeys, {-100, 100});
        } else if (attribute == AttributeIds::fixtureYPosition) {
            setNumberAttribute<float>(ItemType::fixture(), "yposition", "Y Position", ids, valueKeys, {-100, 100});
        } else if (attribute == AttributeIds::fixtureRotation) {
            setNumberAttribute<float>(ItemType::fixture(), "rotation", "Rotation", ids, valueKeys, angleInfos);
        } else if (attribute == AttributeIds::fixtureInvertPan) {
            setBoolAttribute(ItemType::fixture(), "invertpan", "Invert Pan", ids, valueKeys);
        } else {
            error("Unknown Fixture Attribute.");
        }
    } else if (selectionType == ItemType::group().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::group(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::group(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::group(), "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::groupFixtures) || !attributes.contains(Keys::Attribute)) {
            setItemListAttribute(ItemType::group(), "Fixtures", ids, valueKeys, ItemType::fixture(), "group_fixtures");
        } else {
            error("Unknown Group Attribute.");
        }
    } else if (selectionType == ItemType::intensity().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::intensity(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::intensity(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::intensity(), "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::intensityDimmer) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::intensity(), "Dimmer Model Exception", ids, attributes.value(ItemType::model().getKey()), valueKeys, ItemType::model(), "intensity_model_dimmer", percentageInfos);
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::intensity(), "Dimmer Fixture Exception", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "intensity_fixture_dimmer", percentageInfos);
            } else {
                setNumberAttribute<float>(ItemType::intensity(), "dimmer", "Dimmer", ids, valueKeys, percentageInfos);
            }
        } else if (attribute == AttributeIds::intensityRaws) {
            setItemListAttribute(ItemType::intensity(), "Raws", ids, valueKeys, ItemType::raw(), "intensity_raws");
        } else {
            error("Unknown Intensity Attribute.");
        }
    } else if (selectionType == ItemType::color().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::color(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::color(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::color(), "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::colorHue) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::color(), "Hue Model Exception", ids, attributes.value(ItemType::model().getKey()), valueKeys, ItemType::model(), "color_model_hue", angleInfos);
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::color(), "Hue Fixture Exception", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "color_fixture_hue", angleInfos);
            } else {
                setNumberAttribute<float>(ItemType::color(), "hue", "Hue", ids, valueKeys, angleInfos);
            }
        } else if (attribute == AttributeIds::colorSaturation) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::color(), "Saturation Model Exception", ids, attributes.value(ItemType::model().getKey()), valueKeys, ItemType::model(), "color_model_saturation", percentageInfos);
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::color(), "Saturation Fixture Exception", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "color_fixture_saturation", percentageInfos);
            } else {
                setNumberAttribute<float>(ItemType::color(), "saturation", "Saturation", ids, valueKeys, percentageInfos);
            }
        } else if (attribute == AttributeIds::colorQuality) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::color(), "Quality Model Exception", ids, attributes.value(ItemType::model().getKey()), valueKeys, ItemType::model(), "color_model_quality", percentageInfos);
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::color(), "Quality Fixture Exception", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "color_fixture_quality", percentageInfos);
            } else {
                setNumberAttribute<float>(ItemType::color(), "quality", "Quality", ids, valueKeys, percentageInfos);
            }
        } else if (attribute == AttributeIds::colorRaws) {
            setItemListAttribute(ItemType::color(), "Raws", ids, valueKeys, ItemType::raw(), "color_raws");
        } else {
            error("Unknown Color Attribute.");
        }
    } else if (selectionType == ItemType::position().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::position(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::position(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::position(), "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::positionPan) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::position(), "Pan Model Exception", ids, attributes.value(ItemType::model().getKey()), valueKeys, ItemType::model(), "position_model_pan", angleInfos);
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::position(), "Pan Fixture Exception", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "position_fixture_pan", angleInfos);
            } else {
                setNumberAttribute<float>(ItemType::position(), "pan", "Pan", ids, valueKeys, angleInfos);
            }
        } else if (attribute == AttributeIds::positionTilt) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::position(), "Tilt Model Exception", ids, attributes.value(ItemType::model().getKey()), valueKeys, ItemType::model(), "position_model_tilt", {-180, 180, false, "°"});
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::position(), "Tilt Fixture Exception", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "position_fixture_tilt", {-180, 180, false, "°"});
            } else {
                setNumberAttribute<float>(ItemType::position(), "tilt", "Tilt", ids, valueKeys, {-180, 180, false, "°"});
            }
        } else if (attribute == AttributeIds::positionZoom) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::position(), "Zoom Model Exception", ids, attributes.value(ItemType::model().getKey()), valueKeys, ItemType::model(), "position_model_zoom", {0, 180, false, "°"});
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::position(), "Zoom Fixture Exception", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "position_fixture_zoom", {0, 180, false, "°"});
            } else {
                setNumberAttribute<float>(ItemType::position(), "zoom", "Zoom", ids, valueKeys, {0, 180, false, "°"});
            }
        } else if (attribute == AttributeIds::positionFocus) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::position(), "Focus Model Exception", ids, attributes.value(ItemType::model().getKey()), valueKeys, ItemType::model(), "position_model_focus", percentageInfos);
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::position(), "Focus Fixture Exception", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "position_fixture_focus", percentageInfos);
            } else {
                setNumberAttribute<float>(ItemType::position(), "focus", "Focus", ids, valueKeys, percentageInfos);
            }
        } else if (attribute == AttributeIds::positionRaws) {
            setItemListAttribute(ItemType::position(), "Raws", ids, valueKeys, ItemType::raw(), "position_raws");
        } else {
            error("Unknown Position Attribute.");
        }
    } else if (selectionType == ItemType::raw().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::raw(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::raw(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::raw(), "label", "Label", ids, "");
        } else if (attribute.startsWith(QString(AttributeIds::rawChannelValues) + ".")) {
            if (attributes.contains(ItemType::model().getKey())) {
                setItemAndIntegerSpecificNumberAttribute<int>(ItemType::raw(), "Channel Values", ids, attributes.value(ItemType::model().getKey()), attribute, valueKeys, ItemType::model(), "raw_model_channel_values", {1, 512}, {0, 255});
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                setItemAndIntegerSpecificNumberAttribute<int>(ItemType::raw(), "Channel Values", ids, attributes.value(ItemType::fixture().getKey()), attribute, valueKeys, ItemType::fixture(), "raw_fixture_channel_values", {1, 512}, {0, 255});
            } else {
                setIntegerSpecificNumberAttribute<int>(ItemType::raw(), "Channel Values", ids, attribute, valueKeys, "raw_channel_values", {1, 512}, {0, 255});
            }
        } else if (attribute == AttributeIds::rawMoveWhileDark) {
            setBoolAttribute(ItemType::raw(), "movewhiledark", "Move while Dark", ids, valueKeys);
        } else if (attribute == AttributeIds::rawFade) {
            setBoolAttribute(ItemType::raw(), "fade", "Fade", ids, valueKeys);
        } else {
            error("Unknown Raw Attribute.");
        }
    } else if (selectionType == ItemType::effect().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::effect(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::effect(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::effect(), "label", "Label", ids, "");
        } else if ((attribute == AttributeIds::effectSteps) || !attributes.contains(Keys::Attribute)) {
            setNumberAttribute<int>(ItemType::effect(), "steps", "Steps", ids, valueKeys, {2, 99});
        } else if (attribute.startsWith(QString(AttributeIds::effectIntensities) + ".")) {
            setIntegerSpecificItemListAttribute(ItemType::effect(), "Intensities", ids, attribute, valueKeys, ItemType::intensity(), "effect_step_intensities", {1, 99}, true);
        } else if (attribute.startsWith(QString(AttributeIds::effectColors) + ".")) {
            setIntegerSpecificItemListAttribute(ItemType::effect(), "Colors", ids, attribute, valueKeys, ItemType::color(), "effect_step_colors", {1, 99}, true);
        } else if (attribute.startsWith(QString(AttributeIds::effectPositions) + ".")) {
            setIntegerSpecificItemListAttribute(ItemType::effect(), "Positions", ids, attribute, valueKeys, ItemType::position(), "effect_step_positions", {1, 99}, true);
        } else if (attribute.startsWith(QString(AttributeIds::effectRaws) + ".")) {
            setIntegerSpecificItemListAttribute(ItemType::effect(), "Raws", ids, attribute, valueKeys, ItemType::raw(), "effect_step_raws", {1, 99});
        } else if (attribute == AttributeIds::effectHold) {
            setNumberAttribute<float>(ItemType::effect(), "hold", "Hold", ids, valueKeys, {0, 600, false, "s"});
        } else if (attribute.startsWith(QString(AttributeIds::effectHold) + ".")) {
            setIntegerSpecificNumberAttribute<float>(ItemType::effect(), "Hold", ids, attribute, valueKeys, "effect_step_hold", {1, 99}, {0, 600, false, "s"});
        } else if (attribute == AttributeIds::effectFade) {
            setNumberAttribute<float>(ItemType::effect(), "fade", "Fade", ids, valueKeys, {0, 600, false, "s"});
        } else if (attribute.startsWith(QString(AttributeIds::effectFade) + ".")) {
            setIntegerSpecificNumberAttribute<float>(ItemType::effect(), "Fade", ids, attribute, valueKeys, "effect_step_fade", {1, 99}, {0, 600, false, "s"});
        } else if (attribute == AttributeIds::effectPhase) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::effect(), "Phase", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "effect_fixture_phase", angleInfos);
            } else {
                setNumberAttribute<float>(ItemType::effect(), "phase", "Phase", ids, valueKeys, angleInfos);
            }
        } else if (attribute == AttributeIds::effectSineFade) {
            setBoolAttribute(ItemType::effect(), "sinefade", "Sine Fade", ids, valueKeys);
        } else {
            error("Unknown Effect Attribute.");
        }
    } else if (selectionType == ItemType::cuelist().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::cuelist(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::cuelist(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::cuelist(), "label", "Label", ids, "");
        } else if (attribute == AttributeIds::cuelistPriority) {
            setNumberAttribute<int>(ItemType::cuelist(), "priority", "Priority", ids, valueKeys, {1, 200});
        } else if (attribute == AttributeIds::cuelistMoveWhileDark) {
            setBoolAttribute(ItemType::cuelist(), "movewhiledark", "Move while Dark", ids, valueKeys);
        } else {
            error("Unknown Cuelist Attribute.");
        }
    } else if (selectionType == ItemType::cue().getKey()) {
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
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::cue(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::cue(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            setTextAttribute(ItemType::cue(), "label", "Label", ids, "");
        } else if (attribute == AttributeIds::cueIntensities) {
            if (attributes.contains(ItemType::group().getKey())) {
                setItemSpecificItemListAttribute(ItemType::cue(), "Intensities", ids, attributes.value(ItemType::group().getKey()), valueKeys, ItemType::group(), ItemType::intensity(), "cue_group_intensities", true);
            } else {
                error("Can't set Cue Intensities because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueColors) {
            if (attributes.contains(ItemType::group().getKey())) {
                setItemSpecificItemListAttribute(ItemType::cue(), "Colors", ids, attributes.value(ItemType::group().getKey()), valueKeys, ItemType::group(), ItemType::color(), "cue_group_colors", true);
            } else {
                error("Can't set Cue Colors because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cuePositions) {
            if (attributes.contains(ItemType::group().getKey())) {
                setItemSpecificItemListAttribute(ItemType::cue(), "Positions", ids, attributes.value(ItemType::group().getKey()), valueKeys, ItemType::group(), ItemType::position(), "cue_group_positions", true);
            } else {
                error("Can't set Cue Positions because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueRaws) {
            if (attributes.contains(ItemType::group().getKey())) {
                setItemSpecificItemListAttribute(ItemType::cue(), "Raws", ids, attributes.value(ItemType::group().getKey()), valueKeys, ItemType::group(), ItemType::raw(), "cue_group_raws");
            } else {
                error("Can't set Cue Raws because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueEffects) {
            if (attributes.contains(ItemType::group().getKey())) {
                setItemSpecificItemListAttribute(ItemType::cue(), "Effects", ids, attributes.value(ItemType::group().getKey()), valueKeys, ItemType::group(), ItemType::effect(), "cue_group_effects");
            } else {
                error("Can't set Cue Effects because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueBlock) {
            setBoolAttribute(ItemType::cue(), "block", "Block", ids, valueKeys);
        } else if ((attribute == AttributeIds::cueFade) || (!attributes.contains(Keys::Attribute))) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::cue(), "Fade", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "cue_fixture_fade", {0, 600, false, "s"});
            } else {
                setNumberAttribute<float>(ItemType::cue(), "fade", "Fade", ids, valueKeys, {0, 600, false, "s"});
            }
        } else if (attribute == AttributeIds::cueDelay) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                setItemSpecificNumberAttribute<float>(ItemType::cue(), "Delay", ids, attributes.value(ItemType::fixture().getKey()), valueKeys, ItemType::fixture(), "cue_fixture_delay", {0, 600, false, "s"});
            } else {
                setNumberAttribute<float>(ItemType::cue(), "delay", "Delay", ids, valueKeys, {0, 600, false, "s"});
            }
        } else if (attribute == AttributeIds::cueFollow) {
            setBoolAttribute(ItemType::cue(), "follow", "Follow", ids, valueKeys);
        } else if (attribute == AttributeIds::cueSineFade) {
            setBoolAttribute(ItemType::cue(), "sinefade", "Sine Fade", ids, valueKeys);
        } else {
            error("Unknown Cue Attribute.");
            return;
        }
    } else {
        error("Unknown Item type.");
        return;
    }
}

void Terminal::updateSortingKeys(const ItemType item) {
    QSqlQuery idsQuery;
    idsQuery.prepare("SELECT key, id, sortkey FROM " + item.getSelectTable());
    if (idsQuery.exec()) {
        struct IdKey {
            int key;
            QString id;
            int sortkey;
        };
        QList<IdKey> idKeys;
        while (idsQuery.next()) {
            IdKey idKey;
            idKey.key = idsQuery.value(0).toInt();
            idKey.id = idsQuery.value(1).toString();
            idKey.sortkey = idsQuery.value(2).toInt();
            idKeys.append(idKey);
        }
        std::sort(idKeys.begin(), idKeys.end(), [] (IdKey a, IdKey b) { return compareIds(a.id, b.id); });
        for (int index = 1; index <= idKeys.length(); index++) {
            const IdKey idKey = idKeys.at(index - 1);
            if (idKey.sortkey != index) {
                QSqlQuery query;
                query.prepare("UPDATE " + item.getUpdateTable() + " SET sortkey = :sortkey WHERE key = :key");
                query.bindValue(":key", idKey.key);
                query.bindValue(":sortkey", index);
                if (!query.exec()) {
                    qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                    error("Failed to update the sorting key of " + item.getSingular() + " " + idKey.id + ".");
                }
            }
        }
    } else {
        qWarning() << Q_FUNC_INFO << idsQuery.executedQuery() << idsQuery.lastError().text();
        error("Failed to update the " + item.getSingular() + " sorting keys.");
    }
}

bool Terminal::compareIds(const QString a, const QString b) {
    QStringList aParts = a.split(".");
    QStringList bParts = b.split(".");
    for (int part = 0; part < std::min(aParts.length(), bParts.length()); part++) {
        const int aPart = aParts.at(part).toInt();
        const int bPart = bParts.at(part).toInt();
        if (aPart != bPart) {
            return aPart < bPart;
        }
    }
    return (aParts.length() < bParts.length());
}

void Terminal::setCurrentItem(const ItemType item, const QString itemTable, const QList<Keys::Key> idKeys, const QString updateQueryText) {
    const QStringList ids = keysToIds(idKeys);
    if (ids.size() != 1) {
        error("Invalid " + item.getSingular() + " selection given.");
        return;
    }
    QSqlQuery keyQuery;
    keyQuery.prepare("SELECT key FROM " + itemTable + " WHERE id = :id");
    keyQuery.bindValue(":id", ids.first());
    if (!keyQuery.exec()) {
        qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
        error("Can't select " + item.getSingular() + " because the key request for " + item.getSingular() + " " + ids.first() + " failed.");
        return;
    }
    if (!keyQuery.next()) {
        error("Can't select " + item.getSingular() + " " + ids.first() + ".");
        return;
    }
    const int key = keyQuery.value(0).toInt();
    QSqlQuery updateQuery;
    updateQuery.prepare(updateQueryText);
    updateQuery.bindValue(":key", key);
    if (!updateQuery.exec()) {
        qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
        error("Failed to select " + item.getSingular() + ".");
    }
    emit dbChanged();
}

void Terminal::setCueItem(const ItemType item, const QString valueTable, const QList<Keys::Key> idKeys, const bool multipleItemsAllowed) {
    QList<int> itemKeys;
    if ((idKeys.size() != 2) || !idKeys.endsWith(Keys::Minus)) {
        const QStringList ids = keysToIds(idKeys);
        if (ids.isEmpty()) {
            error("Invalid " + item.getSingular() + " selection given.");
            return;
        }
        if (!multipleItemsAllowed && (ids.size() > 1)) {
            error("Can't select multiple " + item.getPlural() + ".");
            return;
        }
        for (QString id : ids) {
            QSqlQuery itemKeyQuery;
            itemKeyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
            itemKeyQuery.bindValue(":id", id);
            if (itemKeyQuery.exec()) {
                if (itemKeyQuery.next()) {
                    itemKeys.append(itemKeyQuery.value(0).toInt());
                } else {
                    warning("Can't select " + item.getSingular() + " " + id + " because this " + item.getSingular() + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << itemKeyQuery.executedQuery() << itemKeyQuery.lastError().text();
                error("Can't set Cue " + item.getPlural() + " because the key request for " + item.getSingular() + " " + ids.first() + " failed.");
            }
        }
        if (itemKeys.isEmpty()) {
            error("No valid " + item.getPlural() + " were given.");
            return;
        }
    }
    QSqlQuery groupKeyQuery;
    if (!groupKeyQuery.exec("SELECT group_key FROM currentitems WHERE group_key IS NOT NULL")) {
        qWarning() << Q_FUNC_INFO << groupKeyQuery.executedQuery() << groupKeyQuery.lastError().text();
        error("Can't set Cue " + item.getPlural() + " because request for the current Group failed.");
        return;
    }
    if (!groupKeyQuery.next()) {
        error("Can't set Cue " + item.getPlural() + " because no Group is currently selected.");
        return;
    }
    const int groupKey = groupKeyQuery.value(0).toInt();
    QList<int> cueKeys;
    QSqlQuery cueKeyQuery;
    if (!cueKeyQuery.exec("SELECT key, sortkey FROM currentcue")) {
        qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
        error("Can't set Cue " + item.getPlural() + " because the request for the current Cue failed.");
        return;
    }
    if (!cueKeyQuery.next()) {
        error("Can't set Cue " + item.getPlural() + " because no Cue is currently selected.");
        return;
    }
    cueKeys.append(cueKeyQuery.value(0).toInt());
    if (trackingButton->isChecked()) {
        QSqlQuery currentCueValueQuery;
        currentCueValueQuery.prepare("SELECT valueItem_key FROM " + valueTable + " WHERE item_key = :cue AND foreignItem_key = :group");
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
                    cueValueQuery.prepare("SELECT valueItem_key FROM " + valueTable + " WHERE item_key = :cue AND foreignItem_key = :group");
                    cueValueQuery.bindValue(":cue", cueKey);
                    cueValueQuery.bindValue(":group", groupKey);
                    if (cueValueQuery.exec()) {
                        QSet<int> cueValueKeys;
                        while (cueValueQuery.next()) {
                            cueValueKeys.insert(cueValueQuery.value(0).toInt());
                        }
                        sameValue = cueValueKeys == currentCueValueKeys;
                        if (sameValue) {
                            cueKeys.append(cueKey);
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
                        error("Can't set Cue " + item.getPlural() + " because the Cue tracking request failed.");
                        return;
                    }
                }
            } else {
                qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
                error("Can't set Cue " + item.getPlural() + " because the Cue tracking request failed.");
                return;
            }
        } else {
            qWarning() << Q_FUNC_INFO << cueKeyQuery.executedQuery() << cueKeyQuery.lastError().text();
            error("Can't set Cue " + item.getPlural() + " because the request for the " + item.getSingular() + " in the current Cue failed.");
            return;
        }
    }
    for (const int cueKey : cueKeys) {
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key = :cue AND foreignItem_key = :group");
        deleteQuery.bindValue(":cue", cueKey);
        deleteQuery.bindValue(":group", groupKey);
        if (!deleteQuery.exec()) {
            qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
            error("Failed deleting Cue " + item.getPlural() + ".");
        }
        for (const int key : itemKeys) {
            QSqlQuery query;
            query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignItem_key, valueItem_key) VALUES (:cue, :group, :item)");
            query.bindValue(":cue", cueKey);
            query.bindValue(":group", groupKey);
            query.bindValue(":item", key);
            if (!query.exec()) {
                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                error("Failed inserting " + item.getSingular() + ".");
            }
        }
    }
    emit dbChanged();
}

void Terminal::createItems(const ItemType item, QStringList ids) {
    Q_ASSERT(!ids.isEmpty());
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        existsQuery.bindValue(":id", id);
        if (existsQuery.exec()) {
            if (!existsQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO " + item.getUpdateTable() + " (id, sortkey) VALUES (:id, 0)");
                insertQuery.bindValue(":id", id);
                if (insertQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                    error("Failed to insert " + item.getSingular() + " " + id + ".");
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << existsQuery.executedQuery() << existsQuery.lastError().text();
            error("Failed to check if " + item.getSingular() + " " + id + " already exists.");
        }
    }
    if (!successfulIds.isEmpty()) {
        success("Created " + item.format(successfulIds) + ".");
    }
    updateSortingKeys(item);
    if (item == ItemType::cue()) {
        for (QString id : successfulIds) {
            QSqlQuery keyQuery;
            keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
            keyQuery.bindValue(":id", id);
            if (keyQuery.exec()) {
                if (keyQuery.next()) {
                    const int key = keyQuery.value(0).toInt();
                    QSqlQuery previousCueQuery;
                    previousCueQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE sortkey = (SELECT MAX(sortkey) FROM " + item.getSelectTable() + " WHERE sortkey < (SELECT sortkey FROM " + item.getSelectTable() + " WHERE key = :key))");
                    previousCueQuery.bindValue(":key", key);
                    if (previousCueQuery.exec()) {
                        if (previousCueQuery.next()) {
                            const int previousCueKey = previousCueQuery.value(0).toInt();
                            QStringList tables;
                            tables.append("cue_group_intensities");
                            tables.append("cue_group_colors");
                            tables.append("cue_group_positions");
                            tables.append("cue_group_raws");
                            tables.append("cue_group_effects");
                            for (QString table : tables) {
                                QSqlQuery valueQuery;
                                valueQuery.prepare("SELECT foreignItem_key, valueItem_key FROM " + table + " WHERE item_key = :key");
                                valueQuery.bindValue(":key", previousCueKey);
                                if (valueQuery.exec()) {
                                    while (valueQuery.next()) {
                                        QSqlQuery updateQuery;
                                        updateQuery.prepare("INSERT INTO " + table + " (item_key, foreignItem_key, valueItem_key) VALUES (:key, :foreignItem, :valueItem)");
                                        updateQuery.bindValue(":key", key);
                                        updateQuery.bindValue(":foreignItem", valueQuery.value(0).toInt());
                                        updateQuery.bindValue(":valueItem", valueQuery.value(1).toInt());
                                        if (!updateQuery.exec()) {
                                            qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                                            error("Failed to copy data of the previous " + item.getSingular() + " to " + item.getSingular() + " " + id + ".");
                                        }
                                    }
                                } else {
                                    qWarning() << Q_FUNC_INFO << valueQuery.executedQuery() << valueQuery.lastError().text();
                                    error("Failed to copy the data of the previous " + item.getSingular() + " to " + item.getSingular() + " " + id + ".");
                                }
                            }
                        }
                    } else {
                        qWarning() << Q_FUNC_INFO << previousCueQuery.executedQuery() << previousCueQuery.lastError().text();
                        error("Failed to get the " + item.getSingular() + " before " + item.getSingular() + " " + id + ".");
                    }
                } else {
                    error(item.getSingular() + " " + id + " wasn't found.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                error("Failed to get " + item.getSingular() + " " + id + ".");
            }
        }
    }
}

void Terminal::deleteItems(const ItemType item, QStringList ids) {
    Q_ASSERT(!ids.isEmpty());
    QMessageBox msgBox;
    msgBox.setText("Delete " + QString::number(ids.length()) + " " + item.getPlural() + "?");
    msgBox.setInformativeText("Do you want to delete " + item.getSingular() + " " + ids.join(", ") + "?");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::Yes);
    if (msgBox.exec() != QMessageBox::Yes) {
        error("Popup canceled.");
        return;
    }
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM " + item.getUpdateTable() + " WHERE key = :key");
                deleteQuery.bindValue(":key", keyQuery.value(0).toInt());
                if (deleteQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    error("Can't delete " + item.getSingular() + " because the request failed.");
                }
            } else {
                warning("Can't delete " + item.getSingular() + " " + id + " because this " + item.getSingular() + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Couldn't delete " + item.getSingular() + " " + id + ": ");
        }
    }
    if (!successfulIds.isEmpty()) {
        success("Deleted " + item.format(successfulIds) + ".");
    }
    updateSortingKeys(item);
    emit dbChanged();
}

void Terminal::moveItems(const ItemType item, QStringList ids, QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    valueKeys.prepend(item.getKey());
    QStringList newIds = keysToIds(valueKeys);
    if (newIds.size() != 1) {
        error("Can't set " + item.getSingular() + " ID because an invalid ID was given.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery existsQuery;
        existsQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        existsQuery.bindValue(":id", newIds.first());
        if (existsQuery.exec()) {
            if (existsQuery.next()) {
                warning("Can't set ID of " + item.getSingular() + " to " + newIds.first() + " because this " + item.getSingular() + " ID is already used.");
            } else {
                QSqlQuery keyQuery;
                keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
                keyQuery.bindValue(":id", id);
                if (keyQuery.exec()) {
                    if (keyQuery.next()) {
                        QSqlQuery updateQuery;
                        updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET id = :newId WHERE key = :key");
                        updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                        updateQuery.bindValue(":newId", newIds.first());
                        if (updateQuery.exec()) {
                            successfulIds.append(id);
                        } else {
                            qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                            error("Failed to update ID of " + item.getSingular() + " " + id + " because the request failed.");
                        }
                    } else {
                        warning("Can't set ID of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " doesn't exist.");
                    }
                } else {
                    qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                    error("Failed loading " + item.getSingular() + " " + id + ".");
                }
            }
        } else {
            qWarning() << Q_FUNC_INFO << existsQuery.executedQuery() << existsQuery.lastError().text();
            error("Error executing check if " + item.getSingular() + " " + newIds.first() + " exists.");
        }
    }
    if (!successfulIds.isEmpty()) {
        success("Set ID of " + item.format(successfulIds) + " to " + newIds.first() + ".");
    }
    updateSortingKeys(item);
    emit dbChanged();
}

void Terminal::setBoolAttribute(const ItemType item, const QString attribute, const QString attributeName, QStringList ids, QList<Keys::Key> valueKeys) {
    Q_ASSERT(!ids.isEmpty());
    int value = 0;
    QString valueText = "False";
    if ((valueKeys.size() == 1) && valueKeys.startsWith(Keys::Zero)) {
    } else if ((valueKeys.size() == 1) && valueKeys.startsWith(Keys::One)) {
        value = 1;
        valueText = "True";
    } else {
        error("Can't set " + item.getSingular() + " " + attributeName + " because no valid value was given.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + attribute + " = :value WHERE key = :key");
                updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                updateQuery.bindValue(":value", value);
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    error("Failed setting " + attributeName + " of " + item.getSingular() + " " + id + ".");
                }
            } else {
                warning("Failed to set " + attributeName + " of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        success("Set " + attributeName + " of " + item.format(successfulIds) + " to " + valueText + ".");
    }
    emit dbChanged();
}

void Terminal::setTextAttribute(const ItemType item, const QString attribute, const QString attributeName, QStringList ids, const QString regex) {
    Q_ASSERT(!ids.isEmpty());
    QString textValue = QString();
    if (ids.length() == 1) {
        QSqlQuery query;
        query.prepare("SELECT " + attribute + " FROM " + item.getSelectTable() + " WHERE id = :id");
        query.bindValue(":id", ids.first());
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
            error("Failed to load current " + attributeName + " of " + item.getSingular() + " " + ids.first() + ".");
            return;
        }
        while (query.next()) {
            textValue = query.value(0).toString();
        }
    }
    bool ok;
    textValue = QInputDialog::getText(this, QString(), (item.getSingular()+ " " + attributeName), QLineEdit::Normal, textValue, &ok);
    if (!ok) {
        error("Popup canceled.");
        return;
    }
    if (!regex.isEmpty() && !textValue.contains(QRegularExpression(regex))) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given value \"" + textValue + "\" is not valid.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + attribute + " = :value WHERE key = :key");
                updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                updateQuery.bindValue(":value", textValue);
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    error("Failed setting " + attributeName + " of " + item.getSingular() + " " + id + ".");
                }
            } else {
                warning("Failed to set " + attributeName + " of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        success("Set " + attributeName + " of " + item.format(successfulIds) + " to \"" + textValue + "\".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setNumberAttribute(const ItemType item, const QString attribute, const QString attributeName, QStringList ids, QList<Keys::Key> valueKeys, const NumberInfos number) {
    Q_ASSERT(!ids.isEmpty());
    const bool difference = valueKeys.startsWith(Keys::Plus);
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
            currentValueQuery.prepare("SELECT " + attribute + " FROM " + item.getSelectTable() + " WHERE id = :id");
            currentValueQuery.bindValue(":id", id);
            if (currentValueQuery.exec()) {
                if (currentValueQuery.next()) {
                    value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
                    if (!valueOk) {
                        error("Invalid value given for " + item.getSingular() + " " + id + ".");
                    }
                } else {
                    error("Failed loading the current " + attributeName + " of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " doesn't exist.");
                    valueOk = false;
                }
            } else {
                qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                error("Failed loading the current " + attributeName + " of " + item.getSingular() + " " + id + ".");
                valueOk = false;
            }
        }
        if (valueOk) {
            QSqlQuery keyQuery;
            keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
            keyQuery.bindValue(":id", id);
            if (keyQuery.exec()) {
                if (keyQuery.next()) {
                    QSqlQuery updateQuery;
                    updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + attribute + " = :value WHERE key = :key");
                    updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                    updateQuery.bindValue(":value", value);
                    if (updateQuery.exec()) {
                        successfulIds.append(id);
                    } else {
                        qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                        error("Failed setting " + attributeName + " of " + item.getSingular() + " " + id + ".");
                    }
                } else {
                    warning("Failed to set " + attributeName + " of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
                error("Failed loading " + item.getSingular() + " " + id + ".");
            }
        }
    }
    if (!successfulIds.isEmpty()) {
        if (difference) {
            success("Changed " + attributeName + " of " + item.format(successfulIds) + " by " + QString::number(value) + number.unit + ".");
        } else {
            success("Set " + attributeName + " of " + item.format(successfulIds) + " to " + QString::number(value) + number.unit + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemAttribute(const ItemType item, const QString attribute, const QString attributeName, QStringList ids, QList<Keys::Key> valueKeys, const ItemType foreignItem) {
    Q_ASSERT(!ids.isEmpty());
    const bool removeItem = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    QString foreignItemId;
    int foreignItemKey;
    QStringList successfulIds;
    if (!removeItem) {
        if (!valueKeys.startsWith(foreignItem.getKey())) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because no " + foreignItem.getSingular() + " was given.");
            return;
        }
        QStringList foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.size() != 1) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because the given " + foreignItem.getSingular() + " ID is invalid.");
            return;
        }
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.getSelectTable() + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemIds.first());
        if (!foreignItemQuery.exec()) {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.getSingular() + " exists.");
            return;
        }
        if (!foreignItemQuery.next()) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because " + foreignItem.getSingular() + " " + foreignItemIds.first() + " doesn't exist.");
            return;
        }
        foreignItemKey = foreignItemQuery.value(0).toInt();
    }
    createItems(item, ids);
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                QSqlQuery updateQuery;
                if (removeItem) {
                    updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + attribute + " = NULL WHERE key = :key");
                } else {
                    updateQuery.prepare("UPDATE " + item.getUpdateTable() + " SET " + attribute + " = :item WHERE key = :key");
                    updateQuery.bindValue(":item", foreignItemKey);
                }
                updateQuery.bindValue(":key", keyQuery.value(0).toInt());
                if (updateQuery.exec()) {
                    successfulIds.append(id);
                } else {
                    qWarning() << Q_FUNC_INFO << updateQuery.executedQuery() << updateQuery.lastError().text();
                    error("Failed setting " + attributeName + " of " + item.getSingular() + " " + id + ".");
                }
            } else {
                warning("Failed to set " + attributeName + " of " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        if (removeItem) {
            success("Removed " + attributeName + " of " + item.format(successfulIds) + ".");
        } else {
            success("Set " + attributeName + " of " + item.format(successfulIds) + " to " + foreignItem.getSingular() + " " + foreignItemId + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemListAttribute(const ItemType item, const QString attributeName, QStringList ids, QList<Keys::Key> valueKeys, const ItemType foreignItem, const QString valueTable) {
    Q_ASSERT(!ids.isEmpty());
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    if ((valueKeys.size() != 1) || !valueKeys.startsWith(Keys::Minus)) {
        if (!valueKeys.startsWith(foreignItem.getKey())) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because no " + foreignItem.getPlural() + " were given.");
            return;
        }
        const QStringList foreignItemIds = keysToIds(valueKeys);
        if (foreignItemIds.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because the given " + foreignItem.getSingular() + " IDs are invalid.");
            return;
        }
        for (QString foreignItemId : foreignItemIds) {
            QSqlQuery foreignItemQuery;
            foreignItemQuery.prepare("SELECT key FROM " + foreignItem.getSelectTable() + " WHERE id = :id");
            foreignItemQuery.bindValue(":id", foreignItemId);
            if (foreignItemQuery.exec()) {
                if (foreignItemQuery.next()) {
                    foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                    foreignItemIdStrings.append(foreignItemId);
                } else {
                    warning("Can't add " + foreignItem.getSingular() + " " + foreignItemId + " to " + item.getSingular() + " " + attributeName + " because this " + foreignItem.getSingular() + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
                error("Failed to execute check if " + foreignItem.getSingular() + " " + foreignItemId + " exists.");
            }
        }
        Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
        if (foreignItemKeys.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because no valid " + foreignItem.getPlural() + " were given.");
            return;
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
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
                        insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, valueItem_key) VALUES (:item, :foreign_item)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        if (!insertQuery.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                            error("Failed to insert a " + foreignItem.getSingular() + " into " + item.getSingular() + " " + id + ".");
                        }
                    }
                } else {
                    allQueriesSuccessful = false;
                    qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                    error("Failed deleting old " + attributeName + " of " + item.getSingular() + " " + id + ".");
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    QString foreignItemString = foreignItem.getPlural() + foreignItemIdStrings.join(", ");
    if (foreignItemIdStrings.size() == 1) {
        foreignItemString = foreignItem.getSingular() + foreignItemIdStrings.join(", ");
    }
    if (!successfulIds.isEmpty()) {
        success("Set " + attributeName + " of " + item.format(successfulIds) + " to " + foreignItemString + ".");
    }
    emit dbChanged();
}

template <typename T> void Terminal::setItemSpecificNumberAttribute(const ItemType item, const QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Keys::Key> valueKeys, const ItemType foreignItem, const QString valueTable, const NumberInfos number) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(!foreignItemIds.isEmpty());
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    const bool difference = valueKeys.startsWith(Keys::Plus);
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
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.getSelectTable() + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.getSingular() + " " + foreignItemId + " because this " + foreignItem.getSingular() + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.getSingular() + " " + foreignItemId + " exists.");
        }
    }
    Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
    if (foreignItemKeys.isEmpty()) {
        error("No valid " + foreignItem.getPlural() + " were found.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                for (const int foreignItemKey : foreignItemKeys) {
                    if (removeValues) {
                        QSqlQuery query;
                        query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND foreignItem_key = :foreign_item");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":foreign_item", foreignItemKey);
                        if (!query.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            error("Failed removing the " + attributeName + " of " + item.getSingular() + " " + id + ".");
                        }
                    } else {
                        bool valueOk = true;
                        if (difference) {
                            QSqlQuery currentValueQuery;
                            currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND foreignItem_key = :foreign_item");
                            currentValueQuery.bindValue(":item", itemKey);
                            currentValueQuery.bindValue(":foreign_item", foreignItemKey);
                            if (currentValueQuery.exec()) {
                                if (currentValueQuery.next()) {
                                    value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
                                } else {
                                    value = keysToFloat(valueKeys, &valueOk, currentValueQuery.value(0).toFloat(), number);
                                }
                                if (!valueOk) {
                                    error("Invalid value given for " + item.getSingular() + " " + id + ".");
                                }
                            } else {
                                qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                                error("Failed loading the current " + attributeName + " of " + item.getSingular() + " " + id + ".");
                                valueOk = false;
                            }
                        }
                        if (valueOk) {
                            QSqlQuery query;
                            query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignItem_key, value) VALUES (:item, :foreign_item, :value)");
                            query.bindValue(":item", itemKey);
                            query.bindValue(":foreign_item", foreignItemKey);
                            query.bindValue(":value", value);
                            if (!query.exec()) {
                                allQueriesSuccessful = false;
                                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                                error("Failed removing the " + attributeName + " of " + item.getSingular() + " " + id + ".");
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
                error("Failed loading " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    QString foreignItemString = foreignItem.getPlural() + " " + foreignItemIdStrings.join(", ");
    if (foreignItemIdStrings.length() == 1) {
        foreignItemString = foreignItem.getSingular() + " " + foreignItemIdStrings.join(", ");
    }
    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            success("Removed " + attributeName + " of " + item.format(successfulIds) + " at " + foreignItemString + ".");
        } else if (difference) {
            success("Changed " + attributeName + " of " + item.format(successfulIds) + " at " + foreignItemString + " by " + QString::number(value) + number.unit + ".");
        } else {
            success("Set " + attributeName + " of " + item.format(successfulIds) + " at " + foreignItemString + " to " + QString::number(value) + number.unit + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setItemSpecificItemListAttribute(const ItemType item, const QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Keys::Key> valueKeys, const ItemType foreignItem, const ItemType valueItem, const QString valueTable, const bool limitToOne) {
    Q_ASSERT(!ids.isEmpty());
    Q_ASSERT(!foreignItemIds.isEmpty());
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    QList<int> valueItemKeys;
    QStringList valueItemIdStrings;
    if (!removeValues) {
        if (!valueKeys.startsWith(valueItem.getKey())) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because no " + valueItem.getPlural() + " were given.");
            return;
        }
        const QStringList valueItemIds = keysToIds(valueKeys);
        if (valueItemIds.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because the given " + valueItem.getSingular() + " IDs are invalid.");
            return;
        }
        if (limitToOne && (valueItemIds.size() != 1)) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because this Attribute only accepts one " + valueItem.getSingular() + " as a value.");
            return;
        }
        for (QString valueItemId : valueItemIds) {
            QSqlQuery valueItemQuery;
            valueItemQuery.prepare("SELECT key FROM " + valueItem.getSelectTable() + " WHERE id = :id");
            valueItemQuery.bindValue(":id", valueItemId);
            if (valueItemQuery.exec()) {
                if (valueItemQuery.next()) {
                    valueItemKeys.append(valueItemQuery.value(0).toInt());
                    valueItemIdStrings.append(valueItemId);
                } else {
                    warning("Can't add " + valueItem.getSingular() + " " + valueItemId + " to " + item.getSingular() + " " + attributeName + " because this " + valueItem.getSingular() + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << valueItemQuery.executedQuery() << valueItemQuery.lastError().text();
                error("Failed to execute check if " + valueItem.getSingular() + " " + valueItemId + " exists.");
            }
        }
        Q_ASSERT(valueItemKeys.length() == valueItemIdStrings.length());
        if (valueItemKeys.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because no valid " + valueItem.getPlural() + " were given.");
            return;
        }
    }
    QList<int> foreignItemKeys;
    QStringList foreignItemIdStrings;
    for (QString foreignItemId : foreignItemIds) {
        QSqlQuery foreignItemQuery;
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.getSelectTable() + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.getSingular() + " " + foreignItemId + " because this " + foreignItem.getSingular() + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.getSingular() + " " + foreignItemId + " exists.");
        }
    }
    Q_ASSERT(foreignItemKeys.size() == foreignItemIdStrings.size());
    if (foreignItemKeys.isEmpty()) {
        error("No valid " + foreignItem.getPlural() + " were found.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                for (const int foreignItemKey : foreignItemKeys) {
                    QSqlQuery deleteQuery;
                    deleteQuery.prepare("DELETE FROM " + valueTable + " WHERE item_key =  :item AND foreignItem_key = :foreign_item");
                    deleteQuery.bindValue(":item", itemKey);
                    deleteQuery.bindValue(":foreign_item", foreignItemKey);
                    if (!deleteQuery.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << deleteQuery.executedQuery() << deleteQuery.lastError().text();
                        error("Failed deleting old " + attributeName + " of " + item.getSingular() + " " + id + ".");
                    }
                    for (const int valueItemKey : valueItemKeys) {
                        QSqlQuery insertQuery;
                        insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, foreignItem_key, valueItem_key) VALUES (:item, :foreign_item, :value_item)");
                        insertQuery.bindValue(":item", itemKey);
                        insertQuery.bindValue(":foreign_item", foreignItemKey);
                        insertQuery.bindValue(":value_item", valueItemKey);
                        if (!insertQuery.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                            error("Failed to insert a " + foreignItem.getSingular() + " into " + item.getSingular() + " " + id + ".");
                        }
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            success("Removed " + attributeName + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + ".");
        } else {
            success("Set " + attributeName + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " to " + valueItem.format(valueItemIdStrings) + ".");
        }
    }
    emit dbChanged();
}

template <typename T> void Terminal::setIntegerSpecificNumberAttribute(const ItemType item, const QString attributeName, QStringList ids, QString numberId, QList<Keys::Key> valueKeys, const QString valueTable, const NumberInfos keyNumber, const NumberInfos valueNumber) {
    Q_ASSERT(!ids.isEmpty());
    QList<QString> numberIdParts = numberId.split(".");
    if (numberIdParts.length() != 2) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    bool ok;
    int key = numberIdParts.last().toInt(&ok);
    if (!ok) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    key = keysToFloat({Keys::Plus, Keys::Zero}, &ok, key, keyNumber);
    if (!ok) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    const bool difference = valueKeys.startsWith(Keys::Plus);
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
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
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
                        error("Failed removing the " + attributeName + " of " + item.getSingular() + " " + id + ".");
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
                                error("Invalid value given for " + item.getSingular() + " " + id + ".");
                            }
                        } else {
                            qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                            error("Failed loading the current " + attributeName + " of " + item.getSingular() + " " + id + ".");
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
                            error("Failed removing the " + attributeName + " of " + item.getSingular() + " " + id + ".");
                        }
                    }
                }
            } else {
                error("Failed loading " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            success("Removed " + attributeName + " of " + item.format(successfulIds) + " at " + QString::number(key) + ".");
        } else if (difference) {
            success("Changed " + attributeName + " of " + item.format(successfulIds) + " at " + QString::number(key) + keyNumber.unit + " by " + QString::number(value) + valueNumber.unit + ".");
        } else {
            success("Set " + attributeName + " of " + item.format(successfulIds) + " at " + QString::number(key) + keyNumber.unit + " to " + QString::number(value) + valueNumber.unit + ".");
        }
    }
    emit dbChanged();
}

void Terminal::setIntegerSpecificItemListAttribute(ItemType item, QString attributeName, QStringList ids, QString integerId, QList<Keys::Key> valueKeys, ItemType valueItem, QString valueTable, NumberInfos keyInteger, bool limitToOne) {
    Q_ASSERT(!ids.isEmpty());
    QList<QString> numberIdParts = integerId.split(".");
    if (numberIdParts.length() != 2) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    bool ok;
    int key = numberIdParts.last().toInt(&ok);
    if (!ok) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    key = keysToFloat({Keys::Plus, Keys::Zero}, &ok, key, keyInteger);
    if (!ok) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    QList<int> valueItemKeys;
    QStringList valueItemIdStrings;
    if (!removeValues) {
        if (!valueKeys.startsWith(valueItem.getKey())) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because no " + valueItem.getPlural() + " were given.");
            return;
        }
        const QStringList valueItemIds = keysToIds(valueKeys);
        if (valueItemIds.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because the given " + valueItem.getSingular() + " IDs are invalid.");
            return;
        }
        if (limitToOne && (valueItemIds.size() != 1)) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because this Attribute only accepts one " + valueItem.getSingular() + " as a value.");
            return;
        }
        for (QString valueItemId : valueItemIds) {
            QSqlQuery valueItemQuery;
            valueItemQuery.prepare("SELECT key FROM " + valueItem.getSelectTable() + " WHERE id = :id");
            valueItemQuery.bindValue(":id", valueItemId);
            if (valueItemQuery.exec()) {
                if (valueItemQuery.next()) {
                    valueItemKeys.append(valueItemQuery.value(0).toInt());
                    valueItemIdStrings.append(valueItemId);
                } else {
                    warning("Can't add " + valueItem.getSingular() + " " + valueItemId + " to " + item.getSingular() + " " + attributeName + " because this " + valueItem.getSingular() + " doesn't exist.");
                }
            } else {
                qWarning() << Q_FUNC_INFO << valueItemQuery.executedQuery() << valueItemQuery.lastError().text();
                error("Failed to execute check if " + valueItem.getSingular() + " " + valueItemId + " exists.");
            }
        }
        Q_ASSERT(valueItemKeys.length() == valueItemIdStrings.length());
        if (valueItemKeys.isEmpty()) {
            error("Can't set " + item.getSingular() + " " + attributeName + " because no valid " + valueItem.getPlural() + " were given.");
            return;
        }
        if (valueItemKeys.isEmpty()) {
            error("No valid " + valueItem.getPlural() + " were found.");
            return;
        }
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
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
                    error("Failed deleting old " + attributeName + " of " + item.getSingular() + " " + id + ".");
                }
                for (const int valueItemKey : valueItemKeys) {
                    QSqlQuery insertQuery;
                    insertQuery.prepare("INSERT INTO " + valueTable + " (item_key, key, valueItem_key) VALUES (:item, :key, :value_item)");
                    insertQuery.bindValue(":item", itemKey);
                    insertQuery.bindValue(":key", key);
                    insertQuery.bindValue(":value_item", valueItemKey);
                    if (!insertQuery.exec()) {
                        allQueriesSuccessful = false;
                        qWarning() << Q_FUNC_INFO << insertQuery.executedQuery() << insertQuery.lastError().text();
                        error("Failed to insert a " + valueItem.getSingular() + " into " + item.getSingular() + " " + id + ".");
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            success("Removed " + attributeName + " of " + item.format(successfulIds) + " at " + QString::number(key) + ".");
        } else {
            success("Set " + attributeName + " of " + item.format(successfulIds) + " at " + QString::number(key) + " to " + valueItem.format(valueItemIdStrings) + ".");
        }
    }
    emit dbChanged();
}

template <typename T> void Terminal::setItemAndIntegerSpecificNumberAttribute(ItemType item, QString attributeName, QStringList ids, QStringList foreignItemIds, QString numberId, QList<Keys::Key> valueKeys, ItemType foreignItem, QString valueTable, NumberInfos keyNumber, NumberInfos valueNumber) {
    Q_ASSERT(!ids.isEmpty());
    QList<QString> numberIdParts = numberId.split(".");
    if (numberIdParts.length() != 2) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    bool ok;
    int key = numberIdParts.last().toInt(&ok);
    if (!ok) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    key = keysToFloat({Keys::Plus, Keys::Zero}, &ok, key, keyNumber);
    if (!ok) {
        error("Can't set " + item.getSingular() + " " + attributeName + " because the given Attribute is not valid.");
        return;
    }
    const bool removeValues = (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus);
    const bool difference = valueKeys.startsWith(Keys::Plus);
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
        foreignItemQuery.prepare("SELECT key FROM " + foreignItem.getSelectTable() + " WHERE id = :id");
        foreignItemQuery.bindValue(":id", foreignItemId);
        if (foreignItemQuery.exec()) {
            if (foreignItemQuery.next()) {
                foreignItemKeys.append(foreignItemQuery.value(0).toInt());
                foreignItemIdStrings.append(foreignItemId);
            } else {
                warning("Can't set " + attributeName + " for " + foreignItem.getSingular() + " " + foreignItemId + " because this " + foreignItem.getSingular() + " doesn't exist.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << foreignItemQuery.executedQuery() << foreignItemQuery.lastError().text();
            error("Failed to execute check if " + foreignItem.getSingular() + " " + foreignItemId + " exists.");
        }
    }
    Q_ASSERT(foreignItemKeys.length() == foreignItemIdStrings.length());
    if (foreignItemKeys.isEmpty()) {
        error("No valid " + foreignItem.getPlural() + " were found.");
        return;
    }
    createItems(item, ids);
    QStringList successfulIds;
    for (QString id : ids) {
        QSqlQuery keyQuery;
        keyQuery.prepare("SELECT key FROM " + item.getSelectTable() + " WHERE id = :id");
        keyQuery.bindValue(":id", id);
        if (keyQuery.exec()) {
            if (keyQuery.next()) {
                const int itemKey = keyQuery.value(0).toInt();
                bool allQueriesSuccessful = true;
                for (const int foreignItemKey : foreignItemKeys) {
                    if (removeValues) {
                        QSqlQuery query;
                        query.prepare("DELETE FROM " + valueTable + " WHERE item_key = :item AND foreignItem_key = :foreign_item AND key = :key");
                        query.bindValue(":item", itemKey);
                        query.bindValue(":foreign_item", foreignItemKey);
                        query.bindValue(":key", key);
                        if (!query.exec()) {
                            allQueriesSuccessful = false;
                            qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                            error("Failed removing the " + attributeName + " of " + item.getSingular() + " " + id + ".");
                        }
                    } else {
                        bool valueOk = true;
                        if (difference) {
                            QSqlQuery currentValueQuery;
                            currentValueQuery.prepare("SELECT value FROM " + valueTable + " WHERE item_key = :item AND foreignItem_key = :foreign_item AND key = :key");
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
                                    error("Invalid value given for " + item.getSingular() + " " + id + ".");
                                }
                            } else {
                                qWarning() << Q_FUNC_INFO << currentValueQuery.executedQuery() << currentValueQuery.lastError().text();
                                error("Failed loading the current " + attributeName + " of " + item.getSingular() + " " + id + ".");
                                valueOk = false;
                            }
                        }
                        if (valueOk) {
                            QSqlQuery query;
                            query.prepare("INSERT OR REPLACE INTO " + valueTable + " (item_key, foreignItem_key, key, value) VALUES (:item, :foreign_item, :key, :value)");
                            query.bindValue(":item", itemKey);
                            query.bindValue(":foreign_item", foreignItemKey);
                            query.bindValue(":key", key);
                            query.bindValue(":value", value);
                            if (!query.exec()) {
                                allQueriesSuccessful = false;
                                qWarning() << Q_FUNC_INFO << query.executedQuery() << query.lastError().text();
                                error("Failed removing the " + attributeName + " of " + item.getSingular() + " " + id + ".");
                            }
                        }
                    }
                }
                if (allQueriesSuccessful) {
                    successfulIds.append(id);
                }
            } else {
                error("Failed loading " + item.getSingular() + " " + id + " because this " + item.getSingular() + " wasn't found.");
            }
        } else {
            qWarning() << Q_FUNC_INFO << keyQuery.executedQuery() << keyQuery.lastError().text();
            error("Failed loading " + item.getSingular() + " " + id + ".");
        }
    }
    if (!successfulIds.isEmpty()) {
        if (removeValues) {
            success("Removed " + attributeName + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " and " + QString::number(key) + keyNumber.unit + ".");
        } else if (difference) {
            success("Changed " + attributeName + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " and " + QString::number(key) + keyNumber.unit + " by " + QString::number(value) + valueNumber.unit + ".");
        } else {
            success("Set " + attributeName + " of " + item.format(successfulIds) + " at " + foreignItem.format(foreignItemIdStrings) + " and " + QString::number(key) + keyNumber.unit + " to " + QString::number(value) + valueNumber.unit + ".");
        }
    }
    emit dbChanged();
}

void Terminal::reload() {
    QList<Keys::Key> idKeys;
    bool append = true;
    for (const Keys::Key key : promptKeys) {
        if (Keys::isItemKey(key)) {
            append = true;
            idKeys.clear();
        } else if ((key == Keys::Attribute) || (key == Keys::Set)) {
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
        emit itemChanged(Keys::keysToString({ idKeys.first() }), keysToIds(idKeys));
    }
}

void Terminal::writeKey(Keys::Key key) {
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

float Terminal::keysToFloat(QList<Keys::Key> keys, bool* ok, const float currentValue, const NumberInfos number) const {
    const bool difference = keys.startsWith(Keys::Plus);
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
            value += number.maxValue - number.minValue;
        }
        while (value >= number.maxValue) {
            value -= number.maxValue - number.minValue;
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

QStringList Terminal::keysToIds(QList<Keys::Key> keys) const {
    if (keys.isEmpty()) {
        return QStringList();
    }
    const Keys::Key itemType = keys.first();
    keys.removeFirst();
    QStringList ids;
    if (keys.isEmpty()) {
        QSqlQuery query;
        if (itemType == ItemType::model().getKey()) {
            query.prepare("SELECT models.id FROM models, currentfixtures WHERE currentfixtures.model_key = models.key");
        } else if (itemType == ItemType::fixture().getKey()) {
            query.prepare("SELECT id FROM currentfixtures");
        } else if (itemType == ItemType::group().getKey()) {
            query.prepare("SELECT groups.id FROM groups, currentitems WHERE groups.key = currentitems.group_key");
        } else if (itemType == ItemType::intensity().getKey()) {
            query.prepare("SELECT intensities.id FROM intensities, currentcue, currentitems, cue_group_intensities WHERE currentitems.group_key = cue_group_intensities.foreignItem_key AND cue_group_intensities.valueItem_key = intensities.key AND cue_group_intensities.item_key = currentcue.key");
        } else if (itemType == ItemType::color().getKey()) {
            query.prepare("SELECT colors.id FROM colors, currentcue, currentitems, cuelists, cue_group_colors WHERE currentitems.group_key = cue_group_colors.foreignItem_key AND cue_group_colors.valueItem_key = colors.key AND cue_group_colors.item_key = currentcue.key");
        } else if (itemType == ItemType::position().getKey()) {
            query.prepare("SELECT positions.id FROM positions, currentcue, currentitems, cuelists, cue_group_positions WHERE currentitems.group_key = cue_group_positions.foreignItem_key AND cue_group_positions.valueItem_key = positions.key AND cue_group_positions.item_key = currentcue.key");
        } else if (itemType == ItemType::raw().getKey()) {
            query.prepare("SELECT raws.id FROM raws, currentcue, currentitems, cuelists, cue_group_raws WHERE currentitems.group_key = cue_group_raws.foreignItem_key AND cue_group_raws.valueItem_key = raws.key AND cue_group_raws.item_key = currentcue.key");
        } else if (itemType == ItemType::effect().getKey()) {
            query.prepare("SELECT effects.id FROM effects, currentcue, currentitems, cuelists, cue_group_effects WHERE currentitems.group_key = cue_group_effects.foreignItem_key AND cue_group_effects.valueItem_key = effects.key AND cue_group_effects.item_key = currentcue.key");
        } else if (itemType == ItemType::cuelist().getKey()) {
            query.prepare("SELECT cuelists.id FROM cuelists, currentitems WHERE cuelists.key = currentitems.cuelist_key");
        } else if (itemType == ItemType::cue().getKey()) {
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
        if (itemType == ItemType::model().getKey()) {
            table = ItemType::model().getSelectTable();
        } else if (itemType == ItemType::fixture().getKey()) {
            table = ItemType::fixture().getSelectTable();
        } else if (itemType == ItemType::group().getKey()) {
            table = ItemType::group().getSelectTable();
        } else if (itemType == ItemType::intensity().getKey()) {
            table = ItemType::intensity().getSelectTable();
        } else if (itemType == ItemType::color().getKey()) {
            table = ItemType::color().getSelectTable();
        } else if (itemType == ItemType::position().getKey()) {
            table = ItemType::position().getSelectTable();
        } else if (itemType == ItemType::raw().getKey()) {
            table = ItemType::raw().getSelectTable();
        } else if (itemType == ItemType::effect().getKey()) {
            table = ItemType::effect().getSelectTable();
        } else if (itemType == ItemType::cuelist().getKey()) {
            table = ItemType::cuelist().getSelectTable();
        } else if (itemType == ItemType::cue().getKey()) {
            table = ItemType::cue().getSelectTable();
        } else if (itemType == Keys::Attribute) {
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

        keys.append(Keys::Plus);
        QStringList thruParts;
        QStringList idParts;
        QList<Keys::Key> currentIdPartKeys;
        bool idAdding = true;
        bool idStartsWithPeriod = false;
        for (const Keys::Key key : keys) {
            if (((key == Keys::Plus) || (key == Keys::Minus)) && currentIdPartKeys.isEmpty()) { // IDs that end with a period
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
            } else if ((key == Keys::Period) || (key == Keys::Plus) || (key == Keys::Minus) || (key == Keys::Thru)) {
                if ((key == Keys::Period) && idParts.isEmpty() && currentIdPartKeys.isEmpty() && !idStartsWithPeriod) {
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
                if ((key == Keys::Plus) || (key == Keys::Minus)) {
                    if (idParts.isEmpty() && !idStartsWithPeriod) {
                        return QStringList();
                    }
                    QString id = idParts.join(".");
                    while (id.endsWith(".0")) {
                        id.chop(2);
                    }
                    if (idStartsWithPeriod) {
                        if (thruParts.isEmpty() || (idParts.size() != 1)) {
                            return QStringList();
                        }
                        for (int ending = thruParts.last().toInt(); ending <= idParts.last().toInt(); ending++) {
                            QStringList currentIdParts = thruParts.first(thruParts.length() - 1);
                            currentIdParts.append(QString::number(ending));
                            QString currentId = currentIdParts.join(".");
                            while (currentId.endsWith(".0")) {
                                currentId.chop(2);
                            }
                            if (idAdding) {
                                ids.append(currentId);
                            } else {
                                ids.removeAll(currentId);
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
                            QString currentId = allIdsExtended.at(index);
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
            if (key == Keys::Thru) {
                if (!thruParts.isEmpty() || idParts.isEmpty() || idStartsWithPeriod) {
                    return QStringList();
                }
                thruParts = idParts;
                idParts.clear();
            } else if (key == Keys::Plus) {
                idAdding = true;
            } else if (key == Keys::Minus) {
                idAdding = false;
            }
        }
    }
    ids.removeDuplicates();
    std::sort(ids.begin(), ids.end(), Terminal::compareIds);
    return ids;
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
