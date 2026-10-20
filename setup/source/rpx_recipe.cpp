#include "setup.hpp"
#include <libs/hashing.hpp>
#include <zlib.h>
#include <cstring>
#include <stdexcept>
#include <array>
#include <vector>
#include <algorithm>
static uint32_t word(const Bytes& b,size_t p){if(p>b.size()||b.size()-p<4)throw std::runtime_error("Truncated patch/RPX");return uint32_t(b[p])<<24|uint32_t(b[p+1])<<16|uint32_t(b[p+2])<<8|b[p+3];}
static void put(Bytes& b,size_t p,uint32_t n){if(p+4>b.size())throw std::runtime_error("Patch offset invalid");for(int i=3;i>=0;--i){b[p+i]=n;n>>=8;}}
static size_t aligned(size_t n){return(n+63)&~size_t(63);}
bool patchExecutable(const std::string& input,const std::string& recipePath){
 Bytes raw=readFile(input),recipe=readFile(recipePath);
 if(recipe.size()<96||std::memcmp(recipe.data(),"GZRPX001",8))throw std::runtime_error("Invalid patch recipe");
 SHA256 hash;hash.add(raw.data(),raw.size());unsigned char digest[32];hash.getHash(digest);
 if(std::memcmp(digest,recipe.data()+8,32))throw std::runtime_error("Unsupported game: requires original EUR v0");
 if(raw.size()<52||std::memcmp(raw.data(),"\x7f" "ELF\x01\x02\x01",7))throw std::runtime_error("Invalid RPX");
 size_t sourceCount=(raw[48]<<8)|raw[49],table=word(raw,32),count=word(recipe,40);
 if(!count||count>256||sourceCount>256||table+sourceCount*40>raw.size())throw std::runtime_error("Invalid RPX table");
 Bytes out(aligned(64+count*40),0);std::memcpy(out.data(),recipe.data()+44,52);put(out,32,64);
 std::vector<std::array<uint32_t,10>> headers;std::vector<Bytes> sections;
 size_t cursor=96;
 for(size_t i=0;i<count;++i){
  auto take=[&](){uint32_t n=word(recipe,cursor);cursor+=4;return n;};
  uint32_t src=take(),size=take();std::array<uint32_t,10> h{};for(auto& n:h)n=take();uint32_t edits=take();
  if(size>64*1024*1024||edits>2000000)throw std::runtime_error("Patch exceeds bounds");
  Bytes data;
  if(src!=0xffffffff){
   if(src>=sourceCount)throw std::runtime_error("Patch source section invalid");
   size_t p=table+src*40;auto kind=word(raw,p+4),flags=word(raw,p+8),offset=word(raw,p+16),len=word(raw,p+20);
   if(kind!=0&&kind!=8){
    if(offset>raw.size()||len>raw.size()-offset)throw std::runtime_error("Source section invalid");
    if(flags&0x08000000){
     if(len<4)throw std::runtime_error("Compressed section truncated");
     uint32_t expanded=word(raw,offset);if(expanded>64*1024*1024)throw std::runtime_error("Section too large");
     data.resize(expanded);uLongf length=expanded;
     if(uncompress(data.data(),&length,raw.data()+offset+4,len-4)!=Z_OK||length!=expanded)throw std::runtime_error("RPX decompression failed");
    }else data.assign(raw.begin()+offset,raw.begin()+offset+len);
   }
  }
  data.resize(size,0);size_t previousEnd=0;
  for(uint32_t n=0;n<edits;++n){size_t pos=take(),len=take();if(pos<previousEnd||pos>data.size()||len>data.size()-pos||cursor>recipe.size()||len>recipe.size()-cursor)throw std::runtime_error("Patch edit out of bounds");std::memcpy(data.data()+pos,recipe.data()+cursor,len);cursor+=len;previousEnd=pos+len;}
  headers.push_back(h);sections.push_back(std::move(data));
 }
 if(cursor!=recipe.size())throw std::runtime_error("Unexpected patch trailing data");
 // Logical section headers remain unchanged; CRC/FILEINFO bytes must be
 // physically within the loader's initial buffer (LiLoadRPLBasics -470078).
 std::vector<size_t> order;for(size_t i=0;i<count;++i)order.push_back(i);
 // LiCheckFileBounds also requires non-overlapping physical groups:
 // writable data, read-only allocated sections, text, then temporary tables.
 auto rank=[&](size_t i){auto kind=headers[i][1],flags=headers[i][2];if(kind==0x80000003)return 0;if(kind==0x80000004)return 1;if(kind==0x80000001||kind==0x80000002)return 3;if(flags&4)return 4;if(flags&2)return (flags&1)?2:3;return 5;};
 std::stable_sort(order.begin(),order.end(),[&](size_t a,size_t b){return rank(a)<rank(b);});
 for(auto i:order){auto& h=headers[i];auto& data=sections[i];if(h[1]==0||h[1]==8)h[4]=0;else{out.resize(aligned(out.size()),0);h[4]=out.size();h[5]=data.size();out.insert(out.end(),data.begin(),data.end());}}
 for(size_t i=0;i<count;++i)for(size_t n=0;n<10;++n)put(out,64+i*40+4*n,headers[i][n]);
 out.resize(aligned(out.size()),0);
 // Independent expanded-section CRC validation before replacing the SD copy.
 size_t crcIndex=count-2,cp=64+40*crcIndex,ct=word(out,cp+16);
 if(word(out,cp+4)!=0x80000003||word(out,cp+20)!=count*4)throw std::runtime_error("Invalid output CRC table");
 for(size_t i=count-2;i<count;++i){auto p=64+i*40;if(word(out,p+16)+word(out,p+20)>=4096)throw std::runtime_error("Native loader metadata is outside initial read buffer");}
 for(size_t i=0;i<count;++i){size_t p=64+i*40;auto kind=word(out,p+4),off=word(out,p+16),len=word(out,p+20);uint32_t value=0;if(i!=crcIndex&&kind!=0&&kind!=8){if(off>out.size()||len>out.size()-off)throw std::runtime_error("Output section invalid");value=crc32(0,out.data()+off,len);}if(value!=word(out,ct+4*i))throw std::runtime_error("Patched section CRC mismatch");}
 SHA256 outputHash;outputHash.add(out.data(),out.size());
 if(hashHex(outputHash.getHash())!="5029e0daf695bf34baa49df072469a0d1889c0391f64c9578a7f9d3192c840da")throw std::runtime_error("Output RPX hash mismatch: "+hashHex(outputHash.getHash()));
 writeFile(input,out);if(fileHash(input)!=hashHex(outputHash.getHash()))throw std::runtime_error("RPX read-back failed");return true;
}
