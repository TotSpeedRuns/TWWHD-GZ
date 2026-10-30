#include "setup.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
std::string workspace;
static std::map<std::string,Bytes> files;
static unsigned writes=0,checks=0;
void ck(bool value){if(!value)throw std::runtime_error("title consumer check");++checks;}
Bytes disk(const std::string& path){std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("input missing");return Bytes(std::istreambuf_iterator<char>(f),{});}
Bytes readFile(const std::string& path){return files.at(path);}
void writeFile(const std::string& path,const Bytes& data){ck(path=="input");files[path]=data;++writes;}
void report(const std::string&){}
int main(int argc,char** argv){
 ck(argc==4);const auto original=disk(argv[1]),recipe=disk(argv[2]),target=disk(argv[3]);
 files["input"]=original;files["recipe"]=recipe;
 ck(patchTitleArchive("input","recipe"));ck(writes==1&&files["input"]==target);
 auto reject=[&](Bytes bad,Bytes source){files["input"]=source;files["bad"]=bad;auto count=writes;bool refused=false;
  try{patchTitleArchive("input","bad");}catch(const std::exception&){refused=true;}
  ck(refused&&writes==count&&files["input"]==source);
 };
 auto bad=recipe;bad.resize(20);reject(bad,original);
 bad=recipe;bad[8]^=1;reject(bad,original);
 bad=recipe;bad[40]^=1;reject(bad,original);
 bad=recipe;bad[80]=255;reject(bad,original);
 bad=recipe;bad[76]=bad[77]=bad[78]=bad[79]=0;reject(bad,original);
 reject(recipe,target);
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"actual_native_CXX_title_body\":true,\"all_output_bytes_match\":true,\"disk_writes\":false}\n";
}
