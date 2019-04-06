/*
 * socket_client.h
 *
 *  Created on: 1 abr. 2019
 *      Author: steve-urbit
 * 
 * This header file provides high level abstractions for socket operations.
 * wW sugest mock socket server interactions using netcat
 * $ netcat -l -p 12121
 * For the socket client
 * $ netcat localhost 12121
 */

#ifndef SRC_SOCKET_CLIENT_H_
#define SRC_SOCKET_CLIENT_H_

#include<stdio.h>
#define RPC_JSON_FMT "{command_id:%d,satellite_id:%d,station_id:%d,payload:%Q}"

typedef enum{
	socket_success,
	socket_failure,
	socket_unknown
}operation_result;

typedef struct rpc
{
	unsigned char command_id;
	int satellite_id;
	int station_id;
	char* payload;
} rpc;

operation_result tcp_init_client();
operation_result tcp_init_server();
operation_result tcp_timeouts(int seconds);
operation_result tcp_connect_to_server(char*  server_ip);
operation_result tcp_send_data(char* data_buffer);
operation_result tcp_send_data_bytes(char* data_buffer, size_t byte_count);
operation_result tcp_recv_data(char* data_buffer);
operation_result tcp_recv_data_bytes(char* data_buffer, size_t byte_count);
operation_result tcp_send_rpc_request(rpc* request);
operation_result tcp_recv_rpc_response(rpc* response);
operation_result tcp_send_file(char* file_name);
operation_result tcp_recv_file(FILE* input_file);
operation_result tcp_recv_file_known_size(FILE* input_file, size_t byte_count);
operation_result tcp_close_connection();

int integerFromArrayTip(char* array);
void load_heading_integer_to_byte_array(int number,char* array);

operation_result udp_connect();

operation_result udp_init_client();
operation_result udp_init_server();
operation_result udp_timeouts(int seconds);
operation_result udp_connect_to_server(char*  server_ip);
operation_result udp_send_data();
operation_result udp_recv_data();
operation_result udp_send_rpc_request(rpc* request);
operation_result udp_recv_rpc_response(rpc* response);


#endif /* SRC_SOCKET_CLIENT_H_ */