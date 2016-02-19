#ifndef FTL_PARAMETERS_H
#define FTL_PARAMETERS_H
#include "jasmine.h"

//-------------------------------
// Map Blocks
//-------------------------------
#define MAP_BLK_PER_BANK    3
#define NUM_MAP_BLK         (MAP_BLK_PER_BANK * NUM_BANKS)
//-------------------------------
// Log Blocks
//-------------------------------
#define NUM_DATA_BLK    ( (NUM_LPAGES + PAGES_PER_BLK - 1) / PAGES_PER_BLK )
#define DATA_BLK_PER_BANK   ( (NUM_DATA_BLK + NUM_BANKS - 1) / NUM_BANKS )

#define LOG_BLK_PER_BANK            (DATA_BLK_PER_BANK) // Here should go the overprovisioning

#define LOG_BMT_BYTES       ((NUM_BANKS * LOG_BLK_PER_BANK * sizeof(UINT16) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

//-------------------------------------
// DRAM buffers
//-------------------------------------
#define NUM_COPY_BUFFERS        NUM_BANKS_MAX
#define NUM_FTL_BUFFERS         NUM_BANKS
#define NUM_HIL_BUFFERS         1
#define NUM_TEMP_BUFFERS        1

#define COPY_BUF_BYTES                      (NUM_COPY_BUFFERS * BYTES_PER_PAGE)                                                                           // 1 MB
#define FTL_BUF_BYTES                       (NUM_FTL_BUFFERS * BYTES_PER_PAGE)                                                                             // 1 MB
#define HIL_BUF_BYTES                       (NUM_HIL_BUFFERS * BYTES_PER_PAGE)                                                                             // 32 KB
#define TEMP_BUF_BYTES                      (NUM_TEMP_BUFFERS * BYTES_PER_PAGE)                                                                           // 32 KB

#define DRAM_BYTES_OTHER    (COPY_BUF_BYTES + \
                            FTL_BUF_BYTES + \
                            HIL_BUF_BYTES + \
                            TEMP_BUF_BYTES)

#define LOG_METADATA_BYTES      ((NUM_FTL_BUFFERS + NUM_GC_BUFFERS + NUM_LOG_BUFFERS + NUM_OW_LOG_BUFFERS) * BYTES_PER_PAGE)
#define HASH_METADATA_BYTES     (HASH_BUCKET_BYTES + HASH_NODE_BYTES)
#define NUM_RW_BUFFERS          ((DRAM_SIZE - DRAM_BYTES_OTHER) / BYTES_PER_PAGE - 1)
#define NUM_RD_BUFFERS          (((NUM_RW_BUFFERS / 2) + NUM_BANKS - 1) / NUM_BANKS * NUM_BANKS)
#define NUM_WR_BUFFERS          (NUM_RW_BUFFERS - NUM_RD_BUFFERS)

#define RD_BUF_BYTES        (NUM_RD_BUFFERS * BYTES_PER_PAGE)
#define WR_BUF_BYTES        (NUM_WR_BUFFERS * BYTES_PER_PAGE)

#endif
