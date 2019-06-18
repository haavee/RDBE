#ifndef STM_UTIL_H
#define STM_UTIL_H

#define NRDBE (sizeof(stm_addr->rdbe)/sizeof(stm_addr->rdbe[0]))

int resolve_host(const char* host, int socktype, int protocol, struct sockaddr_in* dst);

#endif
