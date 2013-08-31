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

#include "recvfilesortfilterproxymodel.h"
#include "constants.h"

RecvFileSortFilterProxyModel::RecvFileSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool RecvFileSortFilterProxyModel::filterAcceptsRow(int sourceRow,
        const QModelIndex &sourceParent) const
{
#if 0
    QModelIndex index = sourceModel()->index(sourceRow, RECV_FILE_VIEW_TRANSFER_STATE_POS, sourceParent);

    return isDisplay(sourceModel()->data(index).toInt());
#endif
    return true;
}

#if 0
bool RecvFileSortFilterProxyModel::isDisplay(int state) const
{
    if (state == RecvFileTransfer_NotStart || state == RecvFileTransfer_Fail) {
        return true;
    }

    if (state == RecvFileTransfer_Ok) {
        return false;
    }

    return false;
}
#endif

