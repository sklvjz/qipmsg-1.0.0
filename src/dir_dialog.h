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

#ifndef DIR_DIALOG_H
#define DIR_DIALOG_H

#include <QDialog>
#include <QFileInfo>

class QDirModel;
class QTreeView;
class QItemSelectionModel;
class QPushButton;
class QWidget;
class QHBoxLayout;
class SendFileModel;

class DirDialog : public QDialog
{
    Q_OBJECT

public:
    DirDialog(SendFileModel *sendFileModel, QWidget *parent = 0);

    virtual ~DirDialog() {}

    virtual QSize sizeHint() const;

signals:
    void dirSelected();

private slots:
    void selectDir();

private:
    void createButtonLayout();
    void createConnections();
    bool isFileExist(QFileInfo fileInfo);
    void expandTree();

    QDirModel *model;
    QTreeView *tree;
    QItemSelectionModel *selectionModel;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QHBoxLayout *buttonsLayout;

    SendFileModel *m_sendFileModel;
};

#endif // !DIR_DIALOG_H
