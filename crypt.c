#include "crpty.h"

struct context send_ctx;
struct context recv_ctx;

void encrypt(byte *data, int len){
	int i, j;
	if(len & 0x0f)
        	len = (len & 0xfffffff0) + 0x10;
	/* encrypt the buffer with AES-CBC-128 */
    	for(i = 0; i < len; i += 0x10){
		for(j = 0; j < 0x10; j++)
            		buffer[i + j] ^= send_ctx.LCT[j];
        	aes_encrypt( &send_ctx.SK, &buffer[i] );
        	memcpy(send_ctx.LCT, &buffer[i], 16);
	}
	/* compute the HMAC-SHA1 of the ciphertext */
	buffer[len] = (send_ctx.p_cntr << 24) & 0xFF;
	buffer[len + 1] = (send_ctx.p_cntr << 16) & 0xFF;
	buffer[len + 2] = (send_ctx.p_cntr << 8) & 0xFF;
	buffer[len + 3] = (send_ctx.p_cntr) & 0xFF;

	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, send_ctx.k_ipad, 64);
	sha1_update(&sha1_ctx, buffer, len + 4);
	sha1_finish(&sha1_ctx, digest);
    	
	sha1_starts(&sha1_ctx );
	sha1_update(&sha1_ctx, send_ctx.k_opad, 64);
	sha1_update(&sha1_ctx, digest, 20);
	sha1_finish(&sha1_ctx, &buffer[len]);
	/* increment the packet counter */
	send_ctx.p_cntr++;
}

bool decrypt()
