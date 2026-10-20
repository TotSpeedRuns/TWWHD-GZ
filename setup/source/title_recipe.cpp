#include "setup.hpp"
#include <libs/hashing.hpp>
#include <cstring>
#include <stdexcept>
#include <algorithm>
static uint32_t u32(const Bytes& b,size_t p){if(p>b.size()||b.size()-p<4)throw std::runtime_error("Truncated title patch");return uint32_t(b[p])<<24|uint32_t(b[p+1])<<16|uint32_t(b[p+2])<<8|b[p+3];}
static bool matches(const Bytes& b,const uint8_t* expected){SHA256 h;h.add(b.data(),b.size());uint8_t digest[32];h.getHash(digest);return !std::memcmp(digest,expected,32);}
static Bytes expandTitle(const Bytes& b){
 if(b.size()<16||std::memcmp(b.data(),"Yaz0",4))throw std::runtime_error("Unsupported title compression");
 size_t size=u32(b,4);if(!size||size>32*1024*1024)throw std::runtime_error("Title exceeds bounds");Bytes out;out.reserve(size);size_t p=16;unsigned bits=0,code=0;
 auto take=[&](){if(p>=b.size())throw std::runtime_error("Truncated title data");return b[p++];};
 while(out.size()<size){if(!bits){code=take();bits=8;}if(code&128)out.push_back(take());else{unsigned a=take(),c=take(),distance=((a&15)<<8)|c,count=a>>4;if(!count)count=take()+18;else count+=2;if(distance>=out.size()||count>size-out.size())throw std::runtime_error("Invalid title backreference");for(unsigned i=0;i<count;++i)out.push_back(out[out.size()-distance-1]);}code=(code<<1)&255;--bits;}
 return out;
}
bool patchTitleArchive(const std::string& input,const std::string& recipePath){
 Bytes original=readFile(input),recipe=readFile(recipePath);if(recipe.size()<80||std::memcmp(recipe.data(),"GZTITLE1",8))throw std::runtime_error("Invalid HD-GZ title patch");
 if(!matches(original,recipe.data()+8))throw std::runtime_error("Unsupported title archive: original EUR v0 required");
 Bytes raw=expandTitle(original);if(raw.size()<16||std::memcmp(raw.data(),"SARC",4))throw std::runtime_error("Invalid title archive");
 size_t size=u32(recipe,72),count=u32(recipe,76),p=80;if(!size||size>32*1024*1024||!count||count>10000)throw std::runtime_error("Invalid title patch bounds");Bytes out;out.reserve(size);
 for(size_t i=0;i<count;++i){if(p>=recipe.size())throw std::runtime_error("Truncated title operation");auto kind=recipe[p++];size_t off=u32(recipe,p),n=u32(recipe,p+4);p+=8;if(n>size-out.size())throw std::runtime_error("Title patch overflow");
  if(kind==0){if(off>raw.size()||n>raw.size()-off)throw std::runtime_error("Title source overflow");out.insert(out.end(),raw.begin()+off,raw.begin()+off+n);}
  else if(kind==1){if(p>recipe.size()||n>recipe.size()-p)throw std::runtime_error("Title literal overflow");out.insert(out.end(),recipe.begin()+p,recipe.begin()+p+n);p+=n;}
  else if(kind==2)out.insert(out.end(),n,0);else throw std::runtime_error("Unknown title operation");
 }
 if(p!=recipe.size()||out.size()!=size)throw std::runtime_error("Title patch size mismatch");
 Bytes encoded={'Y','a','z','0',uint8_t(size>>24),uint8_t(size>>16),uint8_t(size>>8),uint8_t(size),0,0,0,0,0,0,0,0};encoded.reserve(16+size+(size+7)/8);
 for(size_t i=0;i<size;i+=8){encoded.push_back(255);encoded.insert(encoded.end(),out.begin()+i,out.begin()+std::min(i+8,size));}
 if(!matches(encoded,recipe.data()+40))throw std::runtime_error("HD-GZ title output verification failed");writeFile(input,encoded);if(!matches(readFile(input),recipe.data()+40))throw std::runtime_error("HD-GZ title readback failed");return true;
}
