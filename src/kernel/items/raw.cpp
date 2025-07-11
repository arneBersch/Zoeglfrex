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
    for (Model* model : kernel->models->items) {
        for (QString attribute : model->rawListAttributes.keys()) {
            model->rawListAttributes[attribute].removeAll(this);
        }
    }
    for (Fixture* fixture : kernel->fixtures->items) {
        for (QString attribute : fixture->rawListAttributes.keys()) {
            fixture->rawListAttributes[attribute].removeAll(this);
        }
    }
    for (Group* group : kernel->groups->items) {
        for (QString attribute : group->rawListAttributes.keys()) {
            group->rawListAttributes[attribute].removeAll(this);
        }
    }
    for (Intensity* intensity : kernel->intensities->items) {
        for (QString attribute : intensity->rawListAttributes.keys()) {
            intensity->rawListAttributes[attribute].removeAll(this);
        }
    }
    for (Color* color : kernel->colors->items) {
        for (QString attribute : color->rawListAttributes.keys()) {
            color->rawListAttributes[attribute].removeAll(this);
        }
    }
    for (Position* position : kernel->positions->items) {
        for (QString attribute : position->rawListAttributes.keys()) {
            position->rawListAttributes[attribute].removeAll(this);
        }
    }
    for (Raw* raw : kernel->raws->items) {
        for (QString attribute : raw->rawListAttributes.keys()) {
            raw->rawListAttributes[attribute].removeAll(this);
        }
    }
    for (Effect* effect : kernel->effects->items) {
        for (QString attribute : effect->rawListAttributes.keys()) {
            effect->rawListAttributes[attribute].removeAll(this);
        }
        for (int step : effect->rawSteps.keys()) {
            if (effect->rawSteps.contains(step)) {
                effect->rawSteps[step].removeAll(this);
                if (effect->rawSteps[step].isEmpty()) {
                    effect->rawSteps.remove(step);
                }
            }
        }
    }
    for (Cuelist* cuelist : kernel->cuelists->items) {
        for (QString attribute : cuelist->rawListAttributes.keys()) {
            cuelist->rawListAttributes[attribute].removeAll(this);
        }
        for (Cue* cue : cuelist->cues->items) {
            for (QString attribute : cue->rawListAttributes.keys()) {
                cue->rawListAttributes[attribute].removeAll(this);
            }
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
}

QString Raw::info() {
    QString info = Item::info();

    QStringList channelValue;
    for (int channel = 1; channel <= 512; channel++) {
        if (channelValues.contains(channel)) {
            channelValue.append(QString::number(channel) + " @ " + QString::number(channelValues.value(channel)));
        }
    }
    info += "\n" + kernel->RAWCHANNELVALUEATTRIBUTEID + ".x Channel Values: " + channelValue.join(", ");
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

    info += "\n" + kernel->RAWMOVEWHILEDARKATTRIBUTEID + " Move while Dark: ";
    if (boolAttributes.value(kernel->RAWMOVEWHILEDARKATTRIBUTEID)) {
        info += "True";
    } else {
        info += "False";
    }

    info += "\n" + kernel->RAWFADEATTRIBUTEID + " Fade: ";
    if (boolAttributes.value(kernel->RAWFADEATTRIBUTEID)) {
        info += "True";
    } else {
        info += "False";
    }

    return info;
}

void Raw::writeAttributesToFile(QXmlStreamWriter* fileStream) {
    Item::writeAttributesToFile(fileStream);
    for (int channel : channelValues.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", (kernel->RAWCHANNELVALUEATTRIBUTEID + "." + QString::number(channel)));
        fileStream->writeCharacters(QString::number(channelValues.value(channel)));
        fileStream->writeEndElement();
    }
    for (Model* model : modelSpecificChannelValues.keys()) {
        for (int channel : modelSpecificChannelValues.value(model).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", (kernel->RAWCHANNELVALUEATTRIBUTEID + "." + QString::number(channel)));
            fileStream->writeAttribute("Model", model->id);
            fileStream->writeCharacters(QString::number(modelSpecificChannelValues.value(model).value(channel)));
            fileStream->writeEndElement();
        }
    }
    for (Fixture* fixture : fixtureSpecificChannelValues.keys()) {
        for (int channel : fixtureSpecificChannelValues.value(fixture).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", (kernel->RAWCHANNELVALUEATTRIBUTEID + "." + QString::number(channel)));
            fileStream->writeAttribute("Fixture", fixture->id);
            fileStream->writeCharacters(QString::number(fixtureSpecificChannelValues.value(fixture).value(channel)));
            fileStream->writeEndElement();
        }
    }
}

QMap<int, uint8_t> Raw::getChannels(Fixture* fixture) {
    QMap<int, uint8_t> channels;
    for (int channel : channelValues.keys()) {
        channels[channel] = channelValues.value(channel);
    }
    if (modelSpecificChannelValues.contains(fixture->model)) {
        for (int channel : modelSpecificChannelValues.value(fixture->model).keys()) {
            channels[channel] = modelSpecificChannelValues.value(fixture->model).value(channel);
        }
    }
    if (fixtureSpecificChannelValues.contains(fixture)) {
        for (int channel : fixtureSpecificChannelValues.value(fixture).keys()) {
            channels[channel] = fixtureSpecificChannelValues.value(fixture).value(channel);
        }
    }
    return channels;
}
