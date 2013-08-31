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

#include "search_user_dialog.h"
#include "global.h"
#include "preferences.h"

#include <QtGui>
#include <QtCore>

#define USER_SEARCH_HISTORY_COUNT   50

SearchUserDialog::SearchUserDialog(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowModality(Qt::WindowModal);

    createSearchBox();
    createButtonBox();
    createConnections();

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(searchCombox, 0, 0);
    mainLayout->addWidget(searchButton, 0, 1);
    mainLayout->addWidget(allColumnCheckBox, 1, 0);
    mainLayout->addWidget(closeButton, 1, 1);

    setLayout(mainLayout);

    adjustSize();
}

QSize SearchUserDialog::sizeHint()
{
    return QSize(240, 180);
}

void SearchUserDialog::createSearchBox()
{
    searchCombox = new QComboBox();
    searchCombox->setEditable(true);
    searchCombox->setDuplicatesEnabled(false);
    QCompleter *completer
        = new QCompleter(Global::preferences->userSearchList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    searchCombox->setCompleter(completer);

    searchCombox->addItems(Global::preferences->userSearchList);

    allColumnCheckBox = new QCheckBox(tr("Search all columns"));
}

void SearchUserDialog::createButtonBox()
{
    searchButton = new QPushButton(tr("Search"));
    closeButton = new QPushButton(tr("Close"));
}

void SearchUserDialog::createConnections()
{
    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(searchButton, SIGNAL(clicked()),
            this, SLOT(search()));
}

void SearchUserDialog::search()
{
    if (searchCombox->currentText().isEmpty()) {
        return;
    }

    if (!Global::preferences->userSearchList
        .contains(searchCombox->currentText())) {
        Global::preferences->userSearchList
            .prepend(searchCombox->currentText());
        searchCombox->insertItem(0, searchCombox->currentText());
        // update completer
        QCompleter *completer
            = new QCompleter(Global::preferences->userSearchList, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        searchCombox->setCompleter(completer);
    }
    if (Global::preferences->userSearchList.count()
        > USER_SEARCH_HISTORY_COUNT) {
        Global::preferences->userSearchList.removeLast();
    }

    Global::preferences->isSearchAllColumns = allColumnCheckBox->isChecked();

    emit searchUser(searchCombox->currentText());
}

