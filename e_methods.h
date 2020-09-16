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

#define set_ectimer0(VAL) \
  asm volatile ("movts ctimer0, %0" : :"r" (VAL) : /*"ctimer0"*/);

#define set_ectimer1(VAL) \
  asm volatile ("movts ctimer1, %0" : :"r" (VAL) : /*"ctimer1"*/);

#define get_ectimer0() \
({ \
  unsigned tmp; \
  asm volatile ("movfs %0, ctimer0;" : "=r" (tmp) :: ); \
  tmp; \
})

#define get_ectimer1() \
({ \
  unsigned tmp; \
  asm volatile ("movfs %0, ctimer1;" : "=r" (tmp) :: ); \
  tmp; \
})

#define get_econfig() \
({ \
  unsigned tmp; \
  asm volatile ("movfs %0, config;" : "=r" (tmp) :: ); \
  tmp; \
})

#define set_econfig( VAL ) \
  asm volatile ("movts config, %0;" :: "r" (VAL) : )

#define get_estatus() \
({ \
  unsigned tmp; \
  asm volatile ("movfs %0, status;" : "=r" (tmp) :: ); \
  tmp; \
})

#define set_estatus( VAL ) \
  asm volatile ("movts status, %0;" :: "r" (VAL) : )

// get physical core id
#define get_pcoreid() \
({ \
  e_coreid_t cid; \
  asm volatile ("movfs %0, coreid" : "=r" (cid) : : /*"coreid"*/); \
  cid; \
})

// generate physical core id
#define gen_pcoreid( ROW, COL, GID ) \
  (( (ROW) << 6 ) | (COL) | (GID) )

void *get_remote_ptr( unsigned row_id, unsigned col_id, void *ptr ) {
  /* If the address is global, return the pointer unchanged */
  if ((unsigned)ptr & 0xfff00000)
    return ptr;

  unsigned core_id = gen_pcoreid( row_id, col_id, e_group_config.group_id );
  return (void *)((core_id << 20) | (unsigned)ptr);
}

#endif /* #ifndef _E_METHODS_H_ */
