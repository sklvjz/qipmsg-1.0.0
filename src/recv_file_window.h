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

#ifndef RECV_FILE_WINDOW_H
#define RECV_FILE_WINDOW_H

#include <QWidget>
#include <QModelIndexList>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QTableView;

class RecvFileSortFilterProxyModel;

class RecvFileWindow : public QWidget
{
    Q_OBJECT

public:
    RecvFileWindow(QWidget *parent);

    virtual ~RecvFileWindow() {}

    virtual QSize sizeHint() const;

signals:
    void saveFile(QList<int> fileIdList, QString saveFileDir);

private slots:
    void save();
    void selectAll();

private:
    void createRecvFileView();
    void createButtonLayout();
    void createConnections();
    bool isFileExist(QString directory, QModelIndexList &indexList,
            int *existType);
    QString selectSaveDirectory();
    bool isRecvFileExist(QString fileName,
            QModelIndexList &indexList);

    QHBoxLayout *buttonsLayout;

    RecvFileSortFilterProxyModel *proxyModel;
    QTableView *recvFileView;

    QPushButton *saveButton;
    QPushButton *closeButton;
    QPushButton *selectAllButton;
    QPushButton *clearSelectButton;
};

#endif // !RECV_FILE_WINDOW_H

