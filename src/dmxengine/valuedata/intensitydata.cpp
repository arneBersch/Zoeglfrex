/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "intensitydata.h"

IntensityData::IntensityData() {}

IntensityData::IntensityData(int fixtureKey, int intensityKey) {
    dimmer = getFixtureValue(fixtureKey, intensityKey, "intensities", "dimmer", "intensity_model_dimmer", "intensity_fixture_dimmer") / 100;
}

void IntensityData::merge(const IntensityData intensity) {
    dimmer = std::max(dimmer, intensity.dimmer);
}

void IntensityData::fade(const IntensityData lastIntensity, const float fade) {
    dimmer = fadeValue(lastIntensity.dimmer, dimmer, fade);
}

void IntensityData::smoothDim(const float lastDimmer) {
    const float MAX_DIMMER_DIFFERENCE = 0.01;

    if (std::abs(dimmer - lastDimmer) <= MAX_DIMMER_DIFFERENCE) {
        return;
    }

    if (dimmer > lastDimmer) {
        dimmer = lastDimmer + MAX_DIMMER_DIFFERENCE;
    } else {
        dimmer = lastDimmer - MAX_DIMMER_DIFFERENCE;
    }
}

IntensityData IntensityData::highlightValue() {
    IntensityData data;
    data.dimmer = 1;
    return data;
}

float IntensityData::getDimmer() const {
    return dimmer;
}
