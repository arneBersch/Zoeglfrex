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
    steps = item->steps;
    intensitySteps = item->intensitySteps;
    colorSteps = item->colorSteps;
    rawSteps = item->rawSteps;
}

Effect::~Effect() {
    for (Cue *cue : kernel->cues->items) {
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

QString Effect::info() {
    QString info = Item::info();
    info += "\n" + kernel->effects->STEPSATTRIBUTEID + " Steps: " + QString::number(intAttributes.value(kernel->effects->STEPSATTRIBUTEID));
    QStringList intensityStepValues;
    for (int step : intensitySteps.keys()) {
        intensityStepValues.append(QString::number(step) + ": " + intensitySteps.value(step)->name());
    }
    info += "\n" + kernel->effects->INTENSITYSTEPSATTRIBUTEID + " Intensities: " + intensityStepValues.join("; ");
    QStringList colorStepValues;
    for (int step : colorSteps.keys()) {
        colorStepValues.append(QString::number(step) + ": " + colorSteps.value(step)->name());
    }
    info += "\n" + kernel->effects->COLORSTEPSATTRIBUTEID + " Colors: " + colorStepValues.join("; ");
    QStringList rawStepValues;
    for (int step : rawSteps.keys()) {
        QStringList rawStepValueValues;
        for (Raw* raw : rawSteps.value(step)) {
            rawStepValueValues.append(raw->name());
        }
        rawStepValues.append(QString::number(step) + ": " + rawStepValueValues.join(", "));
    }
    info += "\n" + kernel->effects->RAWSTEPSATTRIBUTEID + " Raws: " + rawStepValues.join("; ");
    info += "\n" + kernel->effects->STEPDURATIONATTRIBUTEID + " Step Duration: " + QString::number(floatAttributes.value(kernel->effects->STEPDURATIONATTRIBUTEID));
    info += "\n" + kernel->effects->STEPFADEATTRIBUTEID + " Step Fade: " + QString::number(floatAttributes.value(kernel->effects->STEPFADEATTRIBUTEID));
    info += "\n" + kernel->effects->PHASEATTRIBUTEID + " Phase: " + QString::number(angleAttributes.value(kernel->effects->PHASEATTRIBUTEID));
    QStringList fixturePhaseValues;
    for (Fixture* fixture : fixtureSpecificAngleAttributes.value(kernel->effects->PHASEATTRIBUTEID).keys()) {
        fixturePhaseValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificAngleAttributes.value(kernel->effects->PHASEATTRIBUTEID).value(fixture)) + "°");
    }
    info += "\n    Fixture Exceptions: " + fixturePhaseValues.join("; ");
    return info;
}

void Effect::writeAttributesToFile(QXmlStreamWriter* fileStream) {
    Item::writeAttributesToFile(fileStream);
    for (int step : intensitySteps.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->effects->INTENSITYSTEPSATTRIBUTEID + "." + QString::number(step)));
        fileStream->writeCharacters(intensitySteps.value(step)->id);
        fileStream->writeEndElement();
    }
    for (int step : colorSteps.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->effects->COLORSTEPSATTRIBUTEID + "." + QString::number(step)));
        fileStream->writeCharacters(colorSteps.value(step)->id);
        fileStream->writeEndElement();
    }
    for (int step : rawSteps.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->effects->RAWSTEPSATTRIBUTEID + "." + QString::number(step)));
        QStringList rawIds;
        for (Raw* raw : rawSteps.value(step)) {
            rawIds.append(raw->id);
        }
        fileStream->writeCharacters(rawIds.join("+"));
        fileStream->writeEndElement();
    }
}
