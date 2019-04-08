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
#include <errno.h>
 
#define PORT 12121 
#define SA struct sockaddr

int tcp_sockfd, connfd, client_address_len;
struct sockaddr_in tcp_servaddr, tcp_client_address;


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

operation_result tcp_init_server(){
	// socket create and verification 
    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (tcp_sockfd == -1) { 
        printf("socket creation failed...\n");
        return socket_failure;
    } else {
		printf("Socket successfully created..\n"); 
	}
        
    bzero(&tcp_servaddr, sizeof(tcp_servaddr)); 
  
    // assign IP, PORT 
    tcp_servaddr.sin_family = AF_INET; 
    tcp_servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    tcp_servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(tcp_sockfd, (SA*)&tcp_servaddr, sizeof(tcp_servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        return socket_failure;
    } else {
		printf("Socket successfully binded..\n"); 
	}
    // Now server is ready to listen and verification 
    if ((listen(tcp_sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        return socket_failure;
    } else {
		printf("Server listening..\n"); 
	}
    client_address_len = sizeof(tcp_client_address); 
    // Accept the data packet from client and verification 
    connfd = accept(tcp_sockfd, (SA*)&tcp_client_address, &client_address_len);
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        return socket_failure;
    } else {
		printf("server acccept the client...\n");
	}
	return tcp_timeouts(2);
}

operation_result tcp_timeouts(int seconds){
	struct timeval timeout;      
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;

    if (setsockopt (connfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
		log_error("setsockopt failed\n");
		return socket_failure;
	}
        
    if (setsockopt (connfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
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
		connfd = tcp_sockfd;
		return socket_success;
	}
}

operation_result tcp_send_data(char* data_buffer){
	if(write(connfd, data_buffer, sizeof(data_buffer)) > 0){
		return socket_success;
	} else {
		return socket_failure;
	}
}

operation_result tcp_send_data_bytes(char* data_buffer, size_t byte_count){
	if(write(connfd, data_buffer, byte_count) > 0){
		return socket_success;
	} else {
		return socket_failure;
	}
}

operation_result tcp_recv_data_bytes(char* data_buffer, size_t byte_count){
	if(read(connfd, data_buffer, byte_count) > 0){
	return socket_success;
	} else {
		return socket_failure;
	}
}

operation_result tcp_recv_data(char* data_buffer){
	if(read(connfd, data_buffer, sizeof(data_buffer)) > 0){
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

operation_result tcp_send_rpc(rpc* rpc_message){
	char total_buf[204];
	bzero(total_buf,sizeof(total_buf));

	char* rpc_buf = &total_buf[4];
	struct json_out output = JSON_OUT_BUF(rpc_buf, 200);

	json_printf(&output, RPC_JSON_FMT,
	rpc_message->command_id,
	rpc_message->satellite_id,
	rpc_message->station_id,
	rpc_message->payload);
	log_trace("RPC req: %s\n",rpc_buf);

	load_heading_integer_to_byte_array(strlen(rpc_buf),total_buf);
	
	log_trace("TOTAL req: %c%c%c%c%s\n",total_buf[0],total_buf[1],total_buf[2],total_buf[3],&total_buf[4]);

	if(write(connfd, total_buf, strlen(rpc_buf)+4) > 0){
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

operation_result tcp_recv_rpc(rpc* rpc_message){
	size_t payload_size;
	char input_buf[500];//sizeof only works ok for static arrays i.e. results on 500
	bzero(input_buf, sizeof(input_buf));
	if(read(connfd, input_buf, sizeof(input_buf)) > 0){
		int size_int = integerFromArrayTip(input_buf);
		char* recv_data = &input_buf[4];
		recv_data[size_int]='\0';
		log_trace("size_cadena:%lu,cadena: %s\n",strlen(recv_data),recv_data);
		log_trace("HOLOOO");
		
		json_scanf(recv_data,strlen(recv_data),RPC_JSON_FMT,
		& rpc_message->command_id,
		& rpc_message->satellite_id,
		& rpc_message->station_id,
		& rpc_message->payload);

		log_trace("payload size: %lu, apy:%s", strlen(rpc_message->payload),rpc_message->payload);
		
		log_trace("cid:%i,\nsatid:%i,\nstid:%i,\npay:%s",
		rpc_message->command_id,
		rpc_message->satellite_id,
		rpc_message->station_id,
		rpc_message->payload);
		
		return socket_success;

	} else {
		log_error("FAILED TO RECEIVE RPC MSG! Errno: %s", strerror(errno));
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
			if(write(connfd, file_buffer, strlen(file_buffer)) > 0){
				break;
			} else {
				return socket_failure;
			}
		}
		// send 
		if(write(connfd, file_buffer, NET_BUF_SIZE) <= 0){
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
		if(read(connfd, input_buffer, NET_BUF_SIZE) <= 0){
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