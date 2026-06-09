/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef RAWDATA_H
#define RAWDATA_H

#include <QtWidgets>

#include "valuedata.h"

class RawData : public ValueData {
public:
    RawData();
    RawData(int fixtureKey, QList<int> rawKeys);
    void merge(RawData raws);
private:
    struct RawChannelData {
        uint8_t value = 0;
        bool fading = false;
        bool moveWhileDark = false;
    };
    QHash<int, RawChannelData> channels;
};

#endif // RAWDATA_H
