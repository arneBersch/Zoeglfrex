#ifndef COLOR_H
#define COLOR_H

#include <QtWidgets>

#include "item.h"

class Color : public Item {
public:
    Color(Kernel* core);
    Color(const Color* item);
    ~Color();
    float red = 100;
    float green = 100;
    float blue= 100;
    QString name() override;
};

#endif // COLOR_H
