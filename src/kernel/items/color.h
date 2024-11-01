#ifndef COLOR_H
#define COLOR_H

#include <QtWidgets>

#include "item.h"

class Color : public Item {
public:
    Color();
    float red;
    float green;
    float blue;
};

#endif // COLOR_H
