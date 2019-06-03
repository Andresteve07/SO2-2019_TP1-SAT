#include <stdio.h>
#include "command_router.h"
#include "log.h"

#include <syslog.h>

int main(int argc, char* argv[]){
    openlog("SO2_TP1_SAT",  LOG_PID|LOG_CONS, LOG_USER);
    log_set_level(LOG_TRACE);
	log_set_quiet(0);
    FILE* log_file = fopen("../logs/log-file_00.log","w");
    if (log_file == NULL){
        log_error("LOG FILE IS NULL.");
    }
    log_set_fp(log_file);
    init_router(argc,argv);
    syslog(LOG_ERR,"SATELLITE SERVICE STARTED! \n");
    while(1){
        process_request();
        syslog(LOG_ERR,"SATELLITE LOOP.\n");
    }
    return 0;
}