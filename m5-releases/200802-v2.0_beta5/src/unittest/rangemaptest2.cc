/*
 * Copyright (c) 2006
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
 * Authors: Ali G. Saidi
 */

#include <iostream>
#include <cassert>
#include "sim/host.hh"
#include "base/range_map.hh"

using namespace std;

int main()
{
    range_multimap<Addr,int> r;

    range_multimap<Addr,int>::iterator i;
    std::pair<range_multimap<Addr,int>::iterator,range_multimap<Addr,int>::iterator>
        jk;

    i = r.insert(RangeIn<Addr>(10,40),5);
    assert(i != r.end());
    i = r.insert(RangeIn<Addr>(10,40),6);
    assert(i != r.end());
    i = r.insert(RangeIn<Addr>(60,90),3);
    assert(i != r.end());

    jk = r.find(RangeIn(20,30));
    assert(jk.first != r.end());
    cout << jk.first->first << " " << jk.first->second << endl;
    cout << jk.second->first << " " << jk.second->second << endl;

    i = r.insert(RangeIn<Addr>(0,3),5);
    assert(i != r.end());

    for( i = r.begin(); i != r.end(); i++)
        cout << i->first << " " << i->second << endl;

    jk = r.find(RangeIn(20,30));
    assert(jk.first != r.end());
    cout << jk.first->first << " " << jk.first->second << endl;
    cout << jk.second->first << " " << jk.second->second << endl;


}







