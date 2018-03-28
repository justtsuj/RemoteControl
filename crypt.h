#include "aes.h"
#typedef unsigned char byte
#typedef unsigned long int word

struct context{
	struct aes_context SK;
	byte LCT[16];
	byte k_ipad[64];
	byte k_opad[64];
	word p_cntr;
};
