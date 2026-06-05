#include "ping.h"
#define HTTPSERVER_IMPL
#define EPOLL
#include "httpserver.h/httpserver.h"

extern unsigned char deer_running[];
extern unsigned int deer_running_len;
extern unsigned char deer_sleeping[];
extern unsigned int deer_sleeping_len;

void handle_404(struct http_request_s *request)
{
    const char response_body[] = "Not Found";
    struct http_response_s *response = http_response_init();
    http_response_status(response, 404);
    http_response_header(response, "Content-Type", "text/plain");
    http_response_body(response, response_body, sizeof(response_body));
    http_respond(request, response);
    http_request_free_buffer(request);
}

void handle_favicon(struct http_request_s *request)
{
    handle_404(request);
}

void handle_status(struct http_request_s *request)
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

void get_url_target_length(struct http_string_s *url_target)
{
    char ch = 0;
    unsigned int length = 0;
    while (ch != ' ' && ch != '\n')
    {
        ch = url_target->buf[length++];
    }
    url_target->len = length - 1;
}

void handle_request(struct http_request_s *request)
{
    struct http_string_s url_target = http_request_target(request);
    get_url_target_length(&url_target);

    printf("Got target: '");
    fwrite(url_target.buf, url_target.len, sizeof(char), stdout);
    printf("', size: %d\n", url_target.len);

    const char favicon_target[] = "/favicon.ico";

    if (!memcmp("/favicon.ico", url_target.buf, sizeof(favicon_target) - 1))
    {
        handle_favicon(request);
    }
    else
    {
        handle_status(request);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s IPADDRESS\n", argv[0]);
        return 1;
    }
    ping_set_ip_address(argv[1]);

    int server_port = 0xdee;

    struct http_server_s *server = http_server_init(server_port, handle_request);
    printf("Started server on: 127.0.0.1:%d\n", server_port);
    http_server_listen(server);
    return 0;
}
