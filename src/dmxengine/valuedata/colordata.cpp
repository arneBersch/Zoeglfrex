/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "colordata.h"

ColorData::ColorData() {}

ColorData::ColorData(const int fixtureKey, const int colorKey) {
    const float hue = getFixtureValue(fixtureKey, colorKey, "colors", "hue", "color_model_hue", "color_fixture_hue");
    const float saturation = getFixtureValue(fixtureKey, colorKey, "colors", "saturation", "color_model_saturation", "color_fixture_saturation") / 100;

    const int h = hue / 60;
    const float f = (hue / 60) - h;
    const float p = (1 - saturation);
    const float q = (1 - (saturation * f));
    const float t = (1 - (saturation * (1 - f)));

    if (h == 0) {
        red = 1;
        green = t;
        blue = p;
    } else if (h == 1) {
        red = q;
        green = 1;
        blue = p;
    } else if (h == 2) {
        red = p;
        green = 1;
        blue = t;
    } else if (h == 3) {
        red = p;
        green = q;
        blue = 1;
    } else if (h == 4) {
        red = t;
        green = p;
        blue = 1;
    } else if (h == 5) {
        red = 1;
        green = p;
        blue = q;
    }

    quality = getFixtureValue(fixtureKey, colorKey, "colors", "quality", "color_model_quality", "color_fixture_quality") / 100;
}

void ColorData::fade(ColorData lastColor, float fade) {
    red += (lastColor.red - red) * fade;
    green += (lastColor.green - green) * fade;
    blue += (lastColor.blue - blue) * fade;
    quality += (lastColor.quality - quality) * fade;
}

ColorData ColorData::highlightValue() {
    ColorData data = ColorData();
    data.quality = 0;
    return data;
}

float ColorData::getRed() {
    return red;
}

float ColorData::getGreen() {
    return green;
}

float ColorData::getBlue() {
    return blue;
}

float ColorData::getCyan() {
    return 1 - red;
}

float ColorData::getMagenta() {
    return 1 - green;
}

float ColorData::getYellow() {
    return 1 - blue;
}

float ColorData::getWhite() {
    return white;
}

QColor ColorData::toQColor(IntensityData intensity) {
    return QColor(
        red * intensity.getDimmer() * 255,
        green * intensity.getDimmer() * 255,
        blue * intensity.getDimmer() * 255
    );
}

void ColorData::dim(IntensityData intensity) {
    red *= intensity.getDimmer();
    green *= intensity.getDimmer();
    blue *= intensity.getDimmer();
}

void ColorData::addWhite() {
    white = std::min(std::min(red, green), blue);
    red -= white * quality;
    green -= white * quality;
    blue -= white * quality;
}
