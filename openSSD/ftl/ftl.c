#include "jasmine.h"
#include "ftl.h"
#include "dram_layout.h"
#include "ftl_parameters.h"
#include "ftl_metadata.h"

#include "flash.h" // RETURN_ON_ISSUE RETURN_WHEN_DONE

//----------------------------------
// FTL internal function prototype
//----------------------------------
static void sanity_check (void);
static void format (void);

static void sanity_check ()
{
    if (DRAM_BYTES_OTHER > DRAM_SIZE)
    {
        uart_print_level_1("DRAM Metadata and buffers are too big (not counting SATA buffers)\r\n");
        uart_print_level_1 ("\r\nDATA_BLK_PER_BANK "); uart_print_level_1_int(DATA_BLK_PER_BANK);
        uart_print_level_1 ("\r\nLOG_BLK_PER_BANK "); uart_print_level_1_int(LOG_BLK_PER_BANK);
        uart_print_level_1("\r\nDRAM Metadata and buffers (not SATA): "); uart_print_level_1_int(DRAM_BYTES_OTHER/1024); uart_print_level_1("KB\r\n");
        while(1);
    }
    UINT32 dram_requirement = RD_BUF_BYTES + WR_BUF_BYTES + DRAM_BYTES_OTHER;
    uart_print("Read buffers: "); uart_print_int(RD_BUF_BYTES/1024);
    uart_print(" KB\r\nWrite buffers: "); uart_print_int(WR_BUF_BYTES/1024);
    uart_print(" KB\r\nOthers: "); uart_print_int(DRAM_BYTES_OTHER/1024);
    uart_print(" KB\r\nTotal DRAM Requirements: "); uart_print_int(dram_requirement/1024);
    uart_print(" KB\r\nCopy buffers: "); uart_print_int(COPY_BUF_BYTES/1024);
    uart_print(" KB\r\nFtl buffers: "); uart_print_int(FTL_BUF_BYTES/1024 );
    uart_print(" KB\r\nHil buffers: "); uart_print_int(HIL_BUF_BYTES/1024);
    uart_print(" KB\r\nTemp buffers: "); uart_print_int(TEMP_BUF_BYTES/1024);
    if (dram_requirement > DRAM_SIZE)
    {
        uart_print_level_1("Requires too much DRAM memory\r\n");
        while(1);
    }
}

void ftl_open (void)
{
    uart_print ("FTL v1\r\n");
    uart_print_level_1("Important parameters:\r\n");
    uart_print_level_1("NUM_BANKS "); uart_print_level_1_int(NUM_BANKS); uart_print_level_1("\r\n");
    uart_print_level_1("NUM_CHANNELS "); uart_print_level_1_int(NUM_CHANNELS); uart_print_level_1("\r\n");
    uart_print_level_1("DATA_BLK_PER_BANK "); uart_print_level_1_int(DATA_BLK_PER_BANK); uart_print_level_1("\r\n");
    uart_print_level_1("LOG_BLK_PER_BANK "); uart_print_level_1_int(LOG_BLK_PER_BANK); uart_print_level_1("\r\n");


    uart_print("DRAM Address range: "); uart_print_int(DRAM_BASE); uart_print(" - "); uart_print_int(END_ADDR); uart_print("\r\n");
    uart_print("Total bytes: "); uart_print_int(END_ADDR - DRAM_BASE); uart_print("\r\n");
    uart_print_level_1("Total FTL DRAM metadata size: "); uart_print_level_1_int((UINT32)DRAM_BYTES_OTHER/1024); uart_print_level_1(" KB\r\n");
    uart_print_level_1("Total FTL SRAM metadata size: "); uart_print_level_1_int(SizeSRAMMetadata); uart_print_level_1(" B\r\n");

    sanity_check();
    flash_clear_irq ();
    SETREG (INTR_MASK, FIRQ_DATA_CORRUPT | FIRQ_BADBLK_L | FIRQ_BADBLK_H);
    SETREG (FCONF_PAUSE, FIRQ_DATA_CORRUPT | FIRQ_BADBLK_L | FIRQ_BADBLK_H);
    enable_irq ();
    format ();
    g_ftl_read_buf_id = 0;
    g_ftl_write_buf_id = 0;
    mem_set_sram (g_mem_to_set, INVALID, PAGES_PER_BLK / 8);
    mem_set_sram (g_mem_to_clr, NULL, PAGES_PER_BLK / 8);
}

static void format (void)
{
    uart_print ("do format\r\n");
    uart_print ("NUM_BANKS "); uart_print_int((UINT32)NUM_BANKS);
    uart_print ("\r\nNUM_PSECTORS "); uart_print_int((UINT32)NUM_PSECTORS);
    uart_print ("\r\nNUM_LSECTORS "); uart_print_int(NUM_LSECTORS);
    uart_print ("\r\nVBLKS_PER_BANK "); uart_print_int(VBLKS_PER_BANK);
    uart_print ("\r\nDATA_BLK_PER_BANK "); uart_print_int(DATA_BLK_PER_BANK);
    uart_print ("\r\nLOG_BLK_PER_BANK "); uart_print_int(LOG_BLK_PER_BANK);

    UINT32 lbn, vblock;
    for (UINT32 bank = 0; bank < NUM_BANKS; bank++)
    {
        vblock = 0;
        nand_block_erase_sync (bank, vblock);
        g_bsp_isr_flag[bank] = INVALID;
        uart_print("Initializing bank "); uart_print_int(bank); uart_print("\r\n");
        uart_print("Real bank "); uart_print_int(REAL_BANK(bank)); uart_print("\r\n");
        for (lbn = 0; lbn < LOG_BLK_PER_BANK;)
        {
            vblock++;
            if (vblock >= VBLKS_PER_BANK) { break; }
            nand_block_erase_sync (bank, vblock);
            if (g_bsp_isr_flag[bank] != INVALID)
            {
                // vblock is invalid, so we reset the flag and skip it
                g_bsp_isr_flag[bank] = INVALID;
                continue;
            }
            uart_print("using vblock "); uart_print_int(vblock); uart_print("\r\n");
            
            // TODO: here you should insert vblock in a data structure that allows you to remember which physical blocks you are using.
            // It is possible to use a mapping between logical block numbers and physical block numbers, or use the physical block numbers
            // directly in the lba mapping table.
            
            lbn++;
        }
        if (lbn < LOG_BLK_PER_BANK)
        {
            uart_print_level_1("ERROR! There are not enough usable blocks!\r\n");
            while (1);
        }
    }
    //----------------------------------------
    // initialize SRAM metadata
    //----------------------------------------
    led (1);
    uart_print_level_1("format complete");
    uart_print_level_1("\r\n");
}

void ftl_flush (void)
{}


void ftl_isr (void)
{
    UINT32 bank;
    UINT32 bsp_intr_flag;
    uart_print("BSP interrupt occured...\r\n");
    SETREG (APB_INT_STS, INTR_FLASH); // interrupt pending clear (ICU)
    for (bank = 0; bank < NUM_BANKS; bank++)
    {
        int count=0;
        while (BSP_FSM (bank) != BANK_IDLE)
        {
            count++;
            if (count > 100000)
            {
                uart_print("Warning on ftl_isr, waiting for bank "); uart_print_int(bank); uart_print("\r\n");
                count=0;
            }
        }
        bsp_intr_flag = BSP_INTR(bank); // get interrupt flag from BSP
        if (bsp_intr_flag == 0) { continue; }
        UINT32 fc = GETREG(BSP_CMD (bank));
        CLR_BSP_INTR(bank, bsp_intr_flag); // BSP clear
        if (bsp_intr_flag & FIRQ_DATA_CORRUPT)
        {
            uart_print("BSP interrupt at bank: ");
            uart_print_int(bank);
            uart_print("\r\nFIRQ_DATA_CORRUPT occured...vblock ");
            uart_print_int(GETREG (BSP_ROW_H (bank)));
            uart_print(", page ");
            uart_print_int(GETREG (BSP_ROW_H (bank)) % PAGES_PER_BLK);
            uart_print("\r\n");
            //g_bsp_isr_flag[bank] = GETREG (BSP_ROW_H (bank)) / PAGES_PER_BLK;
            g_bsp_isr_flag[bank] = INVALID; // Don't want to interfere with erase errors during BER testing
        }
        if (bsp_intr_flag & (FIRQ_BADBLK_H | FIRQ_BADBLK_L))
        {
            if (fc == FC_COL_ROW_IN_PROG || fc == FC_IN_PROG || fc == FC_PROG)
            {
                uart_print("BSP interrupt at bank: ");
                uart_print_int(bank);
                uart_print("\r\n");
                uart_print("find runtime bad block when block program...");
                uart_print("\r\n");
            }
            else
            {
                if(fc == FC_ERASE)
                {
                    uart_print("BSP interrupt at bank: ");
                    uart_print_int(bank);
                    uart_print("\r\n");
                    uart_print("find runtime bad block when block erase...vblock #: ");
                    uart_print_int(GETREG (BSP_ROW_H (bank)) / PAGES_PER_BLK);
                    uart_print("\r\n");
                    g_bsp_isr_flag[bank] = GETREG (BSP_ROW_H (bank)) / PAGES_PER_BLK;
                }
                else
                {
                    uart_print("BSP interrupt at bank: ");
                    uart_print_int(bank);
                    uart_print(" during command: ");
                    uart_print_int(fc);
                    uart_print("\r\n");
                }
            }
        }
        else
        {
            uart_print("BSP interrupt at bank: ");
            uart_print_int(bank);
            uart_print(" with flag: ");
            uart_print_int(bsp_intr_flag);
            uart_print("\r\n");
        }
    }
    uart_print("\r\n");
}

void ftl_trim (UINT32 const lba, UINT32 const num_sectors)
{}

void ftl_read (UINT32 const lba, UINT32 const num_sectors)
{
    uart_print_level_1("ftl_read\r\n");
    
    // Examples:
    
    // 1) use flash-to-SATA-buffer direct operation
    //nand_page_read_to_host(0, 5, 22);

    // 2) Retrieve page in DRAM/or flash and manually copy it to SATA buffer
    // a) Retrieve page
    // b) Copy to SATA buffer
    //
    // Manage FTL SATA read buffer pointer:
    //    g_ftl_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;
    //    SETREG (BM_STACK_RDSET, g_ftl_read_buf_id);
    //    SETREG (BM_STACK_RESET, 0x02);
}



void ftl_write (UINT32 const lba, UINT32 const nSects)
{
    uart_print_level_1("ftl_write\r\n");
    
    // Examples:
    
    // 1) use flash-to-SATA-buffer direct operation
    //nand_page_program_from_host(0, 5, 22);

    // 2) Manually copy data from SATA buffer to custom DRAM buffer, or flash page
    //
    // Manage FTL SATA write buffer pointer:
    //    g_ftl_write_buf_id = (g_ftl_write_buf_id + 1) % NUM_WR_BUFFERS;
    //    SETREG (BM_STACK_WRSET, g_ftl_write_buf_id);
    //    SETREG (BM_STACK_RESET, 0x01);
}
