#ifndef __HDLC_H__
#define __HDLC_H__

#ifndef __uint8_t
typedef unsigned char __uint8_t;
#endif

#ifndef __int32_t
typedef int __int32_t;
#endif

/* define HDLC control struct */
typedef struct _HDLC {
	__uint8_t state; /*HDLC process state */
	__uint8_t *recv_buffer; /* receive destination buffer pointer */
	__int32_t recv_buffer_length; /* receive destination buffer length in bytes*/
	__int32_t recvd_length; /* the counter of received bytes */
} HDLC;

/**
 *hdlc_init_recv - Init receive buffer for the HDLC object. The object save
 *                  the buffer and the max lengthfor process control.
 * @hdlc:       HDLC control object pointer
 * @buffer: The receive buffer pointer for this HDLCcontrol object
 *@buffer_length:  The max receive bufferlength
 * @return: 0success, -1 failed
 */
int hdlc_init_recv(HDLC *hdlc, void *buffer, int buffer_length);

/**
 *hdlc_recv_char - Receiving one char and set it into data stream buffer.
 * @hdlc:   HDLC control object pointer
 * @ch:     A new byte of received data stream
 * @return:
 *      > 0 --> the data queue length ofreceived packet.
 *      0 --> the receiving is not complete.
 *      -1--> receive buffer overflow, failed.
 */
int hdlc_recv_char(HDLC *hdlc, unsigned char ch);

/**
 *hdlc_send_buffer - Make data queue with HDLC, from src to dst.
 * @dst     HDLC destination buffer pointer
 * @src     source data stream buffer pointer
 * @srclen      source data stream length
 * @return:Returns the HDLC data queue length of dst buffer.
 */
int hdlc_send_buffer(void *dst, const void *src, int srclen);

#endif/* __HDLC_H__ */

