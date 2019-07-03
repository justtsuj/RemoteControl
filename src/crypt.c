#include "crypt.h"
#include "sha1.h"

struct context send_ctx;
struct context recv_ctx;

void setup_context(struct context *ctx, char *key, byte *IV){
	int i;	
	struct sha1_context sha1_ctx;
	byte digest[20];
	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, (byte*)key, strlen(key));
	sha1_update(&sha1_ctx, IV, 20);
	sha1_finish(&sha1_ctx, digest);
	aes_set_key(&ctx->SK, digest, 128);
	memcpy(ctx->LCT, IV, 0x10);
	memset(ctx->k_ipad, 0x36, 64);
	memset(ctx->k_opad, 0x5c, 64);
	for(i = 0; i < 20; ++i){
		ctx->k_ipad[i] ^= digest[i];
		ctx->k_opad[i] ^= digest[i];
	}
	ctx->p_cntr = 0;
}

void encrypt(byte *data, int len){
	int i, j;
	struct sha1_context sha1_ctx;
	byte digest[20];
	/* encrypt the buffer with AES-CBC-128 */
    	for(i = 0; i < len; i += 0x10){
		for(j = 0; j < 0x10; j++)
            		data[i + j] ^= send_ctx.LCT[j];
        	aes_encrypt(&send_ctx.SK, &data[i]);
        	memcpy(send_ctx.LCT, &data[i], 16);
	}
	/* compute the HMAC-SHA1 of the ciphertext */
	data[len] = (send_ctx.p_cntr << 24) & 0xFF;
	data[len + 1] = (send_ctx.p_cntr << 16) & 0xFF;
	data[len + 2] = (send_ctx.p_cntr << 8) & 0xFF;
	data[len + 3] = (send_ctx.p_cntr) & 0xFF;

	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, send_ctx.k_ipad, 64);
	sha1_update(&sha1_ctx, data, len + 4);
	sha1_finish(&sha1_ctx, digest);
    	
	sha1_starts(&sha1_ctx );
	sha1_update(&sha1_ctx, send_ctx.k_opad, 64);
	sha1_update(&sha1_ctx, digest, 20);
	sha1_finish(&sha1_ctx, &data[len]);
	/* increment the packet counter */
	send_ctx.p_cntr++;
}

bool decrypt(byte *data, int len){
	byte hmac[0x14], digest[0x14], temp[0x10];
	int blk_len = len - 0x14;
	int i, j;
	struct sha1_context sha1_ctx;
	memcpy(hmac, data + blk_len, 0x14);
	data[blk_len] = (recv_ctx.p_cntr << 24) & 0xFF;
	data[blk_len + 1] = (recv_ctx.p_cntr << 16) & 0xFF;
	data[blk_len + 2] = (recv_ctx.p_cntr <<  8) & 0xFF;
	data[blk_len + 3] = recv_ctx.p_cntr & 0xFF;

	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, recv_ctx.k_ipad, 64);
	sha1_update(&sha1_ctx, data, blk_len + 4);
	sha1_finish(&sha1_ctx, digest);

	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, recv_ctx.k_opad, 64);
	sha1_update(&sha1_ctx, digest, 20);
	sha1_finish(&sha1_ctx, digest);
	if(memcmp(hmac, digest, 20)) return false;
	recv_ctx.p_cntr++;
	for(i = 0; i < blk_len; i += 0x10){
		memcpy(temp, &data[i], 16);
		aes_decrypt(&recv_ctx.SK, &data[i]);
		for(j = 0; j < 16; j++)
			data[i + j] ^= recv_ctx.LCT[j];
		memcpy(recv_ctx.LCT, temp, 16);
	}
	return true;
}
