#ifndef __COMMON_FUNCTION_H__
#define __COMMON_FUNCTION_H__

#include <string>
#include <vector>


int getFileDirnameAndBasenameAndExtname(const char *fileName, std::string &dirName, std::string &baseName, std::string &extensionName);
int createNestedDir(const char *dir); //´´½¨Ç¶Ì×Ä¿Â¼

#endif //__COMMON_FUNCTION_H__
