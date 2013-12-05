#include <sys/types.h>
#include <stdio.h>
#include "hdlc.h"

/****************************************************************
 * HDLC is atype of Data_Link_Level protocols.
 * It uses 0x7eand 0x7d as the keywords for data packet control.
 *
 * Every packetis defined by a pair of 0x7e
 *      0x7e ...data... 0x7e
 *
 * If 0x7e or0x7d has been in the data queue, uses 0x7d to indicate
 * it.
 * When the datais 0x7e, it will be indicated by these 2 bytes
 * 0x7e --> 0x7d, 0x5e
 * When the datais 0x7d, it will be indicated by these 2 bytes
 *      0x7d--> 0x7d, 0x5d
 *
 *****************************************************************/

/* define HDLC keywords */
#define HDLC_KEY_FLAG       0x7e
#define HDLC_KEY_ESC        0x7d

/* define   portreceive state */
#define HDLC_ST_NOSYNC         0
#define HDLC_ST_SYNC            1
#define HDLC_ST_INFO            2
#define HDLC_ST_ESC             3
#define HDLC_ST_INACTIVE        4

/**
 *hdlc_init_recv - Init receive buffer for the HDLC object. The object save
 *                  the buffer and the max lengthfor process control.
 * @hdlc        HDLC control object pointer
 * @buffer  The receive buffer pointer for this HDLCcontrol object
 *@buffer_length   The max receive bufferlength
 * @return: 0success, -1 failed
 */
int hdlc_init_recv(HDLC *hdlc, void *buffer, int buffer_length) {
	hdlc->state = HDLC_ST_NOSYNC;
	hdlc->recv_buffer = (__uint8_t *) buffer;
	hdlc->recv_buffer_length = buffer_length;
	hdlc->recvd_length = 0;

	return 0;
}

/**
 *hdlc_recv_char - Receiving one char and set it into data stream buffer.
 * @hdlc:   HDLC control object pointer
 * @ch:     A new byte of received data stream
 * @return:
 *      > 0 --> the data queue length ofreceived packet.
 *      0 --> the receiving is not complete.
 *      -1--> receive buffer overflow, failed.
 */
int hdlc_recv_char(HDLC *hdlc, unsigned char ch) {
	int ret;
	int i;

	switch (hdlc->state) {
	case HDLC_ST_NOSYNC:
		if (ch == HDLC_KEY_FLAG) {
			hdlc->recvd_length = 0;
			hdlc->state = HDLC_ST_SYNC;
		}
		break;

	case HDLC_ST_SYNC:
		if (ch != HDLC_KEY_FLAG) /* the next byte after 0x7e must not be 0x7e */
		{
			if (ch == HDLC_KEY_ESC) {
				hdlc->state = HDLC_ST_ESC;
			} else {
				if (hdlc->recvd_length == hdlc->recv_buffer_length) /* packet lengthoverflow */
				{
					printf(
							"packetlength overflow at HDLC_ST_SYNC, recvd_length is   \
                            %d,recv_buffer_length is %d\n",
							hdlc->recvd_length, hdlc->recv_buffer_length);
					for (i = 0; i < hdlc->recvd_length; i++)
						printf("%.2x", hdlc->recv_buffer[i]);
					printf("%.2x", ch);
					hdlc->state = HDLC_ST_INACTIVE;
					return 0;
					//return-1;
				}
				hdlc->recv_buffer[hdlc->recvd_length] = ch; /* save the first byte */

				hdlc->recvd_length++;
				hdlc->state = HDLC_ST_INFO;
			}
		}
		break;

	case HDLC_ST_INFO:
		if (ch == HDLC_KEY_FLAG) {
			//thisis not a user data -crazyleen
			//hdlc->recvd_length++;
			hdlc->state = HDLC_ST_NOSYNC;
			ret = hdlc->recvd_length;
			hdlc->recvd_length = 0;
			return ret; /* receive done */
		} else if (ch == HDLC_KEY_ESC) {
			hdlc->state = HDLC_ST_ESC;
		} else {
			if (hdlc->recvd_length == hdlc->recv_buffer_length) /* packet lengthoverflow */
			{
				printf(
						"packetlength overflow at HDLC_ST_INFO, recvd_length is   \
                        %d,recv_buffer_length is %d\n",
						hdlc->recvd_length, hdlc->recv_buffer_length);
				for (i = 0; i < hdlc->recvd_length; i++)
					printf("%.2x", hdlc->recv_buffer[i]);
				printf("%.2x", ch);
				hdlc->state = HDLC_ST_INACTIVE;
				return 0;
				//return-1;
			}
			hdlc->recv_buffer[hdlc->recvd_length] = ch;
			hdlc->recvd_length++;
		}
		break;

	case HDLC_ST_ESC:
		if (hdlc->recvd_length == hdlc->recv_buffer_length) /* packet lengthoverflow */
		{
			printf(
					"packetlength overflow at HDLC_ST_ESC, recvd_length is   \
                %d,recv_buffer_length is %d\n",
					hdlc->recvd_length, hdlc->recv_buffer_length);
			for (i = 0; i < hdlc->recvd_length; i++)
				printf("%.2x", hdlc->recv_buffer[i]);
			printf("%.2x", ch);
			hdlc->state = HDLC_ST_INACTIVE;
			return 0;
			//return-1;
		}
		hdlc->recv_buffer[hdlc->recvd_length] = (ch ^ 0x20); /* get 0x7d or 0x7e */
		hdlc->recvd_length++;
		hdlc->state = HDLC_ST_INFO;
		break;

	case HDLC_ST_INACTIVE:
		if (ch == HDLC_KEY_FLAG) {
			printf("\n");
			hdlc->recvd_length = 0;
			hdlc->state = HDLC_ST_NOSYNC;
			return 0;
		}
		printf("%.2x", ch);
		break;

	default:
		hdlc->state = HDLC_ST_NOSYNC;
		break;
	}/* endswtich */

	return 0;
}

/**
 *hdlc_send_buffer - Make data queue with HDLC, from src to dst.
 * @dst     HDLC destination buffer pointer
 * @src     source data stream buffer pointer
 * @srclen      source data stream length
 * @return:Returns the HDLC data queue length of dst buffer.
 */
int hdlc_send_buffer(void *dst, const void *src, int srclen) {
	int i, len = 0;
	__uint8_t *pdst = (__uint8_t *) dst;
	const __uint8_t *psrc = (const __uint8_t *) src;

	/* start flag*/
	*pdst++ = HDLC_KEY_FLAG;
	len++;

	for (i = 0; i < srclen; i++, psrc++) {
		if (*psrc == HDLC_KEY_FLAG) {
			*pdst++ = HDLC_KEY_ESC;
			*pdst++ = 0x5e;
			len += 2;
		} else if (*psrc == HDLC_KEY_ESC) {
			*pdst++ = HDLC_KEY_ESC;
			*pdst++ = 0x5d;
			len += 2;
		} else {
			*pdst++ = *psrc;
			len++;
		}
	}/* end for*/

	/* end flag;*/
	*pdst = HDLC_KEY_FLAG;
	len++;

	return len;
}

