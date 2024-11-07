#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QtWidgets>

class Kernel;
#include "kernel/items/item.h"

template <class T> class ItemList : public QAbstractTableModel {
public:
    ItemList();
    virtual T* getItem(QString id) const = 0;
    virtual int getItemRow(QString id) const = 0;
    virtual QList<QString> getIds() const = 0;
    virtual bool copyItems(QList<QString> ids, QString targetId) = 0;
    virtual bool deleteItems(QList<QString> ids) = 0;
    bool labelItems(QList<QString> ids, QString label);
    virtual bool moveItems(QList<QString> ids, QString targetId) = 0;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    bool greaterId(QString firstId, QString secondId);
    Kernel *kernel;
    QList<T*> items;
};

#endif // ITEMLIST_H
