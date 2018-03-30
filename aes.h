#ifndef AES
#define AES

#include "basic.h"

struct aes_context
{
    int nr;             /* number of rounds */
    word erk[64];     /* encryption round keys */
    word drk[64];     /* decryption round keys */
};

int  aes_set_key( struct aes_context *ctx, byte *key, int nbits );
void aes_encrypt( struct aes_context *ctx, byte data[16] );
void aes_decrypt( struct aes_context *ctx, byte data[16] );

#endif /* aes.h */
