#include "playbackview.h"
#include "kernel/kernel.h"

PlaybackView::PlaybackView(Kernel* core) {
    kernel = core;

    kernel->mainWindow->setupShortcuts(this);
    setWindowTitle("Zöglfrex Playback View");
    resize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    QTableView* tableView = new QTableView();
    layout->addWidget(tableView);
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setFocusPolicy(Qt::NoFocus);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->hide();

    cuelistModel = new CuelistModel(kernel);
    tableView->setModel(cuelistModel);
}
