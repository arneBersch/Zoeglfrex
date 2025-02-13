#include "raw.h"
#include "kernel/kernel.h"

Raw::Raw(Kernel* core) : Item(core) {}

Raw::Raw(const Raw* item) : Item(item->kernel) {
    label = item->label;
    channel = item->channel;
    value = item->value;
}

Raw::~Raw() {
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

QString Raw::name() {
    if (label.isEmpty()) {
        return Item::name() + QString::number(channel) + " @ " + QString::number(value);
    }
    return Item::name();
}

QString Raw::info() {
    QString info = Item::info();
    info += "\n2 Channel: " + QString::number(channel);
    info += "\n3 Value: " + QString::number(value);
    return info;
}
