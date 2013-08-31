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

#include <QLocale>
#include <QTextCodec>
#include <QtDebug>

#include "transfer_codec.h"
#include "global.h"
#include "preferences.h"


TransferCodec::TransferCodec()
{
#if 0
    initLocaleCodecMap();
    initTransCodec();
#endif

    setTransCodec(Global::preferences->transferCodecName);
}

void TransferCodec::setTransCodec(const QString &codecName)
{
    QByteArray name;
    name.append(codecName);

    if (codecMap.contains(name)) {
        m_codec = codecMap.value(name);
    } else {
        m_codec = QTextCodec::codecForName(name);
        codecMap.insert(name, m_codec);
    }
}

void TransferCodec::initLocaleCodecMap()
{
    // Add your locale and codec here.
#if 0
    localeCodecMap.insert("default", "UTF-8");
    localeCodecMap.insert("zh_CN", "GBK");
    localeCodecMap.insert("en_US", "GBK");
#endif
}

void TransferCodec::initTransCodec()
{
#if 0
    QString localeName = QLocale::system().name();

    if (localeCodecMap.contains(localeName)) {
        m_codec = QTextCodec::codecForName(localeCodecMap.value(localeName));
    } else {
        m_codec = QTextCodec::codecForName(localeCodecMap.value("default"));
    }
#endif
}

QTextCodec * TransferCodec::codec() const
{
    return m_codec;
}

