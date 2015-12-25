/* Copyright (c) 2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include "DebugConnector.h"

#include <stdio.h>
#include <string>
#include <algorithm>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>

#define MSGTYPE_SYSDISCOVER  0
#define MSGTYPE_SYSENUMERATE 1
#define MSGTYPE_SYSSTART     2
#define MSGTYPE_SYSRESET     3
#define MSGTYPE_CPUSTALL     4
#define MSGTYPE_MODSEND      5
#define MSGTYPE_MODREAD      6
#define MSGTYPE_TRACE        7

#define TRACETYPE_ITM 0
#define TRACETYPE_STM 1

DebugConnector::DebugConnector(sc_module_name nm, uint16_t systemid) :
        sc_module(nm), m_systemid(systemid), m_connectionfd(-1), m_port(22000)
{
	SC_THREAD(connection);
}

void DebugConnector::registerDebugModule(DebugModule *mod)
{
    m_debugModules.push_back(mod);
}

bool DebugConnector::sendTrace(DebugModule *mod, TracePacket &packet)
{
    size_t bufsize = packet.getRawPacketSize() + 6;
    uint16_t mod_address = mod->getAddress();
    uint8_t *buffer = (uint8_t*) calloc(bufsize, sizeof(uint8_t));
    buffer[0] = bufsize;
    buffer[1] = MSGTYPE_TRACE;
    buffer[2] = mod->getType();
    buffer[3] = 0; // unused
    buffer[4] = mod_address >> 8;
    buffer[5] = mod_address & 0xff;

    const uint8_t* data = packet.getRawPacket();
    for (int i = 0; i < packet.getRawPacketSize(); i++) {
        buffer[i+6] = data[i];
    }
    int rv = write(m_connectionfd, buffer, bufsize);
    free(buffer);

    if (rv == -1) {
        cerr << "An error occurred during sending: " << strerror(errno);
        return false;
    } else if (rv != bufsize) {
        cerr << "Only " << rv << " bytes written, " << bufsize << " bytes "
                "expected." << endl;
        return false;
    }
    return true;
}

void DebugConnector::connection()
{
    int rv;
    stop();

    int m_listenfd;
    struct sockaddr_in m_servaddr, m_cliaddr;

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);

    int on = 1;
    rv = setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&m_servaddr, sizeof(m_servaddr));
    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_servaddr.sin_port = htons(m_port);

    if (bind(m_listenfd, (sockaddr *) &m_servaddr, sizeof(m_servaddr)) == -1) {
        cerr << "Cannot bind" << endl;
        return;
    }

    while (true) {
        if (listen(m_listenfd, 1) == -1) {
            cerr << "Listen error" << endl;
            return;
        }

        cout << "Listening on port " << m_port << endl;

        socklen_t clilen = sizeof(m_cliaddr);
        m_connectionfd = accept(m_listenfd, (struct sockaddr *) &m_cliaddr,
                                &clilen);
        if (m_connectionfd < 0) {
            cerr << "Error on accept" << endl;
            return;
        }

        cout << "Connected" << endl;

        while (1) {
            char headerbuf[2];

            rv = recv(m_connectionfd, headerbuf, 2, MSG_DONTWAIT);
            if (rv == -1) {
                wait(1, SC_US);
                continue;
            } else if (rv == 0) {
                cout << sc_time_stamp() << " Connection closed or error (rv="
                     << rv << ")" << endl;
                break;
            }

            int size = headerbuf[0];
            int type = headerbuf[1];

            uint8_t *payload;
            unsigned int paylen = size - 2;
            payload = (uint8_t*) malloc(paylen);
            assert(read(m_connectionfd, payload, paylen) == paylen);
            handleMessage(type, payload, paylen);

            free(payload);
        }
        close(m_connectionfd);

        stop();
    }
}

void DebugConnector::handleMessage(int type, uint8_t *payload,
                                   unsigned int paylen)
{
    uint8_t *buf;
    uint16_t *buf_short;
    int rv;

    switch (type) {
    case MSGTYPE_SYSDISCOVER:
        buf = (uint8_t*) malloc(8);
        buf[0] = 8; // size
        buf[1] = MSGTYPE_SYSDISCOVER; // type
        buf[2] = 0xde; // version (hi)
        buf[3] = 0xad; // version (lo)
        buf[4] = ((uint16_t) m_debugModules.size()) >> 8; // num of debug modules (hi)
        buf[5] = ((uint16_t) m_debugModules.size()) & 0xff; // num of debug modules (lo)
        buf[6] = m_systemid >> 8;   // system id (hi)
        buf[7] = m_systemid & 0xff; // system id (lo)
        rv = write(m_connectionfd, buf, 8);
        free(buf);
        if (rv != 8) {
            cerr << "handleMessage(type = MSGTYPE_SYSDISCOVER): Only " << rv
                 << "Bytes written, 8 expected." << endl;
        }
        break;
    case MSGTYPE_SYSENUMERATE:
        // do system enumeration
        buf = (uint8_t*) malloc(6 * m_debugModules.size() + 2);
        buf[0] = 6 * m_debugModules.size() + 2;
        buf[1] = MSGTYPE_SYSENUMERATE;
        buf_short = (uint16_t*) &buf[2];
        int i;
        for (i = 0; i < m_debugModules.size(); i++) {
            buf_short[i*3+0] = i;
            buf_short[i*3+1] = m_debugModules[i]->getType();
            buf_short[i*3+2] = m_debugModules[i]->getVersion();
        }
        rv = write(m_connectionfd, buf, 6 * m_debugModules.size() + 2);
        free(buf);
        if (rv != 6 * m_debugModules.size() + 2) {
            cerr << "handleMessage(type = MSGTYPE_SYSENUMERATE): Only " << rv
                 << "Bytes written, " << (6 * m_debugModules.size() + 2)
                 << " expected." << endl;
        }
        break;
    case MSGTYPE_SYSSTART:
        start();
        buf = (uint8_t*) malloc(2);
        buf[0] = 2;
        buf[1] = MSGTYPE_SYSSTART;
        rv = write(m_connectionfd, buf, 2);
        free(buf);
        if (rv != 2) {
            cerr << "handleMessage(type = MSGTYPE_SYSSTART): Only " << rv
                 << "Bytes written, 2 expected." << endl;
        }
        break;
    case MSGTYPE_SYSRESET:
        // do a reset
        resetSystem();
        break;
    case MSGTYPE_MODSEND:
        uint16_t smaddress;
        DebugModule *smod;
        uint16_t saddress;
        uint16_t ssize;
        uint16_t sresponse;
        smaddress = ((payload[0] << 8) & 0xff00) | (payload[1] = 0xff);
        if (smaddress >= m_debugModules.size()) {
            cerr << "module send for invalid module address" << endl;
            break;
        }
        smod = m_debugModules[smaddress];
        saddress = ((payload[2] << 8) & 0xff00) | (payload[3] & 0xff);
        ssize = paylen - 4;
        sresponse = smod->write(saddress, ssize, (char*) &payload[4]);
        buf = (uint8_t*) malloc(4);
        buf[0] = 4;
        buf[1] = MSGTYPE_MODSEND;
        buf[2] = sresponse >> 8;
        buf[3] = sresponse & 0xff;
        rv = write(m_connectionfd, buf, 4);
        free(buf);
        if (rv != 4) {
            cerr << "handleMessage(type = MSGTYPE_MODSEND): Only " << rv
                 << "Bytes written, 4 expected." << endl;
        }
        break;
    default:
        cout << "unhandled debug request" << endl;
        break;
    }
}
