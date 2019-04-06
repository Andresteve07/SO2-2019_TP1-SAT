/*
 * tcp_socket_client.c
 *
 *  Created on: 1 abr. 2019
 *      Author: steve-urbit
 */
#include "socket_operation.h"
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <strings.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "frozen.h"
#include "log.h"
#include <sys/time.h>
 
#define PORT 12121 
#define SA struct sockaddr

int tcp_sockfd, connfd;
struct sockaddr_in tcp_servaddr, cli;
char* response_buffer;


operation_result tcp_init_client(){
    // socket create and varification 
    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (tcp_sockfd == -1) { 
        log_error("socket creation failed...\n"); 
        return socket_failure; 
    } 
    else {
		log_debug("Socket successfully created..\n");
		return tcp_timeouts(10);
	}
}

operation_result tcp_timeouts(int seconds){
	struct timeval timeout;      
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;

    if (setsockopt (tcp_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
		log_error("setsockopt failed\n");
		return socket_failure;
	}
        
    if (setsockopt (tcp_sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
		log_error("setsockopt failed\n");
		return socket_failure;
	}
    return socket_success;
}

operation_result tcp_connect_to_server(char*  server_ip){
	bzero(&tcp_servaddr, sizeof(tcp_servaddr)); 
	// assign IP, PORT 
    tcp_servaddr.sin_family = AF_INET; 
    tcp_servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    tcp_servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket 
    if (connect(tcp_sockfd, (SA*)&tcp_servaddr, sizeof(tcp_servaddr)) != 0) { 
        log_error("connection with the server failed...\n"); 
        return socket_failure; 
    } else {
		log_debug("connected to the server..\n");
		return socket_success;
	}
}

operation_result tcp_send_data(char* data_buffer){
	if(write(tcp_sockfd, data_buffer, sizeof(data_buffer)) > 0){
		return socket_success;
	} else {
		return socket_failure;
	}
}

operation_result tcp_send_data_bytes(char* data_buffer, size_t byte_count){
	if(write(tcp_sockfd, data_buffer, byte_count) > 0){
		return socket_success;
	} else {
		return socket_failure;
	}
}

operation_result tcp_recv_data_bytes(char* data_buffer, size_t byte_count){
	if(read(tcp_sockfd, data_buffer, byte_count) > 0){
	return socket_success;
	} else {
		return socket_failure;
	}
}

operation_result tcp_recv_data(char* data_buffer){
	if(read(tcp_sockfd, data_buffer, sizeof(data_buffer)) > 0){
	return socket_success;
	} else {
		return socket_failure;
	}
}

void load_heading_integer_to_byte_array(int number,char* array){
	//LSB first
	log_trace("number: %i ", number);
	array[0] = (unsigned char) (number & 0xFF);
	array[1] = (unsigned char) ((number>>8) & 0xFF);
	array[2] = (unsigned char) ((number>>16) & 0xFF);
	array[3] = (unsigned char) ((number>>24) & 0xFF);
	log_trace("bytes:%i,%i,%i,%i\n",array[0],array[1],array[2],array[3]);
}

operation_result tcp_send_rpc_request(rpc* request){
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

	if(write(tcp_sockfd, total_buf, strlen(rpc_buf)+4) > 0){
		return socket_success;
	} else {
		return socket_failure;
	}
	
}
int integerFromArrayTip(char* array){
	int number = 0;
	number = (int)((unsigned char)array[3]);
	number = (number<<8) + (unsigned char) array[2];
	number = (number<<8) + (unsigned char) array[1];
	number = (number<<8) + (unsigned char) array[0];
	log_trace("number:%i",number);
	return number;

}

operation_result tcp_recv_rpc_response(rpc* response){
	size_t payload_size;
	char resp_buf[500];//sizeof only works ok for static arrays i.e. results on 500
	bzero(resp_buf, sizeof(resp_buf));
	if(read(tcp_sockfd, resp_buf, sizeof(resp_buf)) > 0){
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
#define nofile "File Not Found!"
// funtion sending file
int load_file_buffer(FILE* fp, char* buf, int s) 
{ 
    int i, len; 
    if (fp == NULL) { 
        strcpy(buf, nofile); 
        len = strlen(nofile); 
        buf[len] = EOF;
        return 1; 
    } 
  
    char ch; 
    for (i = 0; i < s; i++) { 
        ch = fgetc(fp); 
        buf[i] = ch; 
        if (ch == EOF) 
            return 1; 
    } 
    return 0; 
} 
#define NET_BUF_SIZE 32 
operation_result tcp_send_file(char* file_name){
	FILE *file_ptr;
	file_ptr=fopen(file_name,"r");
	char file_buffer[NET_BUF_SIZE]; 
	while (1) {
		// process 
		if (load_file_buffer(file_ptr, file_buffer, NET_BUF_SIZE)) {
			if(write(tcp_sockfd, file_buffer, strlen(file_buffer)) > 0){
				break;
			} else {
				return socket_failure;
			}
		}
		// send 
		if(write(tcp_sockfd, file_buffer, NET_BUF_SIZE) <= 0){
			return socket_failure;
		}
		bzero(file_buffer,NET_BUF_SIZE);
	}
	if (file_ptr != NULL){
		fclose(file_ptr);
	}
	return socket_success;
}

int scan_input_buf_for_EOF(char* buf, int s) { 
    int i;
    char ch;
    for (i = 0; i < s; i++) {
        ch = buf[i];
        if (ch == EOF){
			if (i+1 < strlen(buf)) {
				buf[i+1] = '\0';
			}
			return 1;
		}
    }
    return 0;
}
operation_result tcp_recv_file(FILE* file_ptr){
	log_trace("RECV FILE");
	char input_buffer[NET_BUF_SIZE];
	while(1){
		bzero(input_buffer,NET_BUF_SIZE);
		if(read(tcp_sockfd, input_buffer, NET_BUF_SIZE) <= 0){
			log_error("Failure to read %i bytes on file transfer.",NET_BUF_SIZE);
			fclose(file_ptr);
			return socket_failure;
		}
		if(fwrite(input_buffer,1,sizeof(input_buffer),file_ptr) <= 0){
			log_error("Failure to write %i bytes into input file.",sizeof(input_buffer));
			fclose(file_ptr);
			return socket_failure;
		}
		if(scan_input_buf_for_EOF(input_buffer, NET_BUF_SIZE)){
			break;
		}	
	}
	fclose(file_ptr);
	log_debug("Successful file transfer.");
	return socket_success;
}

operation_result tcp_recv_file_known_size(FILE* input_file, size_t byte_count){
	return socket_success;
}

operation_result tcp_close_connection(){
	// close the socket 
    close(tcp_sockfd);
	return socket_success;
}