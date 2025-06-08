#ifndef PLAYBACKVIEW_H
#define PLAYBACKVIEW_H

#include <QtWidgets>
#include "cuelistmodel.h"

class Kernel;

class PlaybackView : public QWidget {
public:
    PlaybackView(Kernel* core);
    CuelistModel* cuelistModel;
private:
    Kernel* kernel;
};

#endif // PLAYBACKVIEW_H
