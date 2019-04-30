#include "command_router.h"
#include "command_controller.h"
#include "shared/socket_operation.h"
#include "log.h"
#include <stdlib.h>

void init_router(int params_count, char* program_params[]){
    if (params_count < 2){
        printf("Uso: $ %s client_hostname_or_ip_address\n",program_params[0]);
        log_error("Program params missing.");
        exit(0);
    }
    tcp_init_server();
    log_trace("Program params count %i.",params_count);
    udp_init_server(program_params[1]);
}
void route_commands(rpc* rpc_request){
    int command_code=-1;
    command_code = rpc_request->command_id;
    log_debug("command requested: %i",command_code);
    switch (command_code)
    {
        case 1:
            firmware_update(rpc_request->payload);
            break;
        case 2:
            earth_surfice_scan();
            break;
        case 3:
            system_telemetry();
            break;
        default:
            log_error("Unsupported command code");
            break;
    }
}

void process_request(){
    rpc rpc_request;
    operation_result op_result;
    op_result = tcp_recv_rpc(& rpc_request);
    if (op_result == socket_success) {
        route_commands(&rpc_request);
    }
    op_result = udp_recv_rpc(& rpc_request);
    if (op_result == socket_success) {
        route_commands(&rpc_request);
    }
}

