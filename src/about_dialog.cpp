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

#include "about_dialog.h"
#include "constants.h"
#include "version.h"
#include "global.h"

#include <QtCore>
#include <QtGui>

static QString contr(const QString &author, const QString &thing);
static QString trad(const QString &lang, const QString &author);
static QString link(const QString & url, QString name = "");

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    tabWidget = new QTabWidget;
    tabWidget->addTab(new InfoTab(), tr("Information"));
    tabWidget->addTab(new ContributorTab(), tr("Contrubutors"));
    tabWidget->addTab(new TranslatorTab(), tr("Translators"));
    tabWidget->addTab(new LicenseTab(), tr("License"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setWindowTitle(tr("About QIpMsg"));
    setWindowIcon(*Global::iconSet.value("normal"));

    adjustSize();

    // Put the dialog in the middle of screen
    QRect rect(QApplication::desktop()->screenGeometry());
    move((rect.width() - width()) / 2, (rect.height() - height()) / 2);
}

AboutDialog::~AboutDialog()
{
}

InfoTab::InfoTab(QWidget *parent)
    : QWidget(parent)
{
    QTextBrowser *info = new QTextBrowser;
    info->setOpenExternalLinks(true);
    info->setFrameShape(QFrame::NoFrame);
    info->setFrameShadow(QFrame::Plain);

    info->setText(
            "<b>QIpMsg</b> &copy; 2007-2010 Yichi Zhang &lt;zyichi@gmail.com&gt;<br><br>"
            "<b>" + tr("Version: %1").arg(VERSION) + "</b>" +
            "<br>" +
            tr("Compiled with Qt %1").arg(QT_VERSION_STR) + "<br><br>" +
            tr("Visit our web for updates:") + "<br>" +
            link("http://code.google.com/p/qipmsg") +
            "<br><br>" +
            tr("Join qipmsg group to report a bug or request new features:")
            + "<br>" +
            link("http://groups.google.com/group/qipmsg") +
            "<br><br>"
            );

    QPalette p = info->palette();
    p.setColor(QPalette::Base, palette().color(QPalette::Window));
    info->setPalette(p);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(info);

    setLayout(mainLayout);
}

ContributorTab::ContributorTab(QWidget *parent)
    : QWidget(parent)
{
    QTextBrowser *contributors = new QTextBrowser;
    contributors->setOpenExternalLinks(true);
    contributors->setFrameShape(QFrame::NoFrame);
    contributors->setFrameShadow(QFrame::Plain);

    contributors->setText(getContributor());

    QPalette p = contributors->palette();
    p.setColor(QPalette::Base, palette().color(QPalette::Window));
    contributors->setPalette(p);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(contributors);

    setLayout(mainLayout);
}

QString ContributorTab::getContributor() const
{
    return tr("Some people have helped with their contributions:")
        + "<ul>"
        + contr("Yichi Zhang &lt;zyichi@gmail.com&gt;", tr("Original Author, Packager"))
        + "</ul>"
        + "<ul>"
        + contr("robinlee.sysu &lt;robinlee.sysu@gmail.com&gt;", tr("Packager"))
        + "</ul>"
        + "<ul>"
        + contr("Daijun Cao &lt;huabo2008@gmail.com&gt;", tr("Send file bug fix"))
        + "</ul>"
        ;
}

TranslatorTab::TranslatorTab(QWidget *parent)
    : QWidget(parent)
{
    QTextBrowser *translators = new QTextBrowser;
    translators->setOpenExternalLinks(true);
    translators->setFrameShape(QFrame::NoFrame);
    translators->setFrameShadow(QFrame::Plain);

    translators->setText(getTranslator());

    QPalette p = translators->palette();
    p.setColor(QPalette::Base, palette().color(QPalette::Window));
    translators->setPalette(p);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(translators);

    setLayout(mainLayout);
}

LicenseTab::LicenseTab(QWidget *parent)
    : QWidget(parent)
{
    QTextBrowser *license = new QTextBrowser;
    license->setOpenExternalLinks(true);
    license->setFrameShape(QFrame::NoFrame);
    license->setFrameShadow(QFrame::Plain);

    QString gpl = QString(tr(
    "QIpMsg is free software: you can redistribute it and/or modify"
    "it under the terms of the GNU General Public License as published by"
    "the Free Software Foundation, either version 3 of the License, or"
    "(at your option) any later version."
    "\n\n"
    "QIpMsg is distributed in the hope that it will be useful,"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
    "GNU General Public License for more details."
    "\n\n"
    "QIpMsg is distributed in the hope that it will be useful,"
    "You should have received a copy of the GNU General Public License"
    "along with QIpMsg.  If not, see <http://www.gnu.org/licenses>."
    ));

    license->setText(gpl);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(license);

    setLayout(mainLayout);
}

QString TranslatorTab::getTranslator() const
{
    return QString(
            tr("The following people have contributed with translations:")
            + "<ul>"
            + trad(tr("Simplified-Chinese"), "Yichi Zhang &lt;zyichi@gmail.com&gt;")
            + "</ul>"
            );
}

static QString trad(const QString &lang, const QString &author)
{
    return "<li>"+ QObject::tr("<b>%1</b>: %2").arg(lang).arg(author) + "</li>";
}

static QString contr(const QString &author, const QString &thing)
{
    return "<li>"+ QObject::tr("<b>%1</b> (%2)").arg(author).arg(thing) +"</li>";
}

static QString link(const QString & url, QString name)
{
    if (name.isEmpty()) name = url;
    return QString("<a href=\"" + url + "\">" + name +"</a>");
}

QSize AboutDialog::sizeHint() const
{
    return QSize(480, 360);
}

