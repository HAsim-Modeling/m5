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

#ifndef __BASE_COMPILER_HH__
#define __BASE_COMPILER_HH__

//http://msdn2.microsoft.com/en-us/library/ms937669.aspx
//http://msdn2.microsoft.com/en-us/library/aa448724.aspx
//http://docs.sun.com/source/819-3688/sun.specific.html#marker-998278
//http://gcc.gnu.org/onlinedocs/gcc-3.3.1/gcc/Function-Attributes.html#Function%20Attributes

#if defined(__GNUC__)
#define M5_ATTR_NORETURN  __attribute__((noreturn))
#define M5_PRAGMA_NORETURN(x)
#define M5_DUMMY_RETURN
#define M5_VAR_USED __attribute__((unused))
#elif defined(__SUNPRO_CC)
// this doesn't do anything with sun cc, but why not
#define M5_ATTR_NORETURN  __sun_attr__((__noreturn__))
#define M5_DUMMY_RETURN return (0);
#define DO_PRAGMA(x) _Pragma(#x)
#define M5_VAR_USED
#define M5_PRAGMA_NORETURN(x) DO_PRAGMA(does_not_return(x))
#else
#error "Need to define compiler options in base/compiler.hh"
#endif

#endif // __BASE_COMPILER_HH__
