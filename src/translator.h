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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QTranslator>

class Translator
{
public:
    Translator();
    static Translator* instance();

    void load(QString locale);

private:
    static bool loadCatalog(QTranslator &t, QString name, QString locale,
                            QString dir);

    QTranslator appTrans_;
    QTranslator qtTrans_;
};

#endif // !TRANSLATOR_H
