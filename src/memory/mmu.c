#include "mmu.h"

extern uint64_t __TTBR1_EL1_start;
extern uint64_t __LVL3_1_TRANSLATION_TABLES;

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
	AT(entry_addr) = (AT(entry_addr) & 0xffff000000000fff) | ((inner_addr & 0xfffffffff) << 12);
        //uart_debug("Entry addr = %x\r\n", entry_addr);
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

void set_invalid_entry(uint64_t entry_addr) {
	* (uint64_t *) entry_addr &= MASK(63, 1);
}

void set_invalid_page(uint64_t virtual_addr) {
	// TODO: read physical address and invalidate page
}

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
	// Mark entry as a table entry (cf. ARM ARM 2144)
	entry |= 3;
	//uart_debug("Entry is %x\r\n", entry);
	* ((uint64_t *) addr) = entry;
}

void init_table_entry_sg1(uint64_t entry_addr, uint64_t inner_addr) {
	* ((uint64_t *) entry_addr) =
		// TODO: debug. third access yields fault at address WTF
		// Temporarily remove  previous address. TODO: uncomment
		//((* (uint64_t *) entry_addr) & 0xffff000000000fff) |
		((inner_addr & 0xfffffffff) << 12);
	//uart_debug("%x -> %x\r\n", inner_addr, inner_addr & 0xfffffffff);
	set_table_attributes_sg1(entry_addr, new_table_attributes_sg1());
}

uint64_t get_address_sg1(uint64_t entry_addr) {
	return ((* (uint64_t *) entry_addr) & MASK(47,12)) >> 12;
}

int bind_address(uint64_t virtual_addr, uint64_t physical_addr, block_attributes_sg1 ba) {
        //uart_debug("1\r\n");
	uint64_t lvl2_table_addr = 0;
	if ((physical_addr & MASK(11,0)) != 0)
		return 4;
	if ((virtual_addr & 0xffffc0000000) != 0)
		return 2;
        //uart_debug("2\r\n");
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
        //uart_debug("3\r\n");
	lvl2_table_addr &= 0xfffffffffffe;
	if ((lvl2_table_addr & 0x1fffff) != 0)
		return 3;
        //uart_debug("4\r\n");
	uint64_t lvl2_index  = (virtual_addr & MASK(29,21)) >> 21;
        uint64_t lvl2_offset = 8 * lvl2_index;
	if ((AT(lvl2_table_addr + lvl2_offset) & MASK(1,0)) != 3)
		return 5;
        //uart_debug("5\r\n");
	uint64_t lvl3_table_addr = get_address_sg1(lvl2_table_addr + lvl2_offset);
        //uart_debug("6\r\n");
	uint64_t lvl3_index  = (virtual_addr & MASK(20, 12)) >> 12;
        uint64_t lvl3_offset = 8 * lvl3_index;
        //uart_debug("7\r\n");
        //uart_debug("Addr before fault : %x\r\nTable_addr = %x\r\nOffset = %x\r\n", lvl3_table_addr + lvl3_offset, lvl3_table_addr, lvl3_offset);
	init_block_and_page_entry_sg1(lvl3_table_addr + lvl3_offset, (physical_addr >> 12), ba);
        //uart_debug("8\r\n");
	return 0;
}

void populate_lvl2_table() {
	uint64_t lvl2_address, lvl3_address;
	asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_address) : :);
	lvl3_address = lvl2_address + 0x8000;
	//uart_debug("lvl2_address = %x\r\nlvl3_address = %x\r\n", lvl2_address, lvl3_address);
	for (int i=0; i<512; i++) {
		init_table_entry_sg1(lvl2_address + i * 8, lvl3_address + i * 512 * 8);
	}
	//uart_debug("Populated lvl2 table\r\n");
}

#define RAM_SIZE 1073741824 /* 1 Gio */
#define ID_PAGING_SIZE RAM_SIZE /*2097152 2 Mio */
void identity_paging() {
	populate_lvl2_table();
	/* WARNING : ID_PAGING_SIZE has to be a multiple of 512
	 *           to avoid uninitialized entries in lvl3 table */
	uart_debug("Binding identity\r\n");
	for (uint64_t physical_pnt = 0; physical_pnt < ID_PAGING_SIZE; physical_pnt += 4 * 1024) {
                //uart_debug("Before bind\r\n");
                int status = bind_address(physical_pnt, physical_pnt, new_block_attributes_sg1());
                //uart_debug("Status = %d\r\nPhysical_pnt = %x\r\n", status, physical_pnt);
                assert(!status);
	}
	uart_debug("Binded indentity\r\n");

	uint64_t lvl2_address;
	asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_address) : :);

	uart_debug("Invalidating remaining entries\r\n");
	for (uint64_t invalid_lvl2_table_entries_index = ID_PAGING_SIZE / ((4 * 1024) * 512);
			invalid_lvl2_table_entries_index < 512; invalid_lvl2_table_entries_index++) {
		set_invalid_entry(lvl2_address + invalid_lvl2_table_entries_index * 8);
	}
	uart_debug("Identity paging success\r\n");
	return;
}
