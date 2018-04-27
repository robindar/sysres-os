#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>
#include "../libc/uart/uart.h"
#include "../libc/debug/debug.h"
#include "alloc.h"

#define one_u64 ((uint64_t) 1)
#define AT(addr) (* (uint64_t *) (addr))
#define MASK(f,t) ((f < t) ? _MASK_(t,f) : _MASK_(f,t))
#define _MASK_(f,t) ( f == 63 ? 0 : (one_u64 << (f+1)) ) - (one_u64 << t)

#define RAM_SIZE         1073741824 /* 1 Gio */
#define GRANULE            (0x1000)
#define N_TABLE_ENTRIES         512

enum block_perm_config {
  /* UXN : bit 3
   * PXN : bit 2
   * AP  : bits 1-0 */
  ACCESS_FLAG_SET = 0b10000,
  KERNEL_PAGE     = 0b00000, /* EL0 --X, ELn RWX : UXN(0) PXN(0) AP(00) */
  USER_PAGE       = 0b00001, /* EL0 RWX, ELn RW- : UXN(0) PXN(0) AP(01) */
  IO_PAGE         = 0b01001  /* EL0 RW-, ELn RW- : UXN(1) PXN(0) AP(01) */
};

enum block_cache_config {
    /* See doc/mmu.md for details */
    DEVICE        = 0,
    NORMAL_WT_NT  = 1,
    NORMAL_WT_T   = 2,
    NORMAL_WB_NT  = 3,
    NORMAL_WB_T   = 4,
    NON_CACHEABLE = 5
};

/* Set Block and Page Attributes for Stage 1 Translation
 *
 * Upper Attributes
 *   62-59: PBHA (Not Implemented)
 *   54   : UXN  : Execute never
 *   53   : PXN  : Privileged execute never
 *   52   : Contiguous bit
 *   51   : Dirty bit
 *
 * Lower Attributes
 *   11   : NG : Not global
 *   10   : AF : Access flag
 *   9-8  : Shareability
 *   7-6  : AP : Access Permission
 *   5    : Non secure
 *   4-2  : AttrIndX : Attribute Index Field
 */
typedef struct {
	uint64_t UXN,
			 PXN,
			 ContinuousBit,
			 DirtyBit,
			 NotGlobal,
			 AccessFlag,
			 Shareability,
			 AccessPermission,
			 NonSecure,
			 AttrIndex;
} block_attributes_sg1;

block_attributes_sg1 new_block_attributes_sg1(enum block_perm_config perm_config, enum block_cache_config cache_config);

void init_block_and_page_entry_sg1(uint64_t entry_addr, uint64_t inner_addr, block_attributes_sg1 ba);

void set_block_and_page_attributes_sg1(uint64_t addr, block_attributes_sg1 bas1);

void set_block_and_page_dirty_bit(uint64_t addr);
void set_block_and_page_access_flag(uint64_t addr);

/* Set Table Attributes for Stage 1 Translation
 *
 *    63  : NSTable
 * 61-62  : APTable
 *    60  : UXNTable
 *    59  : PXNTable
 */
typedef struct {
	uint64_t NSTable,
			 APTable,
			 UXNTable,
			 PXNTable;
} table_attributes_sg1;

table_attributes_sg1 new_table_attributes_sg1();

void set_table_attributes_sg1(uint64_t addr, table_attributes_sg1 ta1);

void init_table_entry_sg1(uint64_t entry_addr, uint64_t inner_addr);

bool is_block_entry(uint64_t entry, int lvl);

bool is_table_entry(uint64_t entry);

uint64_t get_address_sg1(uint64_t entry_addr);

uint64_t get_lvl3_entry_phys_address(uint64_t virtual_addr);

/*  Return values
 *
 *  0 : Success
 *  1 : Invalid header of virtual address for TTBR selection
 *  2 : Invalid address (a bit is 1 in range [47:30] but should not)
 *  3 : Improper lvl2 address alignement (a bit is 1 in range [20:0] : cf. ARM ARM : 2727)
 *  4 : Improper physical address alignement (bits [11:0] should be 0)
 *  5 : lvl2 table entry not a table entry
 */

/*  Set table entries such that with the current ASID, virtual address virtual_addr
 *  points to physical address physical_addr.
 *
 *  Warning: physical_addr [11:0] should be 0 (start of 4kB page)
 *  Warning: virtual_addr [11:0] have no power here
 */
int bind_address(uint64_t virtual_addr, uint64_t physical_addr, block_attributes_sg1 ba);

void populate_lvl2_table(uint64_t lvl3_address);
uint64_t identity_paging();

uint64_t c_init_mmu();

void pmapdump();

/* Physical memory map strcture
 * (functions as a stack)
 *
 * Access ith element with phys_mem.map[ phys_mem.head + i ]
 * Insert freed pages at phys_mem.head - 1
 */
struct physical_memory_map_t {
	uint32_t * map; /* address of the actual map */
	uint32_t head, size;
};

int get_new_page(uint64_t virtual_address, enum block_perm_config block_perm, enum block_cache_config cache_config);
void translation_fault_handler(uint64_t fault_address, int level, bool lower_el);
void access_flag_fault_lvl3_handler(uint64_t fault_address, int level, bool lower_lvl);
int free_virtual_page(uint64_t virtual_addr);
#endif
