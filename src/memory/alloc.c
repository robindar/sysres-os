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
        int nb_pages_to_free = (res / GRANULE) - ((res + increment) / GRANULE) + (!(res % GRANULE == 0) ? 1 : 0);
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

void * get_global_base(){
    return global_base;
}

void set_heap_begin(uint64_t val){
    heap_begin = val;
}

void set_end_offset(int val){
    end_offset = val;
}

void set_global_base(void * val){
    global_base = val;
}

#ifdef MALLOC_VERBOSE
#define malloc_verbose(...) uart_verbose(__VA_ARGS__)
#else
#define malloc_verbose(...) ((void) 0)
#endif

// Least power of 2 greater than or equal to n
// see "Hacker's Delight" section 3.2
size_t next_power_of_2 (size_t n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return (n + 1);
}

#define ABLOCK_SIZE sizeof(struct alloc_block)
struct alloc_block {
    size_t size;
    struct alloc_block *next, *prev;
    int free;
};

void print_malloc_list () {
    uart_verbose("Printing malloc block list\r\n");
    struct alloc_block * block = global_base;
    uart_verbose("Global base is at 0x%x\r\n", block);
    while (block) {
        uart_verbose("Block(0x%x) of size 0x%x has attributes prev(0x%x) next(0x%x) %s\r\n",
                block, block->size + ABLOCK_SIZE, block->prev, block->next, block->free ? "free" : "used");
        block = block->next;
    }
    uart_verbose("End of malloc block list\r\n");
}

void split_block (struct alloc_block * block, size_t size) {
  assert(block->size >= size);
  if (block->size > size + ABLOCK_SIZE) {
    struct alloc_block * new;
    new = block + ABLOCK_SIZE + size;
    new->free = 1;
    new->size = block->size - size - ABLOCK_SIZE;
    block->size = size;
    new->prev = block;
    new->next = block->next;
    block->next = new;
    if (new->next)
      new->next->prev = new;
  }
}

// Merge the given block with the next one, assumes the latter is free
void merge_block (struct alloc_block * block) {
  assert(block->next->free);
  assert(block == block->next->prev);
  block->size += ABLOCK_SIZE + block->next->size;
  if (block->next->next)
    block->next->next->prev = block;
  block->next = block->next->next;
}

struct alloc_block * find_free_block (struct alloc_block ** last, size_t size) {
    malloc_verbose("Setting initial cursor\r\n");
    struct alloc_block * current = global_base;
    while (current && !(current->free && current->size >= size)) {
        malloc_verbose("Proceeding to next block : last(%x) current(%x) next(%x)\r\n", *last, current, current->next);
        *last = current;
        current = current->next;
    }
    if (current)
        malloc_verbose("Found free block\r\n");
    else
        malloc_verbose("No free block available\r\n");
    return current;
}

struct alloc_block * extend_heap (struct alloc_block * last, size_t size) {
    struct alloc_block *block;
    block = ksbrk(0);
    void * request = ksbrk(size + ABLOCK_SIZE);
    assert((void *) block == request);
    if (request == (void *) -1)
        return NULL; // ksbrk failed
    block->prev = NULL;
    if (last) { // NULL on first request, prevents NPE
        assert(last->next == NULL);
        last->next = block;
        block->prev = last;
    }
    block->size = size;
    block->next = NULL;
    block->free = 0;
    malloc_verbose("Heap extended with block (%x)\r\n", block);
    assert((void *) block >= ((void *) last) + ABLOCK_SIZE);
    return block;
}

struct alloc_block * get_block_ptr(void * ptr) {
    return ((struct alloc_block *) ptr) - 1;
}

void * kmalloc (size_t size){
    size = next_power_of_2(size + ABLOCK_SIZE) - ABLOCK_SIZE;
    struct alloc_block * block;
    if (size <= 0)
      return NULL;

    if (!global_base) {
        // First call to kmalloc, intial setup
        block = extend_heap(NULL, size);
        if (!block)
          return NULL;
        global_base = block;
    } else {
        struct alloc_block * last = global_base;
        malloc_verbose("Requesting free block\r\n");
        block = find_free_block(&last, size);
        if (!block) { // No free block found
            malloc_verbose("Extending heap\r\n");
            block = extend_heap(last, size);
            if (!block)
                return NULL;
        } else {
          block->free = 0;
          if (block->size > size)
            split_block(block, size);
        }
    }
    malloc_verbose("Returning allocated space\r\n");
#ifdef MALLOC_VERBOSE
    print_malloc_list();
#endif
    return (block + 1);
}

void kfree (void * ptr){
    if (!ptr)
      return;
    struct alloc_block * block_ptr = get_block_ptr(ptr);
    assert(block_ptr->free == 0);
    block_ptr->free = 1;
    if (block_ptr->next && block_ptr->next->free)
      merge_block(block_ptr);
    if (block_ptr->prev && block_ptr->prev->free)
      merge_block(block_ptr->prev);
#ifdef MALLOC_VERBOSE
    print_malloc_list();
#endif
}

void * krealloc (void * ptr, size_t size) {
    if (!ptr)
        return kmalloc(size);
    struct alloc_block * block_ptr = get_block_ptr(ptr);
    if (block_ptr->size >= size)
        // We already have enough space, just leave it unchanged
        return ptr;
    if (block_ptr->next && block_ptr->next->free &&
        block_ptr->next->size + ABLOCK_SIZE + block_ptr->size >= size) {
        merge_block(block_ptr);
        return ptr;
    }
    size = next_power_of_2(size + ABLOCK_SIZE) - ABLOCK_SIZE;
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
