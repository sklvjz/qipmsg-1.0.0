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

#ifndef SEND_FILE_WINDOW_H
#define SEND_FILE_WINDOW_H

#include <QWidget>

#define NAME_COLUMN_NUM     0
#define SIZE_COLUMN_NUM     1
#define PATH_COLUMN_NUM     2

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QTableView;
class QSortFilterProxyModel;

class SendFileModel;

class SendFileWindow : public QWidget
{
    Q_OBJECT

public:
    SendFileWindow(SendFileModel *sendFileModel, QWidget *parent = 0);

    virtual ~SendFileWindow() {}

    virtual QSize sizeHint() const;

private slots:
    void addFile();
    void addDir();
    void delFile();

signals:
    void addFileFinished();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void createButtonLayout();
    void createConnections();
    void createSendFileView(SendFileModel *sendFileModel);

    QSortFilterProxyModel *proxyModel;
    QTableView *sendFileView;

    QVBoxLayout *mainLayout;

    QPushButton *addFileButton;
    QPushButton *addDirButton;
    QPushButton *delButton;
    QPushButton *closeButton;

    QHBoxLayout *buttonLayout;
    SendFileModel *m_sendFileModel;
};

#endif // !SEND_FILE_WINDOW_H

