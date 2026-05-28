#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

char* json_escape(const char *input) {
    if (!input) return NULL;
    size_t len = strlen(input);
    char *escaped = malloc(len * 2 + 1);
    if (!escaped) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '"':  escaped[j++] = '\\'; escaped[j++] = '"'; break;
            case '\\': escaped[j++] = '\\'; escaped[j++] = '\\'; break;
            case '\n': escaped[j++] = '\\'; escaped[j++] = 'n'; break;
            case '\r': escaped[j++] = '\\'; escaped[j++] = 'r'; break;
            case '\t': escaped[j++] = '\\'; escaped[j++] = 't'; break;
            default:   escaped[j++] = input[i];
        }
    }
    escaped[j] = '\0';
    return escaped;
}

char* build_json_request(const char *username, const char *command) {
    if (!username || !command) return NULL;
    char *eu = json_escape(username);
    char *ec = json_escape(command);
    if (!eu || !ec) { free(eu); free(ec); return NULL; }
    
    size_t size = strlen(eu) + strlen(ec) + 50;
    char *json = malloc(size);
    if (json) {
        snprintf(json, size, "{\"login\":\"%s\",\"command\":\"%s\"}", eu, ec);
    }
    free(eu); free(ec);
    return json;
}

int parse_json_request(const char *json_str, Request *req) {
    if (!json_str || !req) return -1;
    const char *p = strstr(json_str, "\"login\":\"");
    if (!p) return -1;
    p += 9;
    const char *e = strchr(p, '"');
    if (!e) return -1;
    size_t len = e - p;
    if (len >= MAX_USERNAME) len = MAX_USERNAME - 1;
    strncpy(req->username, p, len);
    req->username[len] = '\0';
    
    p = strstr(json_str, "\"command\":\"");
    if (!p) return -1;
    p += 11;
    e = strchr(p, '"');
    if (!e) return -1;
    len = e - p;
    if (len >= MAX_COMMAND) len = MAX_COMMAND - 1;
    strncpy(req->command, p, len);
    req->command[len] = '\0';
    return 0;
}

char* build_json_response(int code, const char *result) {
    if (!result) return NULL;
    char *er = json_escape(result);
    if (!er) return NULL;
    size_t size = strlen(er) + 100;
    char *json = malloc(size);
    if (json) {
        snprintf(json, size, "{\"code\":%d,\"result\":\"%s\"}", code, er);
    }
    free(er);
    return json;
}

int parse_json_response(const char *json_str, Response *resp) {
    if (!json_str || !resp) return -1;
    const char *p = strstr(json_str, "\"code\":");
    if (!p) return -1;
    resp->code = atoi(p + 7);
    
    p = strstr(json_str, "\"result\":\"");
    if (!p) return -1;
    p += 10;
    const char *e = strchr(p, '"');
    if (!e) return -1;
    size_t len = e - p;
    if (len >= MAX_RESULT) len = MAX_RESULT - 1;
    strncpy(resp->result, p, len);
    resp->result[len] = '\0';
    return 0;
}
