#include <stdio.h>

int main(int argc, char** argv)
{
 printf("#define RS_BYTES_PER_LONG %d\n",sizeof(long));
 printf("#define RS_BYTES_PER_INT %d\n",sizeof(int));
#if __INT_MAX__ == __LONG_MAX__
 printf("/*__LONG_MAX__ = __INT_MAX__ %d */\n",__LONG_MAX__);
#else
 printf("/*__LONG_MAX__ = %d */\n",__LONG_MAX__);
 printf("/*__INT_MAX__ = %d */\n",__INT_MAX__);
#endif
#ifdef _INTEGRAL_MAX_BITS
 printf("/*_INTEGRAL_MAX_BITS = %d */\n",_INTEGRAL_MAX_BITS);
#endif
}
