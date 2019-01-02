#ifndef __TYRANTNET_COMMON_MD5CALC_H__
#define __TYRANTNET_COMMON_MD5CALC_H__

void MD5_String(const char * string, char * output);
void MD5_Binary(const char * string, unsigned char * output);
void MD5_Salt(unsigned int len, char * output);

#endif //__TYRANTNET_COMMON_MD5CALC_H__
