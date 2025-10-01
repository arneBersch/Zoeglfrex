/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "prompt.h"

Prompt::Prompt(QWidget* parent) : QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout(this);

    promptLabel = new QLabel();
    layout->addWidget(promptLabel);

    new QShortcut(Qt::Key_0, this, [this] { writeKey(Zero); });
    new QShortcut(Qt::Key_1, this, [this] { writeKey(One); });
    new QShortcut(Qt::Key_2, this, [this] { writeKey(Two); });
    new QShortcut(Qt::Key_3, this, [this] { writeKey(Three); });
    new QShortcut(Qt::Key_4, this, [this] { writeKey(Four); });
    new QShortcut(Qt::Key_5, this, [this] { writeKey(Five); });
    new QShortcut(Qt::Key_6, this, [this] { writeKey(Six); });
    new QShortcut(Qt::Key_7, this, [this] { writeKey(Seven); });
    new QShortcut(Qt::Key_8, this, [this] { writeKey(Eight); });
    new QShortcut(Qt::Key_9, this, [this] { writeKey(Nine); });

    new QShortcut(Qt::Key_Plus, this, [this] { writeKey(Plus); });
    new QShortcut(Qt::Key_Minus, this, [this] { writeKey(Minus); });
    new QShortcut(Qt::Key_Period, this, [this] { writeKey(Period); });
    new QShortcut(Qt::Key_Comma, this, [this] { writeKey(Period); });
    new QShortcut(Qt::Key_A, this, [this] { writeKey(Attribute); });
    new QShortcut(Qt::Key_S, this, [this] { writeKey(Set); });

    new QShortcut(Qt::Key_M, this, [this] { writeKey(Model); });
    new QShortcut(Qt::Key_F, this, [this] { writeKey(Fixture); });
    new QShortcut(Qt::Key_G, this, [this] { writeKey(Group); });
    new QShortcut(Qt::Key_I, this, [this] { writeKey(Intensity); });
    new QShortcut(Qt::Key_C, this, [this] { writeKey(Color); });
    new QShortcut(Qt::Key_Q, this, [this] { writeKey(Cue); });

    new QShortcut(Qt::Key_Backspace, this, [this] { removeLastKey(); });
    new QShortcut(Qt::SHIFT | Qt::Key_Backspace, this, [this] { clearPrompt(); });
    new QShortcut(Qt::Key_Enter, this, [this] { execute(); });
    new QShortcut(Qt::Key_Return, this, [this] { execute(); });
}

void Prompt::writeKey(Key key) {
    promptKeys.append(key);
    QString table = QString();
    if (promptKeys.first() == Model) {
        table = "models";
    } else if (promptKeys.first() == Fixture) {
        table = "fixtures";
    } else if (promptKeys.first() == Group) {
        table = "groups";
    } else if (promptKeys.first() == Intensity) {
        table = "intensities";
    } else if (promptKeys.first() == Color) {
        table = "colors";
    } else if (promptKeys.first() == Cue) {
        table = "cues";
    }
    if (!table.isEmpty()) {
        QList<Key> idKeys;
        bool id = true;
        for (const Key key : promptKeys.sliced(1, (promptKeys.length() - 1))) {
            bool ok;
            keysToFloat({key}, &ok);
            if (!ok) {
                id = false;
            }
            if (id) {
                idKeys.append(key);
            }
        }
        bool ok;
        emit itemChanged(table, keysToFloat(idKeys, &ok));
    }
    promptLabel->setText(keysToString(promptKeys));
}

void Prompt::removeLastKey() {
    if (promptKeys.isEmpty()) {
        return;
    }
    promptKeys.removeLast();
    promptLabel->setText(keysToString(promptKeys));
}

void Prompt::clearPrompt() {
    promptKeys.clear();
    promptLabel->setText(keysToString(promptKeys));
}

void Prompt::execute() {
    QList<Key> keys = promptKeys;
    if (keys.isEmpty()) {
        return;
    }
    emit info("> " + keysToString(keys));
    clearPrompt();

    Key selectionType = keys.first();
    keys.removeFirst();
    if (!isItemKey(selectionType)) {
        emit error("No item type specified.");
        return;
    }

    QList<Key> selectionIdKeys;
    QList<Key> attributeKeys;
    QList<Key> value;
    bool attributeReached = false;
    bool valueReached = false;
    for (const Key key : keys) {
        if (key == Set) {
            if (valueReached) {
                emit error("Can't use Set more than one time in one command.");
                return;
            }
            valueReached = true;
        } else if ((isItemKey(key) || (key == Attribute)) && !valueReached) {
            //attribute.append(key);
            attributeReached = true;
        } else {
            if (valueReached) {
                value.append(key);
            } else if (attributeReached) {
                attributeKeys.append(key);
            } else {
                selectionIdKeys.append(key);
            }
        }
    }
    const QList<int> selectionIds = keysToIds(selectionIdKeys);
    if (selectionIds.isEmpty()) {
        emit error("Invalid selection ID given.");
        return;
    }
    bool ok;
    int attributeId = keysToFloat(attributeKeys, &ok);
    if (attributeKeys.isEmpty()) {
        attributeId = -1;
    } else if (!ok) {
        emit error("Invalid Attribute ID given.");
        return;
    }
    emit executed(selectionType, selectionIds, attributeId, value);
}

float Prompt::keysToFloat(QList<Key> keys, bool* ok) const {
    return keysToString(keys).replace(" ", "").toFloat(ok);
}

QList<int> Prompt::keysToIds(QList<Key> keys) const {
    keys.append(Plus);
    QList<int> ids;
    QList<Key> currentIdKeys;
    for (const Key key : keys) {
        if (key == Plus) {
            bool ok;
            const int id = keysToFloat(currentIdKeys, &ok);
            if (!ok || (id < 0)) {
                return QList<int>();
            }
            ids.append(id);
            currentIdKeys.clear();
        } else {
            currentIdKeys.append(key);
        }
    }
    return ids;
}

bool Prompt::isItemKey(Key key) const {
    return (
        (key == Model) ||
        (key == Fixture) ||
        (key == Group) ||
        (key == Intensity) ||
        (key == Color) ||
        (key == Cue)
    );
}

QString Prompt::keysToString(QList<Key> keys) const {
    QString promptString;
    for(const int key: keys) {
        if (key == Zero) {
            promptString += "0";
        } else if (key == One) {
            promptString += "1";
        } else if (key == Two) {
            promptString += "2";
        } else if (key == Three) {
            promptString += "3";
        } else if (key == Four) {
            promptString += "4";
        } else if (key == Five) {
            promptString += "5";
        } else if (key == Six) {
            promptString += "6";
        } else if (key == Seven) {
            promptString += "7";
        } else if (key == Eight) {
            promptString += "8";
        } else if (key == Nine) {
            promptString += "9";
        } else if (key == Model) {
            promptString += " Model ";
        } else if (key == Fixture) {
            promptString += " Fixture ";
        } else if (key == Group) {
            promptString += " Group ";
        } else if (key == Intensity) {
            promptString += " Intensity ";
        } else if (key == Color) {
            promptString += " Color ";
        } else if (key == Cue) {
            promptString += " Cue ";
        } else if (key == Set) {
            promptString += " Set ";
        } else if (key == Attribute) {
            promptString += " Attribute ";
        } else if (key == Plus) {
            promptString += " + ";
        } else if (key == Minus) {
            promptString += " - ";
        } else if (key == Period) {
            promptString += ".";
        } else {
            Q_ASSERT(false);
        }
    }
    return promptString.simplified();
}
