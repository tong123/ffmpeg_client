#include <stdio.h>
#include <QtEndian>
#include "video_decode.h"
//#include <Windows.h>
#include<sys/time.h>
#include <time.h>
#include <QTime>
#include <QDebug>
static QMutex    g_sws_mutex;
static UINT8 g_auc_vod_head[] = {0x00, 0x00, 0x01};
static UINT8 g_auc_ps_head[] = {0x00, 0x00, 0x01, 0xBA};
static UINT8 g_auc_ps_info[] = {0x00, 0x00, 0x01, 0xBC};
static UINT8 g_auc_vpes_head[] = {0x00, 0x00, 0x01, 0xE0};
static UINT8 g_auc_h264_head[] = {0x00, 0x00, 0x00, 0x01};
static UINT8 g_auc_h264_i_frame[] = {0x00, 0x00, 0x00, 0x01, 0x65};

#define PRINT_OPEN 0
#define DECODMIN(a, b) ((a) > (b) ? (b) : (a))

#if 0
#define IMG_NAME       "./monitor"
#define IMG_SUFFIX       ".bmp"

#define BOOL int
#define TRUE 1
#define FALSE 0
#define BI_RGB 0x0

#pragma pack(push)
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER
{
    unsigned short  bfType; //2 位图文件的类型，必须为“BM”
    unsigned long bfSize; //4 位图文件的大小，以字节为单位
    unsigned short bfReserved1; //2 位图文件保留字，必须为0
    unsigned short bfReserved2; //2 位图文件保留字，必须为0
    unsigned long bfOffBits; //4 位图数据的起始位置，以相对于位图文件头的偏移量表示，以字节为单位
} BITMAPFILEHEADER;//该结构占据14个字节。
//   printf("%d\n",sizeof(BITMAPFILEHEADER));

typedef struct tagBITMAPINFOHEADER{
    unsigned long biSize; //4 本结构所占用字节数
    long biWidth; //4 位图的宽度，以像素为单位
    long biHeight; //4 位图的高度，以像素为单位
    unsigned short biPlanes; //2 目标设备的平面数不清，必须为1
    unsigned short biBitCount;//2 每个像素所需的位数，必须是1(双色), 4(16色)，8(256色)或24(真彩色)之一
    unsigned long biCompression; //4 位图压缩类型，必须是 0(不压缩),1(BI_RLE8压缩类型)或2(BI_RLE4压缩类型)之一
    unsigned long biSizeImage; //4 位图的大小，以字节为单位
    long biXPelsPerMeter; //4 位图水平分辨率，每米像素数
    long biYPelsPerMeter; //4 位图垂直分辨率，每米像素数
    unsigned long biClrUsed;//4 位图实际使用的颜色表中的颜色数
    unsigned long biClrImportant;//4 位图显示过程中重要的颜色数
} BITMAPINFOHEADER;//该结构占据40个字节。
#pragma pack(pop)

int CreateBmp(const char *filename, uint8_t *pRGBBuffer, int width, int height, int bpp)
{
    BITMAPFILEHEADER bmpheader;
    BITMAPINFOHEADER bmpinfo;
    FILE *fd = 0;
    static char ac_FileName[512] = {0};
    static int i = 0;

    memset(ac_FileName, 0, sizeof(ac_FileName));
    sprintf(ac_FileName, "%s_%d"IMG_SUFFIX, filename, i++);
    printf("[%s][%s:%d]ac_FileName=%s\n", __FILE__, __FUNCTION__, __LINE__, ac_FileName);
    fd = fopen(ac_FileName, "wb");

    bmpheader.bfType = ('M' <<8)|'B';
    bmpheader.bfReserved1 = 0;
    bmpheader.bfReserved2 = 0;
    bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp/8;

    bmpinfo.biSize = sizeof(BITMAPINFOHEADER);
    bmpinfo.biWidth = width;
    bmpinfo.biHeight = 0 - height;
    bmpinfo.biPlanes = 1;
    bmpinfo.biBitCount = bpp;
    bmpinfo.biCompression = BI_RGB;
    bmpinfo.biSizeImage = 0;
    bmpinfo.biXPelsPerMeter = 100;
    bmpinfo.biYPelsPerMeter = 100;
    bmpinfo.biClrUsed = 0;
    bmpinfo.biClrImportant = 0;

    fwrite(&bmpheader, 1, sizeof(BITMAPFILEHEADER), fd);
    fwrite(&bmpinfo, 1, sizeof(BITMAPINFOHEADER), fd);
    fwrite(pRGBBuffer, 1, width*height*bpp/8, fd);
    fclose(fd);

    return TRUE;
}


int write_file_not_append(char *pc_FileName, unsigned char *pucBuf, int iBufLen)
{
    FILE * fd= NULL;
    int i_len = 0;

    fd = fopen(pc_FileName, "wb");

    i_len = fwrite(pucBuf, 1, iBufLen, fd);
    //printf("[%s][%s:%d]write len = %d\n", __FILE__, __FUNCTION__, __LINE__, i_len);

    fclose(fd);

    return 0;
}

int write_file(char *pc_FileName, unsigned char *pucBuf, int iBufLen)
{
    static FILE * fd= NULL;
    int i_len = 0;

    if (NULL==fd) {
        fd = fopen(pc_FileName, "wb");
    } else {
        fd = fopen(pc_FileName, "ab");
    }

    i_len = fwrite(pucBuf, 1, iBufLen, fd);
    //printf("[%s][%s:%d]write len = %d\n", __FILE__, __FUNCTION__, __LINE__, i_len);

    fclose(fd);

    return 0;
}

int g_file_no = 0;
char g_acFileName[128];
int write_file_each_frame(char *pc_FileName, unsigned char *pucBuf, int iBufLen)
{
    memset(g_acFileName, 0, sizeof(g_acFileName));
    snprintf(g_acFileName, 127, "%s_%d.hex", pc_FileName, g_file_no);
    g_file_no++;
    write_file_not_append(g_acFileName, pucBuf, iBufLen);

    return 0;
}


#endif
//INT32 gettimeofday(struct timeval *tp, void *tzp)
//{
//    time_t clock;
//    struct tm tm;
//    SYSTEMTIME wtm;
//    GetLocalTime(&wtm);

//    tm.tm_year     = wtm.wYear - 1900;
//    tm.tm_mon     = wtm.wMonth - 1;
//    tm.tm_mday     = wtm.wDay;
//    tm.tm_hour     = wtm.wHour;
//    tm.tm_min     = wtm.wMinute;
//    tm.tm_sec     = wtm.wSecond;
//    tm. tm_isdst    = -1;
//    clock = mktime(&tm);
//    tp->tv_sec = clock;
//    tp->tv_usec = wtm.wMilliseconds * 1000;
//    return (0);
//}

INT32 sys_time_getms(UINT64 *puSysMsTime)
{
    static struct timeval s_TimeInitValue = {0, 0};
    struct timeval tv;
    UINT64 temp = 0;

    gettimeofday(&tv, NULL);

    if (0 == s_TimeInitValue.tv_sec){
        s_TimeInitValue.tv_sec = tv.tv_sec;
        s_TimeInitValue.tv_usec = tv.tv_usec;
    }

    temp  = ((UINT64)(tv.tv_sec - s_TimeInitValue.tv_sec))*(UINT64)1000 + (((UINT64)tv.tv_usec) / 1000) -(((UINT64)s_TimeInitValue.tv_usec) / 1000);
    *puSysMsTime = temp;

    return 0;
}

bool judge_h264_i_frame(UINT8 *puc_data)
{
    bool b_ret = false;
    qDebug()<<puc_data[0]<<puc_data[1]<<puc_data[2]<<puc_data[3]<<puc_data[4]<<0x65;
//    if( puc_data[0] == 0x00 && puc_data[1] == 0x00 && puc_data[2] == 0x00 && puc_data[3] == 0x01 && puc_data[4] == 65 ) {
//        b_ret = true;
//    }
    if( memcmp(puc_data, g_auc_h264_i_frame, sizeof(g_auc_h264_i_frame)) == 0 ) {
        b_ret = true;
    }

    return b_ret;
}

INT32 VidConv::init( void )
{
    mui_data_tmp_total = 0;
    mui_frame_data_tmp_max_len = 1024*1024;
    mpuc_data_tmp = new UINT8[mui_frame_data_tmp_max_len]();
    //mst_media_buf_info.puc_buf = (UINT8 *)malloc(mst_media_buf_info.ui_buf_data_len);
    mst_media_buf_info.puc_buf = new UINT8[mst_media_buf_info.ui_buf_data_len]();

    mb_first_frame = true;
    mui_ps_one_frame_data_total = 0;
    mui_ps_one_frame_data_max_len = 128*1024;
    mpuc_ps_one_frame_data = new UINT8[mui_ps_one_frame_data_max_len]();
    if ( (NULL == mpuc_data_tmp) || (NULL == mst_media_buf_info.puc_buf) || (NULL == mpuc_ps_one_frame_data) ) { 
		qCritical("init malloc err");
		return -1; 
	}

    mst_media_buf_info.ui_buf_read_ptr = 0;
    mst_media_buf_info.ui_buf_write_ptr = 0;
    mst_media_buf_info.ui_real_data_len = 0;

    mb_convert = false;
    mb_pthread = false;
    mpf_out_buf = NULL;

    /****************** decode para *****************/
    mpuc_iobuffer = new UINT8[DECODE_BUF_LEN]();
//    if ( NULL == mpuc_iobuffer ) {
//        qCritical("mpuc_iobuffer malloc failed!\n");
//        return -1;
//    }
//    mpuc_iobuffer = (unsigned char *)av_malloc( DECODE_BUF_LEN );
//    mpuc_iobuffer = NULL;
    mpst_av_input_fmt = NULL;
    mpst_avio_ctx_pb = NULL;
    mpst_fmt_ctx = NULL;
    mpst_video_dec_ctx = NULL;
    mpst_frame = NULL;
    mpst_av_frame_rgb = NULL;

    mi_video_index = -1;

    mpst_av_stream = NULL;
    mpst_vid_codec = NULL;

    mpuc_rgb_frame_buf = NULL;
    mll_first_pts = 0;
    mui_start_time = 0;
    mui_end_time = 0;
    mi_sleep = 0;
    mull_frame_interval = 0;
    mp_rgb_data = new unsigned char[640*480*4];
    /**************** end decode para ***************/
    
    return 0;
}

INT32 VidConv::deinit( void )
{
    return 0;
}

VidConv::VidConv ( UINT32 n_buf_len, void *p_user_data )
{
    if (0 == n_buf_len) {
        mst_media_buf_info.ui_buf_data_len = MEDIA_BUF_LEN;
    } else {
        mst_media_buf_info.ui_buf_data_len = n_buf_len;
    }
    mp_user_data = p_user_data;
}

VidConv::~VidConv()
{
    if ( mst_media_buf_info.puc_buf != NULL ) {
        //free(mst_media_buf_info.puc_buf);
        delete [] mst_media_buf_info.puc_buf;
        mst_media_buf_info.puc_buf = NULL;
    }
    
    if ( NULL != mpuc_data_tmp ) {
        delete [] mpuc_data_tmp;
        mpuc_data_tmp = NULL;
    }

    if ( NULL != mpuc_ps_one_frame_data ) {
        delete [] mpuc_ps_one_frame_data;
        mpuc_ps_one_frame_data = NULL;
    }

    if ( NULL != mpuc_iobuffer ) {
        delete [] mpuc_iobuffer;
        mpuc_iobuffer = NULL;
    }
    
    mst_media_buf_info.ui_buf_data_len = 0;
    mst_media_buf_info.ui_buf_read_ptr = 0;
    mst_media_buf_info.ui_buf_write_ptr = 0;
    mst_media_buf_info.ui_real_data_len = 0;

    mpf_out_buf = NULL;
    mb_convert = false;
    mb_pthread = false;
}


UINT32 VidConv::get_used_data_len(void)
{
    return mst_media_buf_info.ui_real_data_len;
}

UINT32 VidConv::get_remain_data_len(void)
{
    return (mst_media_buf_info.ui_buf_data_len - mst_media_buf_info.ui_real_data_len);
}

#if 1
INT32 VidConv::get_data(UINT8 *puc_buf, UINT32 ui_data_len)
{
    UINT8 *puc_data_read_ptr = NULL;
    UINT32 ui_buf_left_len = 0;
    UINT32 ui_buf_real_read_len = ui_data_len;

    if (ui_data_len > get_used_data_len()) { ui_buf_real_read_len = get_used_data_len(); }

    puc_data_read_ptr = &mst_media_buf_info.puc_buf[mst_media_buf_info.ui_buf_read_ptr];
    ui_buf_left_len = mst_media_buf_info.ui_buf_data_len - mst_media_buf_info.ui_buf_read_ptr;

    if (ui_buf_left_len < ui_buf_real_read_len) {
        /* 循环读 */
        UINT32 ui_from_start_len = ui_buf_real_read_len - ui_buf_left_len;

        memcpy(puc_buf, puc_data_read_ptr, ui_buf_left_len);

        puc_data_read_ptr = &mst_media_buf_info.puc_buf[0];
        memcpy(puc_buf + ui_buf_left_len, puc_data_read_ptr, ui_from_start_len);
    } else {
        memcpy(puc_buf, puc_data_read_ptr, ui_buf_real_read_len);
    }

    mst_media_buf_info.ui_real_data_len -= ui_buf_real_read_len;
    mst_media_buf_info.ui_buf_read_ptr = (mst_media_buf_info.ui_buf_read_ptr + ui_buf_real_read_len) % mst_media_buf_info.ui_buf_data_len;

    return ui_buf_real_read_len;
}
#else
INT32 VidConv::get_data(UINT8 *puc_buf, UINT32 ui_data_len)
{
    UINT8 *puc_data_read_prt = NULL;
    UINT32 ui_buf_left_len = 0;
    UINT32 ui_buf_real_read_len = 0;
    MONITOR_HEAD_S st_head;
    UINT32 ui_buf_read_ptr_tmp = 0;

    qDebug("[%s][%s:%d]ui_data_len=%d", __FILE__, __FUNCTION__, __LINE__, ui_data_len);
while(ui_data_len > ui_buf_real_read_len)
{
    if (get_used_data_len() < sizeof(MONITOR_HEAD_S)) { return ui_buf_real_read_len; }

    ui_buf_read_ptr_tmp  = mst_media_buf_info.ui_buf_read_ptr;
    puc_data_read_prt = &mst_media_buf_info.puc_buf[ui_buf_read_ptr_tmp];
    ui_buf_left_len = mst_media_buf_info.ui_buf_data_len - ui_buf_read_ptr_tmp;

    /* 先读数据头 */
    if (ui_buf_left_len < sizeof(MONITOR_HEAD_S)) {
        /* 循环读 */
        UINT32 ui_from_start_len = sizeof(MONITOR_HEAD_S) - ui_buf_left_len;
        memcpy((unsigned char *)&st_head, puc_data_read_prt, ui_buf_left_len);
        puc_data_read_prt = &mst_media_buf_info.puc_buf[0];
        memcpy((unsigned char *)&st_head + ui_buf_left_len, puc_data_read_prt, ui_from_start_len);
    } else {
        memcpy((unsigned char *)&st_head, puc_data_read_prt, sizeof(MONITOR_HEAD_S));
    }

    qDebug("[%s][%s:%d]ui_Magic=%x, ui_type=%d, ui_Length=%d", __FILE__, __FUNCTION__, __LINE__, st_head.ui_Magic, st_head.ui_type, st_head.ui_Length);

    ui_buf_read_ptr_tmp = (mst_media_buf_info.ui_buf_read_ptr + sizeof(MONITOR_HEAD_S)) % mst_media_buf_info.ui_buf_data_len;
    puc_data_read_prt = &mst_media_buf_info.puc_buf[ui_buf_read_ptr_tmp];
    ui_buf_left_len = mst_media_buf_info.ui_buf_data_len - ui_buf_read_ptr_tmp;

    if ( get_used_data_len() < ( sizeof(MONITOR_HEAD_S) + st_head.ui_Length ) ) { return ui_buf_real_read_len; }

    ui_buf_real_read_len += st_head.ui_Length;

    if (ui_buf_left_len < st_head.ui_Length) {
        /* 循环读 */
        UINT32 ui_from_start_len = st_head.ui_Length - ui_buf_left_len;

        memcpy(puc_buf, puc_data_read_prt, ui_buf_left_len);

        puc_data_read_prt = &mst_media_buf_info.puc_buf[0];
        memcpy(puc_buf + ui_buf_left_len, puc_data_read_prt, ui_from_start_len);
    } else {
        memcpy(puc_buf, puc_data_read_prt, st_head.ui_Length);
    }

    mst_media_buf_info.ui_real_data_len -= st_head.ui_Length + sizeof(MONITOR_HEAD_S);
    mst_media_buf_info.ui_buf_read_ptr = (mst_media_buf_info.ui_buf_read_ptr + st_head.ui_Length + sizeof(MONITOR_HEAD_S)) % mst_media_buf_info.ui_buf_data_len;
    qDebug("[%s][%s:%d]st_head.ui_Length=%d,mst_media_buf_info.ui_buf_read_ptr=%d, mst_media_buf_info.ui_real_data_len=%d, ui_buf_real_read_len=%d",
           __FILE__, __FUNCTION__, __LINE__, st_head.ui_Length, mst_media_buf_info.ui_buf_read_ptr, mst_media_buf_info.ui_real_data_len, ui_buf_real_read_len);
}
    qDebug("[%s][%s:%d]ui_buf_real_read_len=%d", __FILE__, __FUNCTION__, __LINE__, ui_buf_real_read_len);
    return ui_buf_real_read_len;
}
#endif
#define WRITE_FILE_NAME "F:/test.mp4"

INT32 VidConv::put_data(UINT8 *puc_buf, UINT32 ui_data_len)
{
    //write_file("F:/put_origin_data.mp4", puc_buf, ui_data_len);
#if PRINT_OPEN
    static UINT64 start = 0;
    UINT64 end = 0;
#endif
#if 1
    MONITOR_HEAD_S *pst_head = NULL;
    UINT32 ui_real_data_len;
    UINT32 ui_data_tmp_read_ptr = 0;
    qDebug()<<"ui_data_len: "<<ui_data_len<<sizeof(MONITOR_HEAD_S);
    if ( ui_data_len <= sizeof(MONITOR_HEAD_S) ) {
        qCritical("data too less");
        return -1;
    }

    pst_head = (MONITOR_HEAD_S *)puc_buf;

    /*
    unsigned int ui_magic = qFromBigEndian(pst_head->ui_Magic);
    unsigned int ui_len  = qFromBigEndian(pst_head->ui_Length);
    unsigned int ui_type = qFromBigEndian(pst_head->ui_type);
    */

    unsigned int ui_magic = pst_head->ui_Magic;
    unsigned int ui_len = pst_head->ui_Length;
    unsigned int ui_type = pst_head->ui_type;
//    qDebug()<<"ui_type: "<<ui_type;

    if ( ui_len != ui_data_len ) {
        qCritical("data not match");
        return -1;
    }
    if ( ui_magic != 0xA0A05A5A ) {
        qDebug("magic error=%x", ui_magic);
        return -1;
    }
    if ( FRAME_DATA_TYPE_PS_HEAD == ui_type ) {
        judge_one_frame(pst_head->uc_data, ui_len - sizeof(MONITOR_HEAD_S));
        mb_first_frame = true;
        return 0;
    } 

    if ( FRAME_DATA_TYPE_H264 == ui_type ) {
        if ( true == mb_first_frame ) {
            if ( judge_h264_i_frame(pst_head->uc_data) == true ) {
                qDebug()<<"IDR frame";
                mb_first_frame = false;
                put_frame(pst_head->uc_data, ui_len - sizeof(MONITOR_HEAD_S));
            } else{
//                put_frame(pst_head->uc_data, ui_len - sizeof(MONITOR_HEAD_S));
                qDebug()<<"NOT IDR frame";
            }
        } else {
            put_frame(pst_head->uc_data, ui_len - sizeof(MONITOR_HEAD_S));
        }

        return 0;
    }

    ui_real_data_len = ui_len - sizeof(MONITOR_HEAD_S);
    
    if ( ( mui_data_tmp_total + ui_real_data_len ) > mui_frame_data_tmp_max_len ) {
        UINT8 *puc_frame_data_tmp = new(std::nothrow) UINT8[mui_data_tmp_total + ui_real_data_len]();
        if ( NULL == puc_frame_data_tmp ) {
            qCritical("malloc error");
            mui_data_tmp_total = 0;
            return -1;
        }
        mui_frame_data_tmp_max_len = mui_data_tmp_total + ui_real_data_len;
        memcpy(puc_frame_data_tmp, mpuc_data_tmp, mui_data_tmp_total);
        delete [] mpuc_data_tmp;
        mpuc_data_tmp = puc_frame_data_tmp;
    }
    memcpy(&mpuc_data_tmp[mui_data_tmp_total], pst_head->uc_data, ui_real_data_len);
    mui_data_tmp_total += ui_real_data_len;

    /* 判断帧是否正确 */
    while (1) {
        if (mui_data_tmp_total < 4) {
            break;
        }
        if ( (memcmp(&mpuc_data_tmp[ui_data_tmp_read_ptr], g_auc_vod_head, sizeof(g_auc_vod_head)) == 0)
           || (memcmp(&mpuc_data_tmp[ui_data_tmp_read_ptr], g_auc_h264_head, sizeof(g_auc_h264_head)) == 0)) {
            if ( mb_first_frame == true ) {
                if ( (memcmp(&mpuc_data_tmp[ui_data_tmp_read_ptr], g_auc_ps_head, sizeof(g_auc_ps_head)) == 0)
                    && (memcmp(&mpuc_data_tmp[ui_data_tmp_read_ptr + 20], g_auc_ps_info, sizeof(g_auc_ps_info)) == 0)) {
                    break;
                }
            } else {
                break;
            }
        }
        ui_data_tmp_read_ptr++;
        mui_data_tmp_total--;
    }

    /* 将正常帧放入 */
    while (mui_data_tmp_total >= ( 4 + 2 )) {
        UINT16 us_one_frame_len = 0;

        if ( mpuc_data_tmp[ui_data_tmp_read_ptr + sizeof(g_auc_vod_head)] == 0xba ) {
            us_one_frame_len = 20;
            if ( us_one_frame_len > mui_data_tmp_total ) {
                break;
            }
            judge_one_frame(&mpuc_data_tmp[ui_data_tmp_read_ptr], us_one_frame_len);
            ui_data_tmp_read_ptr += us_one_frame_len;
            mui_data_tmp_total -= us_one_frame_len;
        } else {
            memcpy(&us_one_frame_len, &mpuc_data_tmp[ui_data_tmp_read_ptr + 4], sizeof(us_one_frame_len));
            us_one_frame_len = qFromBigEndian(us_one_frame_len);
            us_one_frame_len += 4 + sizeof(us_one_frame_len);
            if ( us_one_frame_len > mui_data_tmp_total ) {
                break;
            }
            judge_one_frame(&mpuc_data_tmp[ui_data_tmp_read_ptr], us_one_frame_len);
            ui_data_tmp_read_ptr += us_one_frame_len;
            mui_data_tmp_total -= us_one_frame_len;
        }
    }

    memcpy(mpuc_data_tmp, &mpuc_data_tmp[ui_data_tmp_read_ptr], mui_data_tmp_total);
#else
    put_frame(puc_buf, ui_data_len);
#endif
#if PRINT_OPEN
    sys_time_getms(&end);
    qDebug("[%s][%s:%d]ui_data_len=%d, time diff = %llums", __FILE__, __FUNCTION__, __LINE__, ui_data_len, end - start);
    start = end;
#endif
    return 0;

}

INT32 VidConv::judge_one_frame(UINT8 *puc_buf, UINT32 ui_data_len)
{
//    qDebug()<<QString( (char*)puc_buf );
    if ( memcmp(puc_buf, g_auc_h264_head, sizeof(g_auc_h264_head)) == 0 ) { /* 纯h264帧格式的走该分支 */
//        qDebug()<<"hello judge_one_frame 1";
        put_frame(puc_buf, ui_data_len);
    } else { /* ps格式走该分支 */
//        qDebug()<<"hello judge_one_frame 2";

        if ( memcmp(puc_buf, g_auc_ps_head, sizeof(g_auc_ps_head)) == 0 ) {
            if (mui_ps_one_frame_data_total != 0) {

                if ( true == mb_first_frame ) {
                    mb_first_frame = false;
                } else {
//                    qDebug()<<"hello judge_one_frame 3";

                    put_frame(mpuc_ps_one_frame_data, mui_ps_one_frame_data_total);
                    mui_ps_one_frame_data_total = 0;
                }
            }
        }
        if ( (mui_ps_one_frame_data_total + ui_data_len) > mui_ps_one_frame_data_max_len ) {
            UINT8 *puc_tmp = new UINT8[mui_ps_one_frame_data_total + ui_data_len]();

            if ( NULL == puc_tmp) {
                /* 若没有内存了，则先将数据导入frame，清空缓存再接收数据 */
//                qDebug()<<"hello judge_one_frame 4";

                put_frame(mpuc_ps_one_frame_data, mui_ps_one_frame_data_total);
                mui_ps_one_frame_data_total = 0;
            } else {
                mui_ps_one_frame_data_max_len = mui_ps_one_frame_data_total + ui_data_len;
                memcpy(puc_tmp, mpuc_ps_one_frame_data, mui_ps_one_frame_data_total);
                delete [] mpuc_ps_one_frame_data;
                mpuc_ps_one_frame_data = puc_tmp;
            }
        }
        memcpy(mpuc_ps_one_frame_data + mui_ps_one_frame_data_total, puc_buf, ui_data_len);
        mui_ps_one_frame_data_total += ui_data_len;
    }

    return 0;
}

INT32 VidConv::put_frame(UINT8 *puc_buf, UINT32 ui_data_len)
{
    UINT8 *puc_data_write_ptr = NULL;
    UINT32 ui_buf_left_len = 0;

    this->lock();

//    write_file_each_frame("F:/data/server/1", puc_buf, ui_data_len);
//    write_file("F:/data/server/0", puc_buf, ui_data_len);

    if (ui_data_len > get_remain_data_len()) {
        this->unlock();
        return 1;
    }

    puc_data_write_ptr = &mst_media_buf_info.puc_buf[mst_media_buf_info.ui_buf_write_ptr];
    ui_buf_left_len = mst_media_buf_info.ui_buf_data_len - mst_media_buf_info.ui_buf_write_ptr;

    if (ui_data_len > ui_buf_left_len) {
        /* 循环写 */
        UINT32 ui_from_start_len = ui_data_len - ui_buf_left_len;

        memcpy(puc_data_write_ptr, puc_buf, ui_buf_left_len);

        puc_data_write_ptr = &mst_media_buf_info.puc_buf[0];
        memcpy(puc_data_write_ptr, puc_buf + ui_buf_left_len, ui_from_start_len);
    } else {
        memcpy(puc_data_write_ptr, puc_buf, ui_data_len);
    }

    mst_media_buf_info.ui_real_data_len += ui_data_len;
    mst_media_buf_info.ui_buf_write_ptr = (mst_media_buf_info.ui_buf_write_ptr + ui_data_len) % mst_media_buf_info.ui_buf_data_len;

    this->unlock();
    mb_convert = true;

    return 0;
}

int VidConv::fill_iobuffer(void * opaque, uint8_t *puc_iobuf, int i_bufsize)
{
    int i_read_len = 0;
    VidConv *p_this = (VidConv *)opaque;

    p_this->lock();
    i_read_len = p_this->get_data(puc_iobuf, i_bufsize);
    //if (i_read_len > 0) { write_file("F:/getdata.mp4", puc_iobuf, i_read_len); }

    p_this->unlock();
#if PRINT_OPEN
qDebug("[%s][%s:%d]get_data=%d", __FILE__, __FUNCTION__, __LINE__, i_read_len);
#endif
    if (i_read_len == 0) { QThread::msleep(1); }

    return i_read_len;
}

INT32 VidConv::decode_packet(int *pi_got_picture)
{
//    qDebug()<<"hello decode_packet";
    int i_decoded_len = st_orig_pkt.size;
    int ret = 0;

    if (st_orig_pkt.stream_index == mi_video_index) {
#if PRINT_OPEN
        qDebug("pkt.size=%d, pkt.pts=%lld, pkt.dts=%lld, pkt.data=0x%x.\n", st_orig_pkt.size, st_orig_pkt.pts, st_orig_pkt.dts,(unsigned int)st_orig_pkt.data);
#endif
        ret = avcodec_decode_video2(mpst_video_dec_ctx, mpst_frame, pi_got_picture, &st_orig_pkt);
#if PRINT_OPEN
        qDebug("size=%d, pi_got_picture=%d\n", ret, *pi_got_picture);
#endif
        if (ret <= 0) {
            UINT64 end = 0;
            sys_time_getms(&end);
            return ret ;
        }

        if (*pi_got_picture) {
            if ( 0 == mui_start_time ) {
                if ( st_orig_pkt.pts > 0 ) {
                    mll_first_pts = st_orig_pkt.pts;
                    sys_time_getms(&mui_start_time);
#if PRINT_OPEN
                    qDebug("mui_start_time = %llums, mll_first_pts=%lld\n", mui_start_time, mll_first_pts);
#endif
                }
            }
            mull_frame_interval = ( st_orig_pkt.pts - mll_first_pts ) * 1000 / 90000;
g_sws_mutex.lock();
            static struct SwsContext *img_convert_ctx;
            img_convert_ctx = sws_getContext(mpst_video_dec_ctx->width, mpst_video_dec_ctx->height, mpst_video_dec_ctx->pix_fmt, mpst_video_dec_ctx->width,
                                             mpst_video_dec_ctx->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
            if ( NULL == img_convert_ctx ) {
                qCritical("img_convert_ctx malloc failed!\n");
g_sws_mutex.unlock();
                return -1;
            }
            int PictureSize = avpicture_get_size (AV_PIX_FMT_RGBA, mpst_video_dec_ctx->width, mpst_video_dec_ctx->height);
            if (mpuc_rgb_frame_buf == NULL)
            {
                mpuc_rgb_frame_buf = (uint8_t*)malloc(PictureSize);
                if ( NULL == mpuc_rgb_frame_buf ) {
                    qCritical("mpuc_rgb_frame_buf malloc failed!\n");
                    sws_freeContext (img_convert_ctx);
g_sws_mutex.unlock();
                    return -1;
                }
                memset(mpuc_rgb_frame_buf, 0, PictureSize);
            } else {
                memset(mpuc_rgb_frame_buf, 0, PictureSize);
            }

            avpicture_fill ( (AVPicture *)mpst_av_frame_rgb, mpuc_rgb_frame_buf, AV_PIX_FMT_RGBA, mpst_video_dec_ctx->width, mpst_video_dec_ctx->height);
            sws_scale(img_convert_ctx, (const uint8_t* const*)mpst_frame->data, mpst_frame->linesize, 0,
                      mpst_video_dec_ctx->height, mpst_av_frame_rgb->data, mpst_av_frame_rgb->linesize);

            sws_freeContext (img_convert_ctx);
g_sws_mutex.unlock();
//            mpf_out_buf(mpst_av_frame_rgb->data[0], PictureSize, mpst_video_dec_ctx->width, mpst_video_dec_ctx->height, mp_user_data);
            memcpy( mp_rgb_data, (unsigned char*)mpst_av_frame_rgb->data[0], (unsigned int)PictureSize );
            mpf_out_buf( mp_rgb_data, PictureSize, 640, 480, mp_user_data );
//            emit this->imaged_data( mp_rgb_data, (unsigned int)PictureSize );
//            emit this->imaged_data( (unsigned char*)mpst_av_frame_rgb->data[0], (unsigned int)PictureSize );
            //CreateBmp("monitor", mpst_av_frame_rgb->data[0], mpst_video_dec_ctx->width, mpst_video_dec_ctx->height, 32);

            sys_time_getms(&mui_end_time);
            mi_sleep = mull_frame_interval - ( mui_end_time - mui_start_time );
#if PRINT_OPEN
            qDebug("[%s][%s:%d]mull_frame_interval=%llu, mi_sleep=%d, diff=%llu", __FILE__, __FUNCTION__, __LINE__, mull_frame_interval, mi_sleep, ( mui_end_time - mui_start_time ));
#endif
            if ((mi_sleep < 0)||(mi_sleep > 300)) {
                mi_sleep = 0;
            }
        }
    }

    i_decoded_len = DECODMIN(ret, i_decoded_len);
    
    return i_decoded_len;
}

void VidConv::run()
{
    INT32 i_ret = 0;
    int got_picture;

    mb_pthread = true;

//    qDebug("[%s][%s:%d]priority=%d", __FILE__, __FUNCTION__, __LINE__, priority());

    /* 当调用put_data函数时，会把mb_convert置为true */
    while ( false == mb_convert ) {
        if ( true != mb_pthread ) { return ;}
        QThread::msleep(50);
    }
	/* 做缓冲，避免花屏 */
    //QThread::msleep(500);
//    qDebug()<<"hello run 1";
    av_register_all();
//    qDebug()<<"hello run 2";
    //step1:申请一个AVIOContext
    /* 判断是否有视频配置数据 */
    mpst_avio_ctx_pb = avio_alloc_context(mpuc_iobuffer, DECODE_BUF_LEN, 0, this, &VidConv::fill_iobuffer, NULL, NULL);
    if ( NULL == mpst_avio_ctx_pb ) {
        i_ret = -1;
        qDebug()<<"mpst_avio_ctx_pb malloc failed!";
        return;
    }
    while (1) {
        if (false == mb_pthread) {
            return;
        }

        qDebug()<<"hello run 3";
        //step2:探测流格式
        int n_return = 0;
        if ( (n_return = av_probe_input_buffer(mpst_avio_ctx_pb, &mpst_av_input_fmt, "", NULL, 0, 0)) < 0) {
            qDebug()<<"probe failed!";
            qDebug( "probe return: %d\n", n_return );
            QThread::msleep(50);
        } else {
            qDebug("probe success!\n");
            qDebug("format: %s[%s]\n", mpst_av_input_fmt->name, mpst_av_input_fmt->long_name);
            if( strcmp( mpst_av_input_fmt->name,  "aac") == 0 ) {
                qDebug( "probe aac \n" );
                QThread::msleep(50);
            } else {
                break;
            }
        }
    }
//    qDebug()<<"hello run 5";

    mpst_fmt_ctx = avformat_alloc_context();
    if ( NULL == mpst_fmt_ctx ) {
        i_ret = -1;
        qCritical("mpst_fmt_ctx malloc failed!\n");
        goto ret2;
    } 
    mpst_fmt_ctx->pb = mpst_avio_ctx_pb;

    //step4:打开流
    if (avformat_open_input(&mpst_fmt_ctx, "", NULL, NULL) < 0) {
        qCritical("avformat open failed.");
        i_ret = 1;
        goto ret3;
    } else {
        qDebug("open stream success!\n");
    }

    //以下就和文件处理一致了
    if (avformat_find_stream_info(mpst_fmt_ctx, NULL) < 0) {
        qCritical("could not fine stream.");
        i_ret = 2;
        goto ret3;
    }
//    qDebug()<<"hello run 3";

    av_dump_format(mpst_fmt_ctx, 0, "", 0);
    //st = mpst_fmt_ctx->streams[0];
    //printf("[%s][%s:%d]num=%d, den=%d\n", __FILE__, __FUNCTION__, __LINE__, st->avg_frame_rate.num, st->avg_frame_rate.den);

    for (UINT32 i = 0; i < mpst_fmt_ctx->nb_streams; i++) {
        if ( (mpst_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) &&
             (mi_video_index < 0) ) {
            mi_video_index = i;
        }
    }

    if (mi_video_index < 0) {
        qDebug("mi_video_index=%d\n", mi_video_index);
        i_ret = 3;
        goto ret3;
    }

    mpst_av_stream = mpst_fmt_ctx->streams[mi_video_index];

    mpst_video_dec_ctx = mpst_av_stream->codec;

    mpst_vid_codec = avcodec_find_decoder(mpst_video_dec_ctx->codec_id);
    if (!mpst_vid_codec) {
        qCritical("could not find video decoder!\n");
        i_ret = 4;
        goto ret3;
    }
    if (avcodec_open2(mpst_video_dec_ctx, mpst_vid_codec, NULL) < 0) {
        qCritical("could not open video codec!\n");
        i_ret = 5;
        goto ret3;
    }

    mpst_frame = av_frame_alloc();
    if ( NULL == mpst_frame ) {
        i_ret = -1;
        qCritical("mpst_frame malloc failed!\n");
        goto ret4;
    }
    mpst_av_frame_rgb = av_frame_alloc();
    if ( NULL == mpst_av_frame_rgb ) {
        i_ret = -1;
        qCritical("mpst_frame malloc failed!\n");
        goto ret4;
    }
    av_init_packet(&st_orig_pkt);
    st_orig_pkt.data = NULL;
    st_orig_pkt.size = 0;
    qDebug()<<"hello run 4";

    while ((true == mb_convert)||(true == mb_pthread)) {
#if PRINT_OPEN
qDebug("[%s][%s:%d]real_data_len=%d", __FILE__, __FUNCTION__, __LINE__, mst_media_buf_info.ui_real_data_len);
#endif

#if PRINT_OPEN
        qDebug("[%s][%s:%d]start av_read_frame", __FILE__, __FUNCTION__, __LINE__);
#endif
        if (av_read_frame(mpst_fmt_ctx, &st_orig_pkt) < 0) {
            QThread::msleep(10);
            continue;
        }
#if PRINT_OPEN
        qDebug("[%s][%s:%d]after av_read_frame", __FILE__, __FUNCTION__, __LINE__);
#endif
        //AVPacket pkt = st_orig_pkt;
        do {
            int i_ret = decode_packet(&got_picture);
            if (i_ret <= 0) { break; }
            st_orig_pkt.data += i_ret;
            st_orig_pkt.size -= i_ret;

            QThread::msleep(mi_sleep);
            mi_sleep = 0;

        } while (st_orig_pkt.size > 0);
        av_free_packet(&st_orig_pkt);
    }
    qDebug()<<"hello run 5";

    /* flush cached frames */
    st_orig_pkt.data = NULL;
    st_orig_pkt.size = 0;
    do {
        decode_packet(&got_picture);
    } while (got_picture);
    qDebug()<<"hello run 6";

    free(mpuc_rgb_frame_buf);
    mpuc_rgb_frame_buf = NULL;
    av_free(mpst_frame);
    mpst_frame = NULL;
    av_free(mpst_av_frame_rgb);
    mpst_av_frame_rgb = NULL;

ret4:
    avcodec_close(mpst_video_dec_ctx); 
ret3:
    avformat_free_context(mpst_fmt_ctx);
ret2:
    av_free(mpst_avio_ctx_pb);
ret1:

    QThread::exit(i_ret);
}

int video_decode_init(YHANDLE *p_handle, int i_buf_len, pfOutDataBuf pf_out_data_func, void *p_user_data)
{
    VidConv *po_vid = new(std::nothrow) VidConv(i_buf_len, p_user_data);

    if ( NULL == po_vid ) {
        qCritical("new failed");
        return -1;
    }
    if ( NULL == p_handle ) {
        return -1;
    }

    *p_handle = po_vid;
    if ( po_vid->init() != 0 ) {
        qCritical("init failed");
        return -1;
    }
    qDebug()<<"hello world";
    po_vid->start_convert_video(pf_out_data_func);
    po_vid->start(QThread::TimeCriticalPriority);
    //po_vid->start();

    return 0;
}

int video_decode_deinit(YHANDLE *p_handle)
{
    VidConv *po_vid = (VidConv *)*p_handle;

    if ( NULL == po_vid ) {
        return -1;
    }

    if (true == po_vid->is_pthread_run())
    {
        po_vid->stop_convert_video();
    }
    else
    {
        qDebug("No Pthread Start!");
    }

    delete po_vid;
    *p_handle = NULL;

    return 0;
}

int video_put_data(YHANDLE handle, unsigned char *puc_buf, int i_len)
{
    VidConv *po_vid = (VidConv *)handle;
    
    if ( NULL == po_vid ) {
        return -1;
    }

    return po_vid->put_data(puc_buf, i_len);
}

unsigned int video_get_remain_buf(YHANDLE handle)
{
    VidConv *po_vid = (VidConv *)handle;
    unsigned int ui_remain_len = 0;

    if ( NULL == po_vid ) {
        return 0xFFFFFFFF;
    }

    po_vid->lock();

    ui_remain_len = po_vid->get_remain_data_len();

    po_vid->unlock();

    return ui_remain_len;
}

unsigned int video_get_used_buf(YHANDLE handle)
{
    VidConv *po_vid = (VidConv *)handle;
    unsigned int ui_used_len = 0;

    if ( NULL == po_vid ) {
        return 0xFFFFFFFF;
    }

    po_vid->lock();

    ui_used_len = po_vid->get_used_data_len();

    po_vid->unlock();

    return ui_used_len;
}


