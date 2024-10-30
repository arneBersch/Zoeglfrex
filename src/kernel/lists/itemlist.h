#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QtWidgets>

class ItemList : public QAbstractTableModel {
    Q_OBJECT
public:
    ItemList();
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool greaterId(QString firstId, QString secondId);
};

#endif // ITEMLIST_H
