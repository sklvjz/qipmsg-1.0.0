#!/bin/bash

QTDIR=\
" /usr/include/qt4/Qt/"\
" /usr/include/qt4/QtCore/"\
" /usr/include/qt4/Qt3Support/"\
" /usr/include/qt4/QtAssistant/"\
" /usr/include/qt4/QtDBus/"\
" /usr/include/qt4/QtDesigner/"\
" /usr/include/qt4/QtGui/"\
" /usr/include/qt4/QtNetwork/"\
" /usr/include/qt4/QtOpenGL/"\
" /usr/include/qt4/QtScript/"\
" /usr/include/qt4/QtSql/"\
" /usr/include/qt4/QtSvg/"\
" /usr/include/qt4/QtTest/"\
" /usr/include/qt4/QtUiTools/"\
" /usr/include/qt4/QtXml/"

echo $QTDIR

for dir in $QTDIR
do
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q -a -f qt4-tags $dir
done

