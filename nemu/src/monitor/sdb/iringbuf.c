#include <monitor/iringbuf.h>

IRingBuf iringbuf;

void iringbuf_init(){
    // extern iringbuf;
    iringbuf.curr_ptr = 0;
    memset(iringbuf.logringbuf,0,sizeof(iringbuf.logringbuf));
}

void iringbuf_add(char* logbuf){
    // extern iringbuf;
    strcpy(iringbuf.logringbuf[iringbuf.curr_ptr], logbuf);
    iringbuf.curr_ptr = (iringbuf.curr_ptr + 1) % MAX_RING_LENGTH;
}

void iringbuf_print(){
    // extern iringbuf;
    DLog("Instruction Ring Buffer");
    int ptr = iringbuf.curr_ptr;
    for (int i = 0; i < MAX_RING_LENGTH; i++)
    {
        if(iringbuf.logringbuf[ptr][0]!='\0') {
            log_write("%s\n", iringbuf.logringbuf[ptr]);
            puts(iringbuf.logringbuf[ptr]);
        }
        ptr = (ptr + 1) % MAX_RING_LENGTH;
    }
    

}