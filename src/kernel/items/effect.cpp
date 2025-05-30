/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "effect.h"
#include "kernel/kernel.h"

Effect::Effect(Kernel* core) : Item(core) {}

Effect::Effect(const Effect* item) : Item(item) {
    intensitySteps = item->intensitySteps;
    colorSteps = item->colorSteps;
    positionSteps = item->positionSteps;
    rawSteps = item->rawSteps;
    stepSpecificFloatAttributes = item->stepSpecificFloatAttributes;
}

Effect::~Effect() {
    for (Cuelist *cuelist : kernel->cuelists->items) {
        for (Cue *cue : cuelist->cues->items) {
            for (Group *group : cue->effects.keys()) {
                if (cue->effects.contains(group)) {
                    cue->effects[group].removeAll(this);
                    if (cue->effects[group].isEmpty()) {
                        cue->effects.remove(group);
                    }
                }
            }
        }
    }
}

QString Effect::info() {
    QString info = Item::info();
    info += "\n" + kernel->EFFECTSTEPSATTRIBUTEID + " Steps: " + QString::number(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID));
    QStringList intensityStepValues;
    QStringList colorStepValues;
    QStringList positionStepValues;
    QStringList rawStepValues;
    QStringList stepHoldValues;
    QStringList stepFadeValues;
    for (int step = 1; step <= intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID); step++) {
        if (intensitySteps.contains(step)) {
            intensityStepValues.append(QString::number(step) + ": " + intensitySteps.value(step)->name());
        }
        if (colorSteps.contains(step)) {
            colorStepValues.append(QString::number(step) + ": " + colorSteps.value(step)->name());
        }
        if (positionSteps.contains(step)) {
            positionStepValues.append(QString::number(step) + ": " + positionSteps.value(step)->name());
        }
        if (rawSteps.contains(step)) {
            QStringList rawStepValueValues;
            for (Raw* raw : kernel->raws->items) {
                if (rawSteps.value(step).contains(raw)) {
                    rawStepValueValues.append(raw->name());
                }
            }
            rawStepValues.append(QString::number(step) + ": " + rawStepValueValues.join(", "));
        }
        if (stepSpecificFloatAttributes.value(kernel->EFFECTSTEPHOLDATTRIBUTEID).contains(step)) {
            stepHoldValues.append(QString::number(step) + ": " + QString::number(stepSpecificFloatAttributes.value(kernel->EFFECTSTEPHOLDATTRIBUTEID).value(step)) + "s");
        }
        if (stepSpecificFloatAttributes.value(kernel->EFFECTSTEPFADEATTRIBUTEID).contains(step)) {
            stepFadeValues.append(QString::number(step) + ": " + QString::number(stepSpecificFloatAttributes.value(kernel->EFFECTSTEPFADEATTRIBUTEID).value(step)) + "s");
        }
    }
    info += "\n" + kernel->EFFECTINTENSITYSTEPSATTRIBUTEID + ".x Intensities: " + intensityStepValues.join("; ");
    info += "\n" + kernel->EFFECTCOLORSTEPSATTRIBUTEID + ".x Colors: " + colorStepValues.join("; ");
    info += "\n" + kernel->EFFECTPOSITIONSTEPSATTRIBUTEID + ".x Positions: " + positionStepValues.join("; ");
    info += "\n" + kernel->EFFECTRAWSTEPSATTRIBUTEID + ".x Raws: " + rawStepValues.join("; ");
    info += "\n" + kernel->EFFECTSTEPHOLDATTRIBUTEID + " Step Hold: " + QString::number(floatAttributes.value(kernel->EFFECTSTEPHOLDATTRIBUTEID)) + "s";
    info += "\n    Step Exceptions: " + stepHoldValues.join("; ");
    info += "\n" + kernel->EFFECTSTEPFADEATTRIBUTEID + " Step Fade: " + QString::number(floatAttributes.value(kernel->EFFECTSTEPFADEATTRIBUTEID)) + "s";
    info += "\n    Step Exceptions: " + stepFadeValues.join("; ");
    info += "\n" + kernel->EFFECTPHASEATTRIBUTEID + " Phase: " + QString::number(angleAttributes.value(kernel->EFFECTPHASEATTRIBUTEID)) + "°";
    QStringList fixturePhaseValues;
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixtureSpecificAngleAttributes.value(kernel->EFFECTPHASEATTRIBUTEID).contains(fixture)) {
            fixturePhaseValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificAngleAttributes.value(kernel->EFFECTPHASEATTRIBUTEID).value(fixture)) + "°");
        }
    }
    info += "\n    Fixture Exceptions: " + fixturePhaseValues.join("; ");
    return info;
}

void Effect::writeAttributesToFile(QXmlStreamWriter* fileStream) {
    Item::writeAttributesToFile(fileStream);
    for (int step : intensitySteps.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->EFFECTINTENSITYSTEPSATTRIBUTEID + "." + QString::number(step)));
        fileStream->writeCharacters(intensitySteps.value(step)->id);
        fileStream->writeEndElement();
    }
    for (int step : colorSteps.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->EFFECTCOLORSTEPSATTRIBUTEID + "." + QString::number(step)));
        fileStream->writeCharacters(colorSteps.value(step)->id);
        fileStream->writeEndElement();
    }
    for (int step : positionSteps.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->EFFECTPOSITIONSTEPSATTRIBUTEID + "." + QString::number(step)));
        fileStream->writeCharacters(positionSteps.value(step)->id);
        fileStream->writeEndElement();
    }
    for (int step : rawSteps.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->EFFECTRAWSTEPSATTRIBUTEID + "." + QString::number(step)));
        QStringList rawIds;
        for (Raw* raw : rawSteps.value(step)) {
            rawIds.append(raw->id);
        }
        fileStream->writeCharacters(rawIds.join("+"));
        fileStream->writeEndElement();
    }
    for (QString attribute : stepSpecificFloatAttributes.keys()) {
        for (int step : stepSpecificFloatAttributes.value(attribute).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", (attribute + "." + QString::number(step)));
            fileStream->writeCharacters(QString::number(stepSpecificFloatAttributes.value(attribute).value(step)));
            fileStream->writeEndElement();
        }
    }
}

int Effect::getStep(Fixture* fixture, int frame, float* fade) {
    QList<int> stepFrames = QList<int>(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID), 0);
    QList<int> fadeFrames = QList<int>(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID), 0);
    int totalFrames = 0;
    for (int step = 1; step <= intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID); step++) {
        if (stepSpecificFloatAttributes.value(kernel->EFFECTSTEPFADEATTRIBUTEID).contains(step)) {
            fadeFrames[step - 1] = (kernel->dmxEngine->PROCESSINGRATE * stepSpecificFloatAttributes.value(kernel->EFFECTSTEPFADEATTRIBUTEID).value(step));
        } else {
            fadeFrames[step - 1] = (kernel->dmxEngine->PROCESSINGRATE * floatAttributes.value(kernel->EFFECTSTEPFADEATTRIBUTEID));
        }
        stepFrames[step - 1] = fadeFrames[step - 1];
        if (stepSpecificFloatAttributes.value(kernel->EFFECTSTEPHOLDATTRIBUTEID).contains(step)) {
            stepFrames[step - 1] += (kernel->dmxEngine->PROCESSINGRATE * stepSpecificFloatAttributes.value(kernel->EFFECTSTEPHOLDATTRIBUTEID).value(step));
        } else {
            stepFrames[step - 1] += (kernel->dmxEngine->PROCESSINGRATE * floatAttributes.value(kernel->EFFECTSTEPHOLDATTRIBUTEID));
        }
        totalFrames += stepFrames[step - 1];
    }
    int step = 1;
    if (totalFrames > 0) {
        if (fixtureSpecificAngleAttributes.value(kernel->EFFECTPHASEATTRIBUTEID).contains(fixture)) {
            frame += fixtureSpecificAngleAttributes.value(kernel->EFFECTPHASEATTRIBUTEID).value(fixture) * (float)totalFrames / 360.0;
        } else {
            frame += angleAttributes.value(kernel->EFFECTPHASEATTRIBUTEID) * (float)totalFrames / 360.0;
        }
        frame %= totalFrames;
        while (frame >= stepFrames[step - 1]) {
            frame -= stepFrames[step - 1];
            step++;
        }
        (*fade) = 1;
        if (frame < fadeFrames[step - 1]) {
            (*fade) = 1 - ((float)(fadeFrames[step - 1] - frame) / (float)fadeFrames[step - 1]);
        }
        Q_ASSERT((*fade) <= 1);
    }
    return step;
}

float Effect::getDimmer(Fixture* fixture, int frame) {
    float dimmer = 0;
    if (!intensitySteps.isEmpty()) {
        float fade = 0;
        int step = getStep(fixture, frame, &fade);
        float formerDimmer = 0.0;
        if ((step > 1) && intensitySteps.contains(step - 1)) {
            formerDimmer = intensitySteps.value(step - 1)->getDimmer(fixture);
        } else if ((step == 1) && intensitySteps.contains(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID))) {
            formerDimmer = intensitySteps.value(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID))->getDimmer(fixture);
        }
        if (intensitySteps.contains(step)) {
            dimmer = intensitySteps.value(step)->getDimmer(fixture);
        }
        dimmer = formerDimmer + (dimmer - formerDimmer) * fade;
    }
    return dimmer;
}

rgbColor Effect::getRGB(Fixture* fixture, int frame) {
    rgbColor color = {};
    if (!colorSteps.isEmpty()) {
        float fade = 0;
        int step = getStep(fixture, frame, &fade);
        rgbColor formerColor = {};
        if ((step > 1) && colorSteps.contains(step - 1)) {
            formerColor = colorSteps.value(step - 1)->getRGB(fixture);
        } else if ((step == 1) && colorSteps.contains(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID))) {
            formerColor = colorSteps.value(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID))->getRGB(fixture);
        }
        if (colorSteps.contains(step)) {
            color = colorSteps.value(step)->getRGB(fixture);
        }
        color.red = formerColor.red + (color.red - formerColor.red) * fade;
        color.green = formerColor.green + (color.green - formerColor.green) * fade;
        color.blue = formerColor.blue + (color.blue - formerColor.blue) * fade;
        color.quality = formerColor.quality + (color.quality - formerColor.quality) * fade;
    }
    return color;
}

positionAngles Effect::getPosition(Fixture* fixture, int frame) {
    positionAngles position = {};
    if (!positionSteps.isEmpty()) {
        float fade = 0;
        int step = getStep(fixture, frame, &fade);
        positionAngles formerPosition = {};
        if ((step > 1) && positionSteps.contains(step - 1)) {
            formerPosition = positionSteps.value(step - 1)->getAngles(fixture);
        } else if ((step == 1) && positionSteps.contains(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID))) {
            formerPosition = positionSteps.value(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID))->getAngles(fixture);
        }
        if (positionSteps.contains(step)) {
            position = positionSteps.value(step)->getAngles(fixture);
        }
        if (std::abs(formerPosition.pan - position.pan) > 180) {
            if (formerPosition.pan > position.pan) {
                position.pan += 360;
            } else if (formerPosition.pan < position.pan) {
                formerPosition.pan += 360;
            }
        }
        position.pan = formerPosition.pan + (position.pan - formerPosition.pan) * fade;
        while (position.pan >= 360) {
            position.pan -= 360;
        }
        position.tilt = formerPosition.tilt + (position.tilt - formerPosition.tilt) * fade;
        position.zoom = formerPosition.zoom + (position.zoom - formerPosition.zoom) * fade;
    }
    return position;
}

QMap<int, uint8_t> Effect::getRaws(Fixture* fixture, int frame, bool renderMiBRaws) {
    QMap<int, uint8_t> channels;
    for (int step = 1; step <= intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID); step++) {
        if (rawSteps.contains(step)) {
            for (Raw* raw : rawSteps.value(step)) {
                if (renderMiBRaws || raw->boolAttributes.value(kernel->RAWMOVEINBLACKATTRIBUTEID)) {
                    for (int channel : raw->getChannels(fixture).keys()) {
                        channels[channel] = 0;
                    }
                }
            }
        }
    }
    if (!channels.keys().isEmpty()) {
        float fade = 0;
        int step = getStep(fixture, frame, &fade);
        QList<Raw*> formerRaws;
        if ((step > 1) && rawSteps.contains(step - 1)) {
            formerRaws = rawSteps.value(step - 1);
        } else if ((step == 1) && rawSteps.contains(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID))) {
            formerRaws = rawSteps.value(intAttributes.value(kernel->EFFECTSTEPSATTRIBUTEID));
        }
        for (int channel : channels.keys()) {
            uint8_t formerValue = 0;
            for (Raw* raw : formerRaws) {
                if ((renderMiBRaws || raw->boolAttributes.value(kernel->RAWMOVEINBLACKATTRIBUTEID)) && raw->getChannels(fixture).contains(channel)) {
                    formerValue = raw->getChannels(fixture).value(channel);
                }
            }
            uint8_t value = 0;
            bool fadeThisChannel = true;
            if (rawSteps.contains(step)) {
                for (Raw* raw : rawSteps.value(step)) {
                    if ((renderMiBRaws || raw->boolAttributes.value(kernel->RAWMOVEINBLACKATTRIBUTEID)) && raw->getChannels(fixture).contains(channel)) {
                        fadeThisChannel = raw->boolAttributes.value(kernel->RAWFADEATTRIBUTEID);
                        value = raw->getChannels(fixture).value(channel);
                    }
                }
            }
            if (fadeThisChannel) {
                channels[channel] = formerValue + (uint8_t)((float)(value - formerValue) * fade);
            } else {
                channels[channel] = value;
            }
        }
    }
    return channels;
}
