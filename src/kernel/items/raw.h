#ifndef RAW_H
#define RAW_H

#include <QtWidgets>

#include "item.h"

class Raw : public Item {
public:
    Raw(Kernel* core);
    Raw(const Raw* item);
    ~Raw();
    int channel = 1;
    uint8_t value = 0;
    QString name() override;
    QString info() override;
};

#endif // RAW_H
