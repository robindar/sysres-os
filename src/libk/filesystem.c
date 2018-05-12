#include "filesystem.h"

#define DIRECTORY_ENTRY_SIZE 32
#define FILENAME_MAX_SIZE 28

struct superblock_t {
    uint32_t inode_count,
             last_free_inode,
             free_block_list,
             free_block_count,
             root_inode;
};

struct filesystem_t {
    void * start, * end;
    struct superblock_t * superblock;
    uint32_t inode_count,
             last_free_inode,
             free_block_list,
             free_block_count,
             root_inode,
             block_size;
    uint32_t block_count,
             block_word_size,
             blocktable_size,
             blocktable_offset,
             max_file_size;
    char * type;
};

static struct filesystem_t filesystem;

/* filesystem type is a string of exactly four characters at the end of the block */
char * get_filesystem_type (struct filesystem_t * fs) {
    char * fstype = kmalloc(5);
    memcpy(fstype, ((char *) fs->superblock) + fs->block_size - 4, 4);
    fstype[5] = 0;
    return fstype;
}

#define read_big_endian_int(n) switch_endianness(n)
uint32_t switch_endianness (uint32_t n) {
    return
        ((n >> 24) & 0x000000ff) | // move byte 3 to byte 0
        ((n >>  8) & 0x0000ff00) | // move byte 2 to byte 1
        ((n <<  8) & 0x00ff0000) | // move byte 1 to byte 2
        ((n << 24) & 0xff000000);  // move byte 0 to byte 3
}

void print_filesystem_info () {
    // Initialize some memory to avoid breaking output with malloc info
    void * p = kmalloc(100); kfree(p);

    uart_verbose("Partition default (alive)\r\n");
    uart_verbose("  block_size: %d\r\n", filesystem.block_size);
    uart_verbose("  block_count: %d\r\n", filesystem.block_count);
    uart_verbose("  block_word_size: %d\r\n", filesystem.block_word_size);
    uart_verbose("  i-nodes\r\n");
    uart_verbose("    blocktable_size: %d\r\n", filesystem.blocktable_size);
    uart_verbose("    max_file_size: %d\r\n", filesystem.max_file_size);
    uart_verbose("    blocktable_offset: %d\r\n", filesystem.blocktable_offset);
    uart_verbose("  superblock\r\n");
    uart_verbose("    inode_count: %d\r\n", filesystem.inode_count);
    uart_verbose("    last_free_inode: %d\r\n", filesystem.last_free_inode);
    uart_verbose("    free_block_list: %d\r\n", filesystem.free_block_list);
    uart_verbose("    free_block_count: %d\r\n", filesystem.free_block_count);
    uart_verbose("    fstype: %s\r\n", filesystem.type);
    uart_verbose("    root_inode: %d\r\n", read_big_endian_int(filesystem.superblock->root_inode));
}

void init_filesystem () {
    uint64_t filesystem_start;
    asm volatile ("ldr %0, =__filesystem_start" : "=r"(filesystem_start) : :);
    uint64_t filesystem_end;
    asm volatile ("ldr %0, =__filesystem_end" : "=r"(filesystem_end) : :);

    filesystem.start = (void *) filesystem_start;
    filesystem.end   = (void *) filesystem_end;
    filesystem.superblock = (struct superblock_t *) filesystem_start;
    filesystem.block_size = read_big_endian_int(* ((uint32_t *) filesystem_end - 1));

    filesystem.inode_count = read_big_endian_int(filesystem.superblock->inode_count);
    filesystem.last_free_inode = read_big_endian_int(filesystem.superblock->last_free_inode);
    filesystem.free_block_list = read_big_endian_int(filesystem.superblock->free_block_list);
    filesystem.free_block_count = read_big_endian_int(filesystem.superblock->free_block_count);
    filesystem.root_inode = read_big_endian_int(filesystem.superblock->root_inode);
    filesystem.type = get_filesystem_type(&filesystem);

    uint32_t size = (uint32_t) ((char *) filesystem_end - (char *) filesystem_start);
    filesystem.block_count = (size - 4) / filesystem.block_size;
    filesystem.blocktable_offset = 12;
    filesystem.blocktable_size = (filesystem.block_size - filesystem.blocktable_offset) / 4;
    filesystem.max_file_size = filesystem.block_size * filesystem.blocktable_size;
    filesystem.block_word_size = filesystem.block_size / 4;

    uart_verbose("Filesystem found from 0x%x to 0x%x\r\n", filesystem_start, filesystem_end);
    print_filesystem_info();
}
