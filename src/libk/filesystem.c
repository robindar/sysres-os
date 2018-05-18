#include "filesystem.h"
#include "string.h"

#define DIRECTORY_ENTRY_SIZE 32
#define FILENAME_MAX_SIZE 28
#define MAX_FILE_DESCRIPTORS 64

#ifdef FILESYSTEM_VERBOSE
#define filesystem_very_verbose(...) uart_verbose(__VA_ARGS__)
#else
#define filesystem_very_verbose(...) ((void) 0)
#endif

struct inode_t {
    uint32_t kind,
             nlink,
             size;
    uint32_t * blocks;
};

struct file_descriptor {
    struct inode_t inode;
    uint32_t pos;
    uint32_t closed;
    uint32_t owner_pid;
};

struct superblock_t {
    uint32_t inode_count,
             last_free_inode,
             free_block_list,
             free_block_count,
             root_inode;
};

struct filesystem_t {
    char * start, * end;
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
    char * seek_cursor;
};

static struct filesystem_t filesystem;
static struct file_descriptor * file_descriptor_table;

/* filesystem type is a string of exactly four characters at the end of the block */
char * get_filesystem_type (struct filesystem_t * fs) {
    char * fstype = kmalloc(5);
    memcpy(fstype, ((char *) fs->superblock) + fs->block_size - 4, 4);
    fstype[5] = 0;
    return fstype;
}

void seek_block (int n) {
    filesystem_very_verbose("Seeking block %d(%d) of %d\r\n", n, (unsigned int) n, filesystem.block_count);
    assert((unsigned int) n < filesystem.block_count);
    filesystem.seek_cursor = filesystem.start + n * filesystem.block_size;
}

char * read_block (int n) {
    seek_block(n);
    char * content = kmalloc(filesystem.block_size + 1);
    content[filesystem.block_size] = 0;
    for (unsigned int i = 0; i < filesystem.block_size; i++) {
        content[i] = *(filesystem.seek_cursor + i);
    }
    return content;
}

#define read_big_endian_int(n) switch_endianness(n)
uint32_t switch_endianness (uint32_t n) {
    return
        ((n >> 24) & 0x000000ff) | // move byte 3 to byte 0
        ((n >>  8) & 0x0000ff00) | // move byte 2 to byte 1
        ((n <<  8) & 0x00ff0000) | // move byte 1 to byte 2
        ((n << 24) & 0xff000000);  // move byte 0 to byte 3
}

struct inode_t read_inode (int n) {
    filesystem_very_verbose("Reading inode number %d\r\n", n);
    seek_block(n);
    struct inode_t inode;
    inode.kind  = read_big_endian_int(* ((uint32_t *) filesystem.seek_cursor));
    inode.nlink = read_big_endian_int(* ((uint32_t *) filesystem.seek_cursor + 1));
    inode.size  = read_big_endian_int(* ((uint32_t *) filesystem.seek_cursor + 2));
    assert(inode.size <= filesystem.max_file_size);
    inode.blocks = (uint32_t *) filesystem.seek_cursor + 3;
    filesystem_very_verbose("Read inode %d\r\n", n);
    filesystem_very_verbose("It has kind %d, %d links referencing it," \
           " size %d and its blocks start at %d (content : %d, %d, ...)\r\n",
           inode.kind, inode.nlink, inode.size, inode.blocks,
           read_big_endian_int(inode.blocks[0]), read_big_endian_int(inode.blocks[1]));
    return inode;
}

int get_next_available_file_descriptor_slot () {
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        if (file_descriptor_table[i].closed)
          return i;
    }
    return -1;
}

bool is_at_end_of_file (int file_desc) {
    struct file_descriptor * fd = file_descriptor_table + file_desc;
    return (fd->inode.size <= fd->pos);
}

int fclose (int file_desc) {
    file_descriptor_table[file_desc].closed = 1;
    return 0;
}

int iopen (int inode_number) {
    int slot = get_next_available_file_descriptor_slot();
    if (slot == -1)
        return -1;
    struct file_descriptor * fd = file_descriptor_table + slot;
    fd->inode = read_inode (inode_number);
    fd->pos = 0;
    fd->closed = 0;
    // TODO: set file_descriptor.owner_pid
    return slot;
}

int fopen (const char * path, int oflag) {
    int slot = get_next_available_file_descriptor_slot();
    if (slot == -1)
      return -1;
    // TODO: Init the actual file descriptor here
    return 0;
}

int min (int a, int b) {
  return ( a > b ) ? b : a;
}

/* Reads exactly len bytes. DOES NOT terminate the string, this is the caller's responsibility */
int read (int file_descriptor, char * buffer, int from, int len) {
    assert(from >= 0);
    assert(len >= 0);
    filesystem_very_verbose("Reading file descriptor %d\r\n", file_descriptor);
    struct file_descriptor * fd = file_descriptor_table + file_descriptor;
    uint32_t pos = fd->pos;
    len = min (len, fd->inode.size - fd->pos);
    char * block;
    while (fd->pos < pos + len) {
        filesystem_very_verbose("Reading block %d for %d bytes\r\n", fd->pos / filesystem.block_size, len);
        uint32_t block_number = read_big_endian_int(fd->inode.blocks[fd->pos / filesystem.block_size]);
        filesystem_very_verbose("Block is number %d\r\n", block_number);
        block = read_block((int) block_number);
        int copy_len = min (len, filesystem.block_size - (fd->pos % filesystem.block_size));
        memcpy(buffer + from, block + (fd->pos % filesystem.block_size), copy_len);
        fd->pos += copy_len;
        from += copy_len;
        kfree(block);
    }
    return len;
}

struct dirent_t {
    uint32_t inode;
    char * name;
};

struct dirent_t read_dirent (int file_descriptor) {
    struct file_descriptor * fd = file_descriptor_table + file_descriptor;
    char * dir = kmalloc( DIRECTORY_ENTRY_SIZE * sizeof(char) );
    filesystem_very_verbose("Reading directory entry for descriptor number %d\r\n", file_descriptor);
    read(file_descriptor, dir, 0, DIRECTORY_ENTRY_SIZE);
    struct dirent_t dirent;
    dirent.inode = read_big_endian_int( * ((uint32_t *) (dir + FILENAME_MAX_SIZE)) );
    dirent.name = dir;
    dir[FILENAME_MAX_SIZE] = 0;
    filesystem_very_verbose("Read directory entry for descriptor %d\r\n", file_descriptor);
    return dirent;
}

bool is_special_directory (char * name) {
    return (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0)));
}

void print_directory (int inode, char * dirname, int max_depth) {
    if (max_depth <= 0) return;
    int fd = iopen(inode);
    while (! is_at_end_of_file(fd)) {
        struct dirent_t dirent = read_dirent(fd);
        struct inode_t dir_inode = read_inode(dirent.inode);
        uart_verbose("%d : %s/%s\r\n", dirent.inode, dirname, dirent.name);
        if (dir_inode.kind == 1 && (! is_special_directory(dirent.name))) {
            char * filename = filename_join(dirname, dirent.name);
            print_directory(dirent.inode, filename, max_depth - 1);
            kfree(filename);
        }
        kfree(dirent.name);
    }
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

    uart_verbose("\r\nPrinting directory tree\r\n");
    char * root = kmalloc(sizeof(char));
    root[0] = 0;
    print_directory(1, root, 2);
    assert(0);
}

void init_file_descriptor_table () {
    file_descriptor_table = kmalloc( MAX_FILE_DESCRIPTORS * sizeof(struct file_descriptor));
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
      file_descriptor_table[i].closed = 1;
    }
}

void init_filesystem () {
    uint64_t filesystem_start;
    asm volatile ("ldr %0, =__filesystem_start" : "=r"(filesystem_start) : :);
    uint64_t filesystem_end;
    asm volatile ("ldr %0, =__filesystem_end" : "=r"(filesystem_end) : :);

    filesystem.start = (char *) filesystem_start;
    filesystem.end   = (char *) filesystem_end;
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

    filesystem.seek_cursor = (char *) filesystem_start;

    init_file_descriptor_table();

    uart_verbose("Filesystem found from 0x%x to 0x%x\r\n", filesystem_start, filesystem_end);
    print_filesystem_info();
}
