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

#include <e-lib.h>
#include "config_t.h"
#include "e_methods.h"
#include "b_hw.h"

volatile unsigned test;

int main( void ) {
  test = 0;

  hw_barrier_init();

  e_coreid_t pid = get_pcoreid();

  mbox_t * mbox = (mbox_t *) e_emem_config.base;
  mbox->ready = 1;
  if( pid == gen_pcoreid( 0, 0, e_group_config.group_id ) ) {
    mbox->ready = 1;
    while( ! mbox->go );
    mbox->ready = 0;
  }

  set_ectimer0(E_CTIMER_MAX);
  e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
  hw_barrier();

  e_coreid_t root = gen_pcoreid( 0, 3, e_group_config.group_id );
  if( pid == root )
    test = 3;

// -------------- test ----------------------
  unsigned time_bgn = get_ectimer0();

  bcast( (void*)&test, sizeof(unsigned), root );

  unsigned time_end = get_ectimer0();

  while( test != 3 );

// -------------- test ----------------------

  e_coreid_t lid = e_group_config.core_row * e_group_config.group_cols + e_group_config.core_col;
  mbox->clocks[ lid ] = time_bgn - time_end; // parallella is ticking down

  hw_barrier();

  if( pid == gen_pcoreid( 0, 0, e_group_config.group_id ) ) {
    mbox->go    = 0;
    mbox->ready = 1;
  }

  return 0;
}
