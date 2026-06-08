#include <stdatomic.h>
#define HTTPSERVER_IMPL
#include "httpserver.h/httpserver.h"
#include "ping.h"
#include "server.h"
#include <stdio.h>

extern unsigned char deer_running[];
extern unsigned int deer_running_len;
extern unsigned char deer_sleeping[];
extern unsigned int deer_sleeping_len;

typedef struct server_t
{
    int server_port;
} server_t;

static server_t prv_server =
{
    .server_port = DEFAULT_SERVER_PORT
};

static server_t *get_server_instance(void)
{
    return &prv_server;
}

static void __attribute__((unused))
handle_404(struct http_request_s *request)
{
    const char response_body[] = "Not Found";
    struct http_response_s *response = http_response_init();
    http_response_status(response, 404);
    http_response_header(response, "Content-Type", "text/html");
    http_response_body(response, response_body, sizeof(response_body));
    http_respond(request, response);
    http_request_free_buffer(request);
}

static void handle_status(struct http_request_s *request)
{
    int ping_status = get_ping_status();
    unsigned char *response_body = ping_status ? deer_running : deer_sleeping;
    unsigned int response_body_len = ping_status ? deer_running_len : deer_sleeping_len;
    struct http_response_s *response = http_response_init();
    http_response_status(response, 200);
    http_response_header(response, "Content-Type", "text/html");
    http_response_body(response, (const char *)response_body, response_body_len);
    http_respond(request, response);
}

static void get_url_target_length(struct http_string_s *url_target)
{
    char ch = 0;
    unsigned int length = 0;
    while (ch != ' ' && ch != '\n')
    {
        ch = url_target->buf[length++];
    }
    url_target->len = length - 1;
}

static void request_handler(struct http_request_s *request)
{
    struct http_string_s url_target = http_request_target(request);
    get_url_target_length(&url_target);

    printf("Got target: '");
    fwrite(url_target.buf, url_target.len, sizeof(char), stdout);
    printf("', size: %d\n", url_target.len);

    handle_status(request);
}

void server_start(void)
{
    server_t *pdev = get_server_instance();
    struct http_server_s *server = http_server_init(pdev->server_port, request_handler);
    printf("Started server on: http://127.0.0.1:%d\n", pdev->server_port);
    http_server_listen(server);
}

void server_set_port(unsigned int port)
{
    server_t *pdev = get_server_instance();
    pdev->server_port = port;
}
