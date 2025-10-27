/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TERMINAL_H
#define TERMINAL_H

#include <QtWidgets>
#include <QtSql>

#include "constants.h"

class Terminal : public QWidget {
    Q_OBJECT
private:
    enum Key {
        Zero, // 0
        One, // 1
        Two, // 2
        Three, // 3
        Four, // 4
        Five, // 5
        Six, // 6
        Seven, // 7
        Eight, // 8
        Nine, // 9
        Plus, // +
        Minus, // -
        Thru, // T
        Period, // .
        Set, // S
        Attribute, // A
        Model, // M
        Fixture, // F
        Group, // G
        Intensity, // I
        Color, // C
        Position, // P
        Raw, // R
        Effect, // E
        Cuelist, // L
        Cue, // Q
    };
    struct ItemInfos {
        QString table;
        QString singular;
        QString plural;
        Key key;
    };
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

    struct NumberInfos {
        float minValue;
        float maxValue;
        bool cyclic = false;
        QString unit = QString();
    };
    const NumberInfos percentageInfos = {0, 100, false, "%"};
    const NumberInfos angleInfos = {0, 360, true, "°"};

public:
    Terminal(QWidget *parent = nullptr);
    void info(QString message);
    void success(QString message);
    void warning(QString message);
    void error(QString message);
signals:
    void dbChanged();
    void itemChanged(QString itemType, QStringList ids);
public slots:
    void reload();
private:
    void execute();
    void updateSortingKeys(ItemInfos item);
    static bool compareIds(QString idA, QString idB);
    void createItems(ItemInfos item, QStringList ids);
    void deleteItems(ItemInfos item, QStringList ids);
    void moveItems(ItemInfos item, QStringList ids, QList<Key> valueKeys);
    void setBoolAttribute(ItemInfos item, QString attribute, QString attributeName, QStringList ids, QList<Key> valueKeys);
    void setTextAttribute(ItemInfos item, QString attribute, QString attributeName, QStringList ids, QString regex);
    template <typename T> void setNumberAttribute(ItemInfos item, QString attribute, QString attributeName, QStringList ids, QList<Key> valueKeys, NumberInfos number);
    void setItemAttribute(ItemInfos item, QString attribute, QString attributeName, QStringList ids, QList<Key> valueKeys, ItemInfos foreignItem);
    void setItemListAttribute(ItemInfos item, QString attributeName, QStringList ids, QList<Key> valueKeys, ItemInfos foreignItem, QString listTable, QString listTableItemAttribute, QString listTableForeignItemsAttribute);
    template <typename T> void setItemSpecificNumberAttribute(ItemInfos item, QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Key> valueKeys, ItemInfos foreignItem, QString exceptionTable, QString exceptionTableItemAttribute, QString exceptionTableForeignItemAttribute, QString exceptionTableValueAttribute, NumberInfos number);
    void setItemSpecificItemAttribute(ItemInfos item, QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Key> valueKeys, ItemInfos foreignItem, ItemInfos valueItem, QString valueTable, QString valueTableItemAttribute, QString valueTableForeignItemAttribute, QString valueTableValueAttribute);
    float keysToFloat(QList<Key> keys, bool* ok, float currentValue, NumberInfos number) const;
    QStringList keysToIds(QList<Key> keys) const;
    QString keysToString(QList<Key> keys) const;
    void writeKey(Key key);
    void backspace();
    void clearPrompt();
    QList<Key> promptKeys;
    QPlainTextEdit *messages;
    QLabel* promptLabel;
    const QList<Key> itemKeys = {Model, Fixture, Group, Intensity, Color, Position, Raw, Effect, Cuelist, Cue};
    QMap<Key, QString> keyStrings;
};

#endif // TERMINAL_H
