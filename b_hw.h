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

#include "config_t.h"
#include "e_methods.h"

#define WAND_BIT (1 << 3)

static int hw_bar_init = -1;

static void __attribute__((interrupt)) wand_isr( void )
{
}

void hw_barrier_init( void ) {
  e_irq_global_mask( E_FALSE );
  e_irq_attach( WAND_BIT, wand_isr );
  e_irq_mask( WAND_BIT, E_FALSE );
  hw_bar_init = 0;
}

void hw_barrier( void ) {
  __asm__ __volatile__("wand");
  __asm__ __volatile__("idle");

  unsigned irq_state = get_estatus();
  irq_state &= ~WAND_BIT;
  set_estatus( irq_state ); // clear wand bit
}


/*
  Multicast register

  MULTICAST: 0xF0704 // <- manual is wrong!
  Bits      Name           Function
  [11:0]    MULTICAST_ID   ID to match to destination address[31:20] in the case of an incoming multicast write transaction
  [31:12]   RESERVED       N/A

        E_REG_COREID            = 0xf0704,
        E_REG_MULTICAST         = 0xf0708,
*/
int __hw_multicast( void *p, unsigned len, unsigned root, e_coreid_t core_id )
{
//  volatile unsigned* multicast_reg = (volatile unsigned*) get_remote_ptr( e_group_config.core_row,
//                                                                          e_group_config.core_col,
//                                                                          (void*) 0xF0708 /* E_REG_MULTICAST */ );
  volatile unsigned* multicast_reg = (volatile unsigned*)( (core_id  << 20) | 0xF0708 /* E_REG_MULTICAST */ );
  unsigned multicast_regcnt = *multicast_reg;
  unsigned multicast_id = 1 << 11; // push it to the top, so that it does not interfear with regular ld/st to mesh
  *multicast_reg = multicast_id;

  if( root == core_id ) { /* does only work for 0,1,2,3 */
    // read current config register and set c_id wants to send a MULTICAST
    unsigned config_regcnt = get_econfig();
    unsigned reg_cfg_mmr = ((~(0xF << 12)) & config_regcnt) | (0x3 << 12);
    set_econfig( reg_cfg_mmr );

    unsigned *p_test = (unsigned*)(multicast_id << 20 | (unsigned) p);
    switch(len) {
    case sizeof(char):
      *(volatile char*)p_test = *(volatile char*)p;
      break;
    case sizeof(short):
      *(volatile short*)p_test = *(volatile short*)p;
      break;
    case sizeof(unsigned):
      *(volatile unsigned*)p_test = *(volatile unsigned*)p;
      break;
    default:
      __builtin_unreachable();
    }

    set_econfig( config_regcnt );
  }

  hw_barrier();

  *multicast_reg = multicast_regcnt;

  return 0;
}

int __sw_multicast( void *p, unsigned len, unsigned is_root )
{
  if( is_root ) {
    unsigned row_id, col_id;
    switch(len) {
    case sizeof(char):
    {
      char val = *(volatile char*)p; // put first in register ..
      for(col_id = 0; col_id < e_group_config.group_cols; ++col_id)
        for(row_id = 0; row_id < e_group_config.group_rows; ++row_id) {
          e_coreid_t core_id = gen_pcoreid( row_id, col_id, e_group_config.group_id );
          *(volatile char*)( (core_id  << 20) | (unsigned)p) = val;
        }
      break;
    }
    case sizeof(short):
    {
      short val = *(volatile short*)p; // put first in register ..
      for(col_id = 0; col_id < e_group_config.group_cols; ++col_id)
        for(row_id = 0; row_id < e_group_config.group_rows; ++row_id) {
          e_coreid_t core_id = gen_pcoreid( row_id, col_id, e_group_config.group_id );
          *(volatile short*)( (core_id  << 20) | (unsigned)p) = val;
      }
      break;
    }
    case sizeof(unsigned):
    {
      unsigned val = *(volatile unsigned*)p; // put first in register ..
      for(col_id = 0; col_id < e_group_config.group_cols; ++col_id)
        for(row_id = 0; row_id < e_group_config.group_rows; ++row_id) {
          e_coreid_t core_id = gen_pcoreid( row_id, col_id, e_group_config.group_id );
          *(volatile unsigned*)( (core_id  << 20) | (unsigned)p) = val;
        }
      break;
    }
    default:
      __builtin_unreachable();
    }
  }
  hw_barrier();

  return 0;
}

#define bcast( PTR, LEN, ROOT )  \
({ \
  int ret; \
  do { \
    e_coreid_t ____COREID = get_pcoreid(); \
    typeof(PTR) ____PTR = (PTR); \
    typeof(LEN) ____LEN = (LEN); \
    typeof(LEN) ____ROOT = (ROOT); \
    switch(____LEN) { \
    case sizeof(char): \
    case sizeof(short): \
    case sizeof(unsigned): \
      if(hw_bar_init) \
        hw_barrier_init(); /* just in case */ \
      if( gen_pcoreid(0, 0, e_group_config.group_id) <= ____ROOT \
          && ____ROOT <= gen_pcoreid(0, 3, e_group_config.group_id) ) \
        ret = __hw_multicast( ____PTR, ____LEN, ____ROOT, ____COREID ); \
      else \
        ret = __sw_multicast( ____PTR, ____LEN, ____ROOT == ____COREID ); \
      break; \
    default: \
      ret = -1; \
      break; \
    } \
  } while (0); \
  ret; \
})
