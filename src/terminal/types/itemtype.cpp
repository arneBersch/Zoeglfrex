/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "itemtype.h"

ItemType::ItemType(QString table, QString singular, QString plural, Keys::Key key) : ItemType(table, table, singular, plural, key) {}

ItemType::ItemType(QString itemSelectTable, QString itemUpdateTable, QString itemSingular, QString itemPlural, Keys::Key itemKey) {
    selectTable = itemSelectTable;
    updateTable = itemUpdateTable;
    singular = itemSingular;
    plural = itemPlural;
    key = itemKey;
}

QString ItemType::getSelectTable() const {
    return selectTable;
}

QString ItemType::getUpdateTable() const {
    return updateTable;
}

QString ItemType::getSingular() const {
    return singular;
}

QString ItemType::getPlural() const {
    return plural;
}

Keys::Key ItemType::getKey() const {
    return key;
}

QString ItemType::format(const QStringList ids) const {
    if (ids.length() == 0) {
        return "0 " + plural;
    }
    if (ids.length() == 1) {
        return singular + " " + ids.first();
    }
    return plural + ids.join(", ");
}

bool ItemType::operator==(ItemType item) const {
    return (item.selectTable == selectTable) && (item.updateTable == updateTable) && (item.singular == singular) && (item.plural == plural) && (item.key == key);
}

ItemType ItemType::model() {
    return ItemType("models", "models", "Model", "Models", Keys::Model);
}

ItemType ItemType::fixture() {
    return ItemType("fixtures", "fixtures", "Fixture", "Fixtures", Keys::Fixture);
}

ItemType ItemType::group() {
    return ItemType("groups", "groups", "Group", "Groups", Keys::Group);
}

ItemType ItemType::intensity() {
    return ItemType("intensities", "intensities", "Intensity", "Intensities", Keys::Intensity);
}

ItemType ItemType::color() {
    return ItemType("colors", "colors", "Color", "Colors", Keys::Color);
}

ItemType ItemType::position() {
    return ItemType("positions", "positions", "Position", "Positions", Keys::Position);
}

ItemType ItemType::raw() {
    return ItemType("raws", "raws", "Raw", "Raws", Keys::Raw);
}

ItemType ItemType::effect() {
    return ItemType("effects", "effects", "Effect", "Effects", Keys::Effect);
}

ItemType ItemType::cuelist() {
    return ItemType("cuelists", "cuelists", "Cuelist", "Cuelists", Keys::Cuelist);
}

ItemType ItemType::cue() {
    return ItemType("currentcuelist_cues", "cues", "Cue", "Cues", Keys::Cue);
}

QList<ItemType> ItemType::allTypes() {
    return {
        ItemType::model(),
        ItemType::fixture(),
        ItemType::group(),
        ItemType::intensity(),
        ItemType::color(),
        ItemType::position(),
        ItemType::raw(),
        ItemType::effect(),
        ItemType::cuelist(),
        ItemType::cue(),
    };
}
