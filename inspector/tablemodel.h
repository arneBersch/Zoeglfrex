#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QtWidgets>

struct TableRow {
    QString id;
    QString label;
};

class TableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TableModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
    void setRows(QList<TableRow> rows);
private:
    QList<TableRow> rowList;
};

#endif // TABLEMODEL_H
