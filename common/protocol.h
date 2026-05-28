#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>

#define MAX_USERNAME 256
#define MAX_COMMAND 4096
#define MAX_RESULT 65536

typedef struct {
    char username[MAX_USERNAME];
    char command[MAX_COMMAND];
} Request;

typedef struct {
    int code;
    char result[MAX_RESULT];
} Response;

char* build_json_request(const char *username, const char *command);
int parse_json_request(const char *json_str, Request *req);
char* build_json_response(int code, const char *result);
int parse_json_response(const char *json_str, Response *resp);
char* json_escape(const char *input);

#endif
