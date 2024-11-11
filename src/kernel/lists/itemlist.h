#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QtWidgets>

class Kernel;

template <class T> class ItemList : public QAbstractTableModel {
public:
    ItemList(QString singularName, QString pluralName);
    T* getItem(QString id) const;
    int getItemRow(QString id) const;
    QList<QString> getIds() const;
    void copyItems(QList<QString> ids, QString targetId);
    bool deleteItems(QList<QString> ids);
    bool labelItems(QList<QString> ids, QString label);
    bool moveItems(QList<QString> ids, QString targetId);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    bool greaterId(QString firstId, QString secondId);
    Kernel *kernel;
    QList<T*> items;
private:
    QString singularItemName = "item";
    QString pluralItemName = "items";
};

#endif // ITEMLIST_H
