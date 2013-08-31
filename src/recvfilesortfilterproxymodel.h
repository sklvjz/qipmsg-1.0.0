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

#ifndef RECVFILESORTFILTERPROXYMODEL_H
#define RECVFILESORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class RecvFileSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    RecvFileSortFilterProxyModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool isDisplay(int state) const;
};


#endif // !RECVFILESORTFILTERPROXYMODEL_H

