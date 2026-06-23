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
#include "types/itemtype.h"
#include "attributes/attribute.h"

class Terminal : public QWidget {
    Q_OBJECT
public:
    Terminal(QWidget *parent = nullptr);
    static QString formatInfoMessage(QString text);
    static QString formatSuccessMessage(QString text);
    static QString formatWarningMessage(QString text);
    static QString formatErrorMessage(QString text);
signals:
    void dbChanged();
    void itemChanged(QString itemType, QStringList ids);
public slots:
    void reload();
private:
    void execute();
    QStringList setCurrentItem(ItemType item, QString itemTable, QList<Keys::Key> idKeys, QString updateQueryText) const;
    QStringList setCueItem(ItemType item, QString valueTable, QList<Keys::Key> idKeys, bool multipleItemsAllowed) const;
    void writeKey(Keys::Key key);
    void printMessage(QString message);
    void backspace();
    void clearPrompt();
    QSet<Attribute*> attributes;
    QList<Keys::Key> promptKeys;
    QPlainTextEdit *messages;
    QLabel* promptLabel;
    QPushButton* blindButton;
    QPushButton* trackingButton;
};

#endif // TERMINAL_H
