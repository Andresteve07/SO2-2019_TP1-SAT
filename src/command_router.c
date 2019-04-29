#include "command_router.h"
#include "command_controller.h"
#include "shared/socket_operation.h"
#include "log.h"

void init_router(){
    tcp_init_server();
    udp_init_server("127.0.0.1");
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

