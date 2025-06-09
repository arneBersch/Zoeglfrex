#include "playbackview.h"
#include "kernel/kernel.h"

PlaybackView::PlaybackView(Kernel* core) {
    kernel = core;

    kernel->mainWindow->setupShortcuts(this);
    setWindowTitle("Zöglfrex Playback View");
    resize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    tableView = new QTableView();
    layout->addWidget(tableView);
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setFocusPolicy(Qt::NoFocus);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->hide();

    cuelistModel = new CuelistModel(kernel);
    tableView->setModel(cuelistModel);
}

void PlaybackView::reset() {
    cuelistModel->reset();
    for (Cuelist* cuelist : kernel->cuelists->items) {
        int cuelistRow = kernel->cuelists->items.indexOf(cuelist);
        QPushButton* goButton = new QPushButton("GO");
        // connect(goButton, &QPushButton::clicked, this, [cuelist] () { qInfo() << "GO Cuelist" << cuelist->name(); });
        tableView->setIndexWidget(cuelistModel->index(cuelistRow, CuelistModelColumns::goButton), goButton);

        QPushButton* goBackButton = new QPushButton("GO BACK");
        // connect(goBackButton, &QPushButton::clicked, this, [cuelist] () { qInfo() << "GO BACK Cuelist" << cuelist->name(); });
        tableView->setIndexWidget(cuelistModel->index(cuelistRow, CuelistModelColumns::goBackButton), goBackButton);
    }
}
