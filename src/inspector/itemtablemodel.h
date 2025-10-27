#ifndef ITEMTABLEMODEL_H
#define ITEMTABLEMODEL_H

#include <QtSql>

class ItemTableModel : public QSqlQueryModel {
    Q_OBJECT
public:
    ItemTableModel();
    void setTable(QString table, QStringList ids);
    QVariant data(const QModelIndex &index, int role) const override;
private:
    QStringList ids;
};

#endif // ITEMTABLEMODEL_H
