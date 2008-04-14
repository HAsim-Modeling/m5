/*
 * Copyright (c) 2007 MIPS Technologies, Inc.  All Rights Reserved
 *
 * This software is part of the M5 simulator.
 *
 * THIS IS A LEGAL AGREEMENT.  BY DOWNLOADING, USING, COPYING, CREATING
 * DERIVATIVE WORKS, AND/OR DISTRIBUTING THIS SOFTWARE YOU ARE AGREEING
 * TO THESE TERMS AND CONDITIONS.
 *
 * Permission is granted to use, copy, create derivative works and
 * distribute this software and such derivative works for any purpose,
 * so long as (1) the copyright notice above, this grant of permission,
 * and the disclaimer below appear in all copies and derivative works
 * made, (2) the copyright notice above is augmented as appropriate to
 * reflect the addition of any new copyrightable work in a derivative
 * work (e.g., Copyright (c) <Publication Year> Copyright Owner), and (3)
 * the name of MIPS Technologies, Inc. ($(B!H(BMIPS$(B!I(B) is not used in any
 * advertising or publicity pertaining to the use or distribution of
 * this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED $(B!H(BAS IS.$(B!I(B  MIPS MAKES NO WARRANTIES AND
 * DISCLAIMS ALL WARRANTIES, WHETHER EXPRESS, STATUTORY, IMPLIED OR
 * OTHERWISE, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT OF THIRD PARTY RIGHTS, REGARDING THIS SOFTWARE.
 * IN NO EVENT SHALL MIPS BE LIABLE FOR ANY DAMAGES, INCLUDING DIRECT,
 * INDIRECT, INCIDENTAL, CONSEQUENTIAL, SPECIAL, OR PUNITIVE DAMAGES OF
 * ANY KIND OR NATURE, ARISING OUT OF OR IN CONNECTION WITH THIS AGREEMENT,
 * THIS SOFTWARE AND/OR THE USE OF THIS SOFTWARE, WHETHER SUCH LIABILITY
 * IS ASSERTED ON THE BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE OR
 * STRICT LIABILITY), OR OTHERWISE, EVEN IF MIPS HAS BEEN WARNED OF THE
 * POSSIBILITY OF ANY SUCH LOSS OR DAMAGE IN ADVANCE.
 *
 * Authors: Brett Miller
 */

#include "arch/mips/isa_traits.hh"
#include "arch/mips/dsp.hh"
#include "config/full_system.hh"
#include "cpu/static_inst.hh"
#include "sim/serialize.hh"
#include "base/bitfield.hh"
#include "base/misc.hh"

using namespace MipsISA;
using namespace std;

int32_t
MipsISA::bitrev( int32_t value )
{
    int32_t result = 0;
    int i, shift;

    for( i=0; i<16; i++ )
    {
        shift = 2*i - 15;

        if( shift < 0 )
            result |= (value & 1L<<i) << -shift;
        else
            result |= (value & 1L<<i) >> shift;
    }

    return result;
}

uint64_t
MipsISA::dspSaturate( uint64_t value, int32_t fmt, int32_t sign, uint32_t *overflow )
{
    int64_t svalue;

    svalue = (int64_t)value;

    switch( sign )
    {
      case SIGNED:
        if( svalue > (int64_t)FIXED_SMAX[fmt] )
        {
            *overflow = 1;
            svalue = (int64_t)FIXED_SMAX[fmt];
        }
        else if( svalue < (int64_t)FIXED_SMIN[fmt] )
        {
            *overflow = 1;
            svalue = (int64_t)FIXED_SMIN[fmt];
        }
        break;
      case UNSIGNED:
        if( svalue > (int64_t)FIXED_UMAX[fmt] )
        {
            *overflow = 1;
            svalue = FIXED_UMAX[fmt];
        }
        else if( svalue < (int64_t)FIXED_UMIN[fmt] )
        {
            *overflow = 1;
            svalue = FIXED_UMIN[fmt];
        }
        break;
    }

    return( (uint64_t)svalue );
}

uint64_t
MipsISA::checkOverflow( uint64_t value, int32_t fmt, int32_t sign, uint32_t *overflow )
{
    int64_t svalue;

    svalue = (int64_t)value;

    switch( sign )
    {
      case SIGNED:
        if( svalue > (int64_t)FIXED_SMAX[fmt] || svalue < (int64_t)FIXED_SMIN[fmt] )
            *overflow = 1;
        break;
      case UNSIGNED:
        if( svalue > (int64_t)FIXED_UMAX[fmt] || svalue < (int64_t)FIXED_UMIN[fmt] )
            *overflow = 1;
        break;
    }

    return( (uint64_t)svalue );
}

uint64_t
MipsISA::signExtend( uint64_t value, int32_t fmt )
{
    int32_t signpos = SIMD_NBITS[fmt];
    uint64_t sign = uint64_t(1)<<(signpos-1);
    uint64_t ones = ~(0ULL);

    if( value & sign )
        value |= (ones << signpos); // extend with ones
    else
        value &= (ones >> (64 - signpos)); // extend with zeros

    return value;
}

uint64_t
MipsISA::addHalfLsb( uint64_t value, int32_t lsbpos )
{
    return( value += ULL(1) << (lsbpos-1) );
}

int32_t
MipsISA::dspAbs( int32_t a, int32_t fmt, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    int64_t svalue;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, SIGNED );

    for( i=0; i<nvals; i++ )
    {
        svalue = (int64_t)a_values[i];

        if( a_values[i] == FIXED_SMIN[fmt] )
        {
            a_values[i] = FIXED_SMAX[fmt];
            ouflag = 1;
        }
        else if( svalue < 0 )
        {
            a_values[i] = uint64_t( 0 - svalue );
        }
    }

    simdPack( a_values, &result, fmt );

    if( ouflag )
        writeDSPControl( dspctl, (ouflag<<4)<<DSP_CTL_POS[DSP_OUFLAG], 1<<DSP_OUFLAG);

    return( result );
}

int32_t
MipsISA::dspAdd( int32_t a, int32_t b, int32_t fmt, int32_t saturate, int32_t sign, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<nvals; i++ )
    {
        if( saturate )
            a_values[i] = dspSaturate( a_values[i] + b_values[i], fmt, sign, &ouflag );
        else
            a_values[i] = checkOverflow( a_values[i] + b_values[i], fmt, sign, &ouflag );
    }

    simdPack( a_values, &result, fmt );

    if( ouflag )
        writeDSPControl( dspctl, (ouflag<<4)<<DSP_CTL_POS[DSP_OUFLAG], 1<<DSP_OUFLAG);

    return( result );
}

int32_t
MipsISA::dspAddh( int32_t a, int32_t b, int32_t fmt, int32_t round, int32_t sign )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<nvals; i++ )
    {
        if( round )
            a_values[i] = addHalfLsb( a_values[i] + b_values[i], 1 ) >> 1;
        else
            a_values[i] = ( a_values[i] + b_values[i] ) >> 1;
    }

    simdPack( a_values, &result, fmt );

    return( result );
}

int32_t
MipsISA::dspSub( int32_t a, int32_t b, int32_t fmt, int32_t saturate, int32_t sign, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<nvals; i++ )
    {
        if( saturate )
            a_values[i] = dspSaturate( a_values[i] - b_values[i], fmt, sign, &ouflag );
        else
            a_values[i] = checkOverflow( a_values[i] - b_values[i], fmt, sign, &ouflag );
    }

    simdPack( a_values, &result, fmt );

    if( ouflag )
        writeDSPControl( dspctl, (ouflag<<4)<<DSP_CTL_POS[DSP_OUFLAG], 1<<DSP_OUFLAG);

    return( result );
}

int32_t
MipsISA::dspSubh( int32_t a, int32_t b, int32_t fmt, int32_t round, int32_t sign )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<nvals; i++ )
    {
        if( round )
            a_values[i] = addHalfLsb( a_values[i] - b_values[i], 1 ) >> 1;
        else
            a_values[i] = ( a_values[i] - b_values[i] ) >> 1;
    }

    simdPack( a_values, &result, fmt );

    return( result );
}

int32_t
MipsISA::dspShll( int32_t a, uint32_t sa, int32_t fmt, int32_t saturate, int32_t sign, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];

    sa = bits( sa, SIMD_LOG2N[fmt]-1, 0 );
    simdUnpack( a, a_values, fmt, sign );

    for( i=0; i<nvals; i++ )
    {
        if( saturate )
            a_values[i] = dspSaturate( a_values[i] << sa, fmt, sign, &ouflag );
        else
            a_values[i] = checkOverflow( a_values[i] << sa, fmt, sign, &ouflag );
    }

    simdPack( a_values, &result, fmt );

    if( ouflag )
        writeDSPControl( dspctl, (ouflag<<6)<<DSP_CTL_POS[DSP_OUFLAG], 1<<DSP_OUFLAG);

    return( result );
}

int32_t
MipsISA::dspShrl( int32_t a, uint32_t sa, int32_t fmt, int32_t sign )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint64_t a_values[SIMD_MAX_VALS];

    sa = bits( sa, SIMD_LOG2N[fmt]-1, 0 );

    simdUnpack( a, a_values, fmt, UNSIGNED );

    for( i=0; i<nvals; i++ )
        a_values[i] = a_values[i] >> sa;

    simdPack( a_values, &result, fmt );

    return( result );
}

int32_t
MipsISA::dspShra( int32_t a, uint32_t sa, int32_t fmt, int32_t round, int32_t sign, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint64_t a_values[SIMD_MAX_VALS];

    sa = bits( sa, SIMD_LOG2N[fmt]-1, 0 );

    simdUnpack( a, a_values, fmt, SIGNED );

    for( i=0; i<nvals; i++ )
    {
        if( round )
            a_values[i] = addHalfLsb( a_values[i], sa ) >> sa;
        else
            a_values[i] = a_values[i] >> sa;
    }

    simdPack( a_values, &result, fmt );

    return( result );
}

int32_t
MipsISA::dspMulq( int32_t a, int32_t b, int32_t fmt, int32_t saturate, int32_t round, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int sa = SIMD_NBITS[fmt];
    int32_t result;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    int64_t temp;

    simdUnpack( a, a_values, fmt, SIGNED );
    simdUnpack( b, b_values, fmt, SIGNED );

    for( i=0; i<nvals; i++ )
    {
        if( round )
            temp = (int64_t)addHalfLsb( a_values[i] * b_values[i] << 1, sa ) >> sa;
        else
            temp = (int64_t)(a_values[i] * b_values[i]) >> (sa - 1);

        if( a_values[i] == FIXED_SMIN[fmt] &&
            b_values[i] == FIXED_SMIN[fmt] )
        {
            ouflag = 1;

            if( saturate )
                temp = FIXED_SMAX[fmt];
        }

        a_values[i] = temp;
    }

    simdPack( a_values, &result, fmt );

    if( ouflag )
        writeDSPControl( dspctl, (ouflag<<5)<<DSP_CTL_POS[DSP_OUFLAG], 1<<DSP_OUFLAG);

    return( result );
}

int32_t
MipsISA::dspMul( int32_t a, int32_t b, int32_t fmt, int32_t saturate, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, SIGNED );
    simdUnpack( b, b_values, fmt, SIGNED );

    for( i=0; i<nvals; i++ )
    {
        if( saturate )
            a_values[i] = dspSaturate( a_values[i] * b_values[i], fmt, SIGNED, &ouflag );
        else
            a_values[i] = checkOverflow( a_values[i] * b_values[i], fmt, SIGNED, &ouflag );
    }

    simdPack( a_values, &result, fmt );

    if( ouflag )
        writeDSPControl( dspctl, (ouflag<<5)<<DSP_CTL_POS[DSP_OUFLAG], 1<<DSP_OUFLAG);

    return( result );
}

int32_t
MipsISA::dspMuleu( int32_t a, int32_t b, int32_t mode, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[SIMD_FMT_PH];
    int32_t result;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, SIMD_FMT_QB, UNSIGNED );
    simdUnpack( b, b_values, SIMD_FMT_PH, UNSIGNED );

    switch( mode )
    {
      case MODE_L:
        for( i=0; i<nvals; i++ )
            b_values[i] = dspSaturate( a_values[i+2] * b_values[i], SIMD_FMT_PH, UNSIGNED, &ouflag );
        break;
      case MODE_R:
        for( i=0; i<nvals; i++ )
            b_values[i] = dspSaturate( a_values[i] * b_values[i], SIMD_FMT_PH, UNSIGNED, &ouflag );
        break;
    }

    simdPack( b_values, &result, SIMD_FMT_PH );

    if( ouflag )
        writeDSPControl( dspctl, (ouflag<<5)<<DSP_CTL_POS[DSP_OUFLAG], 1<<DSP_OUFLAG);

    return( result );
}

int32_t
MipsISA::dspMuleq( int32_t a, int32_t b, int32_t mode, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[SIMD_FMT_W];
    int32_t result;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    uint64_t c_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, SIMD_FMT_PH, SIGNED );
    simdUnpack( b, b_values, SIMD_FMT_PH, SIGNED );

    switch( mode )
    {
      case MODE_L:
        for( i=0; i<nvals; i++ )
            c_values[i] = dspSaturate( a_values[i+1] * b_values[i+1] << 1,
                                       SIMD_FMT_W, SIGNED, &ouflag );
        break;
      case MODE_R:
        for( i=0; i<nvals; i++ )
            c_values[i] = dspSaturate( a_values[i] * b_values[i] << 1,
                                       SIMD_FMT_W, SIGNED, &ouflag );
        break;
    }

    simdPack( c_values, &result, SIMD_FMT_W );

    if( ouflag )
        writeDSPControl( dspctl, (ouflag<<5)<<DSP_CTL_POS[DSP_OUFLAG], 1<<DSP_OUFLAG);

    return( result );
}

int64_t
MipsISA::dspDpaq( int64_t dspac, int32_t a, int32_t b, int32_t ac, int32_t infmt,
                  int32_t outfmt, int32_t postsat, int32_t mode, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[infmt];
    int64_t result = 0;
    int64_t temp = 0;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, infmt, SIGNED );
    simdUnpack( b, b_values, infmt, SIGNED );

    for( i=0; i<nvals; i++ )
    {
        switch( mode )
        {
          case MODE_X:
            if( a_values[nvals-1-i] == FIXED_SMIN[infmt] &&
                b_values[i] == FIXED_SMIN[infmt] )
            {
                result += FIXED_SMAX[outfmt];
                ouflag = 1;
            }
            else
                result += a_values[nvals-1-i] * b_values[i] << 1;
            break;
          default:
            if( a_values[i] == FIXED_SMIN[infmt] &&
                b_values[i] == FIXED_SMIN[infmt] )
            {
                result += FIXED_SMAX[outfmt];
                ouflag = 1;
            }
            else
                result += a_values[i] * b_values[i] << 1;
            break;
        }
    }

    if( postsat )
    {
        if( outfmt == SIMD_FMT_L )
        {
            int signa = bits( dspac, 63, 63 );
            int signb = bits( result, 63, 63 );

            temp = dspac + result;

            if( ( signa == signb ) &&
                ( bits( temp, 63, 63 ) != signa ) )
            {
                ouflag = 1;
                if( signa )
                    dspac = FIXED_SMIN[outfmt];
                else
                    dspac = FIXED_SMAX[outfmt];
            }
            else
                dspac = temp;
        }
        else
            dspac = dspSaturate( dspac + result, outfmt, SIGNED, &ouflag );
    }
    else
        dspac += result;

    if( ouflag )
        *dspctl = insertBits( *dspctl, 16+ac, 16+ac, 1 );

    return( dspac );
}

int64_t
MipsISA::dspDpsq( int64_t dspac, int32_t a, int32_t b, int32_t ac, int32_t infmt,
                  int32_t outfmt, int32_t postsat, int32_t mode, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[infmt];
    int64_t result = 0;
    int64_t temp = 0;
    uint32_t ouflag = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, infmt, SIGNED );
    simdUnpack( b, b_values, infmt, SIGNED );

    for( i=0; i<nvals; i++ )
    {
        switch( mode )
        {
          case MODE_X:
            if( a_values[nvals-1-i] == FIXED_SMIN[infmt] &&
                b_values[i] == FIXED_SMIN[infmt] )
            {
                result += FIXED_SMAX[outfmt];
                ouflag = 1;
            }
            else
                result += a_values[nvals-1-i] * b_values[i] << 1;
            break;
          default:
            if( a_values[i] == FIXED_SMIN[infmt] &&
                b_values[i] == FIXED_SMIN[infmt] )
            {
                result += FIXED_SMAX[outfmt];
                ouflag = 1;
            }
            else
                result += a_values[i] * b_values[i] << 1;
            break;
        }
    }

    if( postsat )
    {
        if( outfmt == SIMD_FMT_L )
        {
            int signa = bits( dspac, 63, 63 );
            int signb = bits( -result, 63, 63 );

            temp = dspac - result;

            if( ( signa == signb ) &&
                ( bits( temp, 63, 63 ) != signa ) )
            {
                ouflag = 1;
                if( signa )
                    dspac = FIXED_SMIN[outfmt];
                else
                    dspac = FIXED_SMAX[outfmt];
            }
            else
                dspac = temp;
        }
        else
            dspac = dspSaturate( dspac - result, outfmt, SIGNED, &ouflag );
    }
    else
        dspac -= result;

    if( ouflag )
        *dspctl = insertBits( *dspctl, 16+ac, 16+ac, 1 );

    return( dspac );
}

int64_t
MipsISA::dspDpa( int64_t dspac, int32_t a, int32_t b, int32_t ac,
                 int32_t fmt, int32_t sign, int32_t mode )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<2; i++ )
    {
        switch( mode )
        {
          case MODE_L:
            dspac += a_values[nvals-1-i] * b_values[nvals-1-i];
            break;
          case MODE_R:
            dspac += a_values[nvals-3-i] * b_values[nvals-3-i];
            break;
          case MODE_X:
            dspac += a_values[nvals-1-i] * b_values[i];
            break;
        }
    }

    return dspac;
}

int64_t
MipsISA::dspDps( int64_t dspac, int32_t a, int32_t b, int32_t ac,
                 int32_t fmt, int32_t sign, int32_t mode )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<2; i++ )
    {
        switch( mode )
        {
          case MODE_L:
            dspac -= a_values[nvals-1-i] * b_values[nvals-1-i];
            break;
          case MODE_R:
            dspac -= a_values[nvals-3-i] * b_values[nvals-3-i];
            break;
          case MODE_X:
            dspac -= a_values[nvals-1-i] * b_values[i];
            break;
        }
    }

    return dspac;
}

int64_t
MipsISA::dspMaq( int64_t dspac, int32_t a, int32_t b, int32_t ac,
                 int32_t fmt, int32_t mode, int32_t saturate, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt-1];
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    int64_t temp = 0;
    uint32_t ouflag = 0;

    simdUnpack( a, a_values, fmt, SIGNED );
    simdUnpack( b, b_values, fmt, SIGNED );

    for( i=0; i<nvals; i++ )
    {
        switch( mode )
        {
          case MODE_L:
            temp = a_values[i+1] * b_values[i+1] << 1;
            if( a_values[i+1] == FIXED_SMIN[fmt] && b_values[i+1] == FIXED_SMIN[fmt] )
            {
                temp = (int64_t)FIXED_SMAX[fmt-1];
                ouflag = 1;
            }
            break;
          case MODE_R:
            temp = a_values[i] * b_values[i] << 1;
            if( a_values[i] == FIXED_SMIN[fmt] && b_values[i] == FIXED_SMIN[fmt] )
            {
                temp = (int64_t)FIXED_SMAX[fmt-1];
                ouflag = 1;
            }
            break;
        }

        temp += dspac;

        if( saturate )
            temp = dspSaturate( temp, fmt-1, SIGNED, &ouflag );
        if( ouflag )
            *dspctl = insertBits( *dspctl, 16+ac, 16+ac, 1 );
    }

    return temp;
}

int64_t
MipsISA::dspMulsa( int64_t dspac, int32_t a, int32_t b, int32_t ac, int32_t fmt )
{
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, SIGNED );
    simdUnpack( b, b_values, fmt, SIGNED );

    dspac += a_values[1] * b_values[1] - a_values[0] * b_values[0];

    return dspac;
}

int64_t
MipsISA::dspMulsaq( int64_t dspac, int32_t a, int32_t b, int32_t ac, int32_t fmt, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    int64_t temp[2];
    uint32_t ouflag = 0;

    simdUnpack( a, a_values, fmt, SIGNED );
    simdUnpack( b, b_values, fmt, SIGNED );

    for( i=nvals-1; i>-1; i-- )
    {
        temp[i] = a_values[i] * b_values[i] << 1;
        if( a_values[i] == FIXED_SMIN[fmt] &&
            b_values[i] == FIXED_SMIN[fmt] )
        {
            temp[i] = FIXED_SMAX[fmt-1];
            ouflag = 1;
        }
    }

    dspac += temp[1] - temp[0];

    if( ouflag )
        *dspctl = insertBits( *dspctl, 16+ac, 16+ac, 1 );

    return dspac;
}

void
MipsISA::dspCmp( int32_t a, int32_t b, int32_t fmt, int32_t sign, int32_t op, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int ccond = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<nvals; i++ )
    {
        int cc = 0;

        switch( op )
        {
          case CMP_EQ: cc = ( a_values[i] == b_values[i] ); break;
          case CMP_LT: cc = ( a_values[i] < b_values[i] ); break;
          case CMP_LE: cc = ( a_values[i] <= b_values[i] ); break;
        }

        ccond |= cc << ( DSP_CTL_POS[DSP_CCOND] + i );
    }

    writeDSPControl( dspctl, ccond, 1<<DSP_CCOND );
}

int32_t
MipsISA::dspCmpg( int32_t a, int32_t b, int32_t fmt, int32_t sign, int32_t op )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<nvals; i++ )
    {
        int cc = 0;

        switch( op )
        {
          case CMP_EQ: cc = ( a_values[i] == b_values[i] ); break;
          case CMP_LT: cc = ( a_values[i] < b_values[i] ); break;
          case CMP_LE: cc = ( a_values[i] <= b_values[i] ); break;
        }

        result |= cc << i;
    }

    return( result );
}

int32_t
MipsISA::dspCmpgd( int32_t a, int32_t b, int32_t fmt, int32_t sign, int32_t op, uint32_t *dspctl )
{
    int i = 0;;
    int nvals = SIMD_NVALS[fmt];
    int32_t result = 0;
    int ccond = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, sign );
    simdUnpack( b, b_values, fmt, sign );

    for( i=0; i<nvals; i++ )
    {
        int cc = 0;;

        switch( op )
        {
          case CMP_EQ: cc = ( a_values[i] == b_values[i] ); break;
          case CMP_LT: cc = ( a_values[i] < b_values[i] ); break;
          case CMP_LE: cc = ( a_values[i] <= b_values[i] ); break;
        }

        result |= cc << i;
        ccond |= cc << ( DSP_CTL_POS[DSP_CCOND] + i );
    }

    writeDSPControl( dspctl, ccond, 1<<DSP_CCOND );

    return( result );
}

int32_t
MipsISA::dspPrece( int32_t a, int32_t infmt, int32_t insign, int32_t outfmt, int32_t outsign, int32_t mode )
{
    int i = 0;
    int sa = 0;
    int ninvals = SIMD_NVALS[infmt];
    int noutvals = SIMD_NVALS[outfmt];
    int32_t result;
    uint64_t in_values[SIMD_MAX_VALS];
    uint64_t out_values[SIMD_MAX_VALS];

    if( insign == SIGNED && outsign == SIGNED )
      sa = SIMD_NBITS[infmt];
    else if( insign == UNSIGNED && outsign == SIGNED )
      sa = SIMD_NBITS[infmt] - 1;
    else if( insign == UNSIGNED && outsign == UNSIGNED )
      sa = 0;

    simdUnpack( a, in_values, infmt, insign );

    for( i=0; i<noutvals; i++ )
    {
        switch( mode )
        {
          case MODE_L: out_values[i] = in_values[i+(ninvals>>1)] << sa; break;
          case MODE_R: out_values[i] = in_values[i] << sa; break;
          case MODE_LA: out_values[i] = in_values[(i<<1)+1] << sa; break;
          case MODE_RA: out_values[i] = in_values[i<<1] << sa; break;
        }
    }

    simdPack( out_values, &result, outfmt );

    return( result );
}

int32_t
MipsISA::dspPrecrqu( int32_t a, int32_t b, uint32_t *dspctl )
{
    int i = 0;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    uint64_t r_values[SIMD_MAX_VALS];
    uint32_t ouflag = 0;
    int32_t result = 0;

    simdUnpack( a, a_values, SIMD_FMT_PH, SIGNED );
    simdUnpack( b, b_values, SIMD_FMT_PH, SIGNED );

    for( i=0; i<2; i++ )
    {
        r_values[i] = dspSaturate( (int64_t)b_values[i] >> SIMD_NBITS[SIMD_FMT_QB] - 1,
                                   SIMD_FMT_QB, UNSIGNED, &ouflag );
        r_values[i+2] = dspSaturate( (int64_t)a_values[i] >> SIMD_NBITS[SIMD_FMT_QB] - 1,
                                     SIMD_FMT_QB, UNSIGNED, &ouflag );
    }

    simdPack( r_values, &result, SIMD_FMT_QB );

    if( ouflag )
        *dspctl = insertBits( *dspctl, 22, 22, 1 );

    return result;
}

int32_t
MipsISA::dspPrecrq( int32_t a, int32_t b, int32_t fmt, uint32_t *dspctl )
{
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    uint64_t r_values[SIMD_MAX_VALS];
    uint32_t ouflag = 0;
    int32_t result;

    simdUnpack( a, a_values, fmt, SIGNED );
    simdUnpack( b, b_values, fmt, SIGNED );

    r_values[1] = dspSaturate( (int64_t)addHalfLsb( a_values[0], 16 ) >> 16,
                                   fmt+1, SIGNED, &ouflag );
    r_values[0] = dspSaturate( (int64_t)addHalfLsb( b_values[0], 16 ) >> 16,
                                   fmt+1, SIGNED, &ouflag );

    simdPack( r_values, &result, fmt+1 );

    if( ouflag )
        *dspctl = insertBits( *dspctl, 22, 22, 1 );

    return result;
}

int32_t
MipsISA::dspPrecrSra( int32_t a, int32_t b, int32_t sa, int32_t fmt, int32_t round )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    uint64_t c_values[SIMD_MAX_VALS];
    int32_t result = 0;

    simdUnpack( a, a_values, fmt, SIGNED );
    simdUnpack( b, b_values, fmt, SIGNED );

    for( i=0; i<nvals; i++ )
    {
        if( round )
        {
            c_values[i] = addHalfLsb( b_values[i], sa ) >> sa;
            c_values[i+1] = addHalfLsb( a_values[i], sa ) >> sa;
        }
        else
        {
            c_values[i] = b_values[i] >> sa;
            c_values[i+1] = a_values[i] >> sa;
        }
    }

    simdPack( c_values, &result, fmt+1 );

    return result;
}

int32_t
MipsISA::dspPick( int32_t a, int32_t b, int32_t fmt, uint32_t *dspctl )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int32_t result;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    uint64_t c_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, UNSIGNED );
    simdUnpack( b, b_values, fmt, UNSIGNED );

    for( i=0; i<nvals; i++ )
    {
        int condbit = DSP_CTL_POS[DSP_CCOND] + i;
        if( bits( *dspctl, condbit, condbit ) == 1 )
            c_values[i] = a_values[i];
        else
            c_values[i] = b_values[i];
    }

    simdPack( c_values, &result, fmt );

    return( result );
}

int32_t
MipsISA::dspPack( int32_t a, int32_t b, int32_t fmt )
{
    int32_t result;
    uint64_t a_values[SIMD_MAX_VALS];
    uint64_t b_values[SIMD_MAX_VALS];
    uint64_t c_values[SIMD_MAX_VALS];

    simdUnpack( a, a_values, fmt, UNSIGNED );
    simdUnpack( b, b_values, fmt, UNSIGNED );

    c_values[0] = b_values[1];
    c_values[1] = a_values[0];

    simdPack( c_values, &result, fmt );

    return( result );
}

int32_t
MipsISA::dspExtr( int64_t dspac, int32_t fmt, int32_t sa, int32_t round, int32_t saturate, uint32_t *dspctl )
{
    int32_t result = 0;
    uint32_t ouflag = 0;
    int64_t temp = 0;

    sa = bits( sa, 4, 0 );

    if( sa > 0 )
    {
        if( round )
        {
            temp = (int64_t)addHalfLsb( dspac, sa );

            if( dspac > 0 && temp < 0 )
            {
                ouflag = 1;
                if( saturate )
                    temp = FIXED_SMAX[SIMD_FMT_L];
            }
            temp = temp >> sa;
        }
        else
            temp = dspac >> sa;
    }
    else
        temp = dspac;

    dspac = checkOverflow( dspac, fmt, SIGNED, &ouflag );

    if( ouflag )
    {
        *dspctl = insertBits( *dspctl, 23, 23, ouflag );

        if( saturate )
            result = (int32_t)dspSaturate( temp, fmt, SIGNED, &ouflag );
        else
            result = (int32_t)temp;
    }
    else
        result = (int32_t)temp;

    return( result );
}

int32_t
MipsISA::dspExtp( int64_t dspac, int32_t size, uint32_t *dspctl )
{
    int32_t pos = 0;
    int32_t result = 0;

    pos = bits( *dspctl, 5, 0 );
    size = bits( size, 4, 0 );

    if( pos - (size+1) >= -1 )
    {
        result = bits( dspac, pos, pos-size );
        *dspctl = insertBits( *dspctl, 14, 14, 0 );
    }
    else
    {
        result = 0;
        *dspctl = insertBits( *dspctl, 14, 14, 1 );
    }

    return( result );
}

int32_t
MipsISA::dspExtpd( int64_t dspac, int32_t size, uint32_t *dspctl )
{
    int32_t pos = 0;
    int32_t result = 0;

    pos = bits( *dspctl, 5, 0 );
    size = bits( size, 4, 0 );

    if( pos - (size+1) >= -1 )
    {
        result = bits( dspac, pos, pos-size );
        *dspctl = insertBits( *dspctl, 14, 14, 0 );
        if( pos - (size+1) >= 0 )
            *dspctl = insertBits( *dspctl, 5, 0, pos - (size+1) );
        else if( (pos - (size+1)) == -1 )
            *dspctl = insertBits( *dspctl, 5, 0, 63 );
    }
    else
    {
        result = 0;
        *dspctl = insertBits( *dspctl, 14, 14, 1 );
    }

    return( result );
}

void
MipsISA::simdPack( uint64_t *values_ptr, int32_t *reg, int32_t fmt )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int nbits = SIMD_NBITS[fmt];

    *reg = 0;

    for( i=0; i<nvals; i++ )
        *reg |= (int32_t)bits( values_ptr[i], nbits-1, 0 ) << nbits*i;
}

void
MipsISA::simdUnpack( int32_t reg, uint64_t *values_ptr, int32_t fmt, int32_t sign )
{
    int i = 0;
    int nvals = SIMD_NVALS[fmt];
    int nbits = SIMD_NBITS[fmt];

    switch( sign )
    {
    case SIGNED:
        for( i=0; i<nvals; i++ )
        {
            values_ptr[i] = (uint64_t)bits( reg, nbits*(i+1)-1, nbits*i );
            values_ptr[i] = signExtend( values_ptr[i], fmt );
        }
        break;
    case UNSIGNED:
        for( i=0; i<nvals; i++ )
        {
            values_ptr[i] = (uint64_t)bits( reg, nbits*(i+1)-1, nbits*i );
        }
        break;
    }
}

void
MipsISA::writeDSPControl( uint32_t *dspctl, uint32_t value, uint32_t mask )
{
    uint32_t fmask = 0;

    if( mask & 0x01 ) fmask |= DSP_CTL_MASK[DSP_POS];
    if( mask & 0x02 ) fmask |= DSP_CTL_MASK[DSP_SCOUNT];
    if( mask & 0x04 ) fmask |= DSP_CTL_MASK[DSP_C];
    if( mask & 0x08 ) fmask |= DSP_CTL_MASK[DSP_OUFLAG];
    if( mask & 0x10 ) fmask |= DSP_CTL_MASK[DSP_CCOND];
    if( mask & 0x20 ) fmask |= DSP_CTL_MASK[DSP_EFI];

    *dspctl &= ~fmask;
    value &= fmask;
    *dspctl |= value;
}

uint32_t
MipsISA::readDSPControl( uint32_t *dspctl, uint32_t mask )
{
    uint32_t fmask = 0;

    if( mask & 0x01 ) fmask |= DSP_CTL_MASK[DSP_POS];
    if( mask & 0x02 ) fmask |= DSP_CTL_MASK[DSP_SCOUNT];
    if( mask & 0x04 ) fmask |= DSP_CTL_MASK[DSP_C];
    if( mask & 0x08 ) fmask |= DSP_CTL_MASK[DSP_OUFLAG];
    if( mask & 0x10 ) fmask |= DSP_CTL_MASK[DSP_CCOND];
    if( mask & 0x20 ) fmask |= DSP_CTL_MASK[DSP_EFI];

    return( *dspctl & fmask );
}
