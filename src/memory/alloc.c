#include "alloc.h"

//Note : we thank GCC who was kindly putting this variable at the same address as physical memeory map (without static) and thus init_alloc was actually modifying physical memory map for our grestest pleasure
static uint64_t heap_begin;
static int end_offset;

void * global_base;

void init_alloc(){
    uart_info("Init Alloc...\r\n");
    asm volatile ("ldr %0, =__end" : "=r"(heap_begin) : :);
    end_offset = 0;
    global_base = NULL;

    uart_info("Init Alloc done\r\n");
    return;
}

void * ksbrk(int increment) {
    /* heap_bin + end_offset is the address of the first byte non allocated */
    uart_verbose("ksbrk called with increment : %d and end_offset : 0x%x\r\n",increment, end_offset);
    int res = end_offset + heap_begin;
    end_offset += increment;
    if (res + increment > STACK_END)
      assert(0); // TODO: Heap overflow
    if(increment < 0){
        int nb_pages_to_free = (res / GRANULE) - ((res + increment) / GRANULE) + !(res % GRANULE == 0);
        for(int i = 0; i < nb_pages_to_free; i ++){
            free_virtual_page(res - i * GRANULE);
        }
        res += increment;
    }
    return (void *) ((uint64_t )res);
}

uint64_t get_heap_begin(){
    return heap_begin;
}

uint64_t get_end_offset(){
    return end_offset;
}

struct alloc_block {
    size_t size;
    struct alloc_block *next;
    int free;
};

void print_malloc_list () {
    uart_verbose("Printing malloc block list\r\n");
    struct alloc_block * block = global_base;
    while (block) {
        uart_verbose("Block(0x%x) has attributes size(0x%x) next(0x%x) free(0x%x)\r\n",
                block, block->size, block->next, block->free);
        block = block->next;
    }
    uart_verbose("End of malloc block list\r\n");
}

struct alloc_block * find_free_block (struct alloc_block ** last, size_t size) {
    uart_verbose("Setting initial cursor\r\n");
    struct alloc_block * current = global_base;
    while (current && !(current->free && current->size >= size)) {
        uart_verbose("Proceeding to next block : last(%x) current(%x) next(%x)\r\n", last, current, current->next);
        *last = current;
        current = current->next;
    }
    if (current)
        uart_verbose("Found free block\r\n");
    else
        uart_verbose("No free block available\r\n");
    return current;
}

struct alloc_block * extend_heap (struct alloc_block * last, size_t size) {
    struct alloc_block *block;
    block = ksbrk(0);
    void * request = ksbrk(size + sizeof(struct alloc_block));
    assert((void *) block == request);
    if (request == (void *) -1)
        return NULL; // ksbrk failed
    if (last) { // NULL on first request, prevents NPE
        assert(last->next == NULL);
        last->next = block;
    }
    block->size = size;
    block->next = NULL;
    block->free = 0;
    uart_verbose("Heap extended with block (%x)\r\n", block);
    assert((void *) block >= ((void *) last) + sizeof(struct alloc_block));
    return block;
}

struct alloc_block * get_block_ptr(void * ptr) {
    return ((struct alloc_block *) ptr) - 1;
}

void * kmalloc (size_t size){
    struct alloc_block * block;
    if (size <= 0)
      return NULL;
    print_malloc_list();

    if (!global_base) {
        uart_verbose("First call to kmalloc, initial setup\r\n");
        // First call to kmalloc, intial setup
        block = extend_heap(NULL, size);
        if (!block)
          return NULL;
        global_base = block;
    } else {
        uart_verbose("Further call to kmalloc\r\n");
        struct alloc_block * last = global_base;
        uart_verbose("Requesting free block\r\n");
        block = find_free_block(&last, size);
        if (!block) { // No free block found
            uart_verbose("Extending heap\r\n");
            block = extend_heap(last, size);
            if (!block)
                return NULL;
        } else
          block->free = 0;
    }
    uart_verbose("Returning allocated space\r\n");
    return (block + 1);
}

void split_block (struct alloc_block * block, size_t size) {}

// Merge the given block with the next one, assumes both are free
void merge_block (struct alloc_block * block) {}

void kfree (void * ptr){
    if (!ptr)
      return;
    struct alloc_block * block_ptr = get_block_ptr(ptr);
    assert(block_ptr->free == 0);
    block_ptr->free = 1;
}

void * krealloc (void * ptr, size_t size) {
    if (!ptr)
        return kmalloc(size);
    struct alloc_block * block_ptr = get_block_ptr(ptr);
    if (block_ptr->size >= size)
        // We already have enough space, just leave it unchanged
        return ptr;
    void * new_ptr;
    new_ptr = kmalloc(size);
    if (!new_ptr)
        // TODO: errno to set here
        return NULL;
    memcpy(new_ptr, ptr, block_ptr->size);
    kfree(ptr);
    return new_ptr;
}

void * kcalloc (size_t element_count, size_t element_size) {
  size_t size = element_count * element_size; // TODO: check for overflow here
  void * ptr = kmalloc(size);
  memset(ptr, 0, size); // TODO: check that ptr is not garbage
  return ptr;
}
