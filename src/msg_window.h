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

#ifndef MSG_WINDOW_H
#define MSG_WINDOW_H

#include <QWidget>
#include <QList>
#include <QModelIndexList>

#include "owner.h"
#include "msg.h"
#include "recv_file_map.h"
#include "recv_file_model.h"


class QTextEdit;
class QGroupBox;
class QVBoxLayout;
class QLabel;
class QHBoxLayout;
class QCheckBox;
class QPushButton;
class QStandardItemModel;
class QMessageBox;

class RecvFileThread;

class MsgWindow : public QWidget
{
    Q_OBJECT

public:
    MsgWindow(Msg msg, QWidget *parent = 0);

    virtual QSize sizeHint() const;

    const RecvFileModel& recvFileModel() const { return m_recvFileModel; }

signals:
    void stopTransfer();
    void abortTransfer();

private slots:
    void showReplyMsgBox();
    void openEnvelope();
    void showRecvFile();
    void saveFile(QList<int> fileIdList, QString saveFileDir);
    void recvFileFinish();
    void recvFileError(QString errorString);
    void updateFileCount();
    void closeMsgWindow();
    void cancelTransfer();
    void runRecvFileThread();
    void updateTransferStatsInfo();
    void retryRecvFile();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void createGroupBox();
    void createButtonLayout();
    void createTextEdit();
    QString getSenderInfo();
    void logRecvMsg();
    void initRecvFileInfo();
    bool canClose();
    void retryTransfer();

    bool isAttachFile();
    bool isSealed();

    bool hasRecvFile () const;

    void updateConnections();
    void resetConnections();

    void removeRecvOkFile();

    QGroupBox *groupBox;
    QTextEdit *textEdit;
    QLabel *senderInfoLabel;
    QHBoxLayout *buttonBox;
    QCheckBox *quoteMsgCheckBox;
    QPushButton *sealedButton;
    QPushButton *fileInfoButton;

    QPushButton *closeButton;
    QPushButton *replyButton;

    Msg m_msg;

    RecvFileModel m_recvFileModel;
    RecvFileMap m_recvFileMap;
    RecvFileThread *m_recvFileThread;
    QMessageBox *m_cancelTransferMessageBox;
    bool isCancelTransferCalled;

    QTimer m_timer;
};

#endif // !MSG_WINDOW_H

