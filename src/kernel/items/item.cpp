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
    intAttributes = item->intAttributes;
    floatAttributes = item->floatAttributes;
    angleAttributes = item->angleAttributes;
}

Item::~Item() {}

QString Item::name() {
    return id + " " + label;
}

QString Item::info() {
    QString info = kernel->models->IDATTRIBUTEID + " ID: " + id;
    info += "\n" + kernel->models->LABELATTRIBUTEID + " Label: \"" + label + "\"";
    return info;
}
