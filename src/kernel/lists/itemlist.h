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
    void copyItems(QList<QString> ids, QString sourceId);
    void deleteItems(QList<QString> ids);
    void labelItems(QList<QString> ids, QString label);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    Kernel *kernel;
    QList<T*> items;
    void setAttribute(QList<QString> ids, QMap<int, QString> attribute, QList<int> values, QString text);
protected:
    T* recordItem(QString id);
    virtual void setOtherAttribute(QList<QString> ids, QMap<int, QString> attribute, QList<int> values, QString text) = 0;
private:
    int findRow(QString id);
    QString singularItemName = "item";
    QString pluralItemName = "items";
};

#endif // ITEMLIST_H
