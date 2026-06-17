/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "terminal.h"
#include "constants.h"
#include "attributes/boolattribute.h"
#include "attributes/textattribute.h"
#include "attributes/itemattribute.h"
#include "attributes/itemlistattribute.h"
#include "attributes/itemspecificitemlistattribute.h"
#include "attributes/numberattribute.h"
#include "attributes/numberattribute.cpp"
#include "attributes/itemspecificnumberattribute.h"
#include "attributes/itemspecificnumberattribute.cpp"
#include "attributes/itemandintegerspecificnumberattribute.h"
#include "attributes/itemandintegerspecificnumberattribute.cpp"
#include "attributes/integerspecificnumberattribute.h"
#include "attributes/integerspecificnumberattribute.cpp"
#include "attributes/integerspecificitemlistattribute.h"
#include "attributes/integerspecificitemlistattribute.cpp"

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
            TextAttribute attribute = TextAttribute(ItemType::model(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if ((attribute == AttributeIds::modelChannels) || !attributes.contains(Keys::Attribute)) {
            TextAttribute attribute = TextAttribute(ItemType::model(), "channels", "Channels", "^[01DdRrGgBbWwCcMmYyHhSsPpTtZzFf]+$", this);
            attribute.set(ids);
            emit dbChanged();
        } else if (attribute == AttributeIds::modelPanRange) {
            NumberAttribute<float> attribute(ItemType::model(), "panrange", "Pan Range", NumberType::panRange());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::modelTiltRange) {
            NumberAttribute<float> attribute(ItemType::model(), "tiltrange", "Tilt Range", NumberType::tiltRange());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::modelMinZoom) {
            NumberAttribute<float> attribute(ItemType::model(), "minzoom", "Minimal Zoom", NumberType::zoomRange());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::modelMaxZoom) {
            NumberAttribute<float> attribute(ItemType::model(), "maxzoom", "Maximal Zoom", NumberType::zoomRange());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Model Attribute.");
        }
    } else if (selectionType == ItemType::fixture().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::fixture(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::fixture(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::fixture(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureModel) {
            ItemAttribute attribute = ItemAttribute(ItemType::fixture(), "model_key", "Model", ItemType::model());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureUniverse) {
            NumberAttribute<int> attribute(ItemType::fixture(), "universe", "Universe", NumberType::universe());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::fixtureAddress) || !attributes.contains(Keys::Attribute)) {
            NumberAttribute<int> attribute(ItemType::fixture(), "address", "Address", NumberType::address());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureXPosition) {
            NumberAttribute<float> attribute(ItemType::fixture(), "xposition", "X Position", NumberType::coordinate());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureYPosition) {
            NumberAttribute<float> attribute(ItemType::fixture(), "yposition", "Y Position", NumberType::coordinate());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureRotation) {
            NumberAttribute<float> attribute(ItemType::fixture(), "rotation", "Rotation", NumberType::angle());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureInvertPan) {
            BoolAttribute attribute(ItemType::fixture(), "invertpan", "Invert Pan");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Fixture Attribute.");
        }
    } else if (selectionType == ItemType::group().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::group(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::group(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::group(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if ((attribute == AttributeIds::groupFixtures) || !attributes.contains(Keys::Attribute)) {
            ItemListAttribute attribute = ItemListAttribute(ItemType::group(), "Fixtures", ItemType::fixture(), "group_fixtures");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Group Attribute.");
        }
    } else if (selectionType == ItemType::intensity().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::intensity(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::intensity(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::intensity(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if ((attribute == AttributeIds::intensityDimmer) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::intensity(), "Dimmer Model Exception", ItemType::model(), "intensity_model_dimmer", NumberType::percentage());
                attribute.set(ids, attributes.value(ItemType::model().getKey()), valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::cue(), "Dimmer Fixture Exception", ItemType::fixture(), "intensity_fixture_dimmer", NumberType::percentage());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::intensity(), "dimmer", "Dimmer", NumberType::percentage());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::intensityRaws) {
            ItemListAttribute attribute = ItemListAttribute(ItemType::intensity(), "Raws", ItemType::raw(), "intensity_raws");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Intensity Attribute.");
        }
    } else if (selectionType == ItemType::color().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::color(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::color(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::color(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if ((attribute == AttributeIds::colorHue) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::color(), "Hue Model Exception", ItemType::model(), "color_model_hue", NumberType::angle());
                attribute.set(ids, attributes.value(ItemType::model().getKey()), valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::color(), "Hue Fixture Exception", ItemType::fixture(), "color_fixture_hue", NumberType::angle());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::color(), "hue", "Hue", NumberType::angle());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::colorSaturation) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::color(), "Saturation Model Exception", ItemType::model(), "color_model_saturation", NumberType::percentage());
                attribute.set(ids, attributes.value(ItemType::model().getKey()), valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::color(), "Saturation Fixture Exception", ItemType::fixture(), "color_fixture_saturation", NumberType::percentage());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::color(), "saturation", "Saturation", NumberType::percentage());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::colorQuality) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::color(), "Quality Model Exception", ItemType::model(), "color_model_quality", NumberType::percentage());
                attribute.set(ids, attributes.value(ItemType::model().getKey()), valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::color(), "Quality Fixture Exception", ItemType::fixture(), "color_fixture_quality", NumberType::percentage());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::color(), "quality", "Quality", NumberType::percentage());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::colorRaws) {
            ItemListAttribute attribute = ItemListAttribute(ItemType::color(), "Raws", ItemType::raw(), "color_raws");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Color Attribute.");
        }
    } else if (selectionType == ItemType::position().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::position(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::position(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::position(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if ((attribute == AttributeIds::positionPan) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::position(), "Pan Model Exception", ItemType::model(), "position_model_pan", NumberType::angle());
                attribute.set(ids, attributes.value(ItemType::model().getKey()), valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::position(), "Pan Fixture Exception", ItemType::fixture(), "position_fixture_pan", NumberType::angle());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::position(), "pan", "Pan", NumberType::angle());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::positionTilt) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::position(), "Tilt Model Exception", ItemType::model(), "position_model_tilt", NumberType::tilt());
                attribute.set(ids, attributes.value(ItemType::model().getKey()), valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::position(), "Tilt Fixture Exception", ItemType::fixture(), "position_fixture_tilt", NumberType::tilt());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::position(), "tilt", "Tilt", NumberType::tilt());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::positionZoom) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::position(), "Zoom Model Exception", ItemType::model(), "position_model_zoom", NumberType::zoom());
                attribute.set(ids, attributes.value(ItemType::model().getKey()), valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::position(), "Zoom Fixture Exception", ItemType::fixture(), "position_fixture_zoom", NumberType::zoom());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::position(), "zoom", "Zoom", NumberType::zoom());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::positionFocus) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::position(), "Focus Model Exception", ItemType::model(), "position_model_focus", NumberType::percentage());
                attribute.set(ids, attributes.value(ItemType::model().getKey()), valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::position(), "Focus FixtureException", ItemType::fixture(), "position_fixture_focus", NumberType::percentage());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::position(), "focus", "Focus", NumberType::percentage());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::positionRaws) {
            ItemListAttribute attribute = ItemListAttribute(ItemType::position(), "Raws", ItemType::raw(), "position_raws");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Position Attribute.");
        }
    } else if (selectionType == ItemType::raw().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::raw(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::raw(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::raw(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::rawChannelValues) + ".")) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemAndIntegerSpecificNumberAttribute<int> numberAttribute(ItemType::raw(), "Channel Values", ItemType::model(), "raw_model_channel_values", NumberType::channel(), NumberType::dmxValue());
                numberAttribute.set(ids, attributes.value(ItemType::model().getKey()), attribute, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemAndIntegerSpecificNumberAttribute<int> numberAttribute(ItemType::raw(), "Channel Values", ItemType::fixture(), "raw_fixture_channel_values", NumberType::channel(), NumberType::dmxValue());
                numberAttribute.set(ids, attributes.value(ItemType::fixture().getKey()), attribute, valueKeys);
                emit dbChanged();
            } else {
                IntegerSpecificNumberAttribute<int> numberAttribute(ItemType::raw(), "Channel Values", "raw_channel_values", NumberType::channel(), NumberType::dmxValue());
                numberAttribute.set(ids, attribute, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::rawMoveWhileDark) {
            BoolAttribute attribute(ItemType::raw(), "movewhiledark", "Move while Dark");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::rawFade) {
            BoolAttribute attribute(ItemType::raw(), "fade", "Fade");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Raw Attribute.");
        }
    } else if (selectionType == ItemType::effect().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::effect(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::effect(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::effect(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if ((attribute == AttributeIds::effectSteps) || !attributes.contains(Keys::Attribute)) {
            NumberAttribute<int> attribute(ItemType::position(), "steps", "Steps", NumberType::step());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectIntensities) + ".")) {
            IntegerSpecificItemListAttribute numberAttribute(ItemType::effect(), "Intensities", "effect_step_intensities", ItemType::intensity(), NumberType::step(), false);
            numberAttribute.set(ids, attribute, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectColors) + ".")) {
            IntegerSpecificItemListAttribute numberAttribute(ItemType::effect(), "Colors", "effect_step_colors", ItemType::color(), NumberType::step(), false);
            numberAttribute.set(ids, attribute, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectPositions) + ".")) {
            IntegerSpecificItemListAttribute numberAttribute(ItemType::effect(), "Positions", "effect_step_positions", ItemType::position(), NumberType::step(), false);
            numberAttribute.set(ids, attribute, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectRaws) + ".")) {
            IntegerSpecificItemListAttribute numberAttribute(ItemType::effect(), "Raws", "effect_step_raws", ItemType::raw(), NumberType::step(), true);
            numberAttribute.set(ids, attribute, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::effectHold) {
            NumberAttribute<float> attribute(ItemType::position(), "hold", "Hold", NumberType::time());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectHold) + ".")) {
            IntegerSpecificNumberAttribute<float> numberAttribute(ItemType::effect(), "Hold", "effect_step_hold", NumberType::step(), NumberType::time());
            numberAttribute.set(ids, attribute, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::effectFade) {
            NumberAttribute<float> attribute(ItemType::position(), "fade", "Fade", NumberType::time());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectFade) + ".")) {
            IntegerSpecificNumberAttribute<float> numberAttribute(ItemType::effect(), "Fade", "effect_step_fade", NumberType::step(), NumberType::time());
            numberAttribute.set(ids, attribute, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::effectPhase) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::effect(), "Phase", ItemType::fixture(), "effect_fixture_phase", NumberType::angle());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::effect(), "phase", "Phase", NumberType::angle());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::effectSineFade) {
            BoolAttribute attribute(ItemType::effect(), "sinefade", "Sine Fade");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Effect Attribute.");
        }
    } else if (selectionType == ItemType::cuelist().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            deleteItems(ItemType::cuelist(), ids);
        } else if (attribute == AttributeIds::id) {
            moveItems(ItemType::cuelist(), ids, valueKeys);
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::cuelist(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if (attribute == AttributeIds::cuelistPriority) {
            NumberAttribute<int> attribute(ItemType::cuelist(), "priority", "Priority", NumberType::priority());
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::cuelistMoveWhileDark) {
            BoolAttribute attribute(ItemType::cuelist(), "movewhiledark", "Move while Dark");
            attribute.set(ids, valueKeys);
            emit dbChanged();
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
            TextAttribute attribute = TextAttribute(ItemType::cue(), "label", "Label", "", this);
            attribute.set(ids);
            emit dbChanged();
        } else if (attribute == AttributeIds::cueIntensities) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), "Intensities", ItemType::group(), ItemType::intensity(), "cue_group_intensities", false);
                attribute.set(ids, attributes.value(ItemType::group().getKey()), valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Intensities because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueColors) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), "Colors", ItemType::group(), ItemType::color(), "cue_group_colors", false);
                attribute.set(ids, attributes.value(ItemType::group().getKey()), valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Colors because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cuePositions) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), "Positions", ItemType::group(), ItemType::position(), "cue_group_positions", false);
                attribute.set(ids, attributes.value(ItemType::group().getKey()), valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Positions because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueRaws) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), "Raws", ItemType::group(), ItemType::raw(), "cue_group_raws", true);
                attribute.set(ids, attributes.value(ItemType::group().getKey()), valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Raws because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueEffects) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), "Effects", ItemType::group(), ItemType::effect(), "cue_group_effects", true);
                attribute.set(ids, attributes.value(ItemType::group().getKey()), valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Effects because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueBlock) {
            BoolAttribute attribute(ItemType::cue(), "block", "Block");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::cueFade) || (!attributes.contains(Keys::Attribute))) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::cue(), "Fade", ItemType::fixture(), "cue_fixture_fade", NumberType::time());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::cue(), "fade", "Fade", NumberType::time());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::cueDelay) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute<float> attribute(ItemType::cue(), "Delay", ItemType::fixture(), "cue_fixture_delay", NumberType::time());
                attribute.set(ids, attributes.value(ItemType::fixture().getKey()), valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute<float> attribute(ItemType::cue(), "delay", "Delay", NumberType::time());
                attribute.set(ids, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::cueFollow) {
            BoolAttribute attribute(ItemType::cue(), "follow", "Follow");
            attribute.set(ids, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::cueSineFade) {
            BoolAttribute attribute(ItemType::cue(), "sinefade", "Sine Fade");
            attribute.set(ids, valueKeys);
            emit dbChanged();
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
