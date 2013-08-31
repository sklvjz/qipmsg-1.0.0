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

#include <QtCore>
#include <QtGui>

#include <unistd.h>     // For usleep()

#include "qipmsg.h"
#include "helper.h"
#include "global.h"
#include "translator.h"
#include "lockfile.h"
#include "file_server.h"
#include "msg_thread.h"
#include "send_msg.h"
#include "recv_msg.h"
#include "user_manager.h"
#include "constants.h"

static void createHomeDirectory();
static void myMessageOutput(QtMsgType type, const char *msg);
static void checkTcpServerError();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qInstallMsgHandler(myMessageOutput);

    qRegisterMetaType<Msg>("Msg");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    // Create application home directory
    createHomeDirectory();
    QString iniPath = Helper::appHomePath();
    Helper::setIniPath(iniPath);
    Helper::setAppPath(app.applicationDirPath());

    QString fileName = Helper::appHomePath() + "/qipmsg_internal.log";
    Helper::setInternalLogFileName(fileName);

    // Make sure there is only one instance running.
    LockFile::instance()->setLockFile(Helper::lockFile());
    if (!LockFile::instance()->lock()) {
        qWarning("main: lock '%s' failed, exit.",
                 Helper::lockFile().toUtf8().data());
        return -1;
    }

    // Set seed value.
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    // Set init package number as random.
    Helper::setPacketNo(qrand() % 1024);

    // XXX TODO: language can be selected.
    // Auto detect language.
    QString language = "";
    Translator::instance()->load(language);

    QIpMsg *qipmsg = new QIpMsg;

    checkTcpServerError();

    Global::msgThread->start();
    // this make sure thead started
    while (!Global::msgThread->isRunning()) {
        usleep(200000);
    }

    Global::userManager->broadcastEntry();

    app.setQuitOnLastWindowClosed(false);
    int rc = app.exec();

    Global::userManager->broadcastExit();

    delete qipmsg;

    return rc;
}

static void createHomeDirectory()
{
    // Create qipmsg home directories
    if (!QFile::exists(Helper::appHomePath())) {
        QDir d;
        if (!d.mkdir(Helper::appHomePath())) {
            qWarning("createHomeDirectory: can't create %s",
                     Helper::appHomePath().toUtf8().data());
        }
    }
}

static void myMessageOutput(QtMsgType type, const char *msg) {
    QString line = "[" + QTime::currentTime().toString() + "] " +
        QString::fromUtf8(msg);

    switch (type) {
        case QtDebugMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Debug: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog(line);
            break;
        case QtWarningMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Warning: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog("WARNING: " + line);
            break;
        case QtFatalMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Fatal: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog("FATAL: " + line);
            abort();                    // deliberately core dump
        case QtCriticalMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Critical: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog("CRITICAL: " + line);
            break;
    }
}

static void checkTcpServerError()
{
    // Check tcp server
    if (!Global::fileServer->isListening()) {
        QString errorString(QObject::tr("Start tcp server error!"));

        QMessageBox::critical(0, QObject::tr("Start QIpMsg"),
                          errorString + ":\n"
                          + Global::fileServer->errorString());


        qDebug() << "main::checkTcpServerError:"
            << Global::fileServer->errorString();

        exit(-1);
    }
}

