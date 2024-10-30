#include "itemlist.h"

ItemList::ItemList() {
    //
}

int ItemList::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

bool ItemList::greaterId(QString firstId, QString secondId) {
    QList<QString> firstIdParts = firstId.split(".");
    QList<QString> secondIdParts = secondId.split(".");
    if (firstIdParts[0].toInt() < secondIdParts[0].toInt()) {
        return true;
    } else if (firstIdParts[0].toInt() == secondIdParts[0].toInt()) {
        if (firstIdParts[1].toInt() < secondIdParts[1].toInt()) {
            return true;
        } else if (firstIdParts[1].toInt() == secondIdParts[1].toInt()) {
            if (firstIdParts[2].toInt() < secondIdParts[2].toInt()) {
                return true;
            } else if (firstIdParts[2].toInt() == secondIdParts[2].toInt()) {
                if (firstIdParts[3].toInt() < secondIdParts[3].toInt()) {
                    return true;
                } else if (firstIdParts[3].toInt() == secondIdParts[3].toInt()) {
                    if (firstIdParts[4].toInt() < secondIdParts[4].toInt()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
