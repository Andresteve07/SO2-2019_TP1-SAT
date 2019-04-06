/*
 * udp_socket_client.c
 *
 *  Created on: 1 abr. 2019
 *      Author: steve-urbit
 */

#include "socket_operation.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include "frozen.h"
#include "log.h"
#include <sys/time.h>
  
#define PORT	12121 
#define MAXLINE 1024 

int udp_sockfd;
struct sockaddr_in udp_servaddr;

operation_result udp_init_client(){
	// Creating socket file descriptor 
	if ( (udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		log_error("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&udp_servaddr, 0, sizeof(udp_servaddr)); 
	
	// Filling server information 
	udp_servaddr.sin_family = AF_INET; 
	udp_servaddr.sin_port = htons(PORT); 
	udp_servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	log_debug("UDP socket creation succeed!");
	return socket_success;
}
operation_result udp_timeouts(int seconds){
	return socket_success;
}
operation_result udp_connect_to_server(char*  server_ip){
	return socket_success;
}

operation_result udp_connect(){
	operation_result op = socket_success;
		return op;
}

operation_result udp_send_data(){
	operation_result op = socket_success;
	return op;
}
operation_result udp_recv_data(){
	operation_result op = socket_success;
	return op;
}

operation_result udp_send_rpc_request(rpc* request){
	char total_buf[204];
	bzero(total_buf,sizeof(total_buf));
	
	char* rpc_buf = &total_buf[4];
	struct json_out output = JSON_OUT_BUF(rpc_buf, 200);
	
	json_printf(&output, RPC_JSON_FMT,
	request->command_id,
	request->satellite_id,
	request->station_id,
	request->payload);
	log_trace("RPC req: %s\n",rpc_buf);

	load_heading_integer_to_byte_array(strlen(rpc_buf),total_buf);
	
	log_trace("TOTAL req: %c%c%c%c%s\n",total_buf[0],total_buf[1],total_buf[2],total_buf[3],&total_buf[4]);

	if(sendto(udp_sockfd, total_buf, strlen(rpc_buf)+4, 
	MSG_CONFIRM, (const struct sockaddr *) & udp_servaddr, 
	sizeof(udp_servaddr)) > 0){
		return socket_success;
	} else {
		return socket_failure;
	}
}

operation_result udp_recv_rpc_response(rpc* response){
	size_t payload_size;
	char resp_buf[500];//sizeof only works ok for static arrays i.e. results on 500
	bzero(resp_buf, sizeof(resp_buf));
	socklen_t address_size; 
	
	if(recvfrom(udp_sockfd, resp_buf, sizeof(resp_buf),
	MSG_WAITALL, (struct sockaddr *) &udp_servaddr, 
	&address_size) > 0){
		int size_int = integerFromArrayTip(resp_buf);
		char* recv_data = &resp_buf[4];
		recv_data[size_int]='\0';
		log_trace("size_cadena:%lu,cadena: %s\n",strlen(recv_data),recv_data);
		log_trace("HOLOOO");
		
		json_scanf(recv_data,strlen(recv_data),RPC_JSON_FMT,
		& response->command_id,
		& response->satellite_id,
		& response->station_id,
		& response->payload);

		log_trace("payload size: %lu, apy:%s", strlen(response->payload),response->payload);
		
		log_trace("cid:%i,\nsatid:%i,\nstid:%i,\npay:%s",
		response->command_id,
		response->satellite_id,
		response->station_id,
		response->payload);
		
		return socket_success;

	} else {
		return socket_failure;
	}
}

