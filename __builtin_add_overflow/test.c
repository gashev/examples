/**
 * __builtin_add_overflow example.
 * See https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html
 */

#include <stdio.h>

int main(void) {
    unsigned int total;
    unsigned int a = 3523453251;
    unsigned int b = 3324324321;

    if (__builtin_add_overflow(a, b, &total)) {
        puts("Overflow");
        return 1;
    }

    puts("No overflow");
    return 0;
}