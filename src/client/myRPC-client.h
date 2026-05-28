#ifndef MYRPC_CLIENT_H
#define MYRPC_CLIENT_H

typedef struct {
char host[256];
int port;
int socket_type;
char command[4096];
int show_help;
} ClientConfig;

void print_help(const char *prog_name);
int parse_args(int argc, char *argv[], ClientConfig *config);
int connect_to_server(const ClientConfig *config);
int send_request(int socket, const char *username, const char *command);
int receive_response(int socket);