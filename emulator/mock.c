#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h> // open flags
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "jasmine.h"

char * DRAM;
int flashFD;

uint64_t NumBanks = NUM_BANKS;
uint64_t LogBlkPerBank = LOG_BLK_PER_BANK;
const uint64_t BytesPerPage = BYTES_PER_PAGE;
const uint64_t BytesPerSector = BYTES_PER_SECTOR;
const uint64_t DRAMSize = DRAM_SIZE;

FILE * logFile;

void initDRAM()
{
    DRAM = (char *) malloc (DRAMSize);
    if(DRAM == NULL)
    {
        perror("Error allocating DRAM");
        exit(-1);
    }
}

#ifdef __APPLE__
#define OPEN_FLAGS (O_CREAT | O_RDWR)
#define OPEN_PERM_FLAGS (S_IRUSR | S_IWUSR)
#elif linux
#define OPEN_FLAGS (O_CREAT | O_RDWR | O_LARGEFILE)
#define OPEN_PERM_FLAGS ()
#endif

void initFlash(char * fileName)
{
    flashFD = open(fileName, OPEN_FLAGS, OPEN_PERM_FLAGS);
    assert(flashFD > 0);
}

/*********************************************************************************************************
 ****************************************** MOCK MEMORY **************************************************
 *********************************************************************************************************/

void _mem_copy(void* dst, void* src, const UINT32 num_bytes)
{
    void* dstRealAddr;
    void* srcRealAddr;

    const uint64_t dst_ = (uint64_t) dst;
    const uint64_t src_ = (uint64_t) src;

    if(dst_ >= DRAM_BASE && dst_ < (DRAM_BASE+DRAM_SIZE))
    {
        const uint64_t realAddr = dst_ - DRAM_BASE;
        dstRealAddr = (void *)&DRAM[realAddr];
    }
    else { dstRealAddr = dst; }
    if(src_ >= DRAM_BASE && src_ < (DRAM_BASE+DRAM_SIZE))
    {
        uint64_t realAddr = src_ - DRAM_BASE;
        srcRealAddr = (void *)&DRAM[realAddr];
    }
    else { srcRealAddr = src; }

    memcpy(dstRealAddr, srcRealAddr, num_bytes);
}

UINT32 _mem_bmp_find_sram(const void* const bitmap, UINT32 const num_bytes, UINT32 const val)
{
    printf("_mem_bmp_find_sram!");
    while(1);
}

UINT32 _mem_bmp_find_dram(const void* const bitmap, UINT32 const num_bytes, UINT32 const val)
{
    printf("_mem_bmp_find_dram!");
    while(1);
}

void _mem_set_sram(void * addr, UINT32 const val, UINT32 num_bytes)
{
    memset(addr, val, num_bytes);
}

void _mem_set_dram(UINT32 addr, UINT32 const val, UINT32 num_bytes)
{
    if((UINT32)addr % SDRAM_ECC_UNIT != 0)
    {
        uart_print_level_1("_mem_set_dram: addr not aligned to SDRAM_ECC_UNIT (128). addr value: "); uart_print_level_1_int(addr); uart_print_level_1("\r\n");
        while(1);
    }
    if(num_bytes % SDRAM_ECC_UNIT != 0)
    {
        uart_print_level_1("_mem_set_dram: num_bytes not multiple of SDRAM_ECC_UNIT (128). num_bytes value: "); uart_print_level_1_int(num_bytes); uart_print_level_1("\r\n");
        while(1);
    }
    if((UINT32) addr < DRAM_BASE)
    {
        uart_print_level_1("_mem_set_dram: addr smaller than DRAM start address. addr value: "); uart_print_level_1_int(addr); uart_print_level_1("\r\n");
        while(1);
    }
    //fprintf(stdout, "_mem_set_dram\n");
    UINT32 realAddr = addr - DRAM_BASE;
    memset((void *)&DRAM[realAddr] , val, num_bytes);
}

UINT32 _mem_search_min_max(const void* const addr, UINT32 const num_bytes_per_item, UINT32 const num_items, UINT32 const cmd)
{
    printf("Error: _mem_search_min_max not implemented");
    while(1);
}

UINT32 _mem_search_equ(const void* const addr, UINT32 const num_bytes_per_item, UINT32 const num_items, UINT32 const cmd, UINT32 const val)
{
    return (num_items+1); // note (fabio): this should mean that the item was not found. We need this to avoid finding bad blocks in bad block list when mocking
}

UINT32 _mem_search_equ_4_bytes(const UINT32 addr, UINT32 const size, UINT32 const cmd, UINT32 const val)
{
    UINT32 realAddr = addr - DRAM_BASE;
    UINT32 * startSearchAddress = (UINT32*)(DRAM + realAddr);
    UINT32 position=0;
    for(position=0; position < size; ++position)
    {
        if(*(startSearchAddress+position) == val)
        {
            return position;
        }
    }
    return position;

}

void _write_dram_32(UINT32 const addr, UINT32 const val)
{
    assert(addr >= DRAM_BASE);
    assert(addr < DRAM_BASE + DRAM_SIZE);
    UINT32 realAddr = addr - DRAM_BASE;
    *(UINT32 *)(DRAM + realAddr) = val;
}

void _write_dram_16(UINT32 const addr, UINT16 const val)
{
    assert(addr >= DRAM_BASE);
    assert(addr < DRAM_BASE + DRAM_SIZE);
    UINT32 realAddr = addr - DRAM_BASE;
    *(UINT16 *)(DRAM + realAddr) = val;
}

void _write_dram_8(UINT32 const addr, UINT8 const val)
{
    assert(addr >= DRAM_BASE);
    assert(addr < DRAM_BASE + DRAM_SIZE);
    UINT32 realAddr = addr - DRAM_BASE;
    *(UINT8 *)(DRAM + realAddr) = val;
}

void _set_bit_dram(UINT32 const base_addr, UINT32 const bit_offset)
{}

void _clr_bit_dram(UINT32 const base_addr, UINT32 const bit_offset)
{}

BOOL32 _tst_bit_dram(UINT32 const base_addr, UINT32 const bit_offset)
{
    return FALSE; // note (fabio): this function is used by is_bad_block to test scan list. We return false every time as if all blocks were good.
}

UINT8 _read_dram_8(UINT32 const addr)
{
    assert(addr >= DRAM_BASE);
    assert(addr < DRAM_BASE + DRAM_SIZE);
    UINT32 realAddr = addr - DRAM_BASE;
    return *(UINT8 *)(DRAM + realAddr);
}

UINT16 _read_dram_16(UINT32 const addr)
{
    assert(addr >= DRAM_BASE);
    assert(addr < DRAM_BASE + DRAM_SIZE);
    UINT32 realAddr = addr - DRAM_BASE;
    return *(UINT16 *)(DRAM + realAddr);
}

UINT32 _read_dram_32(UINT32 const addr)
{
    assert(addr >= DRAM_BASE);
    assert(addr < DRAM_BASE + DRAM_SIZE);
    UINT32 realAddr = addr - DRAM_BASE;
    return *(UINT32 *)(DRAM + realAddr);
}

UINT32 _mem_cmp_sram(const void* const addr1, const void* const addr2, const UINT32 num_bytes)
{
    printf("_mem_cmp_sram!");
    while(1);
    return 0;
}

UINT32 _mem_cmp_dram(const void* const addr1, const void* const addr2, const UINT32 num_bytes)
{
    printf("_mem_cmp_dram!");
    while(1);
    return 0;
}

/*********************************************************************************************************
 ******************************************* MOCK FLASH **************************************************
 *********************************************************************************************************/

off_t getPositionInFile(const UINT32 bank, const UINT32 vblock, const UINT32 page, const UINT32 sect)
{
    return ((((bank * LOG_BLK_PER_BANK + vblock) * PAGES_PER_BLK + page) * SECTORS_PER_PAGE + sect) * BYTES_PER_SECTOR);
}

void nand_page_read(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const buf_addr)
{
#if PrintStats
    uart_print_level_1("FR ");
    uart_print_level_1_int(SECTORS_PER_PAGE);
    uart_print_level_1("\r\n");
#endif
    off_t position = getPositionInFile(bank, vblock, page_num, 0);
    off_t numBytes = SECTORS_PER_PAGE * BYTES_PER_SECTOR;

    UINT32 realAddr = buf_addr - DRAM_BASE;
    ssize_t bytesRead = pread(flashFD, (void*)&DRAM[realAddr], numBytes, position);
    assert(bytesRead == numBytes);

}

void nand_page_ptread(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors, UINT32 const buf_addr, UINT32 const issue_flag)
{
#if PrintStats
    uart_print_level_1("FR ");
    uart_print_level_1_int(num_sectors);
    uart_print_level_1("\r\n");
#endif

    off_t position = getPositionInFile(bank, vblock, page_num, sect_offset);
    off_t numBytes = num_sectors * BYTES_PER_SECTOR;

    UINT32 realAddr = buf_addr + (sect_offset * BYTES_PER_SECTOR) - DRAM_BASE;

    ssize_t bytesRead = pread(flashFD, (void*)&DRAM[realAddr], numBytes, position);
    assert(bytesRead == numBytes);

}

void nand_page_read_to_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num)
{
#if PrintStats
    uart_print_level_1("FR ");
    uart_print_level_1_int(SECTORS_PER_PAGE);
    uart_print_level_1("\r\n");
#endif
    nand_page_read(bank, vblock, page_num, RD_BUF_PTR(g_ftl_read_buf_id));
    g_ftl_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;
}

void nand_page_ptread_to_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors)
{
#if PrintStats
    uart_print_level_1("FR ");
    uart_print_level_1_int(num_sectors);
    uart_print_level_1("\r\n");
#endif
    UINT32 bufAddr = RD_BUF_PTR(g_ftl_read_buf_id);
    nand_page_ptread(bank, vblock, page_num, sect_offset, num_sectors, bufAddr, 0);
    g_ftl_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;
}

void nand_page_program(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const buf_addr, UINT32 const issue_flag)
{

#if PrintStats
    uart_print_level_1("FP ");
    uart_print_level_1_int(SECTORS_PER_PAGE);
    uart_print_level_1("\r\n");
#endif

    off_t position = getPositionInFile(bank, vblock, page_num, 0);
    off_t numBytes = SECTORS_PER_PAGE * BYTES_PER_SECTOR;

    UINT32 realAddr = buf_addr - DRAM_BASE;

    ssize_t bytesWritten = pwrite(flashFD, (void*)&DRAM[realAddr], numBytes, position);
    assert(bytesWritten == numBytes);

}

void nand_page_ptprogram(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors, UINT32 const buf_addr, UINT32 const issue_flag)
{
#if PrintStats
    uart_print_level_1("FP ");
    uart_print_level_1_int(num_sectors);
    uart_print_level_1("\r\n");
#endif

    off_t position = getPositionInFile(bank, vblock, page_num, sect_offset);
    off_t numBytes = num_sectors * BYTES_PER_SECTOR;

    UINT32 realAddr = buf_addr + (sect_offset * BYTES_PER_SECTOR) - DRAM_BASE;

    ssize_t bytesWritten = pwrite(flashFD, (void*)&DRAM[realAddr], numBytes, position);
    assert(bytesWritten == numBytes);

}

void nand_page_program_from_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num)
{
#if PrintStats
    uart_print_level_1("FP ");
    uart_print_level_1_int(SECTORS_PER_PAGE);
    uart_print_level_1("\r\n");
#endif
    nand_page_program(bank, vblock, page_num, WR_BUF_PTR(g_ftl_write_buf_id), 0);
    g_ftl_write_buf_id = (g_ftl_write_buf_id + 1) % NUM_WR_BUFFERS;
}

void nand_page_ptprogram_from_host(UINT32 const bank, UINT32 const vblock, UINT32 const page_num, UINT32 const sect_offset, UINT32 const num_sectors)
{
#if PrintStats
    uart_print_level_1("FP ");
    uart_print_level_1_int(num_sectors);
    uart_print_level_1("\r\n");
#endif
    UINT32 bufAddr = WR_BUF_PTR(g_ftl_write_buf_id);
    nand_page_ptprogram(bank, vblock, page_num, sect_offset, num_sectors, bufAddr, 0);
    g_ftl_write_buf_id = (g_ftl_write_buf_id + 1) % NUM_WR_BUFFERS;}

void nand_page_copyback(UINT32 const bank, UINT32 const src_vblock, UINT32 const src_page, UINT32 const dst_vblock, UINT32 const dst_page)
{
    uart_print_level_1("nand_page_copyback not implemented\r\n");
}

void nand_page_modified_copyback(UINT32 const bank, UINT32 const src_vblock, UINT32 const src_page, UINT32 const dst_vblock, UINT32 const dst_page, UINT32 const sect_offset, UINT32 dma_addr, UINT32 const dma_count)
{
    uart_print_level_1("nand_page_modified_copyback not implemented\r\n");
}

void nand_block_erase(UINT32 const bank, UINT32 const vblock)
{
    //uart_print_level_1("nand_block_erase not implemented\r\n");
    off_t position = getPositionInFile(bank, vblock, 0, 0);
    off_t numBytes = SECTORS_PER_VBLK * BYTES_PER_SECTOR;

    uint8_t * buffer = (uint8_t *) malloc(numBytes);
    memset(buffer, -1, numBytes);

    ssize_t bytesWritten = pwrite(flashFD, buffer, numBytes, position);
}

void nand_block_erase_sync(UINT32 const bank, UINT32 const vblock)
{
    nand_block_erase(bank, vblock);
}


/*********************************************************************************************************
 ******************************************* MOCK UART ***************************************************
 *********************************************************************************************************/

void uart_init(void)
{}

void uart_print_level_1(char* string)
{
    fprintf(logFile, "%s", string);
    fflush(logFile);
    //fprintf(stdout, "%s", string);
}

void uart_print_level_1_int(const UINT32 num)
{
    fprintf(logFile, "%d", num);
    fflush(logFile);
    //fprintf(stdout, "%d", num);
}

void uart_print_level_2(char* string)
{
    fprintf(logFile, "%s", string);
    fflush(logFile);
}

void uart_print_level_2_int(const UINT32 num)
{
    fprintf(logFile, "%d", num);
    fflush(logFile);
}

#if OPTION_UART_DEBUG == 1

void uart_print(char* string)
{
    fprintf(logFile, "%s", string);
    fflush(logFile);
    //fprintf(stdout, "%s", string);
}

void uart_print_int(const UINT32 num)
{
    fprintf(logFile, "%d", num);
    fflush(logFile);
    //fprintf(stdout, "%d", num);
}

void uart_print_hex(UINT32 num)
{
    printf(logFile, "%d", num);
    fflush(logFile);
}
#endif

/*********************************************************************************************************
 ******************************************* MOCK FLASH **************************************************
 *********************************************************************************************************/

const UINT8 c_bank_map[NUM_BANKS] = BANK_MAP;

UINT8 isBankBusy(UINT32 const bank)
{
    return TRUE;
}

void dumpBankStates(UINT32 const bank)
{}

void predumpBankStates(UINT32 const bank)
{}

void waitBusyBank(UINT32 const bank)
{}

void flash_issue_cmd(UINT32 const bank, UINT32 const sync)
{}

void flash_copy(UINT32 const bank, UINT32 const dst_row, UINT32 const src_row)
{}

void flash_modify_copy(UINT32 const bank, UINT32 const dst_row, UINT32 const src_row, UINT32 const sct_offset, UINT32 dma_addr, UINT32 const dma_count)
{}

void flash_erase(UINT32 const bank, UINT16 const vblk_offset)
{}

void flash_finish(void)
{}

void flash_reset(void)
{}

void flash_reset_one_bank(UINT32 bank)
{}

void flash_clear_irq()
{}

/*********************************************************************************************************
 ******************************************** MOCK MISC **************************************************
 *********************************************************************************************************/
void led(BOOL32 on)
{}

void led_blink(void)
{}

void test_nand_blocks(void)
{}

void start_interval_measurement(UINT32 const timer, UINT32 const prescale)
{}

void start_timer(UINT32 const timer, UINT32 const prescale, UINT32 const init_val)
{}

#if OPTION_UART_DEBUG == 1

void ptimer_start(void)
{}

void ptimer_stop_and_uart_print(void)
{}

#endif

/*********************************************************************************************************
 ******************************************** MOCK TARGET_H **********************************************
 *********************************************************************************************************/

UINT32 disable_irq(void){return 0;}
void enable_irq(void){}
UINT32 disable_fiq(void){return 0;}
void enable_fiq(void){}

void SETREG(UINT32 ADDR, UINT32 VAL){}
UINT32 GETREG(UINT32 ADDR)
{
    switch (ADDR)
    {
        case BM_WRITE_LIMIT:
            {
                return g_ftl_write_buf_id;
            }
        default:
            {
                return 0;
            }
    }
}

/*********************************************************************************************************
 ******************************************** MOCK SATA I/F **********************************************
 *********************************************************************************************************/

void writeToOpenSSD(const uint64_t lba, const uint64_t numSectors, void * buffer)
{
    assert((lba % SECTORS_PER_PAGE) + numSectors <= SECTORS_PER_PAGE);

    uint64_t sectOffset = lba % SECTORS_PER_PAGE;

    UINT32 bufAddr = WR_BUF_PTR(g_ftl_write_buf_id) + sectOffset * BYTES_PER_SECTOR;

    UINT32 realAddr = bufAddr - DRAM_BASE;
    memcpy(&DRAM[realAddr], buffer, numSectors * BYTES_PER_SECTOR);
    ftl_write(lba, numSectors);
}

void readFromOpenSSD(const uint64_t lba, const uint64_t numSectors, void * buffer)
{
    assert((lba % SECTORS_PER_PAGE) + numSectors <= SECTORS_PER_PAGE);

    uint64_t sectOffset = lba % SECTORS_PER_PAGE;
    UINT32 bufAddr = RD_BUF_PTR(g_ftl_read_buf_id) + sectOffset * BYTES_PER_SECTOR;
    UINT32 realAddr = bufAddr - DRAM_BASE;
    ftl_read(lba, numSectors);
    memcpy(buffer, &DRAM[realAddr], numSectors * BYTES_PER_SECTOR);
}
