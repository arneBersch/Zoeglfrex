/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef COLORDATA_H
#define COLORDATA_H

#include "valuedata.h"
#include "intensitydata.h"

class ColorData : public ValueData {
public:
    ColorData();
    ColorData(int fixtureKey, int colorKey);
    void fade(ColorData lastColor, float fade);
    static ColorData highlightValue();
    QColor toQColor(IntensityData intensity);
    void dim(IntensityData intensity);
    void addWhite();
    float getRed() const;
    float getGreen() const;
    float getBlue() const;
    float getCyan() const;
    float getMagenta() const;
    float getYellow() const;
    float getWhite() const;
    float getHue() const;
    float getSaturation() const;
private:
    float red = 1; // 0 <= red <= 1
    float green = 1; // 0 <= green <= 1
    float blue = 1; // 0 <= blue <= 1
    float white = 0; // 0 <= white <= 1
    float quality = 1; // 0 <= quality <= 1
};

#endif // COLORDATA_H
