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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "send_file_model.h"
#include "send_file_map.h"
#include "search_user_dialog.h"

#include <QMainWindow>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QMap>
#include <QFileInfoList>


class QStandardItemModel;
class QSortFilterProxyModel;
class QTableView;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QCheckBox;
class QSplitter;
class QModelIndex;
class QTextEdit;

class SendFileWindow;
class SendFileModel;
class SendMsg;
class RecvMsg;
class MsgWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString text = "", QString ip = "",
            QWidget *parent = 0);

    virtual ~MainWindow();

    virtual QSize sizeHint() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);

protected slots:
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);

private slots:
    void updateUserCount(int i);
    void sendMessage();
    void refreshUserList();
    void showSendFileList();
    void updateFileCount();
    void addSendFile();
    void addSendDir();
    void selectGroup();
    void search(QString searchString);
    void showSearchDialog();

signals:
    void messageReplyed();

private:
    void createPeerWidget();
    void createInputWidget();
    void createSplitterWidget();
    void createSendLayout();
    void createActions();
    void createClientModel();
    void setSourceModel();
    void createConnections();
    void selectUser();
    void logSendMsg(QString s, int row);

    bool hasSendFile();
    QString constructFileInfoStr();

    void updateSelectGroupMenu();

    QList<QStandardItem*> searchItems(QString) const;

    QSortFilterProxyModel *proxyUserModel;

    QWidget *peerWidget;
    QWidget *inputWidget;
    QTextEdit *inputEdit;
    QPushButton *fileListButton;
    QGridLayout *sendGridLayout;
    QHBoxLayout *sendLayout;
    QSplitter *splitter;

    QTableView *userView;
    QLabel *userCountLabel;
    QPushButton *refreshButton;
    QPushButton *sendButton;
    QCheckBox *encapCheckBox;
    QCheckBox *lockCheckBox;

    QAction *sendFileAct;
    QAction *sendDirAct;
    QAction *searchUserAct;
    QAction *setLevelOneAct;
    QAction *setLevelTwoAct;
    QMenu *setDisplayLevelMenu;
    QMenu *selectGroupMenu;

    SendFileWindow *sendFileWindow;
    SendFileModel m_sendFileModel;
    SendFileMap *m_sendFileMap;

    static quint32 m_levelOneCount;
    static quint32 m_levelTwoCount;

    QString m_initText;
    QString m_selectedIp;

    QString m_lastSearch;
    QObject *m_lastSearchDialog;
};

#endif // !MAIN_WINDOW_H

