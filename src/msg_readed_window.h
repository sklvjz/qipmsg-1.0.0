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

#ifndef MSG_READED_WINDOW_H
#define MSG_READED_WINDOW_H

#include <QWidget>
#include <QDialog>

class QPushButton;

class MsgReadedWindow : public QWidget
{
    Q_OBJECT

public:
    MsgReadedWindow(QString nickName);

    virtual ~MsgReadedWindow() {}

    virtual QSize sizeHint() const;

protected:
    void closeEvent(QCloseEvent *event);

private:
    void createUi();
    void createConnections();

    QPushButton *okButton;
    QString string;
};

#endif // !MSG_READED_WINDOW_H

