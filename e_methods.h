//  Copyright (c) 2017, Dipl.-Inf. Patrick Siegl
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _E_METHODS_H_
#define _E_METHODS_H_

#include <e-lib.h> // e_group_config, e_ctimer_set(), e_ctimer_start()

ALWAYS_INLINE unsigned _e_get_ctimer0()
{
  register unsigned tmp asm("r0");
  asm volatile ("movfs %0, ctimer0;" : "=r" (tmp) :: );
  return tmp;
}

ALWAYS_INLINE unsigned _e_reg_read_config( void )
{
  register unsigned tmp asm("r0");
  asm volatile ("movfs %0, config;" : "=r" (tmp) :: );
  return tmp;
}

ALWAYS_INLINE void _e_reg_write_config( register unsigned val )
{
  asm volatile ("movts config, %0;" :: "r" (val) : );
}

ALWAYS_INLINE unsigned _e_reg_read_status( void )
{
  register unsigned tmp asm("r0");
  asm volatile ("movfs %0, status;" : "=r" (tmp) :: );
  return tmp;
}

ALWAYS_INLINE void _e_reg_write_status( register unsigned val )
{
  asm volatile ("movts status, %0;" :: "r" (val) : );
}


unsigned * get_remote_ptr( unsigned id, void * ptr ) {
// --------------------------------------------
// needs to be adjusted for the 64 core version
  unsigned col_id = id & 0x3;
  unsigned row_id = id >> 2;
  unsigned core_id = (row_id * 0x40 + col_id) + e_group_config.group_id;
// --------------------------------------------
  unsigned * new_ptr = (unsigned *)((core_id << 20) | (unsigned)ptr);
  return new_ptr;
}

#endif /* #ifndef _E_METHODS_H_ */
