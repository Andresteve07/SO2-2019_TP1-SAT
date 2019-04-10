#include "command_controller.h"
#include "system_stats_service.h"
#include "shared/socket_operation.h"
#include "frozen.h"
#include "log.h"
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#define TELEMETRY_JSON_FMT "{cpu_usage:%f,firmware_version:%d,mem_usage:%f,satellite_id:%d,uptime:%f}"
#define SCAN_RESULT_JSON_FMT "{slice_quantity:%d,slices_dataset:%M}"

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
long find_scan_slice_size(char file_name[]) 
{ 
    char full_path[256];
    bzero(full_path,sizeof(full_path));
    sprintf(full_path, "../assets/scans/%s",file_name);
    // opening the file in read mode 
    FILE* fp = fopen(full_path, "r"); 
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found!\n"); 
        return -1; 
    } 
    fseek(fp, 0L, SEEK_END); 
    // calculating the size of the file 
    long res = ftell(fp); 
    // closing the file 
    fclose(fp);   
    return res; 
} 

slice_meta slices_dataset[300];
int slice_count = 0;
int calculate_scan_result(scan_result* result){
    
    DIR * dirp;
    struct dirent * entry;
    long file_size = 0;

    dirp = opendir("../assets/scans"); /* There should be error handling after this */
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            file_size = find_scan_slice_size(entry->d_name);
            log_trace("SCAN FILE. name:%s,size%lu",entry->d_name,file_size);
            if (file_size > 0) {
                int name_length = strlen(entry->d_name);
                log_trace("NAME SIZE: %i",name_length);
                slice_meta silce_data;
                silce_data.slice_size_bytes = file_size;
                silce_data.slice_name=malloc(name_length + 1);
                bzero(silce_data.slice_name,name_length+1);
                strcpy(silce_data.slice_name, entry->d_name);
                log_trace("SLICE DATA. name:%s,size%i",silce_data.slice_name,silce_data.slice_size_bytes);
                slices_dataset[slice_count]=silce_data;
                slice_count++;
            }
        }
    }
    result->slices_quantity = slice_count;
    result ->slices_dataset = slices_dataset;
    closedir(dirp);
    return 0;
}

int send_all_slices_orig(){
    DIR * dirp;
    struct dirent * entry;
    operation_result tcp_result;
    char full_path[256];
    

    dirp = opendir("../assets/scans"); /* There should be error handling after this */
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            log_trace("Trying to send file: %s",entry->d_name);
            bzero(full_path,sizeof(full_path));
            sprintf(full_path, "../assets/scans/%s",entry->d_name);
            tcp_result = tcp_send_file(full_path);
            if (tcp_result == socket_failure) {
                return -1;
            }
            log_trace("Succes on sending file: %s",entry->d_name);
        }
    }    
    closedir(dirp);
    return 0;
}

int send_all_slices(){
    operation_result tcp_result;
    char full_path[256];
    
    for (int i = 0; i < slice_count; i++){
        slice_meta slice_data = slices_dataset[i];
        if (strlen(slice_data.slice_name) > 0 && slice_data.slice_size_bytes > 0){
            log_trace("Trying to send file: %s",slice_data.slice_name);
            bzero(full_path,sizeof(full_path));
            sprintf(full_path, "../assets/scans/%s",slice_data.slice_name);
            tcp_result = tcp_send_file_known_size(full_path,slice_data.slice_size_bytes);
            if (tcp_result == socket_failure) {
                return -1;
            }
            log_trace("Succes on sending file: %s",slice_data.slice_name);
            sleep(1);
        }
    }
    return 0;
}

int print_slices_array(struct json_out* out, va_list* ap){
	slice_meta* array = va_arg(*ap, slice_meta*);
	log_trace("ARRAY_ADDR: %p", array);
	int number_items = va_arg(*ap, int);
	log_trace("CANTIDAD: %i",number_items);
	int bytes_writen=0;
	bytes_writen += json_printf(out, "[", 1);
	for (int i = 0; i < number_items; i++){
		slice_meta this_slice = array[i];
        log_trace("Printing slice no:%i, name: %s",i,this_slice.slice_name);
		bytes_writen+=json_printf(out,"{name:%Q,size:%d}",this_slice.slice_name,this_slice.slice_size_bytes);
		if (i < (number_items-1)){
			bytes_writen+= json_printf(out, ", ");
		}
	}
	bytes_writen += json_printf(out, "]", 1);
	return bytes_writen;
}

int earth_surfice_scan(){
    scan_result result;
    log_trace("HOLO1");
    calculate_scan_result(& result);
    log_trace("HOLO2"); 
    char rpc_buf[200];
	struct json_out output = JSON_OUT_BUF(rpc_buf, 200);
    json_printf(&output, SCAN_RESULT_JSON_FMT,
    result.slices_quantity,
    print_slices_array,
    result.slices_dataset,
    result.slices_quantity);
    
    log_trace("RESULT: %s",rpc_buf); 
    rpc scan_response = {
        SCAN_COMMAND_CODE,
        STATION_ID,
        SATELLITE_ID,
        rpc_buf,
        NULL};
    log_trace("HOLO4"); 
    tcp_send_rpc(& scan_response);
    
    send_all_slices();

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
        rpc_buf,
        NULL};

    udp_send_rpc(&telemetry_response);

    return 0;
}