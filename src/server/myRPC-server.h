#ifndef MYRPC_SERVER_H
#define MYRPC_SERVER_H

#include <netinet/in.h>
#include <signal.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 65536

// Структура конфигурации сервера
typedef struct {
    int port;
    int socket_type;
    int daemon_mode;
    char log_file[256];
    char users_file[256];
} ServerConfig;

// Глобальные флаги для обработки сигналов
extern volatile sig_atomic_t reload_config;
extern volatile sig_atomic_t terminate_server;

// Функции
void signal_handler(int sig);
void daemonize(void);
int load_config(const char *config_path, ServerConfig *config);
int check_user(const char *username, const char *users_file);
int execute_command(const char *command, char *output, size_t output_size);
void handle_client(int client_socket);
#endif