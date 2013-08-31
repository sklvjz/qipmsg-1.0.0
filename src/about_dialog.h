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

#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QDialog>
#include <QString>

class QWidget;
class QTabWidget;
class QDialogButtonBox;

class InfoTab : public QWidget
{
    Q_OBJECT

public:
    InfoTab(QWidget *parent = 0);
};

class ContributorTab : public QWidget
{
    Q_OBJECT

public:
    ContributorTab(QWidget *parent = 0);

private:
    QString getContributor() const;
};

class TranslatorTab : public QWidget
{
    Q_OBJECT

public:
    TranslatorTab(QWidget *parent = 0);

private:
    QString getTranslator() const;
};

class LicenseTab : public QWidget
{
    Q_OBJECT

public:
    LicenseTab(QWidget *parent = 0);
};

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = 0);
    virtual ~AboutDialog();

    virtual QSize sizeHint() const;

private:
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
};

#endif // !ABOUT_DIALOG_H

