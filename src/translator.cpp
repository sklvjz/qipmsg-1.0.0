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

#include <QApplication>
#include <QLocale>

#include "translator.h"
#include "helper.h"
#include "constants.h"

Q_GLOBAL_STATIC(Translator, translator);

Translator::Translator()
{
    qApp->installTranslator(&appTrans_);
    qApp->installTranslator(&qtTrans_);
}

Translator* Translator::instance()
{
    return translator();
}

bool Translator::loadCatalog(QTranslator &t, QString name, QString locale,
                      QString dir)
{
    QString s = name + "_" + locale;
    bool r = t.load(s, dir);
    if (r) {
        qDebug("Translator::loadCatalog: successfully load %s from %s",
               s.toUtf8().data(), dir.toUtf8().data());
    } else {
        qDebug("Translator::loadCatalog: can't load %s from %s",
               s.toUtf8().data(), dir.toUtf8().data());
    }

    return r;
}

void Translator::load(QString locale)
{
    if (locale.isEmpty()) {
        locale = QLocale::system().name();
    }

    QString appTransPath = Helper::translationPath();
    QString qtTransPath = Helper::qtTranslationPath();

#ifdef Q_OS_WIN
    // In windows try to load the qt translation from the app path, as
    // most users won't have Qt installed.
    loadCatalog(qtTrans_, "qt", locale, appTransPath );
#else
    // In linux try to load it first from app path (in case there's an updated
    // translation), if it fails it will try then from the Qt path.
    if (! loadCatalog(qtTrans_, "qt", locale, appTransPath ) ) {
        loadCatalog(qtTrans_, "qt", locale, qtTransPath);
    }
#endif
    loadCatalog(appTrans_, PROGRAM, locale, appTransPath);
}

