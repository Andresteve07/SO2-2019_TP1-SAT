#ifndef SRC_COMMAND_CONTROLLER_H_
#define SRC_COMMAND_CONTROLLER_H_

typedef struct update_params{
	int update_version;
	//char* file_name;
	long file_size_bytes;
} update_params;

typedef struct scan_metadata{
	char* file_name;
	long file_size_bytes;
} scan_metadata;

typedef struct update_result{
	int current_version;
} update_result;
/*
typedef struct scan_params{
	int update_version;
} scan_params;
*/
typedef struct slice_meta{
	char* slice_name;
    long slice_size_bytes;
} slice_meta;

typedef struct scan_result{
	int slices_quantity;
    slice_meta* slices_dataset;
} scan_result;
/*
typedef struct telemetry_params{
	int update_version;
} telemetry_params;
*/
typedef struct telemetry_result{
	int satellite_id;
    float uptime;
	int firm_version;
    float mem_usage;
    float cpu_usage;
} telemetry_result;

int firmware_update(char* params);
int earth_surfice_scan();
int system_telemetry();

#endif /* SRC_COMMAND_CONTROLLER_H_ */