#ifndef __TYPEDEFS_H
#define __TYPEDEFS_H

/*
#ifndef WIN32
	#ifndef PURE 
		#define PURE =0
	#endif //PURE
#endif
*/

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { delete p; p = 0; }
#endif //SAFE_DELETE

typedef int				int32;				
typedef unsigned int	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8;
typedef uint32			uint;




#endif//__TYPEDEFS_H

