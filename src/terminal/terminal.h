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
        Cue, // Q
        Cuelist, // L
    };
public:
    Terminal(QWidget *parent = nullptr);
    void info(QString message);
    void success(QString message);
    void warning(QString message);
    void error(QString message);
signals:
    void dbChanged();
    void itemChanged(QString itemType, QList<int> ids);
private:
    void execute();
    void createItems(QString table, QString itemSingularName, QString itemPluralName, QList<int> ids);
    void deleteItems(QString table, QString itemSingularName, QString itemPluralName, QList<int> ids);
    void setTextAttribute(QString table, QString itemSingularName, QString itemPluralName, QString attribute, QString attributeName, QList<int> ids, QString regex);
    template <typename T> void setNumberAttribute(QString table, QString itemSingularName, QString itemPluralName, QString attribute, QString attributeName, QList<int> ids, QList<Key> valueKeys, QString unit, T minValue, T maxValue, bool cyclic);
    void setItemAttribute(QString table, QString itemSingularName, QString itemPluralName, QString attribute, QString attributeName, QList<int> ids, QList<Key> valueKeys, QString foreignItemTable, QString foreignItemName, Key foreignItemKey);
    float keysToFloat(QList<Key> keys, bool* ok) const;
    QList<int> keysToIds(QList<Key> keys) const;
    QString keysToString(QList<Key> keys) const;
    void updatePrompt();
    void writeKey(Key key);
    void backspace();
    void clearPrompt();
    QList<Key> promptKeys;
    QPlainTextEdit *messages;
    QLabel* promptLabel;
    const QList<Key> itemKeys = {Model, Fixture, Group, Intensity, Color, Position, Raw, Effect, Cue, Cuelist};
    QMap<Key, QString> keyStrings;
};

#endif // TERMINAL_H
