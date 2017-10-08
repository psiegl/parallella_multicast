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

  e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);

  unsigned c_id = e_group_config.core_row * e_group_config.group_cols + e_group_config.core_col;

  hw_barrier_init();

  mbox_t * mbox = (mbox_t *) e_emem_config.base;
  mbox->ready = 1;
  if( ! c_id ) {
    mbox->ready = 1;
    while( ! mbox->go );
    mbox->ready = 0;
  }

  e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

/*
  Multicast register

  MULTICAST: 0xF0704 // <- manual is wrong!
  Bits      Name           Function
  [11:0]    MULTICAST_ID   ID to match to destination address[31:20] in the case of an incoming multicast write transaction
  [31:12]   RESERVED       N/A

        E_REG_COREID            = 0xf0704,
        E_REG_MULTICAST         = 0xf0708,
*/

  // set for all the capability to recognize a multicast with id 1 << 11
  unsigned multicast_id = 1 << 11; // push it to the top, so that it does not interfear with regular ld/st to mesh
  unsigned * multicast_reg = ((unsigned*) get_remote_ptr( c_id, (void*) 0xF0708 /* E_REG_MULTICAST */ ));
  unsigned multicast_original = *multicast_reg;
  *multicast_reg = multicast_id;
  volatile unsigned * p_test = (volatile unsigned *)(multicast_id << 20 | (unsigned) &test);

  unsigned reg_cfg = 0;
  if( ! c_id ) { /* does only work for 0,1,2,3 */
    // read current config register and set c_id wants to send a MULTICAST
    reg_cfg = _e_reg_read_config();
    unsigned reg_cfg_mmr = ((~(0xF << 12)) & reg_cfg) | (0x3 << 12);
    _e_reg_write_config( reg_cfg_mmr );
  }

  hw_barrier();

// -------------- test ----------------------
  unsigned time_start = _e_get_ctimer0();
  if( ! c_id ) {
    *p_test = 1; // this will be broadcasted as multicast
  }
  else {
    while( ! test ); // listening for multicast
  }
  unsigned time_end = _e_get_ctimer0();
// -------------- test ----------------------

  if( ! c_id )
    _e_reg_write_config( reg_cfg );
  *multicast_reg = multicast_original;

  mbox->clocks[ c_id ] = time_start - time_end; // parallella is ticking down

  hw_barrier();

  if( ! c_id ) {
    mbox->go    = 0;
    mbox->ready = 1;
  }

  return 0;
}
