#ifndef FTL_METADATA_H
#define FTL_METADATA_H
#include "jasmine.h"
#include "ftl_parameters.h"

//----------------------------------
// FTL metadata (maintain in SRAM)
//----------------------------------
extern UINT32 g_bsp_isr_flag[NUM_BANKS];
extern UINT8 g_mem_to_clr[PAGES_PER_BLK / 8];
extern UINT8 g_mem_to_set[PAGES_PER_BLK / 8];
extern UINT32 g_ftl_read_buf_id;
extern UINT32 g_ftl_write_buf_id;
//
//extern UINT32 free_list_head[NUM_BANKS];
//extern UINT32 free_list_tail[NUM_BANKS];


#define SizeSRAMMetadata    ((sizeof(UINT32) * NUM_BANKS)            + \
                             (sizeof(UINT8) * (PAGES_PER_BLK/8))    + \
                             (sizeof(UINT8) * (PAGES_PER_BLK/8))    + \
                             (sizeof(UINT32))                       + \
                             (sizeof(UINT32))                       + \
                             0)
                             //(sizeof(UINT32) * NUM_BANKS)           + \
                             //(sizeof(UINT32) * NUM_BANKS))

#endif
