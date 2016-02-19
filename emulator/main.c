#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "jasmine.h"
#include "mock.h"

uint64_t maxLba = 0;

void randomTest(uint64_t maxLba)
{
    srand(NULL);

    printf("randomTest\n");

    for (int i=0; i<10; ++i)
    {
        uint64_t lba = (random() % maxLba) / 8 * 8;
        uint64_t numSect = ((random() % (SECTORS_PER_PAGE - (lba % SECTORS_PER_PAGE))) / 8 * 8) + 8;
        
        assert((lba % SECTORS_PER_PAGE) + numSect <= SECTORS_PER_PAGE);
        
        uint64_t bufferSize = numSect * BYTES_PER_SECTOR;
        uint8_t * bufferWritten = (uint8_t *) malloc(bufferSize);
        uint8_t * bufferRead = (uint8_t *) malloc(bufferSize);
        memset(bufferWritten, random(), bufferSize);
        
        writeToOpenSSD(lba, numSect, bufferWritten);
        
        readFromOpenSSD(lba, numSect, bufferRead);
        
        int ret = memcmp(bufferRead, bufferWritten, bufferSize);
        if (ret != 0)
        {
            printf("Failure at iteration %d : lba = %"PRIu64" (offset = %"PRIu64"), numSect = %"PRIu64"\n",
                   i,
                   lba,
                   lba%SECTORS_PER_PAGE,
                   numSect);
        }
        
        free(bufferWritten);
        free(bufferRead);
    }
}

int main(int argc, const char *argv[])
{

    if (argc != 3)
    {
        printf("Syntax error: ");
        for (int i =0; i< argc; ++i) { printf("%s ", argv[i]); }
        printf("\n");
        printf("Correct call: %s <fileToEmulateFlash> <logFile>\n", argv[0]);
        exit(0);
    }

    logFile = fopen(argv[2], "w");

    initDRAM();
    initFlash(argv[1]);

    ftl_open();

    uint64_t blkPerBank = DATA_BLK_PER_BANK;

    uint64_t numSectorsPerBank = blkPerBank * SECTORS_PER_VBLK;
    uint64_t numBanks = NUM_BANKS;
    maxLba = numSectorsPerBank * numBanks;
    float maxCapacity = (maxLba * 512.0) / (1024.0 * 1024.0 * 1024.0);


    fprintf(stdout, "Blk per bank: %"PRIu64"\n", blkPerBank);
    fprintf(stdout, "numSectorsPerBank = %"PRIu64", numBanks = %"PRIu64"\n", numSectorsPerBank, numBanks);
    fprintf(stdout, "maxLba = %"PRIu64", maxCapacity = %.2f GB\n", maxLba, maxCapacity);

    randomTest(maxLba);

    return 0;
}
