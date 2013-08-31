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

#ifndef TRANSFER_FILE_WINDOW_H
#define TRANSFER_FILE_WINDOW_H

#include <QWidget>
#include <QTableWidget>

class TransferFileModel;
class QSortFilterProxyModel;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;


class TransferFileWindow : public QWidget
{
    Q_OBJECT

public:
    TransferFileWindow(QWidget *parent = 0);

    virtual ~TransferFileWindow() {}

    virtual QSize sizeHint() const;

private slots:
    void deleteTransfer();

private:
    void createTransferFileView();
    void createButtonLayout();
    void createConnections();

    QSortFilterProxyModel *proxyModel;
    QTableView *transferFileView;

    QPushButton *delButton;
    QPushButton *closeButton;

    QHBoxLayout *buttonLayout;
};

#endif // !TRANSFER_FILE_WINDOW_H

