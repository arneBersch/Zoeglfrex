/*
    Copyright (c) Arne Bersch
    This file is part of Zöglfrex.
    Zöglfrex is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Zöglfrex is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Zöglfrex. If not, see <https://www.gnu.org/licenses/>.
*/

#include "color.h"
#include "kernel/kernel.h"

Color::Color(Kernel* core) : Item(core) {}

Color::Color(const Color* item) : Item(item) {}

Color::~Color() {
    for (Effect *effect : kernel->effects->items) {
        for (int step : effect->colorSteps.keys()) {
            if (effect->colorSteps.value(step) == this) {
                effect->colorSteps.remove(step);
            }
        }
    }
    for (Cuelist *cuelist : kernel->cuelists->items) {
        for (Cue *cue : cuelist->cues->items) {
            for (Group *group : cue->colors.keys()) {
                if (cue->colors.value(group) == this) {
                    cue->colors.remove(group);
                }
            }
        }
    }
}

QString Color::name() {
    if (stringAttributes.value(kernel->LABELATTRIBUTEID).isEmpty()) {
        const float saturation = floatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID);
        if (saturation <= 10) {
            return Item::name() + "White";
        }
        QString prefix = QString();
        if (saturation <= 70) {
            prefix = "light ";
        }
        const float hue = angleAttributes.value(kernel->COLORHUEATTRIBUTEID);
        if ((hue <= 30) || (hue > 330)) {
            return Item::name() + prefix + "Red";
        } else if (hue <= 90) {
            return Item::name() + prefix + "Yellow";
        } else if (hue <= 150) {
            return Item::name() + prefix + "Green";
        } else if (hue <= 210) {
            return Item::name() + prefix + "Cyan";
        } else if (hue <= 270) {
            return Item::name() + prefix + "Blue";
        } else  {
            return Item::name() + prefix + "Magenta";
        }
    }
    return Item::name();
}

QString Color::info() {
    QString info = Item::info();
    info += "\n" + kernel->COLORHUEATTRIBUTEID + " Hue: " + QString::number(angleAttributes.value(kernel->COLORHUEATTRIBUTEID)) + "°";
    QStringList modelHueValues;
    QStringList modelSaturationValues;
    for (Model* model : kernel->models->items) {
        if (modelSpecificAngleAttributes.value(kernel->COLORHUEATTRIBUTEID).contains(model)) {
            modelHueValues.append(model->name() + " @ " + QString::number(modelSpecificAngleAttributes.value(kernel->COLORHUEATTRIBUTEID).value(model)) + "°");
        }
        if (modelSpecificFloatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID).contains(model)) {
            modelSaturationValues.append(model->name() + " @ " + QString::number(modelSpecificFloatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID).value(model)) + "%");
        }
    }
    info += "\n    Model Exceptions: " + modelHueValues.join("; ");
    QStringList fixtureHueValues;
    QStringList fixtureSaturationValues;
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixtureSpecificAngleAttributes.value(kernel->COLORHUEATTRIBUTEID).contains(fixture)) {
            fixtureHueValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificAngleAttributes.value(kernel->COLORHUEATTRIBUTEID).value(fixture)) + "°");
        }
        if (fixtureSpecificFloatAttributes.value(kernel->COLORHUEATTRIBUTEID).contains(fixture)) {
            fixtureSaturationValues.append(fixture->name() + " @ " + QString::number(fixtureSpecificFloatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID).value(fixture)) + "%");
        }
    }
    info += "\n    Fixture Exceptions: " + fixtureHueValues.join("; ");
    info += "\n" + kernel->COLORSATURATIONATTRIBUTEID + " Saturation: " + QString::number(floatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID)) + "%";
    info += "\n    Model Exceptions: " + modelSaturationValues.join("; ");
    info += "\n    Fixture Exceptions: " + fixtureSaturationValues.join("; ");
    info += "\n" + kernel->COLORQUALITYATTRIBUTEID + " Quality: " + QString::number(floatAttributes.value(kernel->COLORQUALITYATTRIBUTEID)) + "%";
    return info;
}

rgbColor Color::getRGB(Fixture* fixture) {
    rgbColor color;
    float hue = angleAttributes.value(kernel->COLORHUEATTRIBUTEID);
    if (fixtureSpecificAngleAttributes.value(kernel->COLORHUEATTRIBUTEID).contains(fixture)) {
        hue = fixtureSpecificAngleAttributes.value(kernel->COLORHUEATTRIBUTEID).value(fixture);
    } else if (modelSpecificAngleAttributes.value(kernel->COLORHUEATTRIBUTEID).contains(fixture->model)) {
        hue = modelSpecificAngleAttributes.value(kernel->COLORHUEATTRIBUTEID).value(fixture->model);
    }
    const double h = (hue / 60.0);
    const int i = (int)h;
    const double f = h - i;
    float saturation = floatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID);
    if (fixtureSpecificFloatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID).contains(fixture)) {
        saturation = fixtureSpecificFloatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID).value(fixture);
    } else if (modelSpecificFloatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID).contains(fixture->model)) {
        saturation = modelSpecificFloatAttributes.value(kernel->COLORSATURATIONATTRIBUTEID).value(fixture->model);
    }
    const double p = (100.0 - saturation);
    const double q = (100.0 - (saturation * f));
    const double t = (100.0 - (saturation * (1.0 - f)));
    if (i == 0) {
        color.red = 100.0;
        color.green = t;
        color.blue = p;
    } else if (i == 1) {
        color.red = q;
        color.green = 100.0;
        color.blue = p;
    } else if (i == 2) {
        color.red = p;
        color.green = 100.0;
        color.blue = t;
    } else if (i == 3) {
        color.red = p;
        color.green = q;
        color.blue = 100.0;
    } else if (i == 4) {
        color.red = t;
        color.green = p;
        color.blue = 100.0;
    } else if (i == 5) {
        color.red = 100.0;
        color.green = p;
        color.blue = q;
    }
    color.quality = floatAttributes.value(kernel->COLORQUALITYATTRIBUTEID);
    return color;
}
