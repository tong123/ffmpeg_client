#ifndef WIFICLIENT_H
#define WIFICLIENT_H

#include <QObject>
#include "NetDiscover.h"
#include <QList>
#include <QTimer>
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include "decode_process.h"
#include "ms-net/IRProtocol.h"

class WifiClient;
class ReadThread;
class Decode_H264;
#define BUFFER_SIZE 1024*1024*50

class WifiClient : public QObject
{
    Q_OBJECT
public:
    friend class Decode_H264;
    friend class ReadThread;
    explicit WifiClient(QObject *parent = 0);
    Q_INVOKABLE bool connect_to_server( QString s_ip );
    Q_INVOKABLE void search_online_device( );
    Q_INVOKABLE int get_mac_ip_list_count( );
    Q_INVOKABLE QString get_current_mac_ip_content( int n_index );
    Q_INVOKABLE bool connect_to_current_online_device(int n_index );
    Q_INVOKABLE void disconnect_current_server();

    YHANDLE &get_handle() { return m_discover_handle; }
    static bool net_connect( const std::string& s_name, const std::string& s_ip, void *p_user_data );
    void net_connect_data_slot( const std::string& s, const std::string& ip, void * );
    static void server_msg_cb( struct bufferevent *bev, void* arg );
    static void event_cb( struct bufferevent *bev, short event, void *arg );
    Q_INVOKABLE unsigned int get_desktop_width();
    Q_INVOKABLE unsigned int get_desktop_height();
    Q_INVOKABLE unsigned int get_current_font_size( int n_index );
    void get_network_interface( );
    void read_cb_fuc_slot( unsigned char *p_buf, unsigned long u_buf_len );

signals:
    Q_INVOKABLE void update_discover_model( );
    void connect_to_server_result( bool b_status );
    Q_INVOKABLE void set_ir_img_show( unsigned char *puc_out_buf, unsigned int n_data_len );
//    void image_changed( QString puc_out_buf, int n_data_len );
    void image_changed( unsigned int puc_out_buf, int n_data_len );
    void ir_image_changed( unsigned char* puc_out_buf, int n_data_len );
    void disconnect_net_status();
public slots:
    void find_online_device();
    void connect_slot( );
    void disconnect_slot( );
    void data_receive( );
    void display_error(QAbstractSocket::SocketError);
    void ip_find_time_slot();
    void set_ir_img_data( unsigned char *puc_out_buf, unsigned int ui_data_len );
private:
    unsigned int mn_width;
    unsigned int mn_height;
    ReadThread *mp_read_thread;
    YHANDLE m_discover_handle;
    QList<QString> s_mac_ip_list;
    QTimer *m_discover_timer;
    QTcpSocket *mp_socket;
    QHostAddress mp_host_address;
    QString m_network_ip;
    QString m_network_mac;
    QTimer *m_find_timer;
    YHANDLE mh_find_handle;

    qint64 mn_receive_buf_len;
    char *mp_receive_buf;
    qint64 mn_receive_data_len;
    qint64 mn_need_write_data_len;
    qint64 time_tmp;
    Decode_H264 *mp_decode;
    CIRProtocol m_head;
    bool mb_update_status;
};

class ReadThread : public QThread
{
    Q_OBJECT
public:
    friend class WifiClient;
    explicit ReadThread( WifiClient *mp );
    void run();
private:
    WifiClient *mp_wifi_client;
    QString s_server_ip;
    struct event_base *base;
    struct bufferevent *bev;
};

#endif // WIFICLIENT_H
