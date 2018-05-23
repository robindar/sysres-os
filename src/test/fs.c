#include "../libk/user_filesystem.h"
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
    uart_verbose("Printing file content\r\n");
    uart_printf("%s\r\n", content);
    kfree(content);
    uclose(fd);

    // Modify file content
    uart_verbose("Modifying file content\r\n");
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
    uart_verbose("Printing file content\r\n");
    uart_printf("%s\r\n", content);
    kfree(content);
    uclose(fd);
    END_TEST();
}
