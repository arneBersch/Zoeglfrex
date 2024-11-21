#ifndef COLOR_H
#define COLOR_H

#include <QtWidgets>

#include "item.h"

class Color : public Item {
public:
    Color(Kernel* core);
    Color(const Color* item);
    ~Color();
    float hue = 0.0;
    float saturation = 100.0;
    QString name() override;
    QString info() override;
};

#endif // COLOR_H
