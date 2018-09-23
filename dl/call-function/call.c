#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main(int argc, char **argv) {
    char *error;
    void *handle;

    int (*puts_function)(char*);

    handle = dlopen ("/lib64/libc.so.6", RTLD_LAZY);
    if (!handle) {
        fprintf (stderr, "%s\n", dlerror());
        exit(1);
    }

    dlerror();    /* Clear any existing error */

    puts_function = dlsym(handle, "puts");

    if ((error = dlerror()) != NULL)  {
        fprintf (stderr, "%s\n", error);
        exit(1);
    }

    puts_function("test");

    dlclose(handle);

    return 0;
}
