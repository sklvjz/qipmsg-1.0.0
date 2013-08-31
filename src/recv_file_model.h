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

#ifndef RECV_FILE_MODEL_H
#define RECV_FILE_MODEL_H

#include <QHostAddress>
#include <QStandardItemModel>

class RecvFileWindow;
class RecvFile;

class RecvFileModel
{
public:
    friend class RecvFileWindow;

    RecvFileModel(): m_model(0) { createModel(); }
    ~RecvFileModel() { delete m_model; }

    void addRow(const RecvFile &recvFile);
    void removeRow(int id);

    QVariant data(const QModelIndex &i) { return m_model->data(i); }

    int rowCount() const { return m_model->rowCount(); }

private:
    void createModel();

    QStandardItemModel *m_model;
};

#endif // !RECV_FILE_MODEL_H

