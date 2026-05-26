#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

#define HOST_IP_ADDR "192.168.0.242"
#define PORT 8888

int network_socket_init();
int network_socket_send(const void *data, size_t len);
void network_socket_close();
int network_socket_data_publish(const void *data, size_t len);



#endif // NETWORK_SOCKET_H