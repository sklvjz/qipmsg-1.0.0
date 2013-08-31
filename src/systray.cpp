// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#include "systray.h"
#include "main_window.h"
#include "setup_window.h"
#include "window_manager.h"
#include "global.h"
#include "preferences.h"
#include "user_manager.h"
#include "helper.h"
#include "about_dialog.h"
#include "transfer_file_window.h"

#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QStandardItemModel>
#include <QtDebug>


Systray::Systray(QObject *parent)
    : QObject(parent)
{
    trayIcon = new QSystemTrayIcon;

    createActions();
    createMenus();

    createConnections();

    startTimer(500);

    m_iconKey = "normal";
    trayIcon->setIcon(*Global::iconSet.value(m_iconKey));
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("IP Messenger(%1)").arg(0));
}

Systray::~Systray()
{
    delete trayIcon;
    trayIcon = 0;
}

void Systray::timerEvent(QTimerEvent *event)
{
    if (Global::windowManager->hidedMsgWindowCount() > 0) {
        if (m_iconKey == "normal") {
            m_iconKey = "receive";
        } else {
            m_iconKey = "normal";
        }
        trayIcon->setIcon(*Global::iconSet.value(m_iconKey));
    }
}

void Systray::updateToolTip(int i)
{
    trayIcon->setToolTip(tr("IP Messenger (%1)").arg(i));
}

void Systray::createActions()
{
    showTransferFileAction
        = new QAction(tr("Show file transfer monitor (%1)").arg(0), 0);
    delAckWindowAction = new QAction(tr("Delete all acknowledge windows"), 0);
    topWindowAction = new QAction(tr("Top all windows"), 0);
    setupAction = new QAction(tr("Setup preferences..."), 0);
    aboutAction = new QAction(tr("About..."), 0);
    aboutQtAction = new QAction(tr("About Qt"), 0);
    readMsgLogAction = new QAction(tr("Read message log file"), 0);
    quitAction = new QAction(tr("Quit IP Messenger"), 0);
}

void Systray::createMenus()
{
    trayIconMenu = new QMenu;

    trayIconMenu->addAction(showTransferFileAction);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(delAckWindowAction);
    trayIconMenu->addAction(topWindowAction);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(setupAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addAction(readMsgLogAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aboutQtAction);

    trayIconMenu->addSeparator();
    leaveMenu = new QMenu(tr("Leave"));
    leaveMenu->setEnabled(false);
    createLeaveMenu();
    trayIconMenu->addMenu(leaveMenu);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
}

void Systray::createLeaveMenu()
{
    // XXX TODO: not finished yet.
}

void Systray::show()
{
    trayIcon->show();
}

void Systray::createConnections()
{
    connect(showTransferFileAction, SIGNAL(triggered()),
            this, SLOT(showTransferFile()));

    connect(setupAction, SIGNAL(triggered()), this, SLOT(setup()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    connect(delAckWindowAction, SIGNAL(triggered()),
            Global::windowManager, SLOT(destroyMsgReadedWindowList()));
    connect(topWindowAction, SIGNAL(triggered()),
            this, SLOT(setAllWindowVisible()));
    connect(readMsgLogAction, SIGNAL(triggered()),
            this, SLOT(readLog()));

    connect(Global::userManager, SIGNAL(userCountUpdated(int)),
            this, SLOT(updateToolTip(int)));
}

void Systray::setup()
{
    SetupWindow *setupWindow = new SetupWindow;
    setupWindow->show();
}

void Systray::about()
{
    AboutDialog *about = new AboutDialog;
    about->setWindowFlags(Qt::Widget);
    about->show();
}

void Systray::quit()
{
    destroyMainWindowList();

    qApp->quit();
}

void Systray::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        Global::windowManager->visibleAllMsgReadedWindow();
        if (Global::preferences->isSingleClickSendWindow) {
            createMainWindow();
        }
        if (Global::preferences->isNoAutoPopupMsg
            && Global::windowManager->hidedMsgWindowCount() > 0) {
            Global::windowManager->visibleMsgWindow();
            m_iconKey = "normal";
            trayIcon->setIcon(*Global::iconSet.value(m_iconKey));
        }
        break;

    case QSystemTrayIcon::DoubleClick:
        Global::windowManager->visibleAllMsgReadedWindow();
        if (!Global::preferences->isSingleClickSendWindow) {
            createMainWindow();
        }
        break;

    default:
        break;
    }
}

void Systray::createMainWindow()
{
    MainWindow *mainWindow = new MainWindow;
    mainWindow->show();

    // prepend mainWindow to the list
    mainWindowList.insert(0, mainWindow);
}

void Systray::destroyMainWindowList()
{
    while (!mainWindowList.isEmpty()) {
        delete mainWindowList.takeFirst();
    }
}

// XXX NOTE: We do not use external editor.
void Systray::readLog()
{
    // XXX NOTE: Qt use 'xdg-open' to open url, but 'xdg-open' not work
    // properly, so we use the 'qipmsg-xdg-open' to open url. 'qipmsg-xdg-open'
    // use gvfs-open to open url if gvfs-open exist.

    if (Global::preferences->logFilePath.isEmpty()) {
        QMessageBox::information(0, tr("Read Message Log"),
                tr("No message log file specified.")
                );
        return;
    }
    QProcess editor;
    editor.startDetached(Helper::openUrlProgram(),
            QStringList() << Global::preferences->logFilePath);
}

void Systray::setAllWindowVisible()
{
    Global::windowManager->visibleAllMsgWindow();
    visibleAllMainWindow();
    Global::windowManager->visibleAllMsgReadedWindow();
}

void Systray::visibleAllMainWindow()
{
    foreach (MainWindow *w, mainWindowList) {
#if 0
        w->hide();
        w->show();
#else
        w->setWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
        // w->hide();
        QPoint pos = w->pos();
        w->move(pos);
        w->show();
        w->setWindowFlags(w->windowFlags() & ~Qt::WindowStaysOnTopHint);
        w->move(pos);
        w->show();
#endif
    }
}

void Systray::showTransferFile()
{
    qDebug("Systray::showTransferFile");

    TransferFileWindow *transferFileWindow = new TransferFileWindow;

    transferFileWindow->show();
}

void Systray::updateTransferCount(int count)
{
    showTransferFileAction
        ->setText(tr("Show file transfer monitor (%1)").arg(count));
}

