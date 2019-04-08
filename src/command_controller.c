#include "command_controller.h"
#include "system_stats_service.h"
#include "shared/socket_operation.h"
#include "frozen.h"

#define TELEMETRY_JSON_FMT "{cpu_usage:%f,firmware_version:%d,mem_usage:%f,satellite_id:%d,uptime:%f}"
#define SCAN_RESPONSE_JSON_FMT "{slice_quantity:%d,slice_size_bytes:%d}"
#define UPDATE_RESPONSE_JSON_FMT "{current_version:%d}"
#define UPDATE_COMMAND_CODE 1

#define TELEMETY_COMMAND_CODE 3

#define SCAN_COMMAND_CODE 2
#define FIRMWARE_COMMAND_CODE 1

#define STATION_ID 123456
#define SATELLITE_ID 987654

#define FIRMWARE_FILE_PATH "../assets/file_server"

int firmware_update(char* params){
    return 0;
}
int earth_surfice_scan(){
    return 0;
}
int system_telemetry(){
    telemetry_result result;
    result.cpu_usage = get_cpu_usage();
    result.mem_usage = get_mem_usage();
    result.firm_version = get_firmware_version();
    result.uptime = get_uptime();
    result.satellite_id = get_satellite_id();

    char rpc_buf[200];
	struct json_out output = JSON_OUT_BUF(rpc_buf, 200);

	json_printf(&output, TELEMETRY_JSON_FMT,
    result.cpu_usage,
    result.firm_version,
    result.mem_usage,
    result.satellite_id,
    result.uptime);

    rpc telemetry_response = {
        TELEMETY_COMMAND_CODE,
        STATION_ID,
        SATELLITE_ID,
        rpc_buf};

    udp_send_rpc(&telemetry_response);

    return 0;
}