#include <stdio.h>
#include "command_router.h"
#include "log.h"

int main(int argc, char* argv[]){
    log_set_level(LOG_TRACE);
	log_set_quiet(0);
    init_router(argc,argv);
    while(1){
        process_request();
    }
    return 0;
}