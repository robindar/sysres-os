#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#define MASK(f,t) ((f < t) ? ((uint64_t) 1 << (t+1)) - ((uint64_t) 1 << f)  : ((uint64_t) 1 << (f+1)) - ((uint64_t) 1 << t))

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

block_attributes_sg1 new_block_attributes_sg1();

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

uint64_t get_address_sg1(uint64_t entry_addr);

/*  Return values
 *
 *  0 : Success
 *  1 : Invalid header of virtual address for TTBR selection
 *  2 : Invalid address (a bit is 1 in range [47:30] but should not)
 *  3 : Improper lvl2 address alignement (a bit is 1 in range [20:0] : cf. ARM ARM : 2727)
 *  4 : Improper physical address alignement (bits [11:0] should be 0)
 */

/*  Set table entries such that with the current ASID, virtual address virtual_addr
 *  points to physical address physical_addr.
 *
 *  Warning: physical_addr [11:0] should be 0 (start of 4kB page)
 *  Warning: virtual_addr [11:0] have no power here
 */
int bind_address(uint64_t virtual_addr, uint64_t physical_addr, block_attributes_sg1 ba);

#endif
