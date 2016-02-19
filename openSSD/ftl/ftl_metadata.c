#include "ftl_metadata.h"

//----------------------------------
// FTL metadata (maintain in SRAM)
//----------------------------------
UINT32 g_bsp_isr_flag[NUM_BANKS];
UINT8 g_mem_to_clr[PAGES_PER_BLK / 8];
UINT8 g_mem_to_set[PAGES_PER_BLK / 8];

// SATA read/write buffer pointer id
UINT32 g_ftl_read_buf_id;
UINT32 g_ftl_write_buf_id;

//UINT32 free_list_head[NUM_BANKS];
//UINT32 free_list_tail[NUM_BANKS];
