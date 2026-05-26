#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include <sys/socket.h>
#include <netdb.h>
#include "network_socket.h"

static const char *TAG = "NETWORK_SOCKET";

// 全局变量用于存储 socket 文件描述符
static int s_socket = -1;

int network_socket_init()
{
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;
    
    // 1. 设置远程服务器地址信息
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    // 2. 创建 socket
    s_socket = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (s_socket < 0) {
        ESP_LOGE(TAG, "Failed to create socket: %d", errno);
        return -1;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", HOST_IP_ADDR, PORT);

    // 3. 连接服务器
    int err = connect(s_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket connection failed: %d", errno);
        close(s_socket);
        s_socket = -1;
        return -1;
    }
    ESP_LOGI(TAG, "Successfully connected to host.");
    
    return s_socket;
}

int network_socket_send(const void *data, size_t len)
{
    if (s_socket < 0) {
        ESP_LOGE(TAG, "Socket is not initialized or connected.");
        return -1;
    }
    
    // 使用 send() 函数发送数据
    int bytes_sent = send(s_socket, data, len, 0);

    if (bytes_sent < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: %d", errno);
        return -1;
    }
    
    if (bytes_sent != len) {
        // TCP 通常会保证发送完整，但仍需检查
        ESP_LOGW(TAG, "Warning: Expected %zu bytes, but sent %d bytes.", len, bytes_sent);
    }
    return bytes_sent;
}


void network_socket_close()
{
    if (s_socket >= 0) {
        shutdown(s_socket, 0);
        close(s_socket);
        s_socket = -1;
        ESP_LOGI(TAG, "Socket closed.");
    }
}

int network_socket_data_publish(const void *data, size_t len)
{
    if (network_socket_init() < 0)
    {
        ESP_LOGE(TAG, "Failed to connect to host.");
        return ESP_FAIL;
    }
    
    int bytes_sent = network_socket_send(data, len);

    if (bytes_sent != len)
    {
        ESP_LOGE(TAG, "Transmission failed or incomplete!");
    }

    network_socket_close();
    return 0;
}
