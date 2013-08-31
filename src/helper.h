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

#ifndef HELPER_H
#define HELPER_H

#include <QRegExp>
#include <QFile>

class Helper {
public:
    static void setAppPath(QString path);
    static QString appPath();

    static QString appHomePath();

    static QString lockFile();

    static void setIniPath(QString path);
    static QString iniPath();

    static QString translationPath();
    static QString qtTranslationPath();

    static QString soundPath();
    static QString iconPath();

    static QString loginName();
    static QString hostname();

    static QString getEnvironmentVariable(QRegExp regExp);

    static void setPacketNo(qint64 n);
    static QString packetNoString();
    static qint64 packetNo();

    static void setInternalLogFileName(QString filePath);
    static void writeInternalLog(QString line);

    static QString openUrlProgram();

    static QString fileCountString(int fileCount);
    static QString sizeStringUnit(double size = 0.0, QString sep = "");
    static QString secondStringUnit(int second);

private:
    static void openInternalLogFile();

    static QString m_appPath;
    static QString m_iniPath;
    static qint64 m_packetNo;
    static QFile m_internalLogFile;
    static QString m_internalLogFileName;
};

#endif // !HELPER_H
