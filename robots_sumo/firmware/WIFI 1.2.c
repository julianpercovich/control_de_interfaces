#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define TCP_PORT 80
#define HTTP_GET "GET"

static struct tcp_pcb *server_pcb;

static const char *html_page =
    "<!DOCTYPE html><html><head><title>Pico W Control</title>"
    "<script>"
    "  function sendCmd(cmd) { fetch('/wasd/' + cmd); }"
    "  document.addEventListener('keydown', function(event) {"
    "    if (event.repeat) return;" // Evita saturar el servidor si mantienes la tecla apretada
    "    const key = event.key.toLowerCase();"
    "    if (['w', 'a', 's', 'd'].includes(key)) {"
    "      sendCmd(key);"
    "    }"
    "  });"
    "</script>"
    "</head>"
    "<body style=\"font-family:sans-serif;text-align:center;margin-top:40px;\">"
    "<h1>Control Pico W</h1>"
    "<p>Presiona las teclas <b>W, A, S, D</b> en tu teclado.</p>"
    "<div>"
    "<button onclick=\"sendCmd('w')\" style=\"font-size:22px;padding:15px 20px;margin:5px;\">W</button>"
    "</div>"
    "<div>"
    "<button onclick=\"sendCmd('a')\" style=\"font-size:22px;padding:15px 20px;margin:5px;\">A</button>"
    "<button onclick=\"sendCmd('s')\" style=\"font-size:22px;padding:15px 20px;margin:5px;\">S</button>"
    "<button onclick=\"sendCmd('d')\" style=\"font-size:22px;padding:15px 20px;margin:5px;\">D</button>"
    "</div>"
    "</body></html>";

// Arma la respuesta HTTP completa (headers + cuerpo) en buf, devuelve la longitud
static int build_response(char *buf, size_t buf_len) {
    int body_len = strlen(html_page);
    return snprintf(buf, buf_len,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        body_len, html_page);
}

// --- Procesa el comando recibido (acá es donde después se puede llamar a los motores) ---
static void procesar_comando(char letra) {
    switch (letra) {
        case 'w':
            printf("W - Adelante\n");
            break;
        case 'a':
            printf("A - Izquierda\n");
            break;
        case 's':
            printf("S - Atras\n");
            break;
        case 'd':
            printf("D - Derecha\n");
            break;
        default:
            printf("Comando no reconocido: %c\n", letra);
            break;
    }
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    tcp_close(tpcb);
    return ERR_OK;
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        // El cliente cerró la conexión
        tcp_close(tpcb);
        return ERR_OK;
    }

    char request[512];
    int copy_len = p->len < (int)sizeof(request) - 1 ? p->len : (int)sizeof(request) - 1;
    memcpy(request, p->payload, copy_len);
    request[copy_len] = '\0';

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);

    // Solo miramos la primera línea: "GET /ruta HTTP/1.1"
    if (strncmp(request, HTTP_GET, strlen(HTTP_GET)) == 0) {
        if (strstr(request, "GET /wasd/w")) {
            procesar_comando('w');
        } else if (strstr(request, "GET /wasd/a")) {
            procesar_comando('a');
        } else if (strstr(request, "GET /wasd/s")) {
            procesar_comando('s');
        } else if (strstr(request, "GET /wasd/d")) {
            procesar_comando('d');
        }
    }

    static char response[1024];
    int len = build_response(response, sizeof(response));

    tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, tcp_server_sent);
    tcp_write(tpcb, response, len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

static bool tcp_server_open(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("No se pudo crear el PCB\n");
        return false;
    }

    if (tcp_bind(pcb, IP_ADDR_ANY, TCP_PORT) != ERR_OK) {
        printf("No se pudo bindear al puerto %d\n", TCP_PORT);
        return false;
    }

    server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!server_pcb) {
        printf("No se pudo poner el socket en escucha\n");
        tcp_close(pcb);
        return false;
    }

    tcp_accept(server_pcb, tcp_server_accept);
    printf("Servidor web escuchando en el puerto %d\n", TCP_PORT);
    return true;
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms("Samsung7", "12345678", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    }

    printf("Connected.\n");
    uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
    printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    printf("Abri esa IP en el navegador para mandar comandos W/A/S/D\n");

    if (!tcp_server_open()) {
        printf("No se pudo iniciar el servidor\n");
        return -1;
    }

    while (true) {
        // Necesario si usás cyw43_arch en modo "poll"
        cyw43_arch_poll();
        sleep_ms(10);
    }
}
