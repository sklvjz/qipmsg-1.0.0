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

#ifndef RECV_FILE_HANDLE_H
#define RECV_FILE_HANDLE_H

#include "recv_file.h"

#include <QtGlobal>
#include <stdexcept>


// Use counted handle class for the msg_base hierarchy
class RecvFileHandle
{
public:
    // default constructor: unbound handle
    RecvFileHandle() : p(0), use(new qint32(1)) {}
    // attach a handle to a copy of the RecvFile object
    RecvFileHandle(const RecvFile&);
    // copy control members to manage the use count and pointers
    RecvFileHandle(const RecvFileHandle &h): p(h.p), use(h.use) { ++*use; }

    ~RecvFileHandle() { decr_use(); }
    RecvFileHandle& operator=(const RecvFileHandle&);
    // member access operators
    const RecvFile *operator->() const {
        if (p) {
            return p;
        } else {
            throw std::logic_error("unbound RecvFileHandle");
        }
    }
    RecvFile *operator->() {
        if (p) {
            return p;
        } else {
            throw std::logic_error("unbound RecvFileHandle");
        }
    }
    const RecvFile &operator*() const {
        if (p) {
            return *p;
        } else {
            throw std::logic_error("unbound RecvFileHandle");
        }
    }

private:
    RecvFile *p;             // pointer to shared item
    qint32 *use;       // pointer to shared use count

    // called by both destructor and assignment operator for free pointers
    void decr_use() {
        if (--*use == 0) {
            delete p; delete use;
        }
    }
};

#endif // !RECV_FILE_HANDLE_H

