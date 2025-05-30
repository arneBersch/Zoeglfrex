/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "item.h"
#include "kernel/kernel.h"

Item::Item(Kernel* core) {
    kernel = core;
}

Item::Item(const Item* item) {
    kernel = item->kernel;
    stringAttributes = item->stringAttributes;
    intAttributes = item->intAttributes;
    floatAttributes = item->floatAttributes;
    modelSpecificFloatAttributes = item->modelSpecificFloatAttributes;
    fixtureSpecificFloatAttributes = item->fixtureSpecificFloatAttributes;
    angleAttributes = item->angleAttributes;
    modelSpecificAngleAttributes = item->modelSpecificAngleAttributes;
    fixtureSpecificAngleAttributes = item->fixtureSpecificAngleAttributes;
    boolAttributes = item->boolAttributes;
}

Item::~Item() {}

QString Item::name() {
    return id + " " + stringAttributes.value(kernel->LABELATTRIBUTEID);
}

QString Item::info() {
    QString info = kernel->IDATTRIBUTEID + " ID: " + id;
    info += "\n" + kernel->LABELATTRIBUTEID + " Label: \"" + stringAttributes.value(kernel->LABELATTRIBUTEID) + "\"";
    return info;
}

void Item::writeAttributesToFile(QXmlStreamWriter *fileStream) {
    for (QString attribute : stringAttributes.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", attribute);
        fileStream->writeCharacters(stringAttributes.value(attribute));
        fileStream->writeEndElement();
    }
    for (QString attribute : intAttributes.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", attribute);
        fileStream->writeCharacters(QString::number(intAttributes.value(attribute)));
        fileStream->writeEndElement();
    }
    for (QString attribute : floatAttributes.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", attribute);
        fileStream->writeCharacters(QString::number(floatAttributes.value(attribute)));
        fileStream->writeEndElement();
    }
    for (QString attribute : modelSpecificFloatAttributes.keys()) {
        for (Model* model : modelSpecificFloatAttributes.value(attribute).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", attribute);
            fileStream->writeAttribute("Model", model->id);
            fileStream->writeCharacters(QString::number(modelSpecificFloatAttributes.value(attribute).value(model)));
            fileStream->writeEndElement();
        }
    }
    for (QString attribute : fixtureSpecificFloatAttributes.keys()) {
        for (Fixture* fixture : fixtureSpecificFloatAttributes.value(attribute).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", attribute);
            fileStream->writeAttribute("Fixture", fixture->id);
            fileStream->writeCharacters(QString::number(fixtureSpecificFloatAttributes.value(attribute).value(fixture)));
            fileStream->writeEndElement();
        }
    }
    for (QString attribute : angleAttributes.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", attribute);
        fileStream->writeCharacters(QString::number(angleAttributes.value(attribute)));
        fileStream->writeEndElement();
    }
    for (QString attribute : modelSpecificAngleAttributes.keys()) {
        for (Model* model : modelSpecificAngleAttributes.value(attribute).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", attribute);
            fileStream->writeAttribute("Model", model->id);
            fileStream->writeCharacters(QString::number(modelSpecificAngleAttributes.value(attribute).value(model)));
            fileStream->writeEndElement();
        }
    }
    for (QString attribute : fixtureSpecificAngleAttributes.keys()) {
        for (Fixture* fixture : fixtureSpecificAngleAttributes.value(attribute).keys()) {
            fileStream->writeStartElement("Attribute");
            fileStream->writeAttribute("ID", attribute);
            fileStream->writeAttribute("Fixture", fixture->id);
            fileStream->writeCharacters(QString::number(fixtureSpecificAngleAttributes.value(attribute).value(fixture)));
            fileStream->writeEndElement();
        }
    }
    for (QString attribute : boolAttributes.keys()) {
        fileStream->writeStartElement("Attribute");
        fileStream->writeAttribute("ID", attribute);
        QString valueText = "0";
        if (boolAttributes.value(attribute)) {
            valueText = "1";
        }
        fileStream->writeCharacters(valueText);
        fileStream->writeEndElement();
    }
}
