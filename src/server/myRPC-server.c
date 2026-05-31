#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include "myRPC-server.h"
#include "../../common/protocol.h"

// Пути к конфигурационным файлам
#define DEFAULT_CONFIG_PATH "/etc/myRPC/myRPC.conf"
#define DEFAULT_USERS_PATH "/etc/myRPC/users.conf"

// Глобальные флаги сигналов
volatile sig_atomic_t reload_config = 0;
volatile sig_atomic_t terminate_server = 0;

// Простая функция логирования (без внешней библиотеки)
static FILE *log_fp = NULL;
static int use_syslog_flag = 1;

static void init_logging(const char *log_path) {
    if (log_path && strlen(log_path) > 0) {
        log_fp = fopen(log_path, "a");
        if (log_fp) {
            use_syslog_flag = 0;
            return;
        }
    }
    use_syslog_flag = 1;
}

static void write_log(const char *level, const char *msg) {
    if (use_syslog_flag) {
        // В реальной системе здесь был бы syslog
        fprintf(stderr, "[%s] %s\n", level, msg);
    } else if (log_fp) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strlen(time_str) - 1] = '\0';
        fprintf(log_fp, "[%s] [%s] %s\n", time_str, level, msg);
        fflush(log_fp);
    }
}

static void close_logging(void) {
    if (log_fp) {
        fclose(log_fp);
        log_fp = NULL;
    }
}

// Обработчик сигналов
void signal_handler(int sig) {
    switch (sig) {
        case SIGHUP:
            reload_config = 1;
            write_log("INFO", "Получен сигнал SIGHUP - перезагрузка конфигурации");
            break;
        case SIGTERM:
        case SIGINT:
            terminate_server = 1;
            write_log("INFO", "Получен сигнал завершения");
            break;
        case SIGCHLD:
            // Забираем завершенные дочерние процессы
            while (waitpid((pid_t)(-1), NULL, WNOHANG) > 0);
            break;
    }
}

// Демонизация процесса
void daemonize(void) {
    pid_t pid, sid;
    
    pid = fork();
    if (pid < 0) {
        write_log("ERROR", "Ошибка fork() при демонизации");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    sid = setsid();
    if (sid < 0) {
        write_log("ERROR", "Ошибка setsid() при демонизации");
        exit(EXIT_FAILURE);
    }
    
    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null != -1) {
        dup2(dev_null, STDIN_FILENO);
        dup2(dev_null, STDOUT_FILENO);
        dup2(dev_null, STDERR_FILENO);
        if (dev_null > 2) close(dev_null);
    }
    
    chdir("/");
    umask(0);
}

// Загрузка конфигурации
int load_config(const char *config_path, ServerConfig *config) {
    FILE *file = fopen(config_path, "r");
    if (!file) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Не удалось открыть конфиг: %s", config_path);
        write_log("ERROR", msg);
        return -1;
    }
    
    // Значения по умолчанию
    config->port = 8642;
    config->socket_type = SOCK_STREAM;
    config->daemon_mode = 1;
    strcpy(config->log_file, "");
    strcpy(config->users_file, DEFAULT_USERS_PATH);
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *comment = strchr(line, '#');
        if (comment) *comment = '\0';
        
        char *start = line;
        while (isspace(*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && isspace(*end)) *end-- = '\0';
        
        if (strlen(start) == 0) continue;
        
        char key[128], value[128];
        if (sscanf(start, "%127[^=]=%127s", key, value) == 2) {
            char *k = key;
            while (isspace(*k)) k++;
            end = k + strlen(k) - 1;
            while (end > k && isspace(*end)) *end-- = '\0';
            
            if (strcmp(k, "port") == 0) {
                config->port = atoi(value);
            } else if (strcmp(k, "socket_type") == 0) {
                if (strcmp(value, "dgram") == 0) {
                    config->socket_type = SOCK_DGRAM;
                } else {
                    config->socket_type = SOCK_STREAM;
                }
            } else if (strcmp(k, "mode") == 0) {
                config->daemon_mode = (strcmp(value, "daemon") == 0) ? 1 : 0;
            } else if (strcmp(k, "log_file") == 0) {
                strcpy(config->log_file, value);
            }
        }
    }
    
    fclose(file);
    return 0;
}

// Проверка пользователя в whitelist
int check_user(const char *username, const char *users_file) {
    FILE *file = fopen(users_file, "r");
    if (!file) {
        write_log("ERROR", "Не удалось открыть файл пользователей");
        return 0;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *comment = strchr(line, '#');
        if (comment) *comment = '\0';
        
        char *user = line;
        while (isspace(*user)) user++;
        char *end = user + strlen(user) - 1;
        while (end > user && isspace(*end)) *end-- = '\0';
        
        if (strcmp(user, username) == 0) {
            fclose(file);
            return 1;
        }
    }
    
    fclose(file);
    return 0;
}

// Выполнение команды
int execute_command(const char *command, char *output, size_t output_size) {
    char tmp_stdout[64];
    char tmp_stderr[64];
    
    snprintf(tmp_stdout, sizeof(tmp_stdout), "/tmp/myRPC_out_%d", getpid());
    snprintf(tmp_stderr, sizeof(tmp_stderr), "/tmp/myRPC_err_%d", getpid());
    
    char cmd[8192];
    snprintf(cmd, sizeof(cmd), "%s > %s 2> %s", command, tmp_stdout, tmp_stderr);
    
    int ret = system(cmd);
    
    output[0] = '\0';
    
    FILE *fout = fopen(tmp_stdout, "r");
    if (fout) {
        size_t n = fread(output, 1, output_size - 1, fout);
        if (n > 0) output[n] = '\0';
        fclose(fout);
    }
    
    if (strlen(output) == 0) {
        FILE *ferr = fopen(tmp_stderr, "r");
        if (ferr) {
            size_t n = fread(output, 1, output_size - 1, ferr);
            if (n > 0) output[n] = '\0';
            fclose(ferr);
        }
    }
    
    if (strlen(output) == 0) {
        snprintf(output, output_size, "Команда выполнена (код: %d)", WEXITSTATUS(ret));
    }
    
    unlink(tmp_stdout);
    unlink(tmp_stderr);
    
    return WIFEXITED(ret) ? WEXITSTATUS(ret) : -1;
}

// Обработка клиента
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    
    ssize_t bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        write_log("ERROR", "Ошибка получения данных от клиента");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    buffer[bytes] = '\0';
    write_log("DEBUG", "Получен запрос от клиента");
    
    // Парсим запрос
    Request req;
    if (parse_json_request(buffer, &req) != 0) {
        write_log("ERROR", "Ошибка разбора JSON запроса");
        char *error_resp = build_json_response(1, "Неверный формат запроса");
        if (error_resp) {
            send(client_socket, error_resp, strlen(error_resp), 0);
            free(error_resp);
        }
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    char msg[512];
    snprintf(msg, sizeof(msg), "Пользователь %s запросил команду: %s", 
             req.username, req.command);
    write_log("INFO", msg);
    
    // Проверяем пользователя
    if (!check_user(req.username, DEFAULT_USERS_PATH)) {
        snprintf(msg, sizeof(msg), "Доступ запрещен для пользователя: %s", req.username);
        write_log("WARNING", msg);
        char *error_resp = build_json_response(1, "Доступ запрещен");
        if (error_resp) {
            send(client_socket, error_resp, strlen(error_resp), 0);
            free(error_resp);
        }
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    char result[65536];
    int exit_code = execute_command(req.command, result, sizeof(result));
    
    char *response = build_json_response(exit_code == 0 ? 0 : 1, result);
    if (response) {
        send(client_socket, response, strlen(response), 0);
        free(response);
    }
    
    close(client_socket);
    exit(EXIT_SUCCESS);
}
int main(int argc, char *argv[]) {
    ServerConfig config;
    const char *config_path = DEFAULT_CONFIG_PATH;
    
    if (argc > 1) {
        config_path = argv[1];
    }
    
    if (load_config(config_path, &config) != 0) {
        fprintf(stderr, "Ошибка загрузки конфигурации из %s\n", config_path);
        fprintf(stderr, "Используйте: %s [путь_к_конфигу]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    init_logging(config.log_file);
    
    write_log("INFO", "Запуск myRPC сервера");
    
    if (config.daemon_mode) {
        daemonize();
        init_logging(config.log_file);
    }
    
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);
    
    int server_socket = socket(AF_INET, config.socket_type, 0);
    if (server_socket == -1) {
        write_log("ERROR", "Ошибка создания сокета");
        return EXIT_FAILURE;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(config.port);
    
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        write_log("ERROR", "Ошибка привязки сокета");
        close(server_socket);
        return EXIT_FAILURE;
    }
    
    if (config.socket_type == SOCK_STREAM) {
        if (listen(server_socket, MAX_CLIENTS) == -1) {
            write_log("ERROR", "Ошибка прослушивания сокета");
            close(server_socket);
            return EXIT_FAILURE;
        }
    }
    
    char msg[256];
    snprintf(msg, sizeof(msg), "Сервер запущен на порту %d (%s)", 
             config.port, 
             config.socket_type == SOCK_STREAM ? "TCP" : "UDP");
    write_log("INFO", msg);
    
    while (!terminate_server) {
        if (reload_config) {
            write_log("INFO", "Перезагрузка конфигурации...");
            if (load_config(config_path, &config) == 0) {
                write_log("INFO", "Конфигурация перезагружена успешно");
            }
            reload_config = 0;
        }
        
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket;
        
        if (config.socket_type == SOCK_STREAM) {
            client_socket = accept(server_socket, 
                                   (struct sockaddr *)&client_addr, 
                                   &client_len);
            if (client_socket == -1) {
                if (terminate_server) break;
                continue;
            }
            
            pid_t pid = fork();
            if (pid == 0) {
                close(server_socket);
                handle_client(client_socket);
                exit(EXIT_SUCCESS);
            } else {
                close(client_socket);
            }
        } else {
            char udp_buffer[BUFFER_SIZE];
            ssize_t bytes = recvfrom(server_socket, udp_buffer, 
                                     sizeof(udp_buffer), 0,
                                     (struct sockaddr *)&client_addr, 
                                     &client_len);
            if (bytes <= 0) {
                if (terminate_server) break;
                continue;
            }
            
            udp_buffer[bytes] = '\0';
            write_log("DEBUG", "Получен UDP запрос");
            
            pid_t pid = fork();
            if (pid == 0) {
                Request req;
                if (parse_json_request(udp_buffer, &req) == 0) {
                    snprintf(msg, sizeof(msg), "Пользователь %s запросил команду: %s", 
                             req.username, req.command);
                    write_log("INFO", msg);
                    
                    if (check_user(req.username, DEFAULT_USERS_PATH)) {
                        char result[65536];
                        int code = execute_command(req.command, result, sizeof(result));
                        char *response = build_json_response(code == 0 ? 0 : 1, result);
                        if (response) {
                            sendto(server_socket, response, strlen(response), 0,
                                  (struct sockaddr *)&client_addr, client_len);
                            free(response);
                        }
                    } else {
                        snprintf(msg, sizeof(msg), "Доступ запрещен для пользователя: %s", req.username);
                        write_log("WARNING", msg);
                        char *error_resp = build_json_response(1, "Доступ запрещен");
                        if (error_resp) {
                            sendto(server_socket, error_resp, strlen(error_resp), 0,
                                  (struct sockaddr *)&client_addr, client_len);
                            free(error_resp);
                        }
                    }
                }
                exit(EXIT_SUCCESS);
            }
            continue;
        }
    }
    
    write_log("INFO", "Завершение работы сервера...");
    
    while (waitpid((pid_t)(-1), NULL, 0) > 0);
    
    close(server_socket);
    close_logging();
    
    return EXIT_SUCCESS;
}