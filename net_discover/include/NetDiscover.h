#ifndef NETDISCOVER_H
#define NETDISCOVER_H
#define DOG_PORT 56875
#define DATA_PORT 56886
#define CMD_PORT 56885
#include "yf-net-discover.h"

#define server_callback_init        yf_net_discover_server_init
#define server_find                 yf_net_discover_server_find_equipments
#define server_uninit               yf_net_discover_server_uninit

#define Client_init                 yf_net_discover_client_init
#if 1
#include "yf-net-socket.h"
//#include "../ms-net/qt-net-socket.h"

#define socket_tcp_client_init      yf_net_socket_tcp_client_init
#define socket_tcp_client_un_init   yf_net_socket_tcp_client_un_init

#define socket_connect_server       yf_net_socket_connect_server_a
#define socket_send_data_to_server  yf_net_socket_send_data_to_server
#define socket_disconnect_server    yf_net_socket_disconnect_server
#endif

#if 0
#include "../../public/net/yf-net-socket.h"
#define socket_tcp_client_init      tcp_client_init
#define socket_tcp_client_un_init   tcp_client_un_init

#define socket_connect_server       connect_server_a
#define socket_send_data_to_server  send_data_to_server
#define socket_disconnect_server    disconnect_server

#endif

#endif // NETDISCOVER_H
