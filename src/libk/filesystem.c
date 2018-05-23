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
    int index;
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

int min (int a, int b) {
  return ( a > b ) ? b : a;
}

#define read_big_endian_int(n) switch_endianness(n)
uint32_t switch_endianness (uint32_t n) {
    return
        ((n >> 24) & 0x000000ff) | // move byte 3 to byte 0
        ((n >>  8) & 0x0000ff00) | // move byte 2 to byte 1
        ((n <<  8) & 0x00ff0000) | // move byte 1 to byte 2
        ((n << 24) & 0xff000000);  // move byte 0 to byte 3
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

void write_block (int n, const char * content) {
    seek_block(n);
    for (unsigned int i = 0; i < filesystem.block_size; i++) {
        *(filesystem.seek_cursor + i) = content[i];
    }
}

int alloc_block () {
    filesystem_very_verbose("Allocating new bloc from index %d\r\n", filesystem.free_block_list);
    uint32_t * block_index = (uint32_t *) read_block(filesystem.free_block_list);
    int offset = 1;
    if (read_big_endian_int(block_index[offset]) == 0) {
        // Block index is empty, change bloc index
        uint32_t last_free_block_list = filesystem.free_block_list;
        filesystem.free_block_list = read_big_endian_int(block_index[0]);
        filesystem_very_verbose("No free block on block index, new block index is %d\r\n", filesystem.free_block_list);
        return last_free_block_list;
    }
    while (offset + 1 < filesystem.block_size / 4
            && read_big_endian_int(block_index[offset+1] != 0))
        offset++;
    uint32_t free_block = read_big_endian_int(block_index[offset]);
    block_index[offset] = switch_endianness(0);
    write_block(filesystem.free_block_list, block_index);
    kfree(block_index);
    filesystem_very_verbose("Allocating block 0x%x\r\n", free_block);
    return free_block;
}

int free_block () {

}

struct inode_t read_inode (int n) {
    filesystem_very_verbose("Reading inode number %d\r\n", n);
    seek_block(n);
    struct inode_t inode;
    inode.index = n;
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

void write_inode (struct inode_t inode) {
    seek_block(inode.index);
    (* ((uint32_t *) filesystem.seek_cursor)) = switch_endianness(inode.kind);
    (* ((uint32_t *) filesystem.seek_cursor + 1)) = switch_endianness(inode.nlink);
    (* ((uint32_t *) filesystem.seek_cursor + 2)) = switch_endianness(inode.size);
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

void extend_file (int file_descriptor, int len) {
    filesystem_very_verbose("Extending file descriptor %d from %d\r\n", file_descriptor, len);
    struct file_descriptor * fd = file_descriptor_table + file_descriptor;
    if (fd->inode.size >= fd->pos + len) {
        filesystem_very_verbose("Already OK : inode size is %d, pos %d, len %d\r\n", fd->inode.size, fd->pos, len);
        return;
    }
    filesystem_very_verbose("Initial inode size is %d\r\n", fd->inode.size);
    fd->inode.size += min(len, filesystem.block_size - (fd->inode.size % filesystem.block_size));
    filesystem_very_verbose("Initial inode size is %d\r\n", fd->inode.size);
    int index_of_last_block = fd->inode.size / filesystem.block_size;
    while (fd->inode.size < fd->pos + len) {
        fd->inode.size += min(fd->pos + len - fd->inode.size, filesystem.block_size);
        fd->inode.blocks[++index_of_last_block] = alloc_block();
    filesystem_very_verbose("Current inode size is %d\r\n", fd->inode.size);
    }
    write_inode(fd->inode);
    filesystem_very_verbose("Final inode size is %d\r\n", fd->inode.size);
}

int fclose (int file_desc) {
    file_descriptor_table[file_desc].closed = 1;
    return 0;
}

int iopen (int inode_number) {
    int slot = get_next_available_file_descriptor_slot();
    if (slot == -1 || inode_number == -1)
        return -1;
    struct file_descriptor * fd = file_descriptor_table + slot;
    fd->inode = read_inode (inode_number);
    fd->pos = 0;
    fd->closed = 0;
    // TODO: set file_descriptor.owner_pid
    return slot;
}

void fseek (int file_descriptor, int offset, enum seek_t whence) {
    struct file_descriptor * fd = file_descriptor_table + file_descriptor;
    uint32_t pos = fd->pos;
    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos += offset;
            break;
        case SEEK_END:
            assert (offset <= 0);
            pos = fd->inode.size + offset;
            break;
        default:
            assert(0);
    }
    assert(pos >=  0);
    assert(pos <= fd->inode.size);
    fd->pos = pos;
}

/* Reads exactly len bytes. DOES NOT terminate the string, this is the caller's responsibility */
int read (int file_descriptor, char * buffer, int len) {
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
        memcpy(buffer, block + (fd->pos % filesystem.block_size), copy_len);
        fd->pos += copy_len;
        buffer += copy_len;
        kfree(block);
    }
    return len;
}

size_t fread (int file_descriptor, void * buffer, size_t len) {
    return (size_t) read(file_descriptor, (char *) buffer, (int) len);
}

int write (int file_descriptor, const char * buffer, int len) {
    assert(len >=0);
    filesystem_very_verbose("Writing to file descriptor %d\r\n", file_descriptor);
    struct file_descriptor * fd = file_descriptor_table + file_descriptor;
    extend_file(file_descriptor, len);
    uint32_t pos = fd->pos;
    assert(len <= fd->inode.size - fd->pos);
    char * block;
    while (fd->pos < pos + len) {
        uint32_t block_number = read_big_endian_int(fd->inode.blocks[fd->pos / filesystem.block_size]);
        block = read_block((int) block_number);
        int copy_len = min (pos + len - fd->pos, filesystem.block_size - (fd->pos % filesystem.block_size));
        memcpy(block + (fd->pos % filesystem.block_size), buffer, copy_len);
        write_block((int) block_number, block);
        fd->pos += copy_len;
        buffer += copy_len;
        kfree(block);
    }
    return len;
}

size_t fwrite (int file_descriptor, const void * buffer, size_t len) {
    return (size_t) write(file_descriptor, (char *) buffer, (int) len);
}

struct dirent_t {
    uint32_t inode;
    char * name;
};

struct dirent_t read_dirent (int file_descriptor) {
    char * dir = kmalloc( DIRECTORY_ENTRY_SIZE * sizeof(char) );
    filesystem_very_verbose("Reading directory entry for descriptor number %d\r\n", file_descriptor);
    read(file_descriptor, dir, DIRECTORY_ENTRY_SIZE);
    struct dirent_t dirent;
    dirent.inode = read_big_endian_int( * ((uint32_t *) (dir + FILENAME_MAX_SIZE)) );
    dirent.name = dir;
    dir[FILENAME_MAX_SIZE] = 0;
    filesystem_very_verbose("Read directory entry for descriptor %d\r\n", file_descriptor);
    return dirent;
}

/* returns -1 if the name does not match,
 * and the offset at which the / happens if a match is found
 */
int filename_match (const char * path, const char * dirname) {
    int cur = 0, match;
    while (cur < FILENAME_MAX_SIZE) {
        if ((path[cur] == '/' || path[cur] == 0) && dirname[cur] == 0)
            return cur;
        if (path[cur] != dirname[cur])
            return -1;
        cur++;
    }
    return (path[cur] == '/' || path[cur] == 0) ? cur : -1;
}

int inode_of_path (const char * path) {
    if (*path == '/')
      path += 1;
    int fdesc = iopen(filesystem.root_inode);
    struct file_descriptor * fd = file_descriptor_table + fdesc;
    while (! is_at_end_of_file(fdesc)) {
        struct dirent_t dirent = read_dirent(fdesc);
        int match = filename_match(path, dirent.name);
        if (match > 0) {
            path += match;
            if (*path == 0) {
                kfree(dirent.name);
                return dirent.inode;
            }
            path += 1;
            fd->inode = read_inode(dirent.inode);
            fd->pos = 0;
        }
        kfree(dirent.name);
    }
    fclose(fdesc);
    return -1;
}

int fopen (const char * path, int oflag) {
    return iopen(inode_of_path(path));
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
    fclose(fd);
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

    uart_verbose("/config/auto-aux/gethostbyname.c has inode %d\r\n", inode_of_path("/config/auto-aux/gethostbyname.c"));

    uart_verbose("Printing directory tree\r\n");
    char * root = kmalloc(sizeof(char));
    root[0] = 0;
    print_directory(1, root, 1);

    /*
    // Print file content
    char * content = kmalloc(513 * sizeof(char));
    int fd = iopen(155);
    read(fd, content, 512);
    content[512] = 0;
    uart_verbose("Printing file content\r\n");
    uart_printf("%s\r\n", content);
    kfree(content);
    fclose(fd);

    // Modify file content
    uart_verbose("Modifying file content\r\n");
    char * n_content = "Pikabu !! Hahaha";
    fseek(fd, 0, SEEK_SET);
    write(fd, n_content, 16);

    // Print file content (hopefully modified)
    content = kmalloc(513 * sizeof(char));
    fseek(fd, 0, SEEK_SET);
    read(fd, content, 512);
    content[512] = 0;
    uart_verbose("Printing file content\r\n");
    uart_printf("%s\r\n", content);
    kfree(content);
    */

    #define TEST_INODE 822
    // Print file tail
    char * content = kmalloc(513 * sizeof(char));
    int fd = iopen(TEST_INODE);
    assert(fd != -1);
    fseek(fd, -128, SEEK_END);
    read(fd, content, 128);
    content[128] = 0;
    uart_verbose("Printing file content\r\n");
    uart_printf("%s\r\n", content);
    kfree(content);
    fclose(fd);

    // Modify file content
    uart_verbose("Modifying file content\r\n");
    fd = iopen(TEST_INODE);
    assert(fd != -1);
    char * n_content = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678";
    fseek(fd, 0, SEEK_END);
    write(fd, n_content, 100);
    fclose(fd);

    // Print file content (hopefully modified)
    content = kmalloc(512 * sizeof(char));
    fd = iopen(TEST_INODE);
    assert(fd != -1);
    fseek(fd, -128, SEEK_END);
    read(fd, content, 128);
    content[128] = 0;
    uart_verbose("Printing file content\r\n");
    uart_printf("%s\r\n", content);
    kfree(content);
    fclose(fd);

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
    /* print_filesystem_info(); */
}

void move (const char * src_path, const char * dst_path){
    
}
