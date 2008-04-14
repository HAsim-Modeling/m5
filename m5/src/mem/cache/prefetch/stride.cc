/*
 * Copyright (c) 2005
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
 * Authors: Ronald G. Dreslinski Jr
 *          Steven K. Reinhardt
 */

/**
 * @file
 * Stride Prefetcher template instantiations.
 */

#include "mem/cache/prefetch/stride.hh"

void
StridePrefetcher::calculatePrefetch(PacketPtr &pkt, std::list<Addr> &addresses,
                                    std::list<Tick> &delays)
{
//	Addr blkAddr = pkt->paddr & ~(Addr)(this->blkSize-1);
    int cpuID = pkt->req->getCpuNum();
    if (!useCPUId) cpuID = 0;

    /* Scan Table for IAddr Match */
/*	std::list<strideEntry*>::iterator iter;
  for (iter=table[cpuID].begin();
  iter !=table[cpuID].end();
  iter++) {
  if ((*iter)->IAddr == pkt->pc) break;
  }

  if (iter != table[cpuID].end()) {
  //Hit in table

  int newStride = blkAddr - (*iter)->MAddr;
  if (newStride == (*iter)->stride) {
  (*iter)->confidence++;
  }
  else {
  (*iter)->stride = newStride;
  (*iter)->confidence--;
  }

  (*iter)->MAddr = blkAddr;

  for (int d=1; d <= degree; d++) {
  Addr newAddr = blkAddr + d * newStride;
  if (this->pageStop &&
  (blkAddr & ~(TheISA::VMPageSize - 1)) !=
  (newAddr & ~(TheISA::VMPageSize - 1)))
  {
  //Spanned the page, so now stop
  this->pfSpanPage += degree - d + 1;
  return;
  }
  else
  {
  addresses.push_back(newAddr);
  delays.push_back(latency);
  }
  }
  }
  else {
  //Miss in table
  //Find lowest confidence and replace

  }
*/
}
