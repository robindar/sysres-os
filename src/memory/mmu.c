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
  /* TODO: Set this back to 0 to avoid unneccessary caching */
  bas1.AccessFlag = 1;
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
  /* EL0 executable, Higher levels RWX */
  bas1.AccessPermission = 0;
  bas1.NonSecure = 1;
  /* Stage 1 memory attributes index field, cache related buisness, cf ARM ARM 2175) */
  bas1.AttrIndex = 0;
  return bas1;
}

void init_block_and_page_entry_sg1(uint64_t entry_addr, uint64_t inner_addr, block_attributes_sg1 ba) {
	AT(entry_addr) = (AT(entry_addr) & (MASK(63,48) | MASK(11,0))) |
		((inner_addr & MASK(35,0)));
	//uart_debug("Entry addr = %x\r\n", entry_addr);
	set_block_and_page_attributes_sg1(entry_addr, ba);
}

void set_block_and_page_attributes_sg1(uint64_t addr, block_attributes_sg1 bas1) {
	uint64_t entr = * ((uint64_t *) addr);
	entr &= MASK(47, 12);
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
	entr |=  3;             /* WARNING : only for level 3 : Set block identifier (ARM ARM 2144)*/
	* ((uint64_t *) addr) = entr;
}

void set_block_and_page_dirty_bit(uint64_t addr) {
	(void) addr;
}
void set_block_and_page_access_flag(uint64_t addr) {
	(void) addr;
}

void set_invalid_entry(uint64_t entry_addr) {
	AT(entry_addr) &= MASK(63, 1);
}

void set_identifier_table_entry(uint64_t entry_addr){
    AT(entry_addr) &= MASK(63, 2) | 0b11;
}

void set_invalid_page(uint64_t virtual_addr) {
	uint64_t physical_addr_lvl3 = get_lvl3_entry_phys_address(virtual_addr);
	uint64_t status = physical_addr_lvl3 & MASK(2,0);
	if (status)
		uart_error("get_lvl3_entry_phys_address for address 0x%x failed with exit status %d\r\n", virtual_addr, status);
	assert(status == 0); /* i.e. no error in previous call */
	set_invalid_entry(physical_addr_lvl3);
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
	entry &= MASK(58, 0);
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
		((* (uint64_t *) entry_addr) & (MASK(63, 48) | MASK(11, 0))) |
		((inner_addr & MASK(35, 0)) ); /* The bit shift here was also a mistake : ARM ARM 2146*/
	//uart_debug("%x -> %x\r\n", inner_addr, inner_addr & 0xfffffffff);
	set_table_attributes_sg1(entry_addr, new_table_attributes_sg1());
}

uint64_t get_address_sg1(uint64_t entry_addr) {
	return (AT(entry_addr) & MASK(47,12)); /* There was a bug here : the bit shift shoudln't have been here
                                                  (implicit at ARM ARM:2105, for table entry at 2146 or see ARMv8-A Address Translation page 9)*/
}

bool is_block_entry(uint64_t entry, int lvl){
	return ((lvl < 3) && (entry & MASK(1,0)) == 1) || ((lvl == 3) && (entry & MASK(1, 0)) == 3);
}

bool is_table_entry(uint64_t entry){
	return ((entry & MASK(1,0)) == 3);
}


/* This function encounters an error iff one of the three first bit of the result is one, ie MASK(2, 0) & result != 0 */
/* See documention of bind_address for the meaning */
uint64_t get_lvl2_table_entry_phys_address(uint64_t virtual_addr){
	uint64_t lvl2_table_addr = 0;
	if ((virtual_addr & MASK(47, 30)) != 0)
		return 2;
	switch (virtual_addr & MASK(63, 48)) {
		case MASK(63, 48):
			asm volatile ("mrs %0, TTBR1_EL1" : "=r"(lvl2_table_addr) : :);
			break;
		case 0:
			asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_table_addr) : :);
			break;
		default:
			return 1;
	}
	lvl2_table_addr &= MASK(47, 1);
	if ((lvl2_table_addr & MASK(16, 0)) != 0)
		return 3;
	uint64_t lvl2_index  = (virtual_addr & MASK(29,21)) >> 21;
	uint64_t lvl2_offset = 8 * lvl2_index;
        return lvl2_table_addr + lvl2_offset;
}
/* This function encounters an error iff one of the three first bit of the result is one, ie MASK(2, 0) & result != 0 */
/* See documention of bind_address for the meaning */

uint64_t get_lvl3_entry_phys_address(uint64_t virtual_addr){
	uint64_t lvl2_table_entry_phys_address = get_lvl2_table_entry_phys_address(virtual_addr);
        if((lvl2_table_entry_phys_address & MASK(2,0)) != 0){ /* An error happened */
            return lvl2_table_entry_phys_address;
        }
	if (!is_table_entry(AT(lvl2_table_entry_phys_address)))
		return 5;
	uint64_t lvl3_table_addr = get_address_sg1(lvl2_table_entry_phys_address);
	uint64_t lvl3_index  = (virtual_addr & MASK(20, 12)) >> 12;
	uint64_t lvl3_offset = 8 * lvl3_index;
	return lvl3_table_addr + lvl3_offset;
}

int bind_address(uint64_t virtual_addr, uint64_t physical_addr, block_attributes_sg1 ba) {
	if ((physical_addr & MASK(11,0)) != 0)
		return 4;
	uint64_t lvl3_entry_phys_address = get_lvl3_entry_phys_address(virtual_addr);
        unsigned int error_code = lvl3_entry_phys_address & MASK(2, 0);
        switch(error_code){
            case 0:
                init_block_and_page_entry_sg1(lvl3_entry_phys_address, physical_addr, ba);
                return 0;
            case 5:
                set_identifier_table_entry(get_lvl2_table_entry_phys_address(virtual_addr));
                return bind_address(virtual_addr, physical_addr, ba); /* Should not lead to infinite loops */
            default:
                return lvl3_entry_phys_address;
        }
}

/* Warning ignores alignement erros encountered by get_lvl3_entry_phys_address */
uint64_t get_physical_address(uint64_t virtual_addr){
	return get_address_sg1(get_lvl3_entry_phys_address(virtual_addr)) + (virtual_addr & MASK(11, 0));
}

void populate_lvl2_table() {
	uint64_t lvl2_address, lvl3_address;
	asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_address) : :);
	lvl3_address = lvl2_address + 2 * 0x1000; /* leave space for lvl2 tables TTBR0/1 */
	assert(lvl2_address % GRANULE == 0);
	//uart_debug("lvl2_address = %x\r\nlvl3_address = %x\r\n", lvl2_address, lvl3_address);
	for (int i=0; i<512; i++) {
		init_table_entry_sg1(lvl2_address + i * 8, lvl3_address + i * GRANULE);
	}
	//uart_debug("Populated lvl2 table\r\n");
}
void one_step_mapping(){
	uint64_t lvl2_address;
	uart_debug("Beginning One step\r\n");
	block_attributes_sg1 ba = new_block_attributes_sg1();
	ba.AccessFlag = 1;
	ba.AccessPermission = 0; /* With 1 it doesn't workn see ARM ARM 2162 */
	asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_address) : :);
	assert(lvl2_address % GRANULE == 0);
	init_block_and_page_entry_sg1(lvl2_address, 0x0, ba);
	/* init_block_and_page_entry_sg1(lvl2_address + 0x1f9, GPIO_BASE, ba); *\\* 0x1f9 : corresponds to bits[29-21] of GPIO_BASE */
	uart_debug("Done One step\r\n");
	return;
}

/*** IDENTITY PAGING ***/
void check_identity_paging(uint64_t id_paging_size){
	uart_debug("Checking identity paging\r\n");
	uint64_t lvl3_entry_phys_addr;
	for (uint64_t physical_pnt = 0; physical_pnt < id_paging_size; physical_pnt += GRANULE) {
		lvl3_entry_phys_addr = get_lvl3_entry_phys_address(physical_pnt);
		if((lvl3_entry_phys_addr & MASK(2, 0)) != 0) {
			uart_error("Error in get_lvl3_phys_address : %d\r\n", lvl3_entry_phys_addr);
			abort();
		}
		if(!is_block_entry(AT(lvl3_entry_phys_addr), 3)){
			uart_error("Not a block entry\r\nPhysical_pnt = %x\r\nLvl3 entry = %x\r\n", physical_pnt, AT(lvl3_entry_phys_addr));
			abort();
		}
		if(get_physical_address(physical_pnt) != physical_pnt){
			uart_error("Physical address = %x\r\nRetrieved physical address = %x\r\n", physical_pnt, get_physical_address(physical_pnt));
			abort();
		}
	}
	uart_debug("Checked identity paging\r\n");
}


void identity_paging() {
	populate_lvl2_table();
	uart_debug("Binding identity\r\n");
	block_attributes_sg1 ba = new_block_attributes_sg1();
	ba.AccessFlag = 1;
	ba.AccessPermission = 0; /* With 1 it doesn't workn see ARM ARM 2162 */
	uint64_t id_paging_size;
	asm volatile ("ldr %0, =__end" : "=r"(id_paging_size) : :);
	for (uint64_t physical_pnt = 0; physical_pnt < id_paging_size; physical_pnt += GRANULE) {
		//uart_debug("Before bind\r\n");
		int status = bind_address(physical_pnt, physical_pnt, ba);
		if (status)
			uart_verbose("Invalid status found at 0x%x : %d\r\n", physical_pnt, status);
		assert(!status);
	}
	uart_debug("Binded indentity\r\n");

	uint64_t lvl2_address;
	asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_address) : :);

	uart_debug("Invalidating remaining entries\r\n");
	uint64_t current_table_index = (id_paging_size / GRANULE) % 512;
	for (uint64_t invalid_lvl2_table_entries_index =
			  id_paging_size / (GRANULE * 512) + (current_table_index == 0 ? 0 : 1);
			invalid_lvl2_table_entries_index < 512; invalid_lvl2_table_entries_index++) {
		set_invalid_entry(lvl2_address + invalid_lvl2_table_entries_index * 8);
	}
	if (current_table_index)
		for ( uint64_t i = 0; i < 512 - current_table_index; i++)
			set_invalid_page(id_paging_size + i * GRANULE);
	uart_debug("Identity paging success\r\n");
	check_identity_paging(id_paging_size);
	return;
}

uint64_t get_physical_memory_map_addr () {
	uint64_t memmap_addr = 0;
	asm volatile ("ldr %0, =__physical_memory_map" : "=r"(memmap_addr) : :);
	uart_verbose("Physical memory map is at 0x%x", memmap_addr);
	return memmap_addr;
}

struct physical_memory_map_t physical_memory_map;

#define SKIPPED_PAGES 3
void init_physical_memory_map (uint64_t id_paging_size) {
	uint64_t memmap_addr = get_physical_memory_map_addr ();
	physical_memory_map.map = (uint32_t *) memmap_addr;
	physical_memory_map.head = 0;
	physical_memory_map.size = 0x100000;

	uint32_t delta = 0; // Number of pages skipped for uart
	for (uint32_t i = 0; i < (RAM_SIZE - id_paging_size) / GRANULE - SKIPPED_PAGES; i++) {
		/* skip SKIPPED_PAGES pages for uart */
		switch ( (i + delta) * GRANULE + id_paging_size) {
			case GPIO_BASE:
			case GPIO_BASE + 0x1000:
			case GPIO_BASE + 0x15000:
				delta += 1;
				break;
		}
		physical_memory_map.map[ physical_memory_map.head + i ] = (i + delta) * GRANULE + id_paging_size;
	}
}

static inline uint64_t get_unbound_physical_page() {
	return physical_memory_map.map[ physical_memory_map.head++ ];
}

uint64_t get_new_page(uint64_t virtual_address) {
	uint64_t physical_address = get_unbound_physical_page();
	bind_address(virtual_address, physical_address, new_block_attributes_sg1());
}

void free_page(uint64_t physical_addr) {
	assert(physical_memory_map.head > 0);
	assert(physical_addr % GRANULE == 0);
	physical_memory_map.map[ --physical_memory_map.head ] = physical_addr;
}

void translation_fault_handler(uint64_t fault_address, int level, bool lower_el){
	if (!lower_el) {
		get_new_page(fault_address);
	}
};
