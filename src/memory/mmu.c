#include "mmu.h"

/* To be able to reuse these functions for procs we will need this*/
/* Thus we'll be able to manipulate tables without the need for TTBRO_EL1 to be set*/
/* Which is necessary for MMU initialization for procs */
/* So we can keep kernel MMU on during setup */
static uint64_t lvl2_table_address;
/* See doc/proc.md for where it is initialized */

static volatile uint64_t id_paging_size;
#define MMU_VERBOSE
#ifdef MMU_VERBOSE
#define mmu_verbose(...) uart_verbose(__VA_ARGS__)
#else
#define mmu_verbose(...) ((void) 0)
#endif

block_attributes_sg1 new_block_attributes_sg1(enum block_perm_config perm_config, enum block_cache_config cache_config) {
    block_attributes_sg1 bas1;
    bas1.UXN = (perm_config >> 3) & 1;
    bas1.PXN = (perm_config >> 2) & 1;
    bas1.ContinuousBit = 0;
    bas1.DirtyBit = 0;
    /* For kernel, we set as global */
    bas1.NotGlobal = ((perm_config & MASK(3, 0)) == KERNEL_PAGE || (perm_config & MASK(3, 0)) == IO_PAGE) ? 0 : 1;
    bas1.AccessFlag = (perm_config >> 4) & 1;
    /* Shareability
     * 00 : Non-shareable
     * 01 : unpredictable
     * 10 : Inner shareable
     * 11 : Outer shareable
     The meaning of Inner/Outer shareable is controled by the Cache Level ID register. See ARM ARM 2405
     */
    bas1.Shareability = 0;
    bas1.AccessPermission = perm_config & 0b11;
    bas1.NonSecure = 1;
    /* Stage 1 memory attributes index field, cache related buisness, cf ARM ARM 2175) */
    bas1.AttrIndex = cache_config;
    return bas1;
}

void init_block_and_page_entry_sg1(uint64_t entry_addr, uint64_t inner_addr, block_attributes_sg1 ba) {
    AT(entry_addr) = (AT(entry_addr) & (MASK(63,48) | MASK(11,0))) |
        ((inner_addr & MASK(35,0)));
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

/* Set dirty bit to 1 */
void set_entry_dirty_bit(uint64_t entry_addr) {
    AT(entry_addr) |= (((uint64_t) 1) << 51);
}

/* Set access flag to 1 */
void set_entry_access_flag(uint64_t entry_addr) {
    AT(entry_addr) |= (1 << 10);
}

void clear_entry_access_flag(uint64_t entry_addr){
    AT(entry_addr) &= ~(1 << 10);
}

void set_invalid_entry(uint64_t entry_addr) {
    AT(entry_addr) &= MASK(63, 1);
}

void set_identifier_table_entry(uint64_t entry_addr){
    AT(entry_addr) |= 0b11;
}

void set_invalid_page(uint64_t virtual_addr) {
    uint64_t physical_addr_lvl3 = get_lvl3_entry_phys_address(virtual_addr);
    uint64_t status = physical_addr_lvl3 & MASK(2,0);
    if (status)
        uart_error("set_invalid_page : get_lvl3_entry_phys_address for address 0x%x failed with exit status %d\r\n", virtual_addr, status);
    assert(status == 0); /* i.e. no error in previous call */
    set_invalid_entry(physical_addr_lvl3);
}

void set_page_access_flag(uint64_t virtual_addr){
    uint64_t physical_addr_lvl3 = get_lvl3_entry_phys_address(virtual_addr);
    uint64_t status = physical_addr_lvl3 & MASK(2,0);
    if (status)
        uart_error("set_page_access_flag : get_lvl3_entry_phys_address for address 0x%x failed with exit status %d\r\n", virtual_addr, status);
    assert(status == 0); /* i.e. no error in previous call */
    set_entry_access_flag(physical_addr_lvl3);
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
    ta1.APTable  = 0;
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
    * ((uint64_t *) addr) = entry;
}

void init_table_entry_sg1(uint64_t entry_addr, uint64_t inner_addr) {
    * ((uint64_t *) entry_addr) =
        ((* (uint64_t *) entry_addr) & (MASK(63, 48) | MASK(11, 0))) |
        ((inner_addr & MASK(35, 0)) ); /* The bit shift here was also a mistake : ARM ARM 2146*/
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
    if ((virtual_addr & MASK(47, 30)) != 0)
        return 2;
    /* FOR NOW TTBR1 is DEPRECATED */
    /* switch (virtual_addr & MASK(63, 48)) { */
    /*  case MASK(63, 48): */
    /*      asm volatile ("mrs %0, TTBR1_EL1" : "=r"(lvl2_table_addr) : :); */
    /*      break; */
    /*  case 0: */
    /*      asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_table_addr) : :); */
    /*      break; */
    /*  default: */
    /*      return 1; */
    /* } */
    switch (virtual_addr & MASK(63, 48)) {
        case MASK(63, 48):
            uart_error("TTBR1 is unused for now but virtual address has bits 63-48 set to 1\r\n");
            return 1;
        case 0:
            /* Everything's fine */
            break;
        default:
            return 1;
    }
    if ((lvl2_table_address & MASK(16, 0)) != 0)
        return 3;
    uint64_t lvl2_index  = (virtual_addr & MASK(29,21)) >> 21;
    uint64_t lvl2_offset = 8 * lvl2_index;
    return lvl2_table_address + lvl2_offset;
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

void init_new_lvl3_table(uint64_t lvl2_entry_phys_address){
    mmu_verbose("Initializing new table at %x\r\n", lvl2_entry_phys_address);
    set_identifier_table_entry(lvl2_entry_phys_address);
    assert(is_table_entry(AT(lvl2_entry_phys_address)));
    uint64_t lvl3_entry_phys_address;
    for(int i = 0; i < 512; i++){
        lvl3_entry_phys_address = get_address_sg1(lvl2_entry_phys_address) + i * sizeof(uint64_t);
        set_invalid_entry(lvl3_entry_phys_address);
    }
    mmu_verbose("New table initialization done\r\n");
}

uint64_t get_physical_memory_map_addr () {
    uint64_t memmap_addr = 0;
    asm volatile ("ldr %0, =__physical_memory_map" : "=r"(memmap_addr) : :);
    mmu_verbose("Physical memory map is at 0x%x\r\n", memmap_addr);
    return memmap_addr;
}

static struct physical_memory_map_t physical_memory_map;

uint8_t * get_bind_counter_address (uint64_t physical_addr) {
    return physical_memory_map.bind_counter +
      (physical_addr / GRANULE) - physical_memory_map.bind_counter_offset;
}

int get_bind_counter (uint64_t physical_addr) {
    return (int) * get_bind_counter_address(physical_addr);
}

void set_bind_counter (uint64_t physical_addr, int value) {
    * get_bind_counter_address(physical_addr) = value;
}

void increment_bind_counter (uint64_t physical_addr) {
    mmu_verbose("Incrementing bind counter at 0x%x for address 0x%x\r\n", get_bind_counter_address(physical_addr), physical_addr);
    assert((uint64_t) get_bind_counter_address(physical_addr) < id_paging_size);
    (* get_bind_counter_address(physical_addr)) ++;
}

void decrement_bind_counter (uint64_t physical_addr) {
    mmu_verbose("Decrementing bind counter at 0x%x for address 0x%x\r\n", get_bind_counter_address(physical_addr), physical_addr);
    (* get_bind_counter_address(physical_addr)) --;
}

int bind_address(uint64_t virtual_addr, uint64_t physical_addr, block_attributes_sg1 ba) {
    if ((physical_addr & MASK(11,0)) != 0)
        return 4;
    uint64_t lvl3_entry_phys_address = get_lvl3_entry_phys_address(virtual_addr);
    uint64_t lvl2_entry_phys_address;
    unsigned int error_code = lvl3_entry_phys_address & MASK(2, 0);
    switch(error_code){
        case 0:
            init_block_and_page_entry_sg1(lvl3_entry_phys_address, physical_addr, ba);
            if (physical_addr >= id_paging_size)
                increment_bind_counter(physical_addr);
            return 0;
        case 5:
            /* Invalid table : we init a new one */
            lvl2_entry_phys_address = get_lvl2_table_entry_phys_address(virtual_addr);
            init_new_lvl3_table(lvl2_entry_phys_address);
            /* Should not lead to infinite loops */
            return bind_address(virtual_addr, physical_addr, ba);
        default:
            return lvl3_entry_phys_address;
    }
}

/* Warning ignores alignement erros encountered by get_lvl3_entry_phys_address */
uint64_t get_physical_address(uint64_t virtual_addr){
    uint64_t lvl3_entry_phys_addr = get_lvl3_entry_phys_address(virtual_addr);
    int status = lvl3_entry_phys_addr & MASK(2,0);
    assert(status == 0);
    return get_address_sg1(lvl3_entry_phys_addr) + (virtual_addr & MASK(11, 0));
}

uint64_t get_ASID_from_TTBR0(uint64_t ttbr0_el1){
    return (ttbr0_el1 & MASK(63, 48)) >> 48;
}

uint64_t get_lvl2_table_address_from_TTBR0(uint64_t ttbr0_el1){
    return (ttbr0_el1 & MASK(47,1));
}

void set_lvl2_address(uint64_t lvl2_addr){
    lvl2_table_address = lvl2_addr;
}

void set_lvl2_address_from_pid(int pid){
    set_lvl2_address(get_lvl2_address_from_sys_state(pid));
}


void set_lvl2_address_from_TTBR0_EL1(){
    uint64_t lvl2_address;
    asm volatile ("mrs %0, TTBR0_EL1" : "=r"(lvl2_address) : :);
    mmu_verbose("Setting global var lvl2_table_address for ASID %d\r\n", get_ASID_from_TTBR0(lvl2_address));
    /* Remove ASID (these func will be reused for procs) */
    lvl2_address &= MASK(47,1);
    lvl2_table_address = lvl2_address;
    return;
}

void populate_lvl2_table(uint64_t lvl3_address) {
    /* TODO : improve this -> tables should be allocated on demand : change bind_address */
    assert(lvl2_table_address % GRANULE == 0);
    assert(lvl3_address % GRANULE == 0);
    for (int i=0; i<N_TABLE_ENTRIES - 1; i++) { /* As explained in doc/mmu.md, we only have N_TABLE_ENTRIES lvl3 tables */
        init_table_entry_sg1(lvl2_table_address + i * sizeof(uint64_t), lvl3_address + i * GRANULE);
    }
    uart_debug("Populated lvl2 table\r\n");
}

/* For debugging purposes only */
/* DEPRECATED for procs */
void one_step_mapping(){
    uint64_t lvl2_address;
    uart_debug("Beginning One step\r\n");
    block_attributes_sg1 ba = new_block_attributes_sg1(KERNEL_PAGE, NORMAL_WT_NT);
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
void check_identity_paging() {
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

void map_peripheral_pages(){
    block_attributes_sg1 ba = new_block_attributes_sg1(IO_PAGE | ACCESS_FLAG_SET, DEVICE);
    /* Warning Access falg is set to 1 : you can set it to zero if you want but make sure the Access flag fault handling uses no uart o/w you'll end up in an infinite loop */
    bind_address(GPIO_BASE,GPIO_BASE, ba);
    bind_address(GPIO_BASE + 0x1000,GPIO_BASE + 0x1000, ba);
    bind_address(GPIO_BASE + 0x15000,GPIO_BASE + 0x15000, ba);
    ba = new_block_attributes_sg1(KERNEL_PAGE | ACCESS_FLAG_SET, DEVICE);
    bind_address(TIMER_IRQ_PAGE, TIMER_IRQ_PAGE, ba);
}

void map_table_zone(block_attributes_sg1 ba){
    uint64_t mmu_tables_start, physical_pnt;
    int status;
    asm volatile ("ldr %0, =__mmu_tables_start" : "=r"(mmu_tables_start) : :);
    mmu_verbose("Binding memory tables zone\r\n");
    /* Tables zones : data_end (~0xb000) - id_paging_size (~0x4300000) */
    /* Conatin : mmu tables, phys memory map*/
    /* See ARM ARM 2178 for the pages alignement constraints : for GRANULE = 4kB, 16 is needed */
    ba.ContinuousBit = 1;
    uint64_t n_pages = (id_paging_size - mmu_tables_start) / GRANULE;
    /* Everything has to be 512 pages = 0x200000 aligned for tables */
    /* (hence we don't have to invalidate any entry in lvl3_tables) */
    assert(n_pages % 512 == 0);
    for (physical_pnt = mmu_tables_start; physical_pnt < id_paging_size; physical_pnt += GRANULE) {
        if((physical_pnt - mmu_tables_start) / GRANULE > 16 * (n_pages / 16)) ba.ContinuousBit = 0;
        status = bind_address(physical_pnt, physical_pnt, ba);
        if (status)
            uart_error("Invalid status found at 0x%x : %d\r\n", physical_pnt, status);
    }
}

/* This paramter is used to enable one process, the mem. manager */
/* to access translation tables */
void identity_paging() {
    uart_info("Binding identity\r\n");
    block_attributes_sg1 kernel_ba = new_block_attributes_sg1(KERNEL_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
    /* For debugging (so gdb can read code whatever the level) TODO : REMOVE ? */
    block_attributes_sg1 code_ba   = new_block_attributes_sg1(  CODE_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
    block_attributes_sg1 data_ba   = new_block_attributes_sg1(  DATA_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
    uint64_t data_start, data_end;
    /* Everything is GRANULE aligned */
    asm volatile ("ldr %0, =__data_start" : "=r"(data_start) : :);
    asm volatile ("ldr %0, =__data_end" : "=r"(data_end) : :);
    mmu_verbose("Id paging size : 0x%x\r\n", id_paging_size);
    mmu_verbose("__data_start : 0x%x\r\n", data_start);
    mmu_verbose("__data_end : 0x%x\r\n", data_end);
    uint64_t mmu_tables_start;
    asm volatile ("ldr %0, =__mmu_tables_start" : "=r"(mmu_tables_start) : :);
    int status;
    uint64_t physical_pnt;
    mmu_verbose("Binding kernel zone\r\n");
    /* Kernel zone : 0x0 - data_start (~0x6000) */
    for (physical_pnt = 0; physical_pnt < data_start; physical_pnt += GRANULE) {
        status = bind_address(physical_pnt, physical_pnt, code_ba);
        if (status)
            mmu_verbose("Invalid status found at 0x%x : %d\r\n", physical_pnt, status);
    }
    mmu_verbose("Binding data zone\r\n");
    /* Data zone : data_start (~0x6000) - data_end (~0xb000) */
    /* WARNING : we go until mmu tables start so that map_table_zone can work */
    /* on a 512 * GRANULE aligned zone */
    for (physical_pnt = data_start; physical_pnt < mmu_tables_start; physical_pnt += GRANULE) {
        status = bind_address(physical_pnt, physical_pnt, data_ba);
        if (status)
            mmu_verbose("Invalid status found at 0x%x : %d\r\n", physical_pnt, status);
    }
    map_table_zone(kernel_ba);
    uart_info("Binded indentity\r\n");

    uart_info("Invalidating remaining entries\r\n");
    uint64_t current_table_index = (id_paging_size / GRANULE) % 512;
    for (uint64_t invalid_lvl2_table_entries_index =
            id_paging_size / (GRANULE * 512) + (current_table_index == 0 ? 0 : 1);
            invalid_lvl2_table_entries_index < 512; invalid_lvl2_table_entries_index++) {
        set_invalid_entry(lvl2_table_address + invalid_lvl2_table_entries_index * 8);
    }
    if (current_table_index)
        for ( uint64_t i = 0; i < 512 - current_table_index; i++)
            set_invalid_page(id_paging_size + i * GRANULE);
}

#define SKIPPED_PAGES 3
void init_physical_memory_map () {
    uint64_t memmap_addr = get_physical_memory_map_addr ();
    physical_memory_map.map = (uint32_t *) memmap_addr;
    physical_memory_map.head = 0;
    physical_memory_map.size = 0x100000; /* = sizeof(uint32_t) * 512 * 512 -> maybe overkill */
    physical_memory_map.bind_counter_offset = id_paging_size / GRANULE;
    physical_memory_map.bind_counter = (uint8_t *) physical_memory_map.map + physical_memory_map.size;
    for (uint64_t i = id_paging_size / GRANULE; i < 512 * 512; i++) {
        set_bind_counter(i * GRANULE, 0);
    }
    uint32_t delta = 0; // Number of pages skipped for uart
    for (uint32_t i = 0; i < (RAM_SIZE - id_paging_size) / GRANULE - SKIPPED_PAGES; i++) {

        /* skip SKIPPED_PAGES pages for peripherals */
        switch ( (i + delta) * GRANULE + id_paging_size) {
            case GPIO_BASE:
            case GPIO_BASE + 0x1000:
            case GPIO_BASE + 0x15000:
            case TIMER_IRQ_PAGE:
                delta += 1;
                break;
        }
        physical_memory_map.map[ physical_memory_map.head + i] = (i + delta) * GRANULE + id_paging_size;
    }
}

/* Cache initialization */
void init_cache(){
    uart_info("Beginning Cache Initialization\r\n");
    /* Invalidate instruction cache entries (see ARMv8-A Programmer Guide : 115)*/
    /* asm volatile("IC IALLU"); -> this hasn't got anthing to do here ?*/

    /* Init MAIR_EL1 according to choices described in doc/mmu.md for page caching*/
    uint64_t reg = 0;
    reg |= ((uint64_t) 0b10111011) << (8 * 1);
    reg |= ((uint64_t) 0b00110011) << (8 * 2);
    reg |= ((uint64_t) 0b11111111) << (8 * 3);
    reg |= ((uint64_t) 0b01110111) << (8 * 4);
    reg |= ((uint64_t) 0b01000100) << (8 * 5);
    asm volatile("msr mair_el1, %0" : : "r"(reg) :);
    /* Table caching see doc/mmu.md*/
    /* Currently the config is 01 */
    uint64_t config = 0b10;
    asm volatile("mrs %0, tcr_el1" : "=r"(reg) : :);
    /* Clear fields : see ARM ARM 2685 for enconding */
    /* Taking care of TTBRO/TTBR1, Inner/Outer */
    reg &= ~MASK(27, 26);
    reg &= ~MASK(25, 24);
    reg &= ~MASK(11, 10);
    reg &= ~MASK(9, 8);
    /* Setting fields */
    reg |= (config << 26);
    reg |= (config << 24);
    reg |= (config << 10);
    reg |= (config << 8);
    asm volatile("msr tcr_el1, %0" : : "r"(reg) :);
    /* All entries are invalidated in boot.s (has to be done at > EL1) */
    uart_info("Cache Initialization Done\r\n");
}

/* function to be used in proc_init_copy_and_write */
void copy_forked_page(uint64_t page){
    mmu_verbose("Copy forked page: 0x%x\r\n", page);
    assert(page % GRANULE == 0);
    /* this is trickier than the handler for the permission fault*/
    /*(you may want to read this one first) */
    /* we are running in kernel but with lvl2_table_addr for the proc */
    /* so we get the phys addr of this page */
    int status;
    uint64_t phys_address = get_physical_address(page);
    int cnt = get_bind_counter(phys_address);
    assert(cnt == 2);
    decrement_bind_counter(phys_address);
    uint64_t backup_lvl2_table_addr, kernel_lvl2_table_addr;
    kernel_lvl2_table_addr = get_lvl2_address_from_sys_state(KERNEL_PID);
    uint64_t check;
    asm("ldr %0, =__mmu_tables_start":"=r"(check));
    /* I'm sorry I can't help but check */
    /* TODO : remove after tests (keep get_lvl2...)*/
    assert(check == kernel_lvl2_table_addr);
    backup_lvl2_table_addr = lvl2_table_address;
    /* switch to kernel lvl2 addr */
    lvl2_table_address = kernel_lvl2_table_addr;
    /* map this page in kernel memory */
    block_attributes_sg1 kernel_ba =
        new_block_attributes_sg1(KERNEL_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
    status = bind_address(PAGE_RES_1, phys_address, kernel_ba);
    assert(status == 0);
    /* get new page and map it somewhere else */
    status = get_new_page(PAGE_RES_2,
                     KERNEL_PAGE | ACCESS_FLAG_SET,
                     NORMAL_WT_NT);
    assert(status == 0);
    uint64_t new_phys_address = get_physical_address(PAGE_RES_2);
    /* copy data */
    memmove((void *) PAGE_RES_2, (void *) PAGE_RES_1, GRANULE);
    #ifdef STACK_DUMP
    if(page == STACK_PAGE_1){
        for(int i = 1; i <= 10; i++)
            uart_verbose("Parent stack at 0x%x: 0x%x\r\n", STACK_PAGE_1 + GRANULE - i * 8, AT(PAGE_RES_1 + GRANULE - i * 8));
    }
    if(page == STACK_PAGE_1){
        for(int i = 1; i <= 10; i++)
            uart_verbose("Child stack at 0x%x: 0x%x\r\n", STACK_PAGE_1 + GRANULE - i * 8, AT(PAGE_RES_2 + GRANULE - i * 8));
    }
    assert(memcmp((void *) PAGE_RES_2, (void *) PAGE_RES_1, GRANULE) == 0);
    #endif
    block_attributes_sg1 usr_ba =
            new_block_attributes_sg1(USER_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
    /* invalidate evrythings in kernel */
    /* decrement bind counters as bind_addr and get_new_page increment them */
    decrement_bind_counter(phys_address);
    decrement_bind_counter(new_phys_address);
    set_invalid_page(PAGE_RES_1);
    set_invalid_page(PAGE_RES_2);;
    asm volatile("TLBI VAE1  , %0" : : "r"(PAGE_RES_1/GRANULE) :);
    asm volatile("TLBI VAE1  , %0" : : "r"(PAGE_RES_2/GRANULE) :);
    /* switch to proc lvl2 addr */
    lvl2_table_address = backup_lvl2_table_addr;
    /* replace the page by this one */
    bind_address(page, new_phys_address, usr_ba);
    return;
}

void process_init_copy_and_write(int pid){
    mmu_verbose("Beginning process_init_copy_and_write\r\n");
    uint64_t parent_lvl2_table_addr = get_lvl2_address_from_sys_state(get_parent_pid(pid));
    /* Copy kernel lvl2 entries*/
    uint64_t mmu_tables_start;
    asm volatile ("ldr %0, =__mmu_tables_start" : "=r"(mmu_tables_start) : :);
    /* id paging size is  GRANULE * N_TABLE_ENTRIES = 0x200000 aligned, same for mmu_tables_start */
    assert(id_paging_size   % (GRANULE * N_TABLE_ENTRIES) == 0);
    assert(mmu_tables_start % (GRANULE * N_TABLE_ENTRIES) == 0);
    int n_kernel_tables =
        (pid == MEM_MANAGER_PID ? mmu_tables_start : id_paging_size)
        / (GRANULE * N_TABLE_ENTRIES);
    /* No table allocated except for periph after this */
    assert(GPIO_BASE % (GRANULE * N_TABLE_ENTRIES) == 0);
    int n_available_tables = GPIO_BASE / (GRANULE * N_TABLE_ENTRIES);
    mmu_verbose("Process: Mapping kernel tables\r\n");
    for(int i = 0; i < n_kernel_tables; i++)
        AT(lvl2_table_address + i * sizeof(uint64_t)) =
            AT(parent_lvl2_table_addr + i * sizeof(uint64_t));
    /* If it's for the memory manager, it needs to have RW rights on the table zone */
    if(pid == MEM_MANAGER_PID){
        mmu_verbose("Process: Mapping tables\r\n");
        block_attributes_sg1 ba = new_block_attributes_sg1(
            DATA_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
        map_table_zone(ba);
        /* Put back n_kernel_tables to normal no to map the table zone twice */
        n_kernel_tables = id_paging_size / (GRANULE * N_TABLE_ENTRIES);
    }
    uint64_t parent_lvl2_entry_addr, parent_lvl3_entry_addr;
    uint64_t lvl2_entry_addr, lvl3_entry_addr;
    uint64_t virtual_addr;
    block_attributes_sg1 forked_page =
        new_block_attributes_sg1(FORKED_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
    mmu_verbose("Process: Beginning copy_and_write\r\n");
    for(int i = n_kernel_tables; i < n_available_tables; i++){
        /* Iterating through lvl2 entries */
        parent_lvl2_entry_addr = parent_lvl2_table_addr + i * sizeof(uint64_t);
        lvl2_entry_addr = lvl2_table_address + i * sizeof(uint64_t);
        if(!is_table_entry(AT(parent_lvl2_entry_addr))){
            set_invalid_entry(lvl2_entry_addr);
            continue;
        }
        /* Valid table entry ie used */
        for(int j = 0; j < N_TABLE_ENTRIES; j++){
            parent_lvl3_entry_addr =
                get_address_sg1(parent_lvl2_entry_addr) + j * sizeof(uint64_t);
            lvl3_entry_addr =
                get_address_sg1(lvl2_entry_addr) + j * sizeof(uint64_t);
            virtual_addr = GRANULE * (i * N_TABLE_ENTRIES + j);
            if(!is_block_entry(AT(parent_lvl3_entry_addr), 3)){
                set_invalid_entry(lvl3_entry_addr);
                continue;
            }
            if(virtual_addr == TIMER_IRQ_PAGE)
                continue;
            /* Valid block entry, ie used */
            if(virtual_addr != STACK_PAGE_1 && virtual_addr != STACK_PAGE_2)
                /* Switch parent entry to R-X R-X */
                set_block_and_page_attributes_sg1(
                    parent_lvl3_entry_addr, forked_page);
            /* if it's the stack we don't change parent permission */
            AT(lvl3_entry_addr) = AT(parent_lvl3_entry_addr);
            if(get_address_sg1(lvl3_entry_addr) > id_paging_size)
                mmu_verbose("Phys addr: 0x%x\r\n"
                            "i: %d\r\n"
                            "j: %d\r\n"
                            "Virtual addr: 0x%x\r\n"
                            "is_table_entry: %d\r\n"
                            "is_block_entry: %d\r\n"
                            , get_address_sg1(lvl3_entry_addr), i, j,
                            i * GRANULE * N_TABLE_ENTRIES + j * GRANULE,
                            is_table_entry(AT(parent_lvl2_entry_addr)),
                            is_block_entry(AT(parent_lvl3_entry_addr), 3)
                    );
            increment_bind_counter(get_address_sg1(lvl3_entry_addr));
        }
    }
    /* the stack can't be just let in RX/RX */
    copy_forked_page(STACK_PAGE_1);
    copy_forked_page(STACK_PAGE_2);
    mmu_verbose("proc_init_copy_and_write done\r\n");
    return;
}


uint64_t c_init_mmu(uint64_t pid){
    assert(pid < 32);            /* Before removing this to increase he nb of procs, make sure you've taken care of that dear linker.ld */
    /* clean cache */
    asm volatile("TLBI ASIDE1, %0" : : "r"(pid << 48));
    asm volatile("ISB");
    uint64_t tmp;
    asm volatile ("ldr %0, =__end" : "=r"(tmp) : :);
    id_paging_size = tmp;
    mmu_verbose("Got id paging size : 0x%x at 0x%x\r\n", id_paging_size, &id_paging_size);
    uart_info("Beginning C MMU initialization for process of PID %d\r\n", pid);
    uint64_t mmu_tables_start;
    asm volatile("ldr %0, =__mmu_tables_start" : "=r"(mmu_tables_start) : :);
    lvl2_table_address = mmu_tables_start + pid * N_TABLE_ENTRIES * GRANULE;
    mmu_verbose("Lvl2 table address : 0x%x\r\n", lvl2_table_address);
    populate_lvl2_table(lvl2_table_address + GRANULE);
    mmu_verbose("Done populated_lvl2_table\r\n");
    if(pid == KERNEL_PID || pid == INIT_PID){
        if(pid == KERNEL_PID){
            /* One memory map to rule them all ie we only use the one set up by process 0*/
            init_physical_memory_map();
            /* For PID > 0, MAIR_EL1 and TCR_EL1 still control addr translation so no need to init them*/
            init_cache();
        }
        identity_paging();
        /* Maybe remove the next line later */
        check_identity_paging();
        /* Stack Initialization */
        int status;
        status = get_new_page(STACK_PAGE_1, DATA_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT) & MASK(2, 0);
        if(status)
            uart_error("Error during stack initialization with status : %d\r\n", status);
        status = get_new_page(STACK_PAGE_2, KERNEL_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT) & MASK(2, 0);
        if(status)
            uart_error("Error during stack initialization with status : %d\r\n", status);
    }
    else process_init_copy_and_write(pid);
    /* Map GPIO and TIMER*/
    map_peripheral_pages();
    /* clean cache */
    uart_info("C MMU Init success for process of PID %d\r\n", pid);
    return lvl2_table_address;
}


uint64_t get_unbound_physical_page() {
    assert(physical_memory_map.head < physical_memory_map.size);
    uint64_t res = physical_memory_map.map[ physical_memory_map.head++ ];
    return res;
}

void pmapdump() {
    mmu_verbose("Physical memory map structure at %x\r\n%x\r\n%x\r\n%x\r\n",
            (uint64_t) &physical_memory_map, (uint64_t)physical_memory_map.map, physical_memory_map.head, physical_memory_map.size);
}

/* Returns bind_address return code */
int get_new_page(uint64_t virtual_address, enum block_perm_config block_perm, enum block_cache_config cache_config) {
    mmu_verbose("Get_new_page called\r\n");
    uint64_t physical_address = get_unbound_physical_page();
    mmu_verbose("Got a new physical page at 0x%x\r\n", physical_address);
    uint64_t status = bind_address(virtual_address, physical_address, new_block_attributes_sg1(block_perm, cache_config));
    return status;
}

/* Warning : do not use directly : use only inside free_virtual_page */
void free_physical_page(uint64_t physical_addr) {
    assert(physical_memory_map.head > 0);
    assert(physical_addr % GRANULE == 0);
    assert(get_bind_counter(physical_addr) == 0);
    physical_memory_map.map[ --physical_memory_map.head ] = physical_addr;
}

void decr_and_free_physical_page(uint64_t physical_address){
    decrement_bind_counter(physical_address);
    if(get_bind_counter(physical_address) == 0)
        free_physical_page(physical_address);
}

/* returns get_lvl3_address status */
/* free the page from any inside address : ie given any virtual address, frees the page that contains it */
int free_virtual_page(uint64_t virtual_addr) {
    uint64_t lvl3_entry_phys_address = get_lvl3_entry_phys_address(virtual_addr);
    uint64_t physical_address = get_address_sg1(lvl3_entry_phys_address);
    set_invalid_entry(lvl3_entry_phys_address);
    mmu_verbose(
            "Freeing page containing virtual address 0x%x at physical address 0x%x\r\n"
            ,virtual_addr, physical_address);
    /* for now TODO : remove ?*/
    assert(physical_address >= id_paging_size);
    if (physical_address >= id_paging_size)
        decr_and_free_physical_page(physical_address);
    /* Clear TLB entry (note this clears only for the current ASID) (see ARMv8-A Address Translation p27)*/
    asm volatile("DSB ISHST");
    /* For details see ARM ARM 335 */
    uint64_t ttbr0_el1, asid;
    asm volatile("mrs %0, TTBR0_EL1":"=r"(ttbr0_el1)::);
    asid = get_ASID_from_TTBR0(ttbr0_el1);
    uint64_t arg = (asid << 48) | (virtual_addr/GRANULE);
    if((AT(lvl3_entry_phys_address) & MASK(9,8)) != 0)
        /* ie if Shareability >= Inner Shareable, then invalidate Inner Sahreable */
        asm volatile("TLBI VAE1IS, %0" : : "r"(arg) :);
    else
        asm volatile("TLBI VAE1  , %0" : : "r"(arg) :);
    /* Clean data cache */
    /* (the next line doesn't actually change anything (on HW) for malloc_test) */
    asm volatile("DC CIVAC, %0" : : "r"(virtual_addr));
    /* On hardware, with tis code, we don't pass malloc_test anymore, without it we pass */
    /* See ARM programmer manual and ARM ARM 2424*/
    /* uint64_t begin_page = virtual_addr & MASK(63, 12); */
    /* for(int i = 0; i < 0x100; i++) asm volatile("DC CIVAC, %0" : : "r"(begin_page + i * 0x10) :); */
    /* Data sync barrier (inner shareable, ARM ARM 100) */
    asm volatile("DSB ISH");
    asm volatile("ISB");
    int status = lvl3_entry_phys_address & MASK(2, 0);
    return status;
}

void translation_fault_handler(uint64_t fault_address, int level, bool lower_el, int pid) {
    (void) level;
    mmu_verbose("Translation fault handler called at %s from PID %d\r\n", lower_el ? "lower EL" : "same EL", pid);
    /* For now no distnction between EL, the same difference is TTBR0 which is detected auto */
    lvl2_table_address = get_lvl2_address_from_sys_state(pid);
    uint64_t stack_pointer = 0;
    /* Don't forget to backup SPSel, you don't know who called (yet we are at EL1 for sure)*/
    uint64_t SPSel = lower_el ? 0 : 1;
    asm volatile (
        "mrs x1, spsel;" \
        "msr spsel, %1;" \
        "mov x0, sp;" \
        "msr spsel, x1;" \
        "mov %0, x0"
        : "=r"(stack_pointer)
        :  "r"(SPSel)
        : "x0", "x1" );
    /* The stack can't "claim" more pages : this is only used for heap memory and to detect stack overflow */
    if (stack_pointer < STACK_END &&
        fault_address > stack_pointer && fault_address < STACK_END){
        err.no = STACK_OVERFLOW;
        uart_error("STACK_OVERFLOW :\r\nFAR : 0x%x\r\nSP : 0x%x\r\nSTACK_END : 0x%x\r\n", fault_address, stack_pointer, STACK_END);
    }
    if (!(fault_address >= get_heap_begin() &&
        fault_address < get_heap_begin() + get_end_offset())) {
        err.no = SEG_FAULT;
        mmu_verbose("SEGFAULT :\r\nFAR : 0x%x\r\nheap_begin : 0x%x\r\nend_offset : 0x%x\r\n", fault_address, get_heap_begin(), get_end_offset());
        assert(0); // TODO: SEGFAULT - kill process
    }
    if (!lower_el)
        get_new_page(fault_address, KERNEL_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
    else
        get_new_page(fault_address,   USER_PAGE | ACCESS_FLAG_SET, NORMAL_WT_NT);
    asm volatile("ISB");
    mmu_verbose("Translation fault handler returns\r\n");
    return;
};

void access_flag_fault_lvl3_handler(uint64_t fault_address, int level, bool lower_lvl, int pid) {
    (void) level;
    (void) lower_lvl;
    mmu_verbose("Access flag fault handler called\r\n");
    lvl2_table_address = get_lvl2_address_from_sys_state(pid);
    set_page_access_flag(fault_address);    return;
}

void handle_shared_page_write(uint64_t fault_address, bool lower_lvl){
    /* A permission fault in this zone is necessqrily a write on a shared page */
    uint64_t phys_address = get_physical_address(fault_address);
    int cnt = get_bind_counter(phys_address);
    assert(cnt > 0);
    if(cnt > 1){
        /* the page is shared */
        mmu_verbose("Shared page : allocating a new one\r\n");
        decrement_bind_counter(phys_address);
        /* get new page and map it somewhere else */
        /* TODO: make constraint */
        uint64_t temp_addr = PAGE_RES_1;
        int status = get_new_page(temp_addr,
                     KERNEL_PAGE | ACCESS_FLAG_SET,
                     NORMAL_WT_NT);
        assert(status == 0);
        uint64_t temp_phys_address = get_physical_address(temp_addr);
        uint64_t aligned_fault_addr = fault_address & MASK(63, 12);
        mmu_verbose("temp_addr: 0x%x\r\n"
                    "aligned_fault_address: 0x%x\r\n",
                    temp_addr, aligned_fault_addr);
        memmove((void *) temp_addr, (void *) aligned_fault_addr, GRANULE);
        block_attributes_sg1 ba =
            new_block_attributes_sg1((lower_lvl ? USER_PAGE : KERNEL_PAGE) | ACCESS_FLAG_SET,NORMAL_WT_NT);
        set_invalid_page(temp_addr);
        asm volatile("TLBI VAE1  , %0" : : "r"(temp_addr/GRANULE) :);
        status = bind_address(aligned_fault_addr, temp_phys_address, ba);
        assert(status == 0);
    }
    else{/* we are alone : we update permissions*/
        mmu_verbose("Alone on shared page : changing permissions\r\n");
        block_attributes_sg1 ba = new_block_attributes_sg1(
            (lower_lvl ? USER_PAGE : KERNEL_PAGE) | ACCESS_FLAG_SET,
            NORMAL_WT_NT);
        uint64_t lvl3_entry_addr = get_lvl3_entry_phys_address(fault_address);
        int status = lvl3_entry_addr & MASK(5,0);
        assert(status == 0);
        set_block_and_page_attributes_sg1(lvl3_entry_addr, ba);
    }
    return;
}

/* The last bool indicates whether we are coming from a data abort (true) or an instruction abort (false) */
void permission_fault_handler(uint64_t fault_address, int level,bool lower_lvl,
                              int pid __attribute__((unused)),
                              bool data_abort __attribute__((unused))){
    mmu_verbose("Permission fault handler called\r\n");
    assert(level == 3);
    assert(lower_lvl);
    if(fault_address < id_paging_size || fault_address > GPIO_BASE){
        err.no = SEG_FAULT;
        mmu_verbose("SEGFAULT :\r\nFAR : 0x%x\r\nheap_begin : 0x%x\r\nend_offset : 0x%x\r\n", fault_address, get_heap_begin(), get_end_offset());
        assert(0); // TODO: SEGFAULT - kill process
    }
    handle_shared_page_write(fault_address, lower_lvl);
}


uint64_t get_page_permission(uint64_t virtual_addr) {
    set_lvl2_address_from_TTBR0_EL1();
    uint64_t lvl3_entry_phys_addr = get_lvl3_entry_phys_address(virtual_addr);
    return (AT(lvl3_entry_phys_addr) & MASK(7,6)) >> 6;
}

/* when a proc exits, we need to free the phyisical pages */
void process_exit(int pid){
    assert(pid != KERNEL_PID);
    assert(pid != INIT_PID);
    assert(pid != MEM_MANAGER_PID);
    int n_kernel_tables = id_paging_size/(GRANULE * N_TABLE_ENTRIES);
    int n_available_tables = GPIO_BASE / (GRANULE * N_TABLE_ENTRIES);
    uint64_t lvl2_entry_addr, lvl3_entry_addr, physical_addr;
    for(int i = n_kernel_tables; i < n_available_tables; i++){
        lvl2_entry_addr = lvl2_table_address + i * 8;
        if(!is_table_entry(AT(lvl2_entry_addr)))
            continue;
        for(int j = 0; j < N_TABLE_ENTRIES; j++){
            lvl3_entry_addr = get_address_sg1(lvl2_entry_addr) + j * 8;
            if(!is_block_entry(AT(lvl3_entry_addr), 3))
                continue;
            physical_addr = get_address_sg1(lvl3_entry_addr);
            decr_and_free_physical_page(physical_addr);
            /* we ake sure everything is invalidated */
            /* (in case things go wrong) */
            set_invalid_entry(lvl3_entry_addr);
        }
        set_invalid_entry(lvl2_entry_addr);
    }
    /* Clean cache (cf free virtual page for doc) */
    uint64_t ttbr0_el1, asid;
    asm volatile("mrs %0, TTBR0_EL1":"=r"(ttbr0_el1)::);
    asid = get_ASID_from_TTBR0(ttbr0_el1);
    /* I can't help but add a check */
    /* If not something has gone wrong */
    assert((uint64_t) pid == asid);
    uint64_t arg = (asid << 48);
    /* We assume everything is inner shareable */
    asm volatile("DSB ISHST");
    asm volatile("TLBI ASIDE1, %0":"=r"(arg)::);
    asm volatile("DSB ISH");
    asm volatile("ISB");
    return;
}

/* Functions for memory manager */
int mem_manager_free_virtual_page(uint64_t virtual_addr, int pid){
    uint64_t backup_lvl2_table_address;
    backup_lvl2_table_address = lvl2_table_address;
    assert(backup_lvl2_table_address == get_lvl2_address_from_sys_state(MEM_MANAGER_PID));
    lvl2_table_address = get_lvl2_address_from_sys_state(pid);
    int status = free_virtual_page(virtual_addr);
    lvl2_table_address = backup_lvl2_table_address;
    return status;
}
