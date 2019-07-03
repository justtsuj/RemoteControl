#ifndef CRYPT
#define CRYPT

#include "basic.h"
#include "aes.h"

struct context{
	struct aes_context SK;
	byte LCT[16];
	byte k_ipad[64];
	byte k_opad[64];
	word p_cntr;
};

#endif /* crypt.h */
