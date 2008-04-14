/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005
 * The Regents of The University of Michigan
 * All Rights Reserved
 *
 * This code is part of the M5 simulator.
 *
 * Permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any
 * purpose, so long as the copyright notice above, this grant of
 * permission, and the disclaimer below appear in all copies made; and
 * so long as the name of The University of Michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
 * UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
 * WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
 * LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGES.
 *
 * Authors: Nathan L. Binkert
 *          Ali G. Saidi
 */


/* @file
 * Implements the user interface to a serial console
 */

#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "base/misc.hh"
#include "base/output.hh"
#include "base/socket.hh"
#include "base/trace.hh"
#include "dev/platform.hh"
#include "dev/simconsole.hh"
#include "dev/uart.hh"

using namespace std;


/*
 * Poll event for the listen socket
 */
SimConsole::ListenEvent::ListenEvent(SimConsole *c, int fd, int e)
    : PollEvent(fd, e), cons(c)
{
}

void
SimConsole::ListenEvent::process(int revent)
{
    cons->accept();
}

/*
 * Poll event for the data socket
 */
SimConsole::DataEvent::DataEvent(SimConsole *c, int fd, int e)
    : PollEvent(fd, e), cons(c)
{
}

void
SimConsole::DataEvent::process(int revent)
{
    if (revent & POLLIN)
        cons->data();
    else if (revent & POLLNVAL)
        cons->detach();
}

/*
 * SimConsole code
 */
SimConsole::SimConsole(const Params *p)
    : SimObject(p), listenEvent(NULL), dataEvent(NULL), number(p->number),
      data_fd(-1), txbuf(16384), rxbuf(16384), outfile(NULL)
#if TRACING_ON == 1
      , linebuf(16384)
#endif
{
    if (!p->output.empty()) {
        if (p->append_name)
            outfile = simout.find(p->output + "." + p->name);
        else
            outfile = simout.find(p->output);

        outfile->setf(ios::unitbuf);
    }

    if (p->port)
        listen(p->port);
}

SimConsole::~SimConsole()
{
    if (data_fd != -1)
        ::close(data_fd);

    if (listenEvent)
        delete listenEvent;

    if (dataEvent)
        delete dataEvent;
}

///////////////////////////////////////////////////////////////////////
// socket creation and console attach
//

void
SimConsole::listen(int port)
{
    while (!listener.listen(port, true)) {
        DPRINTF(Console,
                ": can't bind address console port %d inuse PID %d\n",
                port, getpid());
        port++;
    }

    int p1, p2;
    p2 = name().rfind('.') - 1;
    p1 = name().rfind('.', p2);
    ccprintf(cerr, "Listening for %s connection on port %d\n",
            name().substr(p1+1,p2-p1), port);

    listenEvent = new ListenEvent(this, listener.getfd(), POLLIN);
    pollQueue.schedule(listenEvent);
}

void
SimConsole::accept()
{
    if (!listener.islistening())
        panic("%s: cannot accept a connection if not listening!", name());

    int fd = listener.accept(true);
    if (data_fd != -1) {
        char message[] = "console already attached!\n";
        ::write(fd, message, sizeof(message));
        ::close(fd);
        return;
    }

    data_fd = fd;
    dataEvent = new DataEvent(this, data_fd, POLLIN);
    pollQueue.schedule(dataEvent);

    stringstream stream;
    ccprintf(stream, "==== m5 slave console: Console %d ====", number);

    // we need an actual carriage return followed by a newline for the
    // terminal
    stream << "\r\n";

    write((const uint8_t *)stream.str().c_str(), stream.str().size());

    DPRINTFN("attach console %d\n", number);

    txbuf.readall(data_fd);
}

void
SimConsole::detach()
{
    if (data_fd != -1) {
        ::close(data_fd);
        data_fd = -1;
    }

    pollQueue.remove(dataEvent);
    delete dataEvent;
    dataEvent = NULL;

    DPRINTFN("detach console %d\n", number);
}

void
SimConsole::data()
{
    uint8_t buf[1024];
    int len;

    len = read(buf, sizeof(buf));
    if (len) {
        rxbuf.write((char *)buf, len);
        // Inform the UART there is data available
        uart->dataAvailable();
    }
}

size_t
SimConsole::read(uint8_t *buf, size_t len)
{
    if (data_fd < 0)
        panic("Console not properly attached.\n");

    size_t ret;
    do {
      ret = ::read(data_fd, buf, len);
    } while (ret == -1 && errno == EINTR);


    if (ret < 0)
        DPRINTFN("Read failed.\n");

    if (ret <= 0) {
        detach();
        return 0;
    }

    return ret;
}

// Console output.
size_t
SimConsole::write(const uint8_t *buf, size_t len)
{
    if (data_fd < 0)
        panic("Console not properly attached.\n");

    size_t ret;
    for (;;) {
      ret = ::write(data_fd, buf, len);

      if (ret >= 0)
        break;

      if (errno != EINTR)
      detach();
    }

    return ret;
}

#define MORE_PENDING (ULL(1) << 61)
#define RECEIVE_SUCCESS (ULL(0) << 62)
#define RECEIVE_NONE (ULL(2) << 62)
#define RECEIVE_ERROR (ULL(3) << 62)

uint8_t
SimConsole::in()
{
    bool empty;
    uint8_t c;

    empty = rxbuf.empty();
    assert(!empty);
    rxbuf.read((char *)&c, 1);
    empty = rxbuf.empty();


    DPRINTF(ConsoleVerbose, "in: \'%c\' %#02x more: %d\n",
            isprint(c) ? c : ' ', c, !empty);

    return c;
}

uint64_t
SimConsole::console_in()
{
    uint64_t value;

    if (dataAvailable()) {
        value = RECEIVE_SUCCESS | in();
        if (!rxbuf.empty())
            value  |= MORE_PENDING;
    } else {
        value = RECEIVE_NONE;
    }

    DPRINTF(ConsoleVerbose, "console_in: return: %#x\n", value);

    return value;
}

void
SimConsole::out(char c)
{
#if TRACING_ON == 1
    if (DTRACE(Console)) {
        static char last = '\0';

        if (c != '\n' && c != '\r' ||
            last != '\n' && last != '\r') {
            if (c == '\n' || c == '\r') {
                int size = linebuf.size();
                char *buffer = new char[size + 1];
                linebuf.read(buffer, size);
                buffer[size] = '\0';
                DPRINTF(Console, "%s\n", buffer);
                delete [] buffer;
            } else {
                linebuf.write(c);
            }
        }

        last = c;
    }
#endif

    txbuf.write(c);

    if (data_fd >= 0)
        write(c);

    if (outfile)
        outfile->write(&c, 1);

    DPRINTF(ConsoleVerbose, "out: \'%c\' %#02x\n",
            isprint(c) ? c : ' ', (int)c);

}

SimConsole *
SimConsoleParams::create()
{
    return new SimConsole(this);
}
