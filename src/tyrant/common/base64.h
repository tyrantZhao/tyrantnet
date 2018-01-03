#ifndef _COMMON_BASE64_H_INCLUDED__
#define _COMMON_BASE64_H_INCLUDED__

//base64编码支持
#include <string>

//是否为base64编码
bool is_base64(unsigned char c);
//编码, 从二进制到字符串
::std::string base64_encode(unsigned char const* , unsigned int len);
//解码，从字符串到二进制
::std::string base64_decode(std::string const& s);

#endif //_COMMON_BASE64_H_INCLUDED__
