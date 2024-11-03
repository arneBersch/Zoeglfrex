#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QtWidgets>

class Kernel;
#include "kernel/items/item.h"

class ItemList : public QAbstractTableModel {
    Q_OBJECT
public:
    ItemList();
    virtual Item* getItem(QString id) const = 0;
    virtual int getItemRow(QString id) const = 0;
    virtual Item* getItemByRow(int row) const = 0;
    virtual bool copyItems(QList<QString> ids, QString targetId) = 0;
    virtual bool deleteItems(QList<QString> ids) = 0;
    bool labelItems(QList<QString> ids, QString label);
    virtual bool moveItems(QList<QString> ids, QString targetId) = 0;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override = 0;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    bool greaterId(QString firstId, QString secondId);
    Kernel *kernel;
};

#endif // ITEMLIST_H
