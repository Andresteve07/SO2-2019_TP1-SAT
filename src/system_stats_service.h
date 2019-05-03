#ifndef SRC_SYSTEM_STATS_SERVICE_H_
#define SRC_SYSTEM_STATS_SERVICE_H_
#define FIRMWARE_VERSION 12203
void init_stat_service();
float get_cpu_usage();
float get_mem_usage();
float get_uptime();
int get_satellite_id();
int get_firmware_version();
void close_stat_service();

#endif /* SRC_SYSTEM_STATS_SERVICE_H_ */