#pragma once
#include <string>
#include <vector>
#include <cstdint>
using Bytes=std::vector<uint8_t>;
void report(const std::string&);
Bytes readFile(const std::string&);
void writeFile(const std::string&,const Bytes&);
std::string fileHash(const std::string&);
bool patchExecutable(const std::string&,const std::string&);
extern std::string workspace;

bool patchTitleArchive(const std::string&,const std::string&);

inline std::string hashHex(const std::string& bytes){static const char hex[]="0123456789abcdef";std::string out;out.reserve(bytes.size()*2);for(unsigned char b:bytes){out+=hex[b>>4];out+=hex[b&15];}return out;}
