#ifndef __VIDEO_DECODE_H_
#define __VIDEO_DECODE_H_

//#ifdef _cplusplus
extern "C"
{
//#endif
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include  <libavutil/avstring.h>
#include  <libavutil/opt.h>
#include  <libavutil/parseutils.h>
#include  <libavutil/pixdesc.h>
#include  <libavutil/frame.h>

#include  <libavutil/imgutils.h>
#include  <libavutil/samplefmt.h>
//#include <libavutil/timestamp.h>

#include  <libavformat/avformat.h>
#include  <libavcodec/avcodec.h>
#include  <libswscale/swscale.h>


//#ifdef _cplusplus
}
//#endif

#include <QMutex>
#include <QThread>
#include <qlogging.h>

#include "video_decode_api.h"

#define DECODE_BUF_LEN     (10*1024*1024)// (2*32*1024)//(10*1024*1024)
#define MEDIA_BUF_LEN       (30*1024*1024)

#define MEDIA_PATH          "/root/share/streamdata.mp4"

//typedef char                INT8;
typedef short               INT16;
typedef int                 INT32;
typedef signed long long    INT64;
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;

enum {
    FRAME_DATA_TYPE_OTHER    = 0,
    FRAME_DATA_TYPE_I_FRAME  = 1,
    FRAME_DATA_TYPE_H264     = 2,
    FRAME_DATA_TYPE_PS_HEAD  = 3,

    FRAME_DATA_TYPE_BUTT
};


#pragma pack(push)
#pragma pack(1)
typedef struct tagMONITOR_HEAD_S{
    unsigned int ui_Magic;
    unsigned int ui_type;
    unsigned int ui_Length;
    UINT8        uc_data[0];
}MONITOR_HEAD_S;
#pragma pack(pop)

class VidConv:public QThread
{
        Q_OBJECT
public:
    VidConv ( UINT32 n_buf_len, void *p_user_data );
    ~VidConv();
    INT32 init(void);
    INT32 deinit(void);
    INT32 put_data(UINT8 *puc_buf, UINT32 ui_data_len);
    void start_convert_video(pfOutDataBuf pf_out_data_func) { mpf_out_buf = pf_out_data_func; }
    void stop_convert_video(void) { mb_pthread = false; mb_convert = false; this->wait(); }
    void run();
    bool is_pthread_run (void) { return mb_pthread; }
    void lock()   { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    UINT32 get_used_data_len(void);
    UINT32 get_remain_data_len(void);
signals:
    void imaged_data( unsigned char *puc_out_buf, unsigned int ui_data_len );
private:
    typedef struct MEDIA_BUF_INFO{
        UINT32 ui_real_data_len;    /* 当前buf中的真实数据的大小 */
        UINT32 ui_buf_data_len;     /* 视频注入buf的大小 */
        UINT8 *puc_buf;             /* 视频注入的buf */
        UINT32 ui_buf_read_ptr;     /* buf的读指针 */
        UINT32 ui_buf_write_ptr;    /* buf的读指针 */
        //INT32  i_is_full;           /* 指明当前buf的满空状态，1表示满，0表示空 */
    }MEDIA_BUF_INFO_S;

    MEDIA_BUF_INFO_S mst_media_buf_info;
    QMutex           m_mutex;
    bool             mb_convert;
    bool             mb_pthread;
    pfOutDataBuf     mpf_out_buf;

    UINT8 *mpuc_data_tmp;
    UINT32 mui_frame_data_tmp_max_len;
    UINT32 mui_data_tmp_total;

    /* 用于存放ps流中的完整一帧数据，然后再扔进解码器 */
    bool  mb_first_frame;
    UINT8 *mpuc_ps_one_frame_data;
    UINT32 mui_ps_one_frame_data_max_len;
    UINT32 mui_ps_one_frame_data_total;

    //UINT32 mui_max_recv_frame_len;
    void *mp_user_data;

    /****************** decode para *****************/
    UINT8           *mpuc_iobuffer;
//    unsigned char   *mpuc_iobuffer;
    AVInputFormat   *mpst_av_input_fmt;
    AVIOContext     *mpst_avio_ctx_pb;
    AVFormatContext *mpst_fmt_ctx;
    AVCodecContext  *mpst_video_dec_ctx;
    AVFrame         *mpst_frame;
    AVFrame         *mpst_av_frame_rgb;
    AVPacket         st_orig_pkt;

    int mi_video_index;

    AVStream *mpst_av_stream;
    AVCodec *mpst_vid_codec;

    UINT8 *mpuc_rgb_frame_buf;
    INT64 mll_first_pts;
    UINT64 mui_start_time;
    UINT64 mui_end_time;
    INT32 mi_sleep;
    UINT64 mull_frame_interval;
    /**************** end decode para ***************/

    INT32 get_data(UINT8 *puc_buf, UINT32 ui_data_len);

    INT32 decode_packet(int *pi_got_picture);
    static int fill_iobuffer(void * opaque, uint8_t *puc_iobuf, int i_bufsize);
    INT32 judge_one_frame(UINT8 *puc_buf, UINT32 ui_data_len);
	
    bool b_h264_head;
    unsigned char *mp_rgb_data;
public:
    INT32 put_frame(UINT8 *puc_buf, UINT32 ui_data_len);

};






#endif
//end of video_convert.h

