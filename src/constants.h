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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "ipmsg.h"

#define COMPANY                 ""
#define PROGRAM                 "qipmsg"

#define MSG_VERSION_POS             0
#define MSG_PACKET_NO_POS           1
#define MSG_LOG_NAME_POS            2
#define MSG_HOST_POS                3
#define MSG_FLAGS_POS               4
#define MSG_ADDITION_INFO_POS       5
#define MSG_EXTENDED_INFO_POS       6

#define MSG_NORMAL_FIELD_COUNT      6
#define SEND_MSG_PROCESS_INTERVAL   200
#define MAX_RE_SEND_TIMES           8

#define COMMAND_SEPERATOR       ':'
#define EXTEND_INFO_SEPERATOR   '\0'
#define FILE_NAME_ESCAPE         "\a\a"
#define FILE_NAME_BEFORE        "::"
#define FILE_NAME_AFTER         ":"

// format for attach file
#define RECV_FILE_ID_POS            0
#define RECV_FILE_NAME_POS          1
#define RECV_FILE_SIZE_POS          2
#define RECV_FILE_MTIME_POS         3
#define RECV_FILE_ATTR_POS          4
#define RECV_FILE_EXTEND_ATTR_POS   5

#define RECV_FILE_VIEW_ID_POS               0
#define RECV_FILE_VIEW_NAME_POS             1
#define RECV_FILE_VIEW_SIZE_POS             2

#define RECV_FILE_REGULAR       "1"
#define RECV_FILE_DIR           "2"
#define RECV_FILE_RETPARENT     "3"

// Send File View Position
#define SEND_FILE_VIEW_NAME_POS            0
#define SEND_FILE_VIEW_SIZE_POS            1
#define SEND_FILE_VIEW_LOCATION_POS        2
#define SEND_FILE_VIEW_PATH_POS            3

#define USER_VIEW_NAME_COLUMN              0
#define USER_VIEW_GROUP_COLUMN             1
#define USER_VIEW_HOST_COLUMN              2
#define USER_VIEW_IP_COLUMN                3
#define USER_VIEW_LOGIN_NAME_COLUMN        4
#define USER_VIEW_DISPLAY_LEVEL_COLUMN     5

#define TRANSFER_FILE_VIEW_NO_COLUMN       0
#define TRANSFER_FILE_VIEW_FILE_COLUMN     1
#define TRANSFER_FILE_VIEW_SIZE_COLUMN     2
#define TRANSFER_FILE_VIEW_STATS_COLUMN    3
#define TRANSFER_FILE_VIEW_USER_COLUMN     4
#define TRANSFER_FILE_VIEW_KEY_COLUMN      5

// Support file attachment and encrypt
// #define QIPMSG_CAPACITY         IPMSG_FILEATTACHOPT | IPMSG_ENCRYPTOPT
#define QIPMSG_CAPACITY         IPMSG_FILEATTACHOPT

// Response file packet
#define TRANSFER_FILE_HEADER_SIZE_LENGTH      4
#define TRANSFER_FILE_FILE_SIZE_LENGTH        8

#define ONE_KB     (1024.0)
#define ONE_MB     (1024*1024.0)

#endif // !CONSTANTS_H

