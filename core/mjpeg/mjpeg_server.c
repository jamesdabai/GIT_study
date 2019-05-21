#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <rtthread.h>
#ifdef RT_USING_DFS_NET
#include <dfs_posix.h>
#include <sys/socket.h>
#else
#include <lwip/sockets.h>
#endif

#define MJPEG_BOUNDARY "boundarydonotcross"

#define MJPEG_SERVER_DEBUG

#if defined(MJPEG_SERVER_DEBUG) && defined(RT_DEBUG)
#define debug rt_kprintf
#else
#define debug(fmt, ...)
#endif

struct mjpeg_client_context
{
    int fd;
    struct sockaddr_in addr;
    int start_play;
};

struct mjpeg_server_context
{
    int stop;
    int fd;
    rt_thread_t thread_id;
    int port;
    struct mjpeg_client_context *client;
};

static struct mjpeg_server_context g_server;
static char g_send_buf[1024];

int send_first_response(struct mjpeg_client_context *client)
{
    g_send_buf[0] = 0;
    snprintf(g_send_buf, 1024,
             "HTTP/1.0 200 OK\r\n"
             "Connection: close\r\n"
             "Server: MJPG-Streamer/0.2\r\n"
             "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n"
             "Pragma: no-cache\r\n"
             "Expires: Mon, 3 Jan 2000 12:34:56 GMT\r\n"
             "Content-Type: multipart/x-mixed-replace;boundary=" MJPEG_BOUNDARY
             "\r\n"
             "\r\n"
             "--" MJPEG_BOUNDARY "\r\n");
    if (send(client->fd, g_send_buf, strlen(g_send_buf), 0) < 0)
        return -1;
    return 0;
}

void mjpeg_server_thread(void *arg)
{
    struct mjpeg_server_context *server = (struct mjpeg_server_context *)arg;
    struct sockaddr_in addr;
    socklen_t sock_len                  = sizeof(struct sockaddr_in);
    struct mjpeg_client_context *client = RT_NULL;
    int on;

    server->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->fd < 0)
    {
        debug("mjpeg_server: create server socket failed due to (%s)\n",
              strerror(errno));
        return;
    }

    bzero(&addr, sock_len);
    addr.sin_family      = AF_INET;
    addr.sin_port        = server->port;
    addr.sin_addr.s_addr = INADDR_ANY;

    /* ignore "socket already in use" errors */
    on = 1;
    setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    /* lwip_setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); */

    if (bind(server->fd, (struct sockaddr *)&addr, sock_len) != 0)
    {
        debug("mjpeg_server: bind() failed due to (%s)\n", strerror(errno));
        goto exit;
    }

    if (listen(server->fd, RT_LWIP_TCP_PCB_NUM) != 0)
    {
        debug("mjpeg_server: listen() failed due to (%s)\n", strerror(errno));
        goto exit;
    }

    client = rt_malloc(sizeof(struct mjpeg_client_context));
    if (client == RT_NULL)
    {
        debug("mjpeg_server: create client context failed\n");
        goto exit;
    }
    client->fd         = -1;
    client->start_play = 0;
    server->client     = client;

    while (!server->stop)
    {
        if (client->fd == -1)
        {
            if (client->start_play == 1)
            {
                debug("mjpeg_server: client disconnected\n");
                client->start_play = 0;
            }
            client->fd =
                accept(server->fd, (struct sockaddr *)&client->addr, &sock_len);
            if (client->fd < 0)
                continue;

            debug("mjpeg_server: client connected\n");
            if (send_first_response(client) < 0)
            {
                client->fd = -1;
                continue;
            }
            client->start_play = 1;
        }
        else
        {
            rt_thread_delay(100);
        }
    }

#ifdef RT_USING_DFS_NET
    if (client->fd)
        close(client->fd);
#else
    if (client->fd >= 0)
        lwip_close(client->fd);
#endif
    rt_free(client);
    server->client = RT_NULL;
exit:
#ifdef RT_USING_DFS_NET
    close(server->fd);
#else
    lwip_close(server->fd);
#endif
}

void mjpeg_send_stream(void *data, int size)
{
    g_send_buf[0] = 0;

    if (g_server.client && g_server.client->start_play)
    {
        snprintf(g_send_buf, 1024,
                 "Content-Type: image/jpeg\r\n"
                 "Content-Length: %d\r\n"
                 "\r\n",
                 size);
        if (send(g_server.client->fd, g_send_buf, strlen(g_send_buf), 0) < 0)
        {
#ifdef RT_USING_DFS_NET
            close(g_server.client->fd);
#else
            lwip_close(g_server.client->fd);
#endif
            g_server.client->fd = -1;
            return;
        }

        if (send(g_server.client->fd, data, size, 0) < 0)
        {
#ifdef RT_USING_DFS_NET
            close(g_server.client->fd);
#else
            lwip_close(g_server.client->fd);
#endif
            g_server.client->fd = -1;
            return;
        }

        g_send_buf[0] = 0;
        snprintf(g_send_buf, 1024, "\r\n--" MJPEG_BOUNDARY "\r\n");
        if (send(g_server.client->fd, g_send_buf, strlen(g_send_buf), 0) < 0)
        {
            g_server.client->fd = -1;
        }
    }
}

int mjpeg_start_server(int port)
{
    g_server.stop      = 0;
    g_server.port      = htons(port);
    g_server.thread_id = rt_thread_create("mjpeg_server", mjpeg_server_thread,
                                          &g_server, 1024, 17, 10);
    if (g_server.thread_id != RT_NULL)
        rt_thread_startup(g_server.thread_id);
    else
        return -1;

    return 0;
}

void mjpeg_stop_server(void)
{
    g_server.stop = 1;
    if (g_server.client)
    {
#ifdef RT_USING_DFS_NET
        close(g_server.client->fd);
#else
        lwip_close(g_server.client->fd);
#endif
        rt_free(g_server.client);
        g_server.client = RT_NULL;
    }
    rt_thread_delete(g_server.thread_id);
#ifdef RT_USING_DFS_NET
    close(g_server.fd);
#else
    lwip_close(g_server.fd);
#endif
}
