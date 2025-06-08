#ifndef CUELISTMODEL_H
#define CUELISTMODEL_H

#include <QtWidgets>

class Kernel;

namespace CuelistModelColumns {
enum {
    cuelistName,
    currentCue,
};
}

class CuelistModel : public QAbstractTableModel {
public:
    CuelistModel(Kernel* core);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const override;
    void reset();
private:
    Kernel* kernel;
};

#endif // CUELISTMODEL_H
