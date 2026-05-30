#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <errno.h>

#include "myRPC-client.h"
#include "../../common/protocol.h"

void print_help(const char *prog_name) {
printf("myRPC-client - удаленный вызов команд\n\n");
printf("Использование: %s [ОПЦИИ]\n\n", prog_name);
printf(" -c, --command CMD Команда bash\n");
printf(" -h, --host ADDR IP-адрес сервера\n");
printf(" -p, --port PORT Порт сервера\n");
printf(" -s, --stream TCP сокет\n");
printf(" -d, --dgram UDP сокет\n");
printf(" --help Справка\n\n");
printf("Пример: %s -h 127.0.0.1 -p 8642 -s -c \"ls\"\n", prog_name);
}

int parse_args(int argc, char *argv[], ClientConfig *config) {
memset(config, 0, sizeof(ClientConfig));
config->port = 8642;
config->socket_type = SOCK_STREAM;
strcpy(config->host, "127.0.0.1");

struct option long_options[] = {
{"command", required_argument, 0, 'c'},
{"host", required_argument, 0, 'h'},
{"port", required_argument, 0, 'p'},
{"stream", no_argument, 0, 's'},
{"dgram", no_argument, 0, 'd'},
{"help", no_argument, 0, '?'},
{0, 0, 0, 0}
};

int opt;
while ((opt = getopt_long(argc, argv, "c:h:p:sd", long_options, NULL)) != -1) {
switch (opt) {
case 'c': strncpy(config->command, optarg, 4095); break;
case 'h': strncpy(config->host, optarg, 255); break;
case 'p': config->port = atoi(optarg); break;
case 's': config->socket_type = SOCK_STREAM; break;
case 'd': config->socket_type = SOCK_DGRAM; break;
case '?': config->show_help = 1; return 0;
default: return -1;
}
}

if (strlen(config->command) == 0) {
fprintf(stderr, "Ошибка: не указана команда\n");
return -1;
}
return 0;
}

int connect_to_server(const ClientConfig *config) {
int sock = socket(AF_INET, config->socket_type, 0);
if (sock == -1) {
perror("socket");
return -1;
}

struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = htons(config->port);

if (inet_pton(AF_INET, config->host, &addr.sin_addr) <= 0) {
fprintf(stderr, "Неверный адрес: %s\n", config->host);
close(sock);
return -1;
}

if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
perror("connect");
close(sock);
return -1;
}

printf("Подключен к %s:%d (%s)\n",
config->host, config->port,
config->socket_type == SOCK_STREAM ? "TCP" : "UDP");
return sock;
}

int send_request(int socket, const char *username, const char *command) {
char *json = build_json_request(username, command);
if (!json) {
fprintf(stderr, "Ошибка создания запроса\n");
return -1;
}

printf("Отправка: %s\n", json);

if (send(socket, json, strlen(json), 0) == -1) {
perror("send");
free(json);
return -1;
}

free(json);
return 0;
}

int receive_response(int socket) {
char buffer[65536];
ssize_t n = recv(socket, buffer, sizeof(buffer) - 1, 0);

if (n <= 0) {
if (n == 0) fprintf(stderr, "Сервер закрыл соединение\n");
else perror("recv");
return -1;
}

buffer[n] = '\0';
printf("Ответ: %s\n", buffer);

Response resp;
if (parse_json_response(buffer, &resp) != 0) {
fprintf(stderr, "Ошибка разбора ответа\n");
return -1;
}

printf("\n=== Результат ===\n%s\n=================\n", resp.result);
printf("Код: %d (%s)\n", resp.code, resp.code == 0 ? "Успех" : "Ошибка");

return resp.code;
}

int main(int argc, char *argv[]) {
ClientConfig config;

if (parse_args(argc, argv, &config) != 0 || config.show_help) {
print_help(argv[0]);
return config.show_help ? 0 : 1;
}

struct passwd *pw = getpwuid(getuid());
if (!pw) {
perror("getpwuid");
return 1;
}
printf("Пользователь: %s\n", pw->pw_name);

int sock = connect_to_server(&config);
if (sock == -1) return 1;

if (send_request(sock, pw->pw_name, config.command) != 0) {
close(sock);
return 1;
}

int result = receive_response(sock);
close(sock);

return result == 0 ? 0 : 1;
}