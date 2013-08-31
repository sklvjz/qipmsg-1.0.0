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

#include <QtGui>
#include <QtCore>
#include <QtDebug>
#include <QtGlobal>


#include "msg_window.h"
#include "owner.h"
#include "recv_file_window.h"
#include "constants.h"
#include "global.h"
#include "user_manager.h"
#include "msg_thread.h"
#include "window_manager.h"
#include "main_window.h"
#include "systray.h"
#include "preferences.h"
#include "recv_file_thread.h"
#include "recv_file_finish_dialog.h"
#include "retry_recv_file_dialog.h"
#include "send_msg.h"


MsgWindow::MsgWindow(Msg msg, QWidget *parent)
    : QWidget(parent), m_msg(msg), isCancelTransferCalled(false)
{
    // Delete when close
    setAttribute(Qt::WA_DeleteOnClose, true);

    if (isAttachFile()) {
        initRecvFileInfo();
        fileInfoButton
            = new QPushButton(tr("Show receive file list (%1)")
                              .arg(Global::fileCountString
                                   (m_recvFileModel.rowCount())));
        connect(&(m_recvFileMap.m_timer), SIGNAL(timeout()),
                this, SLOT(updateTransferStatsInfo()));
    }

    if (Global::preferences->isLogMsg) {
        logRecvMsg();
    }

    createGroupBox();
    createTextEdit();
    createButtonLayout();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox);
    if (isAttachFile()) {
        mainLayout->addWidget(fileInfoButton);
        if (isSealed()) {
            fileInfoButton->hide();
        }
        connect(fileInfoButton, SIGNAL(clicked()),
                this, SLOT(showRecvFile()));
    }
    mainLayout->addWidget(textEdit);

    if (isSealed()) {
        sealedButton = new QPushButton(tr("&Open Envelope"));
        mainLayout->addWidget(sealedButton);
        textEdit->hide();
        quoteMsgCheckBox->hide();
        connect(sealedButton, SIGNAL(clicked()),
                this, SLOT(openEnvelope()));
    }

    mainLayout->addLayout(buttonBox);

    setLayout(mainLayout);

    adjustSize();

    setWindowTitle(tr("Receiving Message"));
    setWindowIcon(*Global::iconSet.value("normal"));
    if (isSealed()) {
        sealedButton->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);
        sealedButton->resize(textEdit->size());
    }

    move(Global::randomNearMiddlePoint());
}

QSize MsgWindow::sizeHint() const
{
    return QSize(480, 320);
}

bool MsgWindow::isAttachFile()
{
    return m_msg->flags() & IPMSG_FILEATTACHOPT;
}

bool MsgWindow::isSealed()
{
    return m_msg->flags() & IPMSG_READCHECKOPT;
}

void MsgWindow::createGroupBox()
{
    groupBox = new QGroupBox(tr("receive message from ..."));
    QVBoxLayout *vbox = new QVBoxLayout;

    QDateTime now = QDateTime::currentDateTime();
    QString timeString(tr("Time: "));
    timeString.append(now.toString("yyyy-MM-dd dddd hh:mm:ss"));

    senderInfoLabel = new QLabel;
    senderInfoLabel->setText(QString(tr("%1\n%2")
                                     .arg(getSenderInfo())
                                     .arg(timeString)
                                     ));
    senderInfoLabel->setAlignment(Qt::AlignHCenter);
    vbox->addWidget(senderInfoLabel);

    groupBox->setLayout(vbox);
}

QString MsgWindow::getSenderInfo()
{
    QString senderInfo;

    int row = Global::userManager->ipToRow(m_msg->ip());
    if (row != -1) {    // We have sender
        QString name = Global::userManager->name(row);
        QString group = Global::userManager->group(row);
        QString host = m_msg->owner().host();

        QString s = group.isEmpty() ? "" : group + "/";
        senderInfo  = name + " (" + s + host + ")";
    } else {    // Not have sender
        QString name = m_msg->owner().name();
        QString group = m_msg->owner().group();
        QString host = m_msg->owner().host();

        QString s = group.isEmpty() ? "" : group + "/";
        senderInfo  = name + " (" + s + host + ")";
    }

    return senderInfo;
}

void MsgWindow::createTextEdit()
{
    textEdit = new QTextEdit;
    textEdit->setAcceptRichText(false);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(m_msg->additionalInfo());
}

void MsgWindow::createButtonLayout()
{
    buttonBox = new QHBoxLayout;

    closeButton = new QPushButton(tr("&Close"));
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    replyButton = new QPushButton(tr("&Reply"));
    connect(replyButton, SIGNAL(clicked()),
            this, SLOT(showReplyMsgBox()));

    quoteMsgCheckBox = new QCheckBox(tr("&Quote Message"));
    if (Global::preferences->isQuoteMsg) {
        quoteMsgCheckBox->setCheckState(Qt::Checked);
    }

    buttonBox->addStretch();
    buttonBox->addWidget(closeButton);
    buttonBox->addSpacing(20);
    buttonBox->addWidget(replyButton);
    buttonBox->addSpacing(20);
    buttonBox->addWidget(quoteMsgCheckBox);
}

void MsgWindow::showReplyMsgBox()
{
    QString text = textEdit->toPlainText();

    QString initReply;
    if (quoteMsgCheckBox->isChecked()
            && quoteMsgCheckBox->isVisible()
            && !text.isEmpty()) {
        QStringList sl = text.split(QChar('\n'));

        foreach (QString s, sl) {
            s.prepend(Global::preferences->quoteMark);
            initReply.append(s);
            initReply.append(QChar('\n'));
        }
    }

    MainWindow *window = new MainWindow(initReply, m_msg->ip());
    connect(window, SIGNAL(messageReplyed()),
            this, SLOT(closeMsgWindow()));
    Global::systray->mainWindowList.insert(0, window);

    window->show();
}

void MsgWindow::closeMsgWindow()
{
    if (isAttachFile()) {
        return;
    } else if (Global::preferences->noAutoCloseMsgWindowWhenReply) {
        return;
    } else {
        close();
    }
}

void MsgWindow::openEnvelope()
{
    qDebug("MsgWindow::openEnvelope");

    sealedButton->hide();
    if (isAttachFile()) {
        fileInfoButton->show();
    }
    textEdit->show();
    quoteMsgCheckBox->show();

    replyButton->setFocus(Qt::TabFocusReason);

    int flags = 0;
    flags |= IPMSG_READMSG | IPMSG_READCHECKOPT;

    SendMsg sendMsg(m_msg->ipAddress(), m_msg->port(),
                    m_msg->packetNoString(), ""/* extendedInfo */, flags);

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void MsgWindow::showRecvFile()
{
    qDebug("MsgWindow::showRecvFile");

    RecvFileWindow *recvFileWindow = new RecvFileWindow(this);

    connect(recvFileWindow, SIGNAL(saveFile(QList<int>, QString)),
            this, SLOT(saveFile(QList<int>, QString)));

    recvFileWindow->show();
}

void MsgWindow::closeEvent(QCloseEvent *event)
{
    if (m_recvFileMap.transferState() == RecvFileMap::Transfer) {
        cancelTransfer();
        event->ignore();
        return;
    }

    if (hasRecvFile()) {
        int flags = 0;
        flags |= IPMSG_RELEASEFILES;

        SendMsg sendMsg(m_msg->ipAddress(), m_msg->port(),
                        m_msg->packetNoString(), ""/* extendedInfo */, flags);

        Global::msgThread->addSendMsg(Msg(sendMsg));
    }

    Global::windowManager->removeMsgWindow(this);

    event->accept();
}

bool MsgWindow::hasRecvFile () const
{
    return m_recvFileModel.rowCount() > 0;
}

void MsgWindow::logRecvMsg()
{
    if (!Global::preferences->logFile.isWritable()) {
        return;
    }

    QString name, group, host, ip, loginName;

    host = m_msg->owner().host();
    loginName = m_msg->owner().loginName();
    ip = m_msg->owner().ip();

    int row = Global::userManager->ipToRow(m_msg->ip());
    if (row != -1) {    // We have sender
        name = m_msg->owner().name();
        group = m_msg->owner().group();
    } else {    // Not have sender
        name = Global::userManager->name(row);
        group = Global::userManager->group(row);
    }

    QTextStream ts(&Global::preferences->logFile);

    QString log("=====================================\n");
    log.append(tr(" From: "));
    log.append(name);
    if (Global::preferences->isLogLoginName) {
        log.append("[");
        log.append(loginName);
        log.append("]");
    }

    log.append(" (");
    if (!group.isEmpty()) {
        log.append(group);
        log.append("/");
    }
    log.append(host);
    if (Global::preferences->isLogIP) {
        log.append("/");
        log.append(ip);
    }
    log.append(")\n");
    log.append(tr(" Time: "));
    QDateTime now = QDateTime::currentDateTime();
    log.append(now.toString("yyyy-MM-dd dddd hh:mm:ss"));
    // XXX TODO: if enable encrypt, need log encrypt method.
    if (isSealed()) {
        log.append(tr(" (Sealed)"));
    }
    log.append("\n");

    // Log receive file
    if (isAttachFile()) {
        QString files = "  ("
            + Global::fileCountString(m_recvFileModel.rowCount()) + ") ";
        files.append(m_recvFileMap.fileNameJoin(", "));

        log.append(files);
        log.append(QChar('\n'));
    } // End log receive file

    log.append("-------------------------------------\n");
    log.append(m_msg->additionalInfo());
    log.append("\n\n");

    ts << log << flush;
}

void MsgWindow::updateFileCount()
{
    if (m_recvFileModel.rowCount() == 0) {
        if (fileInfoButton->isVisible()) {
            fileInfoButton->hide();
        }
    } else {
        fileInfoButton->setText(tr("Show receive file list (%1)")
                .arg(Global::fileCountString(m_recvFileModel.rowCount())));
        if (!fileInfoButton->isVisible()) {
            fileInfoButton->show();
        }
    }
}

void MsgWindow::initRecvFileInfo()
{
    qDebug() << "MsgWindow::initRecvFileInfo";

    QStringList infoList = m_msg->extendedInfo().split(QChar('\a'),
            QString::SkipEmptyParts);
    foreach (QString info, infoList) {
        info.replace(QString(FILE_NAME_BEFORE), QString(FILE_NAME_ESCAPE));
        QStringList l = info.split(QChar(COMMAND_SEPERATOR));
        if (l.size() < RECV_FILE_EXTEND_ATTR_POS) {
            continue;
        } else {
            RecvFile recvFile(m_msg->ip(), m_msg->packetNoString(), info);
            m_recvFileMap.addFile(recvFile.fileId(), RecvFileHandle(recvFile));
            m_recvFileModel.addRow(recvFile);
        }
    }
}

void MsgWindow::saveFile(QList<int> fileIdList, QString saveFileDir)
{
    m_recvFileMap.setSaveFilePath(saveFileDir);

    fileInfoButton->setText(tr("Preparing to get file... "));

    // close 'RecvFileWindow'
    RecvFileWindow *senderWindow = qobject_cast<RecvFileWindow *>(sender());
    if (senderWindow) {
        senderWindow->close();
    }

#if 0
    QList<int> fileIdList;
    foreach (QModelIndex i, indexList) {
        int row = i.row();
        int fileId = m_recvFileModel.data(QModelIndex(row, RECV_FILE_ID_POS))
            .toInt();
        qDebug() << fileId;
        fileIdList << fileId;
    }
#endif
    foreach (RecvFileHandle h, m_recvFileMap.m_map) {
        if (fileIdList.contains(h->fileId())) {
            h->setState(RecvFile::ToRecv);
        } else {
            h->setState(RecvFile::NotRecv);
        }
    }

    m_recvFileMap.setState(RecvFileMap::Normal);
    runRecvFileThread();
}

void MsgWindow::runRecvFileThread()
{
    m_recvFileMap.setTransferState(RecvFileMap::Transfer);

    m_recvFileThread = new RecvFileThread(&m_recvFileMap);

    updateConnections();

    connect(m_recvFileThread, SIGNAL(finished()),
            m_recvFileThread, SLOT(deleteLater()));

    connect(this, SIGNAL(stopTransfer()),
            m_recvFileThread, SIGNAL(stopTransfer()));
    connect(this, SIGNAL(abortTransfer()),
            m_recvFileThread, SIGNAL(abortTransfer()));

    connect(m_recvFileThread, SIGNAL(recvFileFinished()),
            this, SLOT(recvFileFinish()));
    connect(m_recvFileThread, SIGNAL(recvFileError(QString)),
            this, SLOT(recvFileError(QString)));

    m_recvFileThread->start();
}

void MsgWindow::removeRecvOkFile()
{
    QMap<int, RecvFileHandle>::iterator i = m_recvFileMap.m_map.begin();

    while (i != m_recvFileMap.m_map.end()) {
        if (i.value()->state() == RecvFile::RecvOk) {
            m_recvFileModel.removeRow(i.value()->fileId());
        }

        ++i;
    }
}

void MsgWindow::recvFileFinish()
{
    qDebug("MsgWindow::recvFileFinish");

    // if is 'cancelTransfer' been called and we get here, m_recvFileThread is
    // not stopped.
    if (isCancelTransferCalled) {
        isCancelTransferCalled = false;
        m_cancelTransferMessageBox->close();
        m_recvFileThread = 0;
    }

    resetConnections();

    updateTransferStatsInfo();

    removeRecvOkFile();

    RecvFileFinishDialog *recvFileFinishDialog
        = new RecvFileFinishDialog(&m_recvFileMap, this);
    connect(recvFileFinishDialog, SIGNAL(fileChecked()),
            this, SLOT(updateFileCount()));

    recvFileFinishDialog->show();
}

void MsgWindow::recvFileError(QString errorString)
{
    qDebug() << "MsgWindow::recvFileError:" << errorString;

    // if is 'cancelTransfer' been called and we get here, m_recvFileThread is
    // not stopped.
    if (isCancelTransferCalled) {
        isCancelTransferCalled = false;
        m_cancelTransferMessageBox->close();
        m_recvFileThread = 0;
    }

    resetConnections();

    updateTransferStatsInfo();

    removeRecvOkFile();

    retryTransfer();
}

void MsgWindow::cancelTransfer()
{
    emit stopTransfer();
    isCancelTransferCalled = true;

    m_cancelTransferMessageBox = new QMessageBox(this);
    // m_cancelTransferMessageBox->setParent(this);
    m_cancelTransferMessageBox->setWindowModality(Qt::WindowModal);

    m_cancelTransferMessageBox
        ->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    m_cancelTransferMessageBox->setWindowTitle(tr("Alert"));
    m_cancelTransferMessageBox->setText(tr("Stop transfer?"));
    m_cancelTransferMessageBox->setIcon(QMessageBox::Warning);
    switch (m_cancelTransferMessageBox->exec()) {
        case QMessageBox::Ok:
            emit abortTransfer();
            m_recvFileThread->resumeTransfer();
            m_recvFileMap.stopTimer();
            resetConnections();
            removeRecvOkFile();
            updateFileCount();
            break;
        case QMessageBox::Cancel:
            m_recvFileThread->resumeTransfer();
            break;
        default:
            // Should never be reached
            break;
    }
}

void MsgWindow::updateConnections()
{
    disconnect(fileInfoButton, SIGNAL(clicked()),
            this, SLOT(showRecvFile()));

    connect(fileInfoButton, SIGNAL(clicked()),
            this, SLOT(cancelTransfer()));
}

void MsgWindow::resetConnections()
{
    disconnect(fileInfoButton, SIGNAL(clicked()),
            this, SLOT(cancelTransfer()));

    connect(fileInfoButton, SIGNAL(clicked()),
            this, SLOT(showRecvFile()));
}

void MsgWindow::retryTransfer()
{
    RetryRecvFileDialog *retryRecvFileDialog
        = new RetryRecvFileDialog(&m_recvFileMap, this);

    connect(retryRecvFileDialog, SIGNAL(retry()),
            this, SLOT(retryRecvFile()));
    connect(retryRecvFileDialog, SIGNAL(retryChecked()),
            this, SLOT(updateFileCount()));

    retryRecvFileDialog->show();
}

void MsgWindow::retryRecvFile()
{
    m_recvFileMap.setState(RecvFileMap::Retry);

    fileInfoButton->setText(tr("Preparing to get file... "));

    runRecvFileThread();
}

void MsgWindow::updateTransferStatsInfo()
{
    fileInfoButton
        ->setText(m_recvFileMap.currentFile()->transferStatsInfo());
}

