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
#include "itemtype.h"
#include "numbertype.h"

class Terminal : public QWidget {
    Q_OBJECT
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
    void updateSortingKeys(ItemType item);
    static bool compareIds(QString idA, QString idB);
    void setCurrentItem(ItemType item, QString itemTable, QList<Keys::Key> idKeys, QString updateQueryText);
    void setCueItem(ItemType item, QString valueTable, QList<Keys::Key> idKeys, bool multipleItemsAllowed);
    void createItems(ItemType item, QStringList ids);
    void deleteItems(ItemType item, QStringList ids);
    void moveItems(ItemType item, QStringList ids, QList<Keys::Key> valueKeys);
    void setIntegerSpecificItemListAttribute(ItemType item, QString attributeName, QStringList ids, QString integerId, QList<Keys::Key> valueKeys, ItemType valueItem, QString valueTable, NumberType keyInteger, bool limitToOne = false);
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
