#include "mmu.h"

block_attributes_sg1 new_block_attributes_sg1() {
  block_attributes_sg1 bas1;
  bas1.UXN = 0;
  bas1.PXN = 0;
  bas1.ContinuousBit = 0;
  bas1.DirtyBit = 0;
  bas1.NotGlobal = 1;
  bas1.AccessFlag = 0;
  /* Shareability
   * 00 : Non-shareable
   * 01 : unpredictable
   * 10 : Inner shareable
   * 11 : Outer shareable
   */
  bas1.Shareability = 0;
  /* Access Permission
   * XX : higher EL : EL0
   * 00 : RW : None
   * 01 : RW : RW
   * 10 : RO : None
   * 11 : RO : RO
   */
  bas1.AccessPermission = 1;
  bas1.NonSecure = 1;
  /* Attribute Index
   * .0 : Invalid
   * 01 : Block / Page
   * 11 : Table
   */
  bas1.AttrIndex = 1;
  return bas1;
}

void init_block_and_page_entry_sg1(uint64_t entry_addr, uint64_t inner_addr, block_attributes_sg1 ba) {
	entry_addr = (entry_addr & 0xffff000000000fff) | ((inner_addr & 0xfffffffff) << 12);
	set_block_and_page_attributes_sg1(entry_addr, ba);
}

void set_block_and_page_attributes_sg1(uint64_t addr, block_attributes_sg1 bas1) {
	uint64_t entr = * ((uint64_t *) addr);
	entr &= 0xfffffffff000; // Keep only bits 12 to 47
	entr |= (bas1.UXN & 1) << 54;
	entr |= (bas1.PXN & 1) << 53;
	entr |= (bas1.ContinuousBit & 1) << 52;
	entr |= (bas1.DirtyBit & 1) << 51;
	entr |= (bas1.NotGlobal & 1) << 11;
	entr |= (bas1.AccessFlag & 1) << 10;
	entr |= (bas1.Shareability & 3) << 8;
	entr |= (bas1.AccessPermission & 3) << 6;
	entr |= (bas1.NonSecure & 1) << 5;
	entr |= (bas1.AttrIndex & 1) << 2;
	* ((uint64_t *) addr) = entr;
}

void set_block_and_page_dirty_bit(uint64_t addr) {}
void set_block_and_page_access_flag(uint64_t addr) {}

table_attributes_sg1 new_table_attributes_sg1() {
	table_attributes_sg1 ta1;
	ta1.NSTable = 1;
	/* Access Permission Table
	 * 00 : No effect
	 * 01 : Access EL0 forbidden
	 * 10 : Write forbidden (any level)
	 * 11 : 01 and 10
	 */
	ta1.APTable = 0;
	ta1.UXNTable = 0;
	ta1.PXNTable = 0;
	return ta1;
}

void set_table_attributes_sg1(uint64_t addr, table_attributes_sg1 ta1) {
	uint64_t entry = * ((uint64_t *) addr);
	entry &= 0x7ffffffffffffff;
	entry |= (ta1.NSTable & 1) << 63;
	entry |= (ta1.APTable & 3) << 61;
	entry |= (ta1.UXNTable & 1) << 60;
	entry |= (ta1.PXNTable & 1) << 59;
	* ((uint64_t *) addr) = entry;
}

void init_table_entry_sg1(uint64_t entry_addr, uint64_t inner_addr) {
	entry_addr = (entry_addr & 0xffff000000000fff) | ((inner_addr & 0xfffffffff) << 12);
	set_table_attributes_sg1(entry_addr, new_table_attributes_sg1());
}

uint64_t get_address_sg1(uint64_t entry_addr) {
	return (* (uint64_t *) entry_addr) & MASK(47,12) >> 12;
}

int bind_address(uint64_t virtual_addr, uint64_t physical_addr, block_attributes_sg1 ba) {
	uint64_t lvl2_table_addr = 0;
	if ((physical_addr & MASK(11,0)) != 0)
		return 4;
	if ((virtual_addr & 0xffffc0000000) != 0)
		return 2;
	switch (virtual_addr & 0xffff000000000000) {
		case 0xffff000000000000:
			asm volatile ("mrs %0, TTBR1_EL1" : "=r"(lvl2_table_addr) : :);
			break;
		case 0:
			asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_table_addr) : :);
			break;
		default:
			return 1;
	}
	lvl2_table_addr &= 0xfffffffffffe;
	if ((lvl2_table_addr & 0x1fffff) != 0)
		return 3;
	uint64_t lvl2_offset = (virtual_addr & MASK(29,21)) >> 21;
	uint64_t lvl3_table_addr = get_address_sg1(lvl2_table_addr + lvl2_offset);
	uint64_t lvl3_offset = (virtual_addr * MASK(20, 12)) >> 12;
	init_block_and_page_entry_sg1(lvl3_table_addr + lvl3_offset, (physical_addr >> 12), ba);
	return 0;
}
