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

#include "sizecolumndelegate.h"

#include <QtCore>
#include <QtGui>


SizeColumnDelegate::SizeColumnDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void SizeColumnDelegate::paint(QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    QStyleOptionViewItem myOption = option;
    myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

    QString s;
    if (text == "Folder") {
        s = tr("Folder");
    } else {
        bool ok;
        qint64 size = text.toLongLong(&ok, 10);
        s = tr("%1 KB").arg(int((size + 1023) / 1024));
    }

    drawDisplay(painter, myOption, myOption.rect, s);
    drawFocus(painter, myOption, myOption.rect);
}

