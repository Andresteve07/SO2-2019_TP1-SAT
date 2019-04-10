#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "fff.h"
#include "frozen.h"
#include "command_controller.h"
#include "log.h"

DEFINE_FFF_GLOBALS
#define TEST_F(SUITE, NAME) void NAME()
#define RUN_TEST(SUITE, TESTNAME) printf(" Running %s.%s: \n", #SUITE, #TESTNAME); setup(); TESTNAME(); printf(" SUCCESS\n");

//FAKE_VALUE_FUNC1(int, modulo, int);

void setup()
{
	//RESET_FAKE(modulo);
}

TEST_F(ModuloTest, easy_json_test){
	char* str = "{ \"a\": 123, \"b\": \"hi\", \"c\": false }";
	int value = 0;
	char* b;
	json_scanf(str, strlen(str), "{a: %d}", &value);
	printf("value %i\n", value);
	assert(value==123);

}

TEST_F(JSONTest, print_qfield_json_test){
	char* json_str = "{ \"a\": 123, \"c\": false }";
	char rpc_buf[200];
	struct json_out output = JSON_OUT_BUF(rpc_buf, 200);
	int result = json_printf(& output, "{str:%Q}",json_str);
	printf("print_result:%i, rpc_buf:%s\n",result, rpc_buf);
	assert(strlen(rpc_buf)==38);
}

TEST_F(JSONTest, qfield_json_test){
	
	char* str = "{ \"a\": 123, \"payload\":\"{\\\"cpu_usage\\\":73.68,\\\"firmware_version\\\":12413,\\\"mem_usage\\\":45.71,\\\"satellite_id\\\":555555,\\\"uptime\\\":98.89}\", \"c\": false }";
	char* b;
	int result = json_scanf(str, strlen(str), "{payload: %Q}", &b);
	printf("result:%i, b:%s\n",result, b);
	assert(strlen(b)==99);
}

TEST_F(JSONTest, telemetry_json_scan_test){
	
	char* str = "{\"cpu_usage\":73.68,\"firmware_version\":12413,\"mem_usage\":45.71,\"satellite_id\":555555,\"uptime\":98.89}";
	char* fmt = "{cpu_usage:%f,firmware_version:%d,mem_usage:%f,satellite_id:%d,uptime:%f}";
	float c,m,u;
	int f,s;
	int result = json_scanf(str, strlen(str), fmt, &c,&f,&m,&s,&u);
	printf("result:%i, c:%f,m:%f,u:%f,f:%i,s:%i\n",result, c,m,u,f,s);
	assert(c==73.68f);
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

TEST_F(JSONTest, print_array_field_json_test){
    slice_meta slices_dataset[5];
	slices_dataset[0] = (slice_meta){"nombre_0",1000};
	slices_dataset[1] = (slice_meta){"nombre_1",1111};
	slices_dataset[2] = (slice_meta){"nombre_2",1222};
	char rpc_buf[200];
	struct json_out output = JSON_OUT_BUF(rpc_buf, 200);
	printf("SIZE:%i\n",sizeof(slices_dataset[0]));
	int result = json_printf(& output, "{a:%d,slices:%M,rand:%d}",666,print_slices_array,slices_dataset,3,888);
	printf("print_result:%i, rpc_buf:%s\n",result, rpc_buf);
	assert(strlen(rpc_buf)==38);
}

int main(void)
{
	log_set_level(LOG_TRACE);
	log_set_quiet(0);
	RUN_TEST(JSONTest, telemetry_json_scan_test);
	RUN_TEST(ModuloTest, easy_json_test);
	RUN_TEST(JSONTest, qfield_json_test);
	RUN_TEST(JSONTest, print_qfield_json_test);
	RUN_TEST(JSONTest, print_array_field_json_test);
	return 0;
}