#ifndef SRC_COMMAND_ROUTER_H_
#define SRC_COMMAND_ROUTER_H_
#include "shared/socket_operation.h"

void init_router(int params_count, char* program_params[]);
void process_request();

#endif /* SRC_COMMAND_ROUTER_H_ */