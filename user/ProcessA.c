#include <Syscall.h>
#include <Printf.h>

void userMain() {
    for (int i = 1; i <= 10; ++ i) {
        uPrintf("This is process A\n");
        //syscallPutchar('a');
    }
    uPrintf("process finish\n");
}