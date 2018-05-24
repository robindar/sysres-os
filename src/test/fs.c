#include "../libk/user_filesystem.h"
#include "../libk/io_lib.h"
#include "../libk/string.h"
#include "test.h"
void fs_test1(){
    BEGIN_TEST();
    char * content = kmalloc(513 * sizeof(char));
    int fd = uopen("/LICENSE");
    if(fd == -1)
        uart_error("Errno: %d\r\n", err.no);
    assert(fd != -1);
    useek(fd, -128, SEEK_END);
    uread(fd, content, 128);
    content[128] = 0;
    uart_printf("Printing file content\r\n");
    uart_printf("%s\r\n", content);
    kfree(content);
    uclose(fd);

    // Modify file content
    uart_printf("Modifying file content\r\n");
    fd = uopen("/LICENSE");
    assert(fd != -1);
    char * n_content = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678";
    useek(fd, 0, SEEK_END);
    uwrite(fd, n_content, 100);
    uclose(fd);

    // Print file content (hopefully modified)
    content = kmalloc(512 * sizeof(char));
    fd = uopen("/LICENSE");
    assert(fd != -1);
    useek(fd, -128, SEEK_END);
    uread(fd, content, 128);
    content[128] = 0;
    uart_printf("Printing file content\r\n");
    uart_printf("%s\r\n", content);
    kfree(content);
    uclose(fd);
    END_TEST();
}

void fs_test2(){
    BEGIN_TEST();
    io_printf("\r\n-> Enter filename:\r\n");
    char * content = kmalloc(513 * sizeof(char));
    char * buff[256];
    io_get_string(buff, 256);
    int fd = uopen(buff);
    if(fd == -1){
        io_error("File not found\r\n");
        return;
    }
    useek(fd, -300, SEEK_END);
    uread(fd, content, 300);
    content[300] = 0;
    io_printf("\r\n-> Printing file content:\r\n");
    io_printf("%s\r\n", content);
    kfree(content);
    uclose(fd);

    // Modify file content
    io_printf("-> Enter text to append:\r\n");
    fd = uopen(buff);
    assert(fd != -1);
    char n_content[256];
    io_get_string(n_content, 256);
    useek(fd, 0, SEEK_END);
    uwrite(fd, n_content, strsize(n_content));
    uclose(fd);

    // Print file content (hopefully modified)
    content = kmalloc(512 * sizeof(char));
    fd = uopen(buff);
    assert(fd != -1);
    useek(fd, -300, SEEK_END);
    uread(fd, content, 300);
    content[300] = 0;
    io_printf("\r\n-> Printing file content\r\n");
    io_printf("%s\r\n", content);
    kfree(content);
    uclose(fd);
    END_TEST();
}
