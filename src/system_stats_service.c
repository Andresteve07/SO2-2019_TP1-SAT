#include "system_stats_service.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "log.h"
#include <sys/sysinfo.h>
#include <errno.h>

struct sysinfo system_info;
void init_stat_service(){
    //memset(system_info, 0, sizeof &system_info);
    //system_info = malloc(sizeof(sysinfo));
    if(sysinfo(&system_info)){
        log_error("Unable to get system info. Cause: %s", strerror(errno));
        exit(0);
    } else {
        log_trace("system_info: uptime:%lu,\nload[0]:%lu,\ntotalram:%lu,\nfreeram:%lu,\nsharedram:%lu,\nbufferram:%lu,\ntotalswap:%lu,\nfreeswap:%lu,\nprocs:%i,\npad:%i,\ntotalhigh:%lu,\nfreehigh:%lu,\nmem_unit:%iu",
        system_info.uptime,
        system_info.loads[0],
        system_info.totalram,
        system_info.freeram,
        system_info.sharedram,
        system_info.bufferram,
        system_info.totalswap,
        system_info.freeswap,
        system_info.procs,
        system_info.pad,
        system_info.totalhigh,
        system_info.freehigh,
        system_info.mem_unit);
    }
}

float get_cpu_usage(){
    float f_load = 1.f / (1 << SI_LOAD_SHIFT);
    printf("load average (1 min): %.2f (%.0f%% CPU)\n",
    system_info.loads[0] * f_load,
    system_info.loads[0] * f_load * 100/get_nprocs());
    return system_info.loads[0] * f_load * 100/get_nprocs();
}
float get_mem_usage(){
    unsigned long used_mem = system_info.totalram - system_info.freeram;
    log_trace("used mem: %lu", used_mem);
    float used_memory_percentage = 100.0f * ((double)used_mem / (double)system_info.totalram);
    log_trace("used mem per: %f", used_memory_percentage);
    return used_memory_percentage;
}
float get_uptime(){
    return system_info.uptime * 1.0f;
}
int get_satellite_id(){
    return 123456;
}
int get_firmware_version(){
    return 20203;
}

void close_stat_service(){
    //free(system_info);
}