#ifndef DRAM_LAYOUT_H
#define DRAM_LAYOUT_H
#include "jasmine.h"
#include "ftl_parameters.h"

///////////////////////////////
// DRAM segmentation
///////////////////////////////

#define RD_BUF_ADDR                                 DRAM_BASE

#define WR_BUF_ADDR                                 (RD_BUF_ADDR + RD_BUF_BYTES)    // base address of SATA write buffers

#define COPY_BUF_ADDR                               (WR_BUF_ADDR + WR_BUF_BYTES)    // base address of flash copy buffers

#define FTL_BUF_ADDR                                (COPY_BUF_ADDR + COPY_BUF_BYTES)    // a buffer dedicated to FTL internal purpose

#define HIL_BUF_ADDR                                (FTL_BUF_ADDR + FTL_BUF_BYTES)

#define TEMP_BUF_ADDR                               (HIL_BUF_ADDR + HIL_BUF_BYTES)    // general purpose buffer

#define END_ADDR                                    (TEMP_BUF_ADDR + TEMP_BUF_BYTES)

//////////////////////////
// Buffer access macros //
//////////////////////////
#define WR_BUF_PTR(BUF_ID)                              (WR_BUF_ADDR + ((UINT32)(BUF_ID)) * BYTES_PER_PAGE)
#define WR_BUF_ID(BUF_PTR)                              ((((UINT32)BUF_PTR) - WR_BUF_ADDR) / BYTES_PER_PAGE)
#define RD_BUF_PTR(BUF_ID)                              (RD_BUF_ADDR + ((UINT32)(BUF_ID)) * BYTES_PER_PAGE)
#define RD_BUF_ID(BUF_PTR)                              ((((UINT32)BUF_PTR) - RD_BUF_ADDR) / BYTES_PER_PAGE)
#define _COPY_BUF(RBANK)                                (COPY_BUF_ADDR + (RBANK) * BYTES_PER_PAGE)
#define COPY_BUF(BANK)                                  _COPY_BUF(REAL_BANK(BANK))
#define FTL_BUF(BANK)                                   (FTL_BUF_ADDR + ((BANK) * BYTES_PER_PAGE))

#endif
