#ifndef PLAYBACKVIEW_H
#define PLAYBACKVIEW_H

#include <QtWidgets>
#include "cuelistmodel.h"

class Kernel;
class Cuelist;

class PlaybackView : public QWidget {
public:
    PlaybackView(Kernel* core);
    void reset();
private:
    QTableView* tableView;
    CuelistModel* cuelistModel;
    Kernel* kernel;
};

#endif // PLAYBACKVIEW_H
