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
    red = fadeValue(lastColor.red, red, fade);
    green = fadeValue(lastColor.green, green, fade);
    blue = fadeValue(lastColor.blue, blue, fade);
    quality = fadeValue(lastColor.quality, quality, fade);
}

ColorData ColorData::highlightValue() {
    ColorData data = ColorData();
    data.quality = 0;
    return data;
}

float ColorData::getRed(const bool whiteChannel) const {
    if (whiteChannel) {
        return red - (getWhite() * quality);
    }

    return red;
}

float ColorData::getGreen(const bool whiteChannel) const {
    if (whiteChannel) {
        return green - (getWhite() * quality);
    }

    return green;
}

float ColorData::getBlue(const bool whiteChannel) const {
    if (whiteChannel) {
        return blue - (getWhite() * quality);
    }

    return blue;
}

float ColorData::getCyan(const bool whiteChannel) const {
    return 1 - getRed(whiteChannel);
}

float ColorData::getMagenta(const bool whiteChannel) const {
    return 1 - getGreen(whiteChannel);
}

float ColorData::getYellow(const bool whiteChannel) const {
    return 1 - getBlue(whiteChannel);
}

float ColorData::getWhite() const {
    return std::min(std::min(red, green), blue);
}

float ColorData::getHue() const {
    const float min = std::min(std::min(red, green), blue);
    const float max = std::max(std::max(red, green), blue);

    if (min == max) {
        return 0;
    }

    float hue;
    if (max >= red) {
        hue = (green - blue) / (max - min);
    } else if (max >= green) {
        hue = (blue - red) / (max - min);
        hue += 2;
    } else if (max >= blue) {
        hue = (red - green) / (max - min);
        hue += 4;
    } else {
        Q_ASSERT(false);
    }

    hue /= 6;

    if (hue < 0) {
        hue += 1;
    }

    return hue;
}

float ColorData::getSaturation() const {
    const float min = std::min(std::min(red, green), blue);
    const float max = std::max(std::max(red, green), blue);

    if (max <= 0) {
        return 0;
    }

    return 1 - (min / max);
}

void ColorData::dim(const float dimmer) {
    red *= dimmer;
    green *= dimmer;
    blue *= dimmer;
}

QColor ColorData::toQColor(const float dimmer) const {
    return QColor(
        red * dimmer * 255,
        green * dimmer * 255,
        blue * dimmer * 255
    );
}
