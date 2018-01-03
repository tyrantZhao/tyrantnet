#ifndef _COMMON_MD5CALC_H_INCLUDED__
#define _COMMON_MD5CALC_H_INCLUDED__

void MD5_String(const char * string, char * output);
void MD5_Binary(const char * string, unsigned char * output);
void MD5_Salt(unsigned int len, char * output);

#endif // _COMMON_MD5CALC_H_INCLUDED__
