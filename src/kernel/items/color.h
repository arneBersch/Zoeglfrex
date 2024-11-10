#ifndef COLOR_H
#define COLOR_H

#include <QtWidgets>

#include "item.h"

class Color : public Item {
public:
    Color(Kernel* core);
    Color(const Color* item);
    ~Color();
    float red;
    float green;
    float blue;
    QString name() override;
};

#endif // COLOR_H
