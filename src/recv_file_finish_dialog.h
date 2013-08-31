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

#ifndef RECV_FILE_FINISH_DIALOG_H
#define RECV_FILE_FINISH_DIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;
class QGroupBox;
class QHBoxLayout;
class QWidget;

class RecvFileMap;

class RecvFileFinishDialog : public QDialog
{
    Q_OBJECT

public:
    RecvFileFinishDialog(RecvFileMap *recvFileMap, QWidget *parent = 0);

signals:
    void fileChecked();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void openFile();
    void openDir();

private:
    void createGroupBox();
    void createButtonLayout();
    void createConnections();

    QLabel *statsLabel;
    QGroupBox *statsGroupBox;

    QHBoxLayout *buttonsLayout;
    QPushButton *openFileButton;
    QPushButton *openDirButton;
    QPushButton *closeButton;

    RecvFileMap *m_recvFileMap;
};

#endif // !RECV_FILE_FINISH_DIALOG_H

