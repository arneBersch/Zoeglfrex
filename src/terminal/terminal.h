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

#include "keys.h"

class Terminal : public QWidget {
    Q_OBJECT
private:
    struct ItemInfos {
        QString selectTable;
        QString updateTable;
        QString singular;
        QString plural;
        Keys::Key key;
    };
    const ItemInfos modelInfos = {"models", "models", "Model", "Models", Keys::Model};
    const ItemInfos fixtureInfos = {"fixtures", "fixtures", "Fixture", "Fixtures", Keys::Fixture};
    const ItemInfos groupInfos = {"groups", "groups", "Group", "Groups", Keys::Group};
    const ItemInfos intensityInfos = {"intensities", "intensities", "Intensity", "Intensities", Keys::Intensity};
    const ItemInfos colorInfos = {"colors", "colors", "Color", "Colors", Keys::Color};
    const ItemInfos positionInfos = {"positions", "positions", "Position", "Positions", Keys::Position};
    const ItemInfos rawInfos = {"raws", "raws", "Raw", "Raws", Keys::Raw};
    const ItemInfos effectInfos = {"effects", "effects", "Effect", "Effects", Keys::Effect};
    const ItemInfos cuelistInfos = {"cuelists", "cuelists", "Cuelist", "Cuelists", Keys::Cuelist};
    const ItemInfos cueInfos = {"currentcuelist_cues", "cues", "Cue", "Cues", Keys::Cue};

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
    void setCurrentItem(ItemInfos item, QString itemTable, QList<Keys::Key> idKeys, QString updateQueryText);
    void setCueItem(ItemInfos item, QString valueTable, QList<Keys::Key> idKeys, bool multipleItemsAllowed);
    void createItems(ItemInfos item, QStringList ids);
    void deleteItems(ItemInfos item, QStringList ids);
    void moveItems(ItemInfos item, QStringList ids, QList<Keys::Key> valueKeys);
    void setBoolAttribute(ItemInfos item, QString attribute, QString attributeName, QStringList ids, QList<Keys::Key> valueKeys);
    void setTextAttribute(ItemInfos item, QString attribute, QString attributeName, QStringList ids, QString regex);
    template <typename T> void setNumberAttribute(ItemInfos item, QString attribute, QString attributeName, QStringList ids, QList<Keys::Key> valueKeys, NumberInfos number);
    void setItemAttribute(ItemInfos item, QString attribute, QString attributeName, QStringList ids, QList<Keys::Key> valueKeys, ItemInfos foreignItem);
    void setItemListAttribute(ItemInfos item, QString attributeName, QStringList ids, QList<Keys::Key> valueKeys, ItemInfos foreignItem, QString valueTable);
    template <typename T> void setItemSpecificNumberAttribute(ItemInfos item, QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Keys::Key> valueKeys, ItemInfos foreignItem, QString valueTable, NumberInfos number);
    void setItemSpecificItemListAttribute(ItemInfos item, QString attributeName, QStringList ids, QStringList foreignItemIds, QList<Keys::Key> valueKeys, ItemInfos foreignItem, ItemInfos valueItem, QString valueTable, bool limitToOne = false);
    template <typename T> void setIntegerSpecificNumberAttribute(ItemInfos item, QString attributeName, QStringList ids, QString integerId, QList<Keys::Key> valueKeys, QString valueTable, NumberInfos keyInteger, NumberInfos valueNumber);
    void setIntegerSpecificItemListAttribute(ItemInfos item, QString attributeName, QStringList ids, QString integerId, QList<Keys::Key> valueKeys, ItemInfos valueItem, QString valueTable, NumberInfos keyInteger, bool limitToOne = false);
    template <typename T> void setItemAndIntegerSpecificNumberAttribute(ItemInfos item, QString attributeName, QStringList ids, QStringList foreignItemIds, QString numberId, QList<Keys::Key> valueKeys, ItemInfos foreignItem, QString valueTable, NumberInfos keyNumber, NumberInfos valueNumber);
    float keysToFloat(QList<Keys::Key> keys, bool* ok, float currentValue, NumberInfos number) const;
    QStringList keysToIds(QList<Keys::Key> keys) const;
    void writeKey(Keys::Key key);
    void backspace();
    void clearPrompt();
    QList<Keys::Key> promptKeys;
    QPlainTextEdit *messages;
    QLabel* promptLabel;
    QPushButton* blindButton;
    QPushButton* trackingButton;
};

#endif // TERMINAL_H
