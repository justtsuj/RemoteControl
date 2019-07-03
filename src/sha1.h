#ifndef SHA1
#define SHA1

#include "basic.h"

struct sha1_context
{
    word total[2];
    word state[5];
    byte buffer[64];
};

void sha1_starts( struct sha1_context *ctx );
void sha1_update( struct sha1_context *ctx, byte *input, int length );
void sha1_finish( struct sha1_context *ctx, byte digest[20] );

#endif /* sha1.h */
