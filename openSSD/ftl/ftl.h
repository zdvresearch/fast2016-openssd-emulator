#ifndef FTL_H
#define FTL_H

#include "jasmine.h" // UINT32
#include "dram_layout.h"
#include "ftl_metadata.h"
#include "ftl_parameters.h"

///////////////////////////////
// FTL public functions
///////////////////////////////

void ftl_open (void);
void ftl_read (UINT32 const lba, UINT32 const num_sectors);
void ftl_write (UINT32 const lba, UINT32 const num_sectors);
void ftl_trim (UINT32 const lba, UINT32 const num_sectors);
void ftl_flush (void);
void ftl_isr (void);

#endif //FTL_H
