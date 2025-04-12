/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "raw.h"
#include "kernel/kernel.h"

Raw::Raw(Kernel* core) : Item(core) {}

Raw::Raw(const Raw* item) : Item(item) {
    channelValues = item->channelValues;
    modelSpecificChannelValues = item->modelSpecificChannelValues;
    fixtureSpecificChannelValues = item->fixtureSpecificChannelValues;
}

Raw::~Raw() {
    for (Effect *effect : kernel->effects->items) {
        for (int step : effect->rawSteps.keys()) {
            if (effect->rawSteps.contains(step)) {
                effect->rawSteps[step].removeAll(this);
                if (effect->rawSteps[step].isEmpty()) {
                    effect->rawSteps.remove(step);
                }
            }
        }
    }
    for (Cue *cue : kernel->cues->items) {
        for (Group *group : cue->raws.keys()) {
            if (cue->raws.contains(group)) {
                cue->raws[group].removeAll(this);
                if (cue->raws[group].isEmpty()) {
                    cue->raws.remove(group);
                }
            }
        }
    }
}

QString Raw::info() {
    QString info = Item::info();
    QStringList channelValue;
    for (int channel = 1; channel <= 512; channel++) {
        if (channelValues.contains(channel)) {
            channelValue.append(QString::number(channel) + " @ " + QString::number(channelValues.value(channel)));
        }
    }
    info += "\n" + kernel->raws->CHANNELVALUEATTRIBUTEID + ".x Channel Values: " + channelValue.join(", ");
    QStringList modelChannelValue;
    for (Model* model : kernel->models->items) {
        if (modelSpecificChannelValues.contains(model)) {
            QStringList modelChannelValueValues;
            for (int channel = 1; channel <= 512; channel++) {
                if (modelSpecificChannelValues.value(model).contains(channel)) {
                    modelChannelValueValues.append(QString::number(channel) + " @ " + QString::number(modelSpecificChannelValues.value(model).value(channel)));
                }
            }
            modelChannelValue.append(model->name() + ": " + modelChannelValueValues.join(", "));
        }
    }
    info += "\n    Model Exceptions: " + modelChannelValue.join("; ");
    QStringList fixtureChannelValue;
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixtureSpecificChannelValues.contains(fixture)) {
            QStringList fixtureChannelValueValues;
            for (int channel = 1; channel <= 512; channel++) {
                if (fixtureSpecificChannelValues.value(fixture).contains(channel)) {
                    fixtureChannelValueValues.append(QString::number(channel) + " @ " + QString::number(fixtureSpecificChannelValues.value(fixture).value(channel)));
                }
            }
            fixtureChannelValue.append(fixture->name() + ": " + fixtureChannelValueValues.join(", "));
        }
    }
    info += "\n    Fixture Exceptions: " + fixtureChannelValue.join("; ");
    return info;
}

void Raw::writeAttributesToFile(QXmlStreamWriter* fileStream) {
    Item::writeAttributesToFile(fileStream);
    for (int channel : channelValues.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->raws->CHANNELVALUEATTRIBUTEID + "." + QString::number(channel)));
        fileStream->writeCharacters(QString::number(channelValues.value(channel)));
        fileStream->writeEndElement();
    }
    for (Model* model : modelSpecificChannelValues.keys()) {
        for (int channel : modelSpecificChannelValues.value(model).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", (kernel->raws->CHANNELVALUEATTRIBUTEID + "." + QString::number(channel)));
            fileStream->writeAttribute("Model", model->id);
            fileStream->writeCharacters(QString::number(modelSpecificChannelValues.value(model).value(channel)));
            fileStream->writeEndElement();
        }
    }
    for (Fixture* fixture : fixtureSpecificChannelValues.keys()) {
        for (int channel : fixtureSpecificChannelValues.value(fixture).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", (kernel->raws->CHANNELVALUEATTRIBUTEID + "." + QString::number(channel)));
            fileStream->writeAttribute("Fixture", fixture->id);
            fileStream->writeCharacters(QString::number(fixtureSpecificChannelValues.value(fixture).value(channel)));
            fileStream->writeEndElement();
        }
    }
}
