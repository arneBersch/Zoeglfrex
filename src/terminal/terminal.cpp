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
#include "startscreen/startscreen.h"

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
    QSqlQuery blindQuery;
    if (blindQuery.exec("SELECT cue_key FROM currentitems WHERE cue_key IS NOT NULL")) {
        blindButton->setChecked(blindQuery.next());
    } else {
        qWarning() << Q_FUNC_INFO << blindQuery.executedQuery() << blindQuery.lastError().text();
    }
    promptLayout->addWidget(blindButton);

    trackingButton = new QPushButton("Tracking");
    trackingButton->setCheckable(true);
    connect(trackingButton, &QPushButton::clicked, this, [this] {
        StartScreen::setFileSetting("tracking", trackingButton->isChecked());
    });
    trackingButton->setChecked(StartScreen::getFileSetting("tracking", true).toBool());
    StartScreen::setFileSetting("tracking", trackingButton->isChecked());
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

    for (ItemType type : ItemType::allTypes()) {
        attributes.insert(new IdAttribute(type));
        attributes.insert(new TextAttribute(type, AttributeIds::label, "Label", "label", "", this));
    }

    attributes.insert(new TextAttribute(ItemType::model(), AttributeIds::modelChannels, "Channels", "channels", "^[01DdRrGgBbWwCcMmYyHhSsPpTtZzFf]+$", this));
    attributes.insert(new NumberAttribute(ItemType::model(), AttributeIds::modelPanRange, "Pan Range", "panrange", NumberType::panRange()));
    attributes.insert(new NumberAttribute(ItemType::model(), AttributeIds::modelTiltRange, "Tilt Range", "tiltrange", NumberType::tiltRange()));
    attributes.insert(new NumberAttribute(ItemType::model(), AttributeIds::modelMinZoom, "Minimal Zoom", "minzoom", NumberType::zoomRange()));
    attributes.insert(new NumberAttribute(ItemType::model(), AttributeIds::modelMaxZoom, "Maximal Zoom", "maxzoom", NumberType::zoomRange()));

    attributes.insert(new ItemAttribute(ItemType::fixture(), AttributeIds::fixtureModel, "Model", "model_key", ItemType::model()));
    attributes.insert(new NumberAttribute(ItemType::fixture(), AttributeIds::fixtureUniverse, "Universe", "universe", NumberType::universe()));
    attributes.insert(new NumberAttribute(ItemType::fixture(), AttributeIds::fixtureAddress, "Address", "address", NumberType::address()));
    attributes.insert(new NumberAttribute(ItemType::fixture(), AttributeIds::fixtureXPosition, "X Position", "xposition", NumberType::coordinate()));
    attributes.insert(new NumberAttribute(ItemType::fixture(), AttributeIds::fixtureYPosition, "Y Position", "yposition", NumberType::coordinate()));
    attributes.insert(new NumberAttribute(ItemType::fixture(), AttributeIds::fixtureRotation, "Rotation", "rotation", NumberType::angle()));
    attributes.insert(new BoolAttribute(ItemType::fixture(), AttributeIds::fixtureInvertPan, "Invert Pan", "invertpan"));

    attributes.insert(new ItemListAttribute(ItemType::group(), AttributeIds::groupFixtures, "Fixtures", ItemType::fixture(), "group_fixtures"));

    attributes.insert(new NumberAttribute(ItemType::intensity(), AttributeIds::intensityDimmer, "Dimmer", "dimmer", NumberType::percentage()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::intensity(), AttributeIds::intensityDimmer, "Dimmer Model Exception", ItemType::model(), "intensity_model_dimmer", NumberType::percentage()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::intensity(), AttributeIds::intensityDimmer, "Dimmer Fixture Exception", ItemType::fixture(), "intensity_fixture_dimmer", NumberType::percentage()));
    attributes.insert(new ItemListAttribute(ItemType::intensity(), AttributeIds::intensityRaws, "Raws", ItemType::raw(), "intensity_raws"));

    attributes.insert(new NumberAttribute(ItemType::color(), AttributeIds::colorHue, "Hue", "hue", NumberType::angle()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::color(), AttributeIds::colorHue, "Hue Model Exception", ItemType::model(), "color_model_hue", NumberType::angle()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::color(), AttributeIds::colorHue, "Hue Fixture Exception", ItemType::fixture(), "color_fixture_hue", NumberType::angle()));
    attributes.insert(new NumberAttribute(ItemType::color(), AttributeIds::colorSaturation, "Saturation", "saturation", NumberType::percentage()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::color(), AttributeIds::colorSaturation, "Saturation Model Exception", ItemType::model(), "color_model_saturation", NumberType::percentage()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::color(), AttributeIds::colorSaturation, "Saturation Fixture Exception", ItemType::fixture(), "color_fixture_saturation", NumberType::percentage()));
    attributes.insert(new NumberAttribute(ItemType::color(), AttributeIds::colorQuality, "Quality", "quality", NumberType::percentage()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::color(), AttributeIds::colorQuality, "Quality Model Exception", ItemType::model(), "color_model_quality", NumberType::percentage()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::color(), AttributeIds::colorQuality, "Quality Fixture Exception", ItemType::fixture(), "color_fixture_quality", NumberType::percentage()));
    attributes.insert(new ItemListAttribute(ItemType::color(), AttributeIds::colorRaws, "Raws", ItemType::raw(), "color_raws"));

    attributes.insert(new NumberAttribute(ItemType::position(), AttributeIds::positionPan, "Pan", "pan", NumberType::angle()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::position(), AttributeIds::positionPan, "Pan Model Exception", ItemType::model(), "position_model_pan", NumberType::angle()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::position(), AttributeIds::positionPan, "Pan Fixture Exception", ItemType::fixture(), "position_fixture_pan", NumberType::angle()));
    attributes.insert(new NumberAttribute(ItemType::position(), AttributeIds::positionTilt, "Tilt", "tilt", NumberType::tilt()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::position(), AttributeIds::positionTilt, "Tilt Model Exception", ItemType::model(), "position_model_tilt", NumberType::tilt()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::position(), AttributeIds::positionTilt, "Tilt Fixture Exception", ItemType::fixture(), "position_fixture_tilt", NumberType::tilt()));
    attributes.insert(new NumberAttribute(ItemType::position(), AttributeIds::positionZoom, "Zoom", "zoom", NumberType::zoom()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::position(), AttributeIds::positionZoom, "Zoom Model Exception", ItemType::model(), "position_model_zoom", NumberType::zoom()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::position(), AttributeIds::positionZoom, "Zoom Fixture Exception", ItemType::fixture(), "position_fixture_zoom", NumberType::zoom()));
    attributes.insert(new NumberAttribute(ItemType::position(), AttributeIds::positionFocus, "Focus", "focus", NumberType::percentage()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::position(), AttributeIds::positionFocus, "Focus Model Exception", ItemType::model(), "position_model_focus", NumberType::percentage()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::position(), AttributeIds::positionFocus, "Focus FixtureException", ItemType::fixture(), "position_fixture_focus", NumberType::percentage()));
    attributes.insert(new ItemListAttribute(ItemType::position(), AttributeIds::positionRaws, "Raws", ItemType::raw(), "position_raws"));

    attributes.insert(new IntegerSpecificNumberAttribute(ItemType::raw(), AttributeIds::rawChannelValues, "Channel Values", "raw_channel_values", NumberType::channel(), NumberType::dmxValue()));
    attributes.insert(new ItemAndIntegerSpecificNumberAttribute(ItemType::raw(), AttributeIds::rawChannelValues, "Channel Values", ItemType::model(), "raw_model_channel_values", NumberType::channel(), NumberType::dmxValue()));
    attributes.insert(new ItemAndIntegerSpecificNumberAttribute(ItemType::raw(), AttributeIds::rawChannelValues, "Channel Values", ItemType::fixture(), "raw_fixture_channel_values", NumberType::channel(), NumberType::dmxValue()));
    attributes.insert(new BoolAttribute(ItemType::raw(), AttributeIds::rawMoveWhileDark, "Move while Dark", "movewhiledark"));
    attributes.insert(new BoolAttribute(ItemType::raw(), AttributeIds::rawFade, "Fade", "fade"));

    attributes.insert(new NumberAttribute(ItemType::effect(), AttributeIds::effectSteps, "Steps", "steps", NumberType::step()));
    attributes.insert(new IntegerSpecificItemListAttribute(ItemType::effect(), AttributeIds::effectIntensities, "Intensities", "effect_step_intensities", ItemType::intensity(), NumberType::step(), false));
    attributes.insert(new IntegerSpecificItemListAttribute(ItemType::effect(), AttributeIds::effectColors, "Colors", "effect_step_colors", ItemType::color(), NumberType::step(), false));
    attributes.insert(new IntegerSpecificItemListAttribute(ItemType::effect(), AttributeIds::effectPositions, "Positions", "effect_step_positions", ItemType::position(), NumberType::step(), false));
    attributes.insert(new IntegerSpecificItemListAttribute(ItemType::effect(), AttributeIds::effectRaws, "Raws", "effect_step_raws", ItemType::raw(), NumberType::step(), true));
    attributes.insert(new NumberAttribute(ItemType::effect(), AttributeIds::effectHold, "Hold", "hold", NumberType::time()));
    attributes.insert(new IntegerSpecificNumberAttribute(ItemType::effect(), AttributeIds::effectHold, "Hold", "effect_step_hold", NumberType::step(), NumberType::time()));
    attributes.insert(new NumberAttribute(ItemType::effect(), AttributeIds::effectFade, "Fade", "fade", NumberType::time()));
    attributes.insert(new IntegerSpecificNumberAttribute(ItemType::effect(), AttributeIds::effectFade, "Fade", "effect_step_fade", NumberType::step(), NumberType::time()));
    attributes.insert(new NumberAttribute(ItemType::effect(), AttributeIds::effectPhase, "Phase", "phase", NumberType::angle()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::effect(), AttributeIds::effectPhase, "Phase", ItemType::fixture(), "effect_fixture_phase", NumberType::angle()));
    attributes.insert(new BoolAttribute(ItemType::effect(), AttributeIds::effectSineFade, "Sine Fade", "sinefade"));

    attributes.insert(new NumberAttribute(ItemType::cuelist(), AttributeIds::cuelistPriority, "Priority", "priority", NumberType::priority()));
    attributes.insert(new BoolAttribute(ItemType::cuelist(), AttributeIds::cuelistMoveWhileDark, "Move while Dark", "movewhiledark"));

    attributes.insert(new ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cueIntensities, "Intensities", ItemType::group(), ItemType::intensity(), "cue_group_intensities", false));
    attributes.insert(new ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cueColors, "Colors", ItemType::group(), ItemType::color(), "cue_group_colors", false));
    attributes.insert(new ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cuePositions, "Positions", ItemType::group(), ItemType::position(), "cue_group_positions", false));
    attributes.insert(new ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cueRaws, "Raws", ItemType::group(), ItemType::raw(), "cue_group_raws", true));
    attributes.insert(new ItemSpecificItemListAttribute(ItemType::cue(), AttributeIds::cueEffects, "Effects", ItemType::group(), ItemType::effect(), "cue_group_effects", true));
    attributes.insert(new BoolAttribute(ItemType::cue(), AttributeIds::cueBlock, "Block", "block"));
    attributes.insert(new NumberAttribute(ItemType::cue(), AttributeIds::cueFade, "Fade", "fade", NumberType::time()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::cue(), AttributeIds::cueFade, "Fade", ItemType::fixture(), "cue_fixture_fade", NumberType::time()));
    attributes.insert(new NumberAttribute(ItemType::cue(), AttributeIds::cueDelay, "Delay", "delay", NumberType::time()));
    attributes.insert(new ItemSpecificNumberAttribute(ItemType::cue(), AttributeIds::cueDelay, "Delay", ItemType::fixture(), "cue_fixture_delay", NumberType::time()));
    attributes.insert(new BoolAttribute(ItemType::cue(), AttributeIds::cueFollow, "Follow", "follow"));
    attributes.insert(new BoolAttribute(ItemType::cue(), AttributeIds::cueSineFade, "Sine Fade", "sinefade"));
}

void Terminal::execute() {
    QList<Keys::Key> keys = promptKeys;
    if (keys.isEmpty()) {
        return;
    }
    printMessage(formatInfoMessage("> " + keysToString(keys)));
    clearPrompt();

    ItemType selectionType = ItemType::cue();
    if (keys.first() == ItemType::model().getKey()) {
        selectionType = ItemType::model();
    } else if (keys.first() == ItemType::fixture().getKey()) {
        selectionType = ItemType::fixture();
    } else if (keys.first() == ItemType::group().getKey()) {
        selectionType = ItemType::group();
    } else if (keys.first() == ItemType::intensity().getKey()) {
        selectionType = ItemType::intensity();
    } else if (keys.first() == ItemType::color().getKey()) {
        selectionType = ItemType::color();
    } else if (keys.first() == ItemType::position().getKey()) {
        selectionType = ItemType::position();
    } else if (keys.first() == ItemType::raw().getKey()) {
        selectionType = ItemType::raw();
    } else if (keys.first() == ItemType::effect().getKey()) {
        selectionType = ItemType::effect();
    } else if (keys.first() == ItemType::cuelist().getKey()) {
        selectionType = ItemType::cuelist();
    } else if (keys.first() != ItemType::cue().getKey()) {
        printMessage(formatErrorMessage("Invalid Item Type given."));
        return;
    }
    keys.removeFirst();

    QList<Keys::Key> selectionIdKeys;
    selectionIdKeys.append(selectionType.getKey());
    QList<Keys::Key> attributeKeys;
    QList<Keys::Key> valueKeys;
    bool attributeReached = false;
    bool valueReached = false;
    for (const Keys::Key key : keys) {
        if (key == Keys::Set) {
            if (valueReached) {
                printMessage(formatErrorMessage("Can't use Set more than one time in one command."));
                return;
            }
            valueReached = true;
        } else if ((Keys::isItemKey(key) || (key == Keys::Attribute)) && !valueReached) {
            attributeKeys.append(key);
            attributeReached = true;
        } else if (valueReached) {
            valueKeys.append(key);
        } else if (attributeReached) {
            attributeKeys.append(key);
        } else {
            selectionIdKeys.append(key);
        }
    }

    if (!attributeReached && !valueReached && (selectionIdKeys == QList<Keys::Key>({ selectionType.getKey(), Keys::Minus}))) {
        for (QString line : selectionType.deselectItems()) {
            printMessage(line);
        }
        emit dbChanged();
        return;
    }

    const QStringList ids = keysToIds(selectionIdKeys);
    if (ids.isEmpty()) {
        printMessage(formatErrorMessage("Invalid selection IDs given."));
        return;
    }

    if (!attributeReached && !valueReached) {
        for (QString line : selectionType.selectItems(ids)) {
            printMessage(line);
        }
        emit dbChanged();
        return;
    }

    QHash<Keys::Key, QStringList> attributeIDs;
    if (!attributeKeys.isEmpty()) {
        attributeKeys.append(Keys::Attribute);
        QList<Keys::Key> currentItemKeys;
        for (const Keys::Key key : attributeKeys) {
            if (Keys::isItemKey(key) || (key == Keys::Attribute)) {
                if (!currentItemKeys.isEmpty()) {
                    const Keys::Key currentItemType = currentItemKeys.first();
                    const QStringList ids = keysToIds(currentItemKeys);
                    if (ids.isEmpty()) {
                        printMessage(formatErrorMessage("Invalid Attribute given: " + keysToString(currentItemKeys)));
                        return;
                    }
                    attributeIDs[currentItemType] = ids;
                }
                currentItemKeys.clear();
            }
            currentItemKeys.append(key);
        }
    }

    if (!attributeIDs.contains(Keys::Attribute)) {
        if (valueKeys == QList<Keys::Key>({Keys::Plus})) {
            for (QString line : selectionType.createItems(ids)) {
                printMessage(line);
            }
            emit dbChanged();
            return;
        } else if (valueKeys == QList<Keys::Key>({Keys::Minus})) {
            for (QString line : selectionType.deleteItems(ids)) {
                printMessage(line);
            }
            emit dbChanged();
            return;
        } else {
            if (valueKeys.isEmpty()) {
                attributeIDs[Keys::Attribute] = { AttributeIds::label };
            } else if (selectionType == ItemType::fixture()) {
                if (valueKeys.startsWith(Keys::Model)) {
                    attributeIDs[Keys::Attribute] = { AttributeIds::fixtureModel };
                } else {
                    attributeIDs[Keys::Attribute] = { AttributeIds::fixtureAddress };
                }
            } else if (selectionType == ItemType::group()) {
                attributeIDs[Keys::Attribute] = { AttributeIds::groupFixtures };
            } else if (selectionType == ItemType::intensity()) {
                if (valueKeys.startsWith(Keys::Raw)) {
                    attributeIDs[Keys::Attribute] = { AttributeIds::intensityRaws };
                } else {
                    attributeIDs[Keys::Attribute] = { AttributeIds::intensityDimmer };
                }
            } else if (selectionType == ItemType::color()) {
                if (valueKeys.startsWith(Keys::Raw)) {
                    attributeIDs[Keys::Attribute] = { AttributeIds::colorRaws };
                } else {
                    attributeIDs[Keys::Attribute] = { AttributeIds::colorHue };
                }
            } else if (selectionType == ItemType::position()) {
                if (valueKeys.startsWith(Keys::Raw)) {
                    attributeIDs[Keys::Attribute] = { AttributeIds::positionRaws };
                } else {
                    attributeIDs[Keys::Attribute] = { AttributeIds::positionPan };
                }
            } else if (selectionType == ItemType::effect()) {
                attributeIDs[Keys::Attribute] = { AttributeIds::effectSteps };
            } else if (selectionType == ItemType::cue()) {
                attributeIDs[Keys::Attribute] = { AttributeIds::cueFade };
            }
        }
    } else if (attributeIDs.value(Keys::Attribute).size() > 1) {
        printMessage(formatErrorMessage("Invalid number of Attribute IDs given."));
        return;
    }

    Attribute* attribute = nullptr;
    for (Attribute* currentAttribute : attributes) {
        if (currentAttribute->matches(selectionType, attributeIDs)) {
            Q_ASSERT(attribute == nullptr);
            attribute = currentAttribute;
        }
    }
    if (attribute == nullptr) {
        printMessage(formatErrorMessage("No matching Attribute found."));
        return;
    }

    for (QString line : attribute->set(ids, attributeIDs, valueKeys)) {
        printMessage(line);
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

void Terminal::printMessage(QString message) {
    messages->appendHtml(message);
    qInfo() << message;
}

QString Terminal::formatInfoMessage(const QString message) {
    return "<span style=\"color: white\">" + message + "</span>";
}

QString Terminal::formatSuccessMessage(const QString message) {
    return "<span style=\"color: green\">" + message + "</span>";
}

QString Terminal::formatWarningMessage(const QString message) {
    return "<span style=\"color: yellow\">" + message + "</span>";
}

QString Terminal::formatErrorMessage(const QString message) {
    return "<span style=\"color: red\">" + message + "</span>";
}
