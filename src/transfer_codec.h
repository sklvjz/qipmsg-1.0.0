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

#ifndef TRANSFER_CODEC_H
#define TRANSFER_CODEC_H

#include <QObject>
#include <QMap>

class TransferCodec
{
public:
    TransferCodec();

    void setTransCodec(const QString &codecName);
    QTextCodec * codec() const;

private:
    void initTransCodec();
    void initLocaleCodecMap();

    QMultiMap<QString, QByteArray> localeCodecMap;
    QMultiMap<QByteArray, QTextCodec *> codecMap;

    QTextCodec *m_codec;   // codec for transfer message
};

#endif // !TRANSFER_CODEC_H

