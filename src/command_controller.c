#include "command_controller.h"
#include "system_stats_service.h"
#include "shared/socket_operation.h"
#include "frozen.h"
#include "log.h"
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#define UPDATE_PARAMS_FMT "{update_version:%d, file_size_bytes:%lu}"
#define UPDATE_RESULT_FMT "{current_version:%d}"
#define TELEMETRY_JSON_FMT "{cpu_usage:%f, firmware_version:%d, mem_usage:%f, satellite_id:%d, uptime:%f}"
#define SCAN_RESULT_JSON_FMT "{slice_quantity:%d, slices_dataset:%M}"
#define RPC_ERROR_FMT "{error_code:%d,error_message:%Q}"

#define UPDATE_COMMAND_CODE 1

#define TELEMETY_COMMAND_CODE 3

#define SCAN_COMMAND_CODE 2
#define FIRMWARE_COMMAND_CODE 1

#define STATION_ID 123456
#define SATELLITE_ID 987654

#define FIRMWARE_FILE_PATH "../assets/upgrade/so2_tp1-sat"

//Only for terminal launch #define FIRMWARE_FILE_PATH "so2_tp1_satellite"

int firmware_update(char* params){
    update_params req_params;
    json_scanf(params, strlen(params), UPDATE_PARAMS_FMT, 
    & req_params.update_version,
    & req_params.file_size_bytes);

    //int upgrade_version_check = check_upgrade_version(req_params.update_version);
    int current_firmware_version = get_firmware_version();
    log_debug("CURRENT VERSION: %i", current_firmware_version);
    if (req_params.update_version > current_firmware_version){
        update_result result;
        result.current_version = current_firmware_version;
        char rpc_buf[RPC_MSG_BUF_SIZE-4];
	    struct json_out output = JSON_OUT_BUF(rpc_buf, sizeof(rpc_buf));
	    json_printf(&output, UPDATE_RESULT_FMT, result.current_version);
        rpc update_response = {
        UPDATE_COMMAND_CODE,
        STATION_ID,
        SATELLITE_ID,
        rpc_buf,
        NULL};
        tcp_send_rpc(& update_response);
    } else {
        char rpc_buf[RPC_MSG_BUF_SIZE-4];
	    struct json_out output = JSON_OUT_BUF(rpc_buf, sizeof(rpc_buf));
	    json_printf(&output, RPC_ERROR_FMT, 402,"Upgrade version is older than current.");
        rpc update_response = {
        UPDATE_COMMAND_CODE,
        STATION_ID,
        SATELLITE_ID,
        NULL,
        rpc_buf};
        tcp_send_rpc(& update_response);
        return 1;
    }
    sleep(1);
    FILE* file_ptr;
    unlink(FIRMWARE_FILE_PATH);
    file_ptr = fopen(FIRMWARE_FILE_PATH,"wb");
    operation_result firm_tranf_result = tcp_recv_file_known_size(file_ptr, req_params.file_size_bytes);
    if (firm_tranf_result == socket_success){
        int chmod_result = chmod(FIRMWARE_FILE_PATH, S_IRWXU);
        log_debug("CHMOD RESULT: %i",chmod_result);
        log_debug("Trying to reset the computer so the uptade takes effect.");
        int system_reset_result = system("sudo shutdown -r now");
        log_debug("Reset result: %i",system_reset_result);
    } else {
        log_error("Failure on firmware update transmision.");
    }
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
int get_scan_metadata(scan_metadata* meta){
    DIR * dirp;
    struct dirent * entry;
    long file_size = 0;
    slice_count = 0;

    dirp = opendir("../assets/scans"); /* There should be error handling after this */
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            file_size = find_scan_slice_size(entry->d_name);
            log_trace("SCAN FILE. name:%s,size%lu",entry->d_name,file_size);
            if (file_size > 500) {
                int name_length = strlen(entry->d_name);
                log_trace("NAME SIZE: %i",name_length);
                meta->file_size_bytes = file_size;
                meta->file_name=malloc(name_length + 1);
                bzero(meta->file_name,name_length+1);
                strcpy(meta->file_name, entry->d_name);
                log_trace("SCAN DATA. name:%s,size%i",meta->file_name,meta->file_size_bytes);
                closedir(dirp);
                return 0;
            }
        }
    }
    closedir(dirp);
    return -1;
}

int calculate_scan_result(scan_result* result){
    
    DIR * dirp;
    struct dirent * entry;
    long file_size = 0;
    slice_count = 0;
    
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
            tcp_result = tcp_send_file(full_path);
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
#define SCAN_META_FMT "{file_name:%Q,file_size_bytes:%lu}"
int earth_surfice_scan(){
    scan_metadata metadata;
    int scan_meta_result = get_scan_metadata(& metadata);


    char rpc_buf[RPC_MSG_BUF_SIZE-4];
	struct json_out output = JSON_OUT_BUF(rpc_buf, sizeof(rpc_buf));

    if(scan_meta_result<0){
        json_printf(&output, RPC_ERROR_FMT, 
        404,
        "No scans where found.");

        rpc scan_response = {
            UPDATE_COMMAND_CODE,
            STATION_ID,
            SATELLITE_ID,
            NULL,
            rpc_buf};
        tcp_send_rpc(& scan_response);
        return -1;
    } else {
        json_printf(&output, SCAN_META_FMT, 
        metadata.file_name,
        metadata.file_size_bytes);

        rpc scan_response = {
            UPDATE_COMMAND_CODE,
            STATION_ID,
            SATELLITE_ID,
            rpc_buf,
            NULL};
        tcp_send_rpc(& scan_response);
        sleep(1);
        char filename[200];
        bzero(filename,200);
        sprintf(filename,"../assets/scans/%s",metadata.file_name);
        log_trace("SCAN REALTIVE PATH: %s",filename);
        if(tcp_send_file(filename)==socket_success){
            log_debug("File: %s SUCCESSFULLY SENT!", filename);
        } else {
            log_error("File: %s COULD NOT BE SENT!", filename);
        }
    }
    return 0;
    
}
int od_earth_surfice_scan(){
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
    
    sleep(1);
    
    send_all_slices();

    return 0;
}
int system_telemetry(){
    init_stat_service();
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
    close_stat_service();
    return 0;
}