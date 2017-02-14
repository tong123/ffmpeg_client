#ifndef QTNETSOCKET_H
#define QTNETSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

#ifndef DEFINE_YHANDLE
    typedef void* YHANDLE;
#define DEFINE_YHANDLE
#endif

//收到对端发送的完整网络数据时的回调函数
//p_buf:收到的网络数据存放区
//u_buf_len:收到的网络数据的长度
//h_peer:发送网络数据的对端的地址
//p_user_data:用户自定义数据
typedef bool ( *READ_CB_FUNC )( char *p_buf, unsigned long u_buf_len, YHANDLE h_peer, void *p_user_data );

class QtNetSocket : public QObject
{
    Q_OBJECT
public:
    explicit QtNetSocket( bool b_client, QTcpSocket *p_sock = 0, QObject *parent = 0 );
    ~QtNetSocket();
    bool stop_server();
    bool disconnect();
    bool send_data( QtNetSocket *p_net, YHANDLE h_data_buf, unsigned long u_buf_len );
    void free_recv_buf();
    bool mb_connected;
signals:

public slots:
protected slots:
    void receive_data();
    void disconnected_slot();
    void new_connect_slot();
    void error_slot(QAbstractSocket::SocketError err);
    void server_error_slot(QAbstractSocket::SocketError err);
    void bytes_written_slot(qint64 n_size);
private:
    //unsigned char *mp_head_buf;
public:
    READ_CB_FUNC mp_read_cb;
    void *mp_user_data;
    QString ms_ip;
    unsigned short mn_port;

    qint64 mn_receive_buf_len;
    char *mp_receive_buf;
    qint64 mn_receive_data_len;
    qint64 mn_need_write_data_len;
    QList<QtNetSocket *> m_client_list;

    QTcpServer *mp_server;
    QTcpSocket *mp_socket;
    qint64 time_tmp;
};

#ifndef YFDLL_YFNETSOCKET_API
#define YFDLL_YFNETSOCKET_API bool
#endif


//获取最新的库函数调用错误
//handle:网络库句柄
//error:错误码
YFDLL_YFNETSOCKET_API yf_net_socket_get_last_error( YHANDLE handle, unsigned int &error );
//网络库服务端初始化
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_tcp_server_init( YHANDLE& h );
//网络库结束
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_tcp_server_un_init( YHANDLE& h );
//启动网络库服务端(start_server_w:支持宽字符服务端ip,start_server_a:支持窄字符服务端ip)
//h:网络库句柄
//s_ip:服务端ip地址
//i_port:服务端监听端口
//p_read_cb_func:服务端收到客户端发送的完整数据时的回调函数
//p_user_data:用户自定义数据
//u_max_recv_packet_len:网络数据接收缓冲区最大长度
YFDLL_YFNETSOCKET_API yf_net_socket_start_server( YHANDLE h, const QString& s_ip, int i_port,
                                                    READ_CB_FUNC p_read_cb_func, void *p_user_data, unsigned long u_max_recv_packet_len );
YFDLL_YFNETSOCKET_API yf_net_socket_start_server_w( YHANDLE h, const std::wstring& s_ip, int i_port,
                                                        READ_CB_FUNC p_read_cb_func, void *p_user_data, unsigned long u_max_recv_packet_len );
YFDLL_YFNETSOCKET_API yf_net_socket_start_server_a( YHANDLE h, const std::string& s_ip, int i_port,
                                                        READ_CB_FUNC p_read_cb_func, void *p_user_data, unsigned long u_max_recv_packet_len );
//获取已连接的客户端个数
//h:网络库句柄
//n_cnt:已连接的客户端个数
YFDLL_YFNETSOCKET_API yf_net_socket_get_client_count( YHANDLE h, int &n_cnt );
//获取指定序号的已连接客户端的地址
//h:网络库句柄
//u_index:已连接的客户端序号(从0开始,如果有4个已连接客户端,则序号分别是0,1,2,3)
//h_peer:指定已连接客户端的地址
YFDLL_YFNETSOCKET_API yf_net_socket_get_peer( YHANDLE h, unsigned long u_index, YHANDLE &h_peer );
//停止服务端
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_stop_server( YHANDLE h );
//向指定网络端发送数据
//h:网络库句柄
//h_peer:数据发送目的网络端地址
//h_data_buf:发送数据存放区
//u_buf_len:发送数据长度
YFDLL_YFNETSOCKET_API yf_net_socket_send_data( YHANDLE h, YHANDLE h_peer, YHANDLE h_data_buf, unsigned long u_buf_len );
//判断是否可以向指定网络端发送数据
//h:网络库句柄
//h_peer:发送数据的目的端地址
YFDLL_YFNETSOCKET_API yf_net_socket_is_peer_writable( YHANDLE h, YHANDLE h_peer );
//网络库客户端初始化
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_tcp_client_init( YHANDLE& h );
//网络库客户端结束
YFDLL_YFNETSOCKET_API yf_net_socket_tcp_client_un_init( YHANDLE& h );
//客户端连接服务端函数(connect_server_w:支持宽字符服务端ip,connect_server_a:支持窄字符服务端ip)
//h:网络库句柄
//s_server_ip:连接服务端的ip
//i_port:连接服务端的端口
//p_read_cb:客户端收到服务端完整数据时的回调函数
//p_user_data:用户自定义数据
//u_max_recv_packet_len:网络数据接收缓冲区最大长度
YFDLL_YFNETSOCKET_API yf_net_socket_connect_server_w( YHANDLE h, const std::wstring& s_server_ip, int i_port,
                                                         READ_CB_FUNC p_read_cb, void *p_user_data, unsigned long u_max_packet_len );
YFDLL_YFNETSOCKET_API yf_net_socket_connect_server_a( YHANDLE h, const std::string& s_server_ip, int i_port,
                                                         READ_CB_FUNC p_read_cb, void *p_user_data, unsigned long u_max_packet_len );
YFDLL_YFNETSOCKET_API yf_net_socket_connect_server( YHANDLE h, QString& s_server_ip, int i_port,
                                                         READ_CB_FUNC p_read_cb, void *p_user_data, unsigned long u_max_packet_len );
//断开与服务端的连接
//h:网络库句柄
YFDLL_YFNETSOCKET_API yf_net_socket_disconnect_server( YHANDLE h );
//往服务端发送数据
//h:网络库句柄
//H_data_buf:发送数据存放区
//u_buf_len:发送数据长度
YFDLL_YFNETSOCKET_API yf_net_socket_send_data_to_server( YHANDLE h, YHANDLE h_data_buf, unsigned long u_buf_len );

#if defined( _UNICODE ) || defined( UNICODE )
    #define start_server yf_net_socket_start_server_w
    #define connect_server yf_net_socket_connect_server_w
#else
    #define start_server yf_net_socket_start_server_a
    #define connect_server yf_net_socket_connect_server_a
#endif

#endif // QTNETSOCKET_H
