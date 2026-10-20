#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
static inline std::string setupProgress(uint64_t current,uint64_t total){
 if(!total)return "[--------------------] --%";
 auto percent=static_cast<unsigned>(100.0L*std::min(current,total)/total);
 auto filled=percent/5;
 return "["+std::string(filled,'#')+std::string(20-filled,'-')+"] "+std::to_string(percent)+"%";
}
