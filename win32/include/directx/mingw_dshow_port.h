#include <wtypes.h>
#include <unknwn.h>
#include <ole2.h>
#include <limits.h>

#define _WINGDI_ 1
#define AM_NOVTABLE
#define _OBJBASE_H_
#undef _X86_
#define _I64_MAX LONG_LONG_MAX
#define EXTERN_GUID(itf,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8)  \
	EXTERN_C static const IID itf = {l1,s1,s2,{c1,c2,c3,c4,c5,c6,c7,c8} }
