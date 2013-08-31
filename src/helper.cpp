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

#include <QLibraryInfo>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QTextStream>

#include "helper.h"
#include "constants.h"

QString Helper::m_iniPath;
QString Helper::m_appPath;
qint64 Helper::m_packetNo;
QFile Helper::m_internalLogFile;
QString Helper::m_internalLogFileName;

void Helper::setAppPath(QString path)
{
    m_appPath = path;
}

QString Helper::appPath()
{
    return m_appPath;
}

void Helper::setIniPath(QString path)
{
    m_iniPath = path;
}

QString Helper::iconPath()
{
#ifdef ICON_PATH
    QString path = QString(ICON_PATH);
    if (!path.isEmpty()) {
        return path;
    } else {
        return m_appPath + "/icons";
    }
#else
    return m_appPath + "/icons";
#endif
}

QString Helper::appHomePath()
{
    return QDir::homePath() + "/.qipmsg";
}

QString Helper::lockFile()
{
    return appHomePath() + "/qipmsg.lock";
}

QString Helper::iniPath()
{
    if (!m_iniPath.isEmpty()) {
        return m_iniPath;
    } else {
        if (QFile::exists(appHomePath())) {
            return appHomePath();
        }
    }
    return "";
}

QString Helper::loginName()
{
    return getEnvironmentVariable(QRegExp("LOGNAME.*"));
}

QString Helper::hostname()
{
    QString name = getEnvironmentVariable(QRegExp("HOSTNAME.*"));

    // if no HOSTNAME environment variable is set, read it from /etc/hostname,
    // for distribution like ubuntu.
    if (name.isEmpty()) {
        QFile file("/etc/hostname");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            name.append(file.readLine());
            name.remove(QChar('\n'));
        }
    }

    if (name.isEmpty()) {
        name = QObject::tr("Unkown hostname");
    }

    return name;
}

QString Helper::translationPath()
{
#ifdef TRANSLATION_PATH
    QString path = QString(TRANSLATION_PATH);
    if (!path.isEmpty()) {
        return path;
    } else {
        return appPath() + "/translations";
    }
#else
    return appPath() + "/translations";
#endif
}

QString Helper::qtTranslationPath()
{
    return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
}

QString Helper::getEnvironmentVariable(QRegExp regExp)
{
    QStringList environment = QProcess::systemEnvironment();

    int index = environment.indexOf(regExp);
    if (index != -1) {
        QStringList sl = environment.at(index).split("=");
        if (sl.size() == 2) {
            return sl.at(1).toUtf8();
        }
    }

    return QString("");
}

void Helper::setPacketNo(qint64 n)
{
    m_packetNo = n;
}

QString Helper::packetNoString()
{
    ++m_packetNo;
    return QString("%1").arg(m_packetNo);
}

qint64 Helper::packetNo()
{
    ++m_packetNo;
    return m_packetNo;
}

void Helper::setInternalLogFileName(QString filePath)
{
    m_internalLogFileName = filePath;
}

void Helper::openInternalLogFile()
{
    if (!m_internalLogFile.isOpen()) {
        m_internalLogFile.setFileName(m_internalLogFileName);
        m_internalLogFile.open(QIODevice::WriteOnly);
    }
}

void Helper::writeInternalLog(QString line)
{
    openInternalLogFile();
    QTextStream ts(&m_internalLogFile);

    ts << line << endl;
}

QString Helper::soundPath()
{
#ifdef SOUND_PATH
    QString path = QString(SOUND_PATH);
    if (!path.isEmpty()) {
        return path;
    } else {
        return m_appPath + "/sounds";
    }
#else
    return m_appPath + "/sounds";
#endif
}

QString Helper::openUrlProgram()
{
#if 0
    return appPath() + "/qipmsg-xdg-open";
#else
    return "xdg-open";
#endif
}

QString Helper::fileCountString(int fileCount)
{
    if (fileCount <= 1) {
        return QString(QObject::tr("%1 file")).arg(fileCount);
    } else {
        return QString(QObject::tr("%1 files")).arg(fileCount);
    }

    // this should never be reached
    return QString();
}

QString Helper::sizeStringUnit(double size, QString sep)
{
    QString s;
    if (size >= ONE_MB) {
        s = QString("%1%2MB")
            .arg(size/ONE_MB, 0, 'f', 1)
            .arg(sep);
    } else {
        s = QString("%1%2KB")
            .arg(qMax(size/ONE_KB, 1.0), 0, 'f', 0)
            .arg(sep);
    }

    return s;
}

QString Helper::secondStringUnit(int second)
{
    if (second <= 1) {
        return QString(QObject::tr("%1 second")).arg(second);
    } else {
        return QString(QObject::tr("%1 seconds")).arg(second);
    }

    // this should never be reached
    return QString();
}

