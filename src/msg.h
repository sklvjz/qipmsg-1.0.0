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

#ifndef MSG_H
#define MSG_H

#include "msg_base.h"

#include <QtGlobal>
#include <stdexcept>


// Use counted handle class for the msg_base hierarchy
class Msg
{
public:
    // default constructor: unbound handle
    Msg() : p(0), use(new qint32(1)) {}
    // attach a handle to a copy of the MsgBase object
    Msg(const MsgBase&);
    // copy control members to manage the use count and pointers
    Msg(const Msg &m): p(m.p), use(m.use) { ++*use; }

    ~Msg() { decr_use(); }
    Msg& operator=(const Msg&);
    // member access operators
    const MsgBase *operator->() const {
        if (p) {
            return p;
        } else {
            throw std::logic_error("unbound Msg");
        }
    }
    MsgBase *operator->() {
        if (p) {
            return p;
        } else {
            throw std::logic_error("unbound Msg");
        }
    }
    const MsgBase &operator*() const {
        if (p) {
            return *p;
        } else {
            throw std::logic_error("unbound Msg");
        }
    }

private:
    MsgBase *p;             // pointer to shared item
    qint32 *use;       // pointer to shared use count

    // called by both destructor and assignment operator for free pointers
    void decr_use() {
        if (--*use == 0) {
            delete p; delete use;
        }
    }
};

#endif // !MSG_H

