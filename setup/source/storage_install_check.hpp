#pragma once
#include <string>
#include <stdexcept>
#include <cstdio>
#include <cstdint>
inline std::string installErrorText(const char* phase,int32_t code,int target){
 char hex[16];std::snprintf(hex,sizeof(hex),"0x%08X",unsigned(uint32_t(code)));
 return std::string(phase)+" failed: "+hex+" ("+std::to_string(code)+")"+
 (target==1?"; USB: check Wii U-formatted storage connection/power":"; internal memory target");
}
inline void checkInstallResult(const char* phase,int32_t code,int target){
 if(code<0)throw std::runtime_error(installErrorText(phase,code,target));
}
inline void requireInstalledStorage(const std::string& path,int choice){
 const std::string expected=choice==0?"/vol/storage_mlc01/usr/title/00050000/10575a01":"/vol/storage_usb01/usr/title/00050000/10575a01";
 if((choice!=0&&choice!=1)||path!=expected)throw std::runtime_error("Installed practice title is on unexpected storage/path; stopped before content copy");
}

#include <utility>
template<class Exists> inline std::pair<std::string,std::string> chooseFreshSetupAttempt(
 const std::string& base,const std::string& output,Exists exists){
 for(unsigned i=1;i<=1000;++i){
  auto stage=base+"/retry-"+std::to_string(i),package=output+"-retry-"+std::to_string(i);
  if(!exists(stage)&&!exists(package))return {stage,package};
 }
 throw std::runtime_error("Too many retained setup attempts; no files removed");
}
