#include "server.h"
#include "mongoose.h"

#define URL_MAX_LEN 255

extern unsigned char deer_alive[];
extern unsigned int deer_alive_len;
extern unsigned char deer_dead[];
extern unsigned int deer_dead_len;

struct mg_mgr mgr;

typedef struct server_t
{
    unsigned int server_port;
} server_t;

static server_t prv_server =
{
    .server_port = DEFAULT_SERVER_PORT
};

static server_t *get_server_instance(void)
{
    return &prv_server;
}

static void server_handler(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        // struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        mg_http_reply(c, 200, "", "%s\n", deer_alive);
    }
}

void server_start(void)
{
    server_t *pdev = get_server_instance();
    char url[URL_MAX_LEN];
    sprintf(url, "http://0.0.0.0:%d", pdev->server_port);

    mg_mgr_init(&mgr);
    mg_log_set(MG_LL_ERROR);

    mg_http_listen(&mgr, url, server_handler, NULL);

    while (1)
    {
        mg_mgr_poll(&mgr, 1000);
    }
}

void server_set_port(unsigned int port)
{
    server_t *pdev = get_server_instance();
    pdev->server_port = port;
}
