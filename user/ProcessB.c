#include <Syscall.h>
#include <Printf.h>

void userMain() {
    for (int i = 1; i <= 5; ++ i) {
        uPrintf("This is process B\n");
        //syscallPutchar('b');
    }   
    uPrintf("process finish\n");
}