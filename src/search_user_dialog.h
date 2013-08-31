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

#ifndef SEARCH_USER_DIALOG_H
#define SEARCH_USER_DIALOG_H

#include <QDialog>
#include <QString>

class QComboBox;
class QCheckBox;

class SearchUserDialog : public QDialog
{
    Q_OBJECT

public:
    SearchUserDialog(QWidget *parent = 0);

    QSize sizeHint();

signals:
    void searchUser(QString);

private slots:
    void search();

private:
    void createSearchBox();
    void createButtonBox();
    void createConnections();

    QComboBox *searchCombox;
    QPushButton *searchButton;
    QPushButton *closeButton;
    QCheckBox *allColumnCheckBox;
};

#endif // !SEARCH_USER_DIALOG_H
