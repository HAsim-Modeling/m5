/*
 * Copyright (c) 2003, 2004, 2005
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
 * Authors: Erik G. Hallnor
 *          Nathan L. Binkert
 */

#ifndef __BASE_COMPRESSION_BASE_HH__
#define __BASE_COMPRESSION_BASE_HH__

/**
 * @file
 * This file defines a base (abstract virtual) compression algorithm object.
 */

#include <inttypes.h>

/**
 * Abstract virtual compression algorithm object.
 */
class CompressionAlgorithm
{
  public:
    virtual ~CompressionAlgorithm() {}

    /**
     * Uncompress the data, causes a fatal since no data should be compressed.
     * @param dest The output buffer.
     * @param src  The compressed data.
     * @param size The number of bytes in src.
     *
     * @retval The size of the uncompressed data.
     */
    virtual int uncompress(uint8_t * dest, uint8_t *src, int size) = 0;

    /**
     * Compress the data, just returns the source data.
     * @param dest The output buffer.
     * @param src  The data to be compressed.
     * @param size The number of bytes in src.
     *
     * @retval The size of the compressed data.
     */
    virtual int compress(uint8_t *dest, uint8_t *src, int size) = 0;
};

#endif //__BASE_COMPRESSION_BASE_HH__
