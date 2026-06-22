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
#include "attributes/itemspecificnumberattribute.h"
#include "attributes/itemandintegerspecificnumberattribute.h"
#include "attributes/integerspecificnumberattribute.h"
#include "attributes/integerspecificitemlistattribute.h"
#include "attributes/idattribute.h"
#include "attributes/deleteattribute.h"

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
            DeleteAttribute attribute(ItemType::model());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::model());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::model(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::modelChannels) || !attributes.contains(Keys::Attribute)) {
            TextAttribute attribute = TextAttribute(ItemType::model(), AttributeIds::modelChannels, "Channels", "channels", "^[01DdRrGgBbWwCcMmYyHhSsPpTtZzFf]+$", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::modelPanRange) {
            NumberAttribute attribute(ItemType::model(), AttributeIds::modelPanRange, "Pan Range", "panrange", NumberType::panRange());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::modelTiltRange) {
            NumberAttribute attribute(ItemType::model(), AttributeIds::modelTiltRange, "Tilt Range", "tiltrange", NumberType::tiltRange());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::modelMinZoom) {
            NumberAttribute attribute(ItemType::model(), AttributeIds::modelMinZoom, "Minimal Zoom", "minzoom", NumberType::zoomRange());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::modelMaxZoom) {
            NumberAttribute attribute(ItemType::model(), AttributeIds::modelMaxZoom, "Maximal Zoom", "maxzoom", NumberType::zoomRange());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Model Attribute.");
        }
    } else if (selectionType == ItemType::fixture().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            DeleteAttribute attribute(ItemType::fixture());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::fixture());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::fixture(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureModel) {
            ItemAttribute attribute = ItemAttribute(ItemType::fixture(), AttributeIds::fixtureModel, "Model", "model_key", ItemType::model());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureUniverse) {
            NumberAttribute attribute(ItemType::fixture(), AttributeIds::fixtureUniverse, "Universe", "universe", NumberType::universe());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::fixtureAddress) || !attributes.contains(Keys::Attribute)) {
            NumberAttribute attribute(ItemType::fixture(), AttributeIds::fixtureUniverse, "Address", "address", NumberType::address());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureXPosition) {
            NumberAttribute attribute(ItemType::fixture(), AttributeIds::fixtureXPosition, "X Position", "xposition", NumberType::coordinate());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureYPosition) {
            NumberAttribute attribute(ItemType::fixture(), AttributeIds::fixtureXPosition, "Y Position", "yposition", NumberType::coordinate());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureRotation) {
            NumberAttribute attribute(ItemType::fixture(), AttributeIds::fixtureRotation, "Rotation", "rotation", NumberType::angle());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::fixtureInvertPan) {
            BoolAttribute attribute(ItemType::fixture(), AttributeIds::fixtureInvertPan, "Invert Pan", "invertpan");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Fixture Attribute.");
        }
    } else if (selectionType == ItemType::group().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            DeleteAttribute attribute(ItemType::group());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::group());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::group(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::groupFixtures) || !attributes.contains(Keys::Attribute)) {
            ItemListAttribute attribute = ItemListAttribute(ItemType::group(), AttributeIds::groupFixtures, "Fixtures", ItemType::fixture(), "group_fixtures");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Group Attribute.");
        }
    } else if (selectionType == ItemType::intensity().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            DeleteAttribute attribute(ItemType::intensity());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::intensity());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::intensity(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::intensityDimmer) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::intensity(), AttributeIds::intensityDimmer, "Dimmer Model Exception", ItemType::model(), "intensity_model_dimmer", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::cue(), AttributeIds::intensityDimmer, "Dimmer Fixture Exception", ItemType::fixture(), "intensity_fixture_dimmer", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::intensity(), AttributeIds::intensityDimmer, "dimmer", "Dimmer", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::intensityRaws) {
            ItemListAttribute attribute = ItemListAttribute(ItemType::intensity(), AttributeIds::intensityRaws, "Raws", ItemType::raw(), "intensity_raws");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Intensity Attribute.");
        }
    } else if (selectionType == ItemType::color().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            DeleteAttribute attribute(ItemType::color());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::color());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::color(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::colorHue) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::color(), AttributeIds::colorHue, "Hue Model Exception", ItemType::model(), "color_model_hue", NumberType::angle());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::color(), AttributeIds::colorHue, "Hue Fixture Exception", ItemType::fixture(), "color_fixture_hue", NumberType::angle());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::color(), AttributeIds::colorHue, "Hue", "hue", NumberType::angle());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::colorSaturation) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::color(), AttributeIds::colorSaturation, "Saturation Model Exception", ItemType::model(), "color_model_saturation", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::color(), AttributeIds::colorSaturation, "Saturation Fixture Exception", ItemType::fixture(), "color_fixture_saturation", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::color(), AttributeIds::colorSaturation, "Saturation", "saturation", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::colorQuality) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::color(), AttributeIds::colorQuality, "Quality Model Exception", ItemType::model(), "color_model_quality", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::color(), AttributeIds::colorQuality, "Quality Fixture Exception", ItemType::fixture(), "color_fixture_quality", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::color(), AttributeIds::colorQuality, "quality", "Quality", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::colorRaws) {
            ItemListAttribute attribute = ItemListAttribute(ItemType::color(), AttributeIds::colorRaws, "Raws", ItemType::raw(), "color_raws");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Color Attribute.");
        }
    } else if (selectionType == ItemType::position().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            DeleteAttribute attribute(ItemType::position());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::position());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::position(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::positionPan) || !attributes.contains(Keys::Attribute)) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::position(), AttributeIds::positionPan, "Pan Model Exception", ItemType::model(), "position_model_pan", NumberType::angle());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::position(), AttributeIds::positionPan, "Pan Fixture Exception", ItemType::fixture(), "position_fixture_pan", NumberType::angle());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::position(), AttributeIds::positionPan, "pan", "Pan", NumberType::angle());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::positionTilt) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::position(), AttributeIds::positionTilt, "Tilt Model Exception", ItemType::model(), "position_model_tilt", NumberType::tilt());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::position(), AttributeIds::positionTilt, "Tilt Fixture Exception", ItemType::fixture(), "position_fixture_tilt", NumberType::tilt());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::position(), AttributeIds::positionTilt, "Tilt", "tilt", NumberType::tilt());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::positionZoom) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::position(), AttributeIds::positionZoom, "Zoom Model Exception", ItemType::model(), "position_model_zoom", NumberType::zoom());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::position(), AttributeIds::positionZoom, "Zoom Fixture Exception", ItemType::fixture(), "position_fixture_zoom", NumberType::zoom());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::position(), AttributeIds::positionZoom, "Zoom", "zoom", NumberType::zoom());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::positionFocus) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::position(), AttributeIds::positionFocus, "Focus Model Exception", ItemType::model(), "position_model_focus", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::position(), AttributeIds::positionFocus, "Focus FixtureException", ItemType::fixture(), "position_fixture_focus", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::position(), AttributeIds::positionFocus, "Focus", "focus", NumberType::percentage());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::positionRaws) {
            ItemListAttribute attribute = ItemListAttribute(ItemType::position(), AttributeIds::positionRaws, "Raws", ItemType::raw(), "position_raws");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Position Attribute.");
        }
    } else if (selectionType == ItemType::raw().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            DeleteAttribute attribute(ItemType::raw());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::raw());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::raw(), AttributeIds::rawChannelValues, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::rawChannelValues) + ".")) {
            if (attributes.contains(ItemType::model().getKey())) {
                ItemAndIntegerSpecificNumberAttribute attribute(ItemType::raw(), AttributeIds::rawChannelValues, "Channel Values", ItemType::model(), "raw_model_channel_values", NumberType::channel(), NumberType::dmxValue());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else if (attributes.contains(ItemType::fixture().getKey())) {
                ItemAndIntegerSpecificNumberAttribute attribute(ItemType::raw(), AttributeIds::rawChannelValues, "Channel Values", ItemType::fixture(), "raw_fixture_channel_values", NumberType::channel(), NumberType::dmxValue());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                IntegerSpecificNumberAttribute attribute(ItemType::raw(), AttributeIds::rawChannelValues, "Channel Values", "raw_channel_values", NumberType::channel(), NumberType::dmxValue());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::rawMoveWhileDark) {
            BoolAttribute attribute(ItemType::raw(), AttributeIds::rawMoveWhileDark, "movewhiledark", "Move while Dark");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::rawFade) {
            BoolAttribute attribute(ItemType::raw(), AttributeIds::rawFade, "Fade", "fade");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Raw Attribute.");
        }
    } else if (selectionType == ItemType::effect().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            DeleteAttribute attribute(ItemType::raw());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::effect());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::effect(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::effectSteps) || !attributes.contains(Keys::Attribute)) {
            NumberAttribute attribute(ItemType::position(), AttributeIds::effectSteps, "Steps", "steps", NumberType::step());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectIntensities) + ".")) {
            IntegerSpecificItemListAttribute attribute(ItemType::effect(), AttributeIds::effectIntensities, "Intensities", "effect_step_intensities", ItemType::intensity(), NumberType::step(), false);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectColors) + ".")) {
            IntegerSpecificItemListAttribute attribute(ItemType::effect(), AttributeIds::effectColors, "Colors", "effect_step_colors", ItemType::color(), NumberType::step(), false);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectPositions) + ".")) {
            IntegerSpecificItemListAttribute attribute(ItemType::effect(), AttributeIds::effectPositions, "Positions", "effect_step_positions", ItemType::position(), NumberType::step(), false);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectRaws) + ".")) {
            IntegerSpecificItemListAttribute attribute(ItemType::effect(), AttributeIds::effectRaws, "Raws", "effect_step_raws", ItemType::raw(), NumberType::step(), true);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::effectHold) {
            NumberAttribute attribute(ItemType::position(), AttributeIds::effectHold, "Hold", "hold", NumberType::time());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectHold) + ".")) {
            IntegerSpecificNumberAttribute attribute(ItemType::effect(), AttributeIds::effectHold, "Hold", "effect_step_hold", NumberType::step(), NumberType::time());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::effectFade) {
            NumberAttribute attribute(ItemType::position(), AttributeIds::effectFade, "Fade", "fade", NumberType::time());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute.startsWith(QString(AttributeIds::effectFade) + ".")) {
            IntegerSpecificNumberAttribute attribute(ItemType::effect(), AttributeIds::effectFade, "Fade", "effect_step_fade", NumberType::step(), NumberType::time());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::effectPhase) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::effect(), AttributeIds::effectPhase, "Phase", ItemType::fixture(), "effect_fixture_phase", NumberType::angle());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::effect(), AttributeIds::effectPhase, "Phase", "phase", NumberType::angle());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::effectSineFade) {
            BoolAttribute attribute(ItemType::effect(), AttributeIds::effectSineFade, "Sine Fade", "sinefade");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else {
            error("Unknown Effect Attribute.");
        }
    } else if (selectionType == ItemType::cuelist().getKey()) {
        if (attributeKeys.isEmpty() && (valueKeys.size() == 1) && valueKeys.startsWith(Keys::Minus)) {
            DeleteAttribute attribute(ItemType::cuelist());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::cuelist());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::cuelist(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::cuelistPriority) {
            NumberAttribute attribute(ItemType::cuelist(), AttributeIds::cuelistPriority, "Priority", "priority", NumberType::priority());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::cuelistMoveWhileDark) {
            BoolAttribute attribute(ItemType::cuelist(), AttributeIds::cuelistMoveWhileDark, "Move while Dark", "movewhiledark");
            attribute.set(ids, attributes, valueKeys);
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
            DeleteAttribute attribute(ItemType::cue());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::id) {
            IDAttribute attribute(ItemType::cue());
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::label) {
            TextAttribute attribute = TextAttribute(ItemType::cue(), AttributeIds::label, "Label", "label", "", this);
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::cueIntensities) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cueIntensities, "Intensities", ItemType::group(), ItemType::intensity(), "cue_group_intensities", false);
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Intensities because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueColors) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cueColors, "Colors", ItemType::group(), ItemType::color(), "cue_group_colors", false);
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Colors because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cuePositions) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cuePositions, "Positions", ItemType::group(), ItemType::position(), "cue_group_positions", false);
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Positions because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueRaws) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cueRaws, "Raws", ItemType::group(), ItemType::raw(), "cue_group_raws", true);
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Raws because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueEffects) {
            if (attributes.contains(ItemType::group().getKey())) {
                ItemSpecificItemListAttribute attribute = ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cueEffects, "Effects", ItemType::group(), ItemType::effect(), "cue_group_effects", true);
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                error("Can't set Cue Effects because no Group Attribute was provided.");
            }
        } else if (attribute == AttributeIds::cueBlock) {
            BoolAttribute attribute(ItemType::cue(), AttributeIds::cueBlock, "Block", "block");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if ((attribute == AttributeIds::cueFade) || (!attributes.contains(Keys::Attribute))) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::cue(), AttributeIds::cueBlock, "Fade", ItemType::fixture(), "cue_fixture_fade", NumberType::time());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::cue(), AttributeIds::cueBlock, "Fade", "fade", NumberType::time());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::cueDelay) {
            if (attributes.contains(ItemType::fixture().getKey())) {
                ItemSpecificNumberAttribute attribute(ItemType::cue(), AttributeIds::cueDelay, "Delay", ItemType::fixture(), "cue_fixture_delay", NumberType::time());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            } else {
                NumberAttribute attribute(ItemType::cue(), AttributeIds::cueDelay, "Delay", "delay", NumberType::time());
                attribute.set(ids, attributes, valueKeys);
                emit dbChanged();
            }
        } else if (attribute == AttributeIds::cueFollow) {
            BoolAttribute attribute(ItemType::cue(), AttributeIds::cueFollow, "Follow", "follow");
            attribute.set(ids, attributes, valueKeys);
            emit dbChanged();
        } else if (attribute == AttributeIds::cueSineFade) {
            BoolAttribute attribute(ItemType::cue(), AttributeIds::cueSineFade, "Sine Fade", "sinefade");
            attribute.set(ids, attributes, valueKeys);
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
