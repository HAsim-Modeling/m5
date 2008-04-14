/*
 * Copyright (c) 2002, 2003, 2004, 2005
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
 */


#include <iostream>
#include <list>
#include <string>
#include <sstream>

#include "base/cprintf.hh"

using namespace std;

volatile int stop = false;

void
handle_alarm(int signal)
{
    stop = true;
}

void
do_test(int seconds)
{
    stop = false;
    alarm(seconds);
}

int
main()
{
    stringstream result;
    int iterations = 0;

    signal(SIGALRM, handle_alarm);

    do_test(10);
    while (!stop) {
        stringstream result;
        ccprintf(result,
                 "this is a %s of %d iterations %3.2f %#x\n",
                 "test", iterations, 51.934, &result);

        iterations += 1;
    }

    cprintf("completed %d iterations of ccprintf in 10s, %f iterations/s\n",
            iterations, iterations / 10.0);

    do_test(10);
    while (!stop) {
        char result[1024];
        sprintf(result,
                 "this is a %s of %d iterations %3.2f %#x\n",
                 "test", iterations, 51.934, &result);

        iterations += 1;
    }

    cprintf("completed %d iterations of sprintf in 10s, %f iterations/s\n",
            iterations, iterations / 10.0);

    return 0;
}
