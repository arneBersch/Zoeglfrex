/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ENGINE_H
#define ENGINE_H

#include <QtWidgets>
#include <QtSql>

class Engine : public QWidget {
    Q_OBJECT
public:
    Engine(QWidget* parent = nullptr);
signals:
    void sendUniverse(int universe, QByteArray data);
private:
    void generateDmx();
    void getCurrentCueItems(const int cueId, const int priority, const QString table, QMap<int, int>* fixtureItemKeys, QMap<int, int>* fixtureItemPriorities);
    float getFixtureValue(int fixtureKey, int itemKey, const QString itemTable, const QString itemTableAttribute, const QString modelExceptionTable, const QString fixtureExceptionTable);
};

#endif // ENGINE_H
