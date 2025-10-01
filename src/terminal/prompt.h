/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef PROMPT_H
#define PROMPT_H

#include <QtWidgets>

class Prompt : public QWidget {
    Q_OBJECT
public:
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
        Set, // S
        Attribute, // A
        Model, // M
        Fixture, // F
        Group, // G
        Intensity, // I
        Color, // C
        Cue, // Q
    };
    Prompt(QWidget* parent = nullptr);
    float keysToFloat(QList<Key> keys, bool* ok) const;
    QList<int> keysToIds(QList<Key> keys) const;
signals:
    void info(QString info);
    void error(QString error);
    void itemChanged(QString table, int id);
    void executed(Key selectionType, QList<int> ids, int attribute, QList<Key> value);
private:
    QString keysToString(QList<Key> keys) const;
    void clearPrompt();
    bool isItemKey(Key key) const;
    void execute();
    void writeKey(Key key);
    void removeLastKey();
    QLabel* promptLabel;
    QList<Key> promptKeys = QList<Key>();
};

#endif // PROMPT_H
