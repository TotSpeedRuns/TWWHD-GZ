#pragma once
#include <filesystem>
#include <fstream>
#include <functional>
#include <vector>
#include <string>
#include <stdexcept>
#include <zlib.h>
#include <fcntl.h>
#include <cstdint>
#include <chrono>
#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#endif
namespace gzcopy {
namespace fs=std::filesystem;
// Avoid libc/C++ stream buffering on the native devoptab. Its fast path
// requires 64-byte alignment; reads cap at 1 MiB and writes at 256 KiB.
struct TransferBuffer{
 static constexpr size_t size=1024*1024;
 std::vector<char> storage=std::vector<char>(size+63);
 char* data(){return reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(storage.data())+63)&~uintptr_t(63));}
};
struct DirectFile{
 int fd=-1;
 DirectFile(const fs::path& p,bool output){
#ifdef _WIN32
  fd=_wopen(p.c_str(),output?(_O_WRONLY|_O_CREAT|_O_TRUNC|_O_BINARY):(_O_RDONLY|_O_BINARY),_S_IREAD|_S_IWRITE);
#else
  fd=::open(p.c_str(),output?(O_WRONLY|O_CREAT|O_TRUNC):O_RDONLY,0666);
#endif
  if(fd<0)throw std::runtime_error(output?"Content write open failed":"Content read open failed");
 }
 DirectFile(const DirectFile&)=delete;
 DirectFile& operator=(const DirectFile&)=delete;
 ~DirectFile(){if(fd>=0)close();}
 bool close(){int old=fd;fd=-1;
#ifdef _WIN32
  return _close(old)==0;
#else
  return ::close(old)==0;
#endif
 }
 void flush(){
#ifdef _WIN32
  auto result=_commit(fd);
#else
  auto result=::fsync(fd);
#endif
  if(result!=0)throw std::runtime_error("Practice file flush failed");
 }
 size_t read(char* p,size_t n){
#ifdef _WIN32
  auto count=_read(fd,p,unsigned(n));
#else
  auto count=::read(fd,p,n);
#endif
  if(count<0)throw std::runtime_error("Content source read failed");return size_t(count);
 }
 void write(const char* p,size_t n){while(n){
#ifdef _WIN32
  auto count=_write(fd,p,unsigned(n));
#else
  auto count=::write(fd,p,n);
#endif
  if(count<=0)throw std::runtime_error("Practice storage write failed");p+=count;n-=size_t(count);
 }}
};
struct Digest{uint64_t size=0;uint32_t crc=0;};
struct TransferStats{uint64_t readUs=0,writeUs=0,verifyUs=0,newBytes=0,existingBytes=0;};
inline uint64_t ioTime(){return uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
inline Digest digest(const fs::path& p,const std::function<bool()>& cancel=[](){return false;}){DirectFile f(p,false);TransferBuffer b;Digest d;for(;;){if(cancel())throw std::runtime_error("Cancelled during verification; partial practice copy retained");auto n=f.read(b.data(),b.size);if(!n)break;d.crc=crc32(d.crc,(const Bytef*)b.data(),unsigned(n));d.size+=n;}if(!f.close())throw std::runtime_error("Copy-check close failed");return d;}
struct ScanProgress {uint64_t entries=0,files=0,bytes=0;std::string path;};
inline uint64_t bytes(const fs::path& root,
 const std::function<bool()>& cancel=[](){return false;},
 const std::function<void(const ScanProgress&)>& progress=[](const ScanProgress&){}){
 auto check=[&](){if(cancel())throw std::runtime_error("Cancelled during source scan; no copy started");};
 ScanProgress scan;scan.path=root.generic_string();check();progress(scan);check();
 fs::recursive_directory_iterator it(root),end;
 while(it!=end){
  check();const auto& e=*it;scan.path=e.path().generic_string();
  if(e.is_symlink())throw std::runtime_error("Source links unsupported");
  if(e.is_regular_file()){
   auto size=e.file_size();if(size>UINT64_MAX-scan.bytes)throw std::runtime_error("Source size overflow");
   scan.bytes+=size;++scan.files;
  }else if(!e.is_directory())throw std::runtime_error("Unsupported source file");
  ++scan.entries;progress(scan);check();++it;
 }
 check();return scan.bytes;
}
// Wii U devoptab mount paths do not support std::filesystem realpath/canonical.
// Inputs come from the checked MCP title path and directory enumeration.
inline fs::path entryRelative(const fs::path& entry,const fs::path& root){
 auto r=entry.lexically_normal().lexically_relative(root.lexically_normal());
 if(r.empty()||r.is_absolute())throw std::runtime_error("Entry is outside checked source root");
 for(const auto& part:r)if(part=="..")throw std::runtime_error("Source entry escapes checked root");
 return r;
}
inline uint64_t copy(const fs::path& source,const fs::path& target,const std::string& skip,
 const std::function<bool()>& cancel,const std::function<void(uint64_t,const std::string&)>& progress,bool fullReadback=true,TransferStats* stats=nullptr){
 auto a=source.lexically_normal(),b=target.lexically_normal();
 auto nested=[](const fs::path& x,const fs::path& y){auto r=x.lexically_relative(y);return !r.empty()&&!r.is_absolute()&&*r.begin()!="..";};
 if(a==b||nested(a,b)||nested(b,a)||fs::is_symlink(target))throw std::runtime_error("Source/destination overlap or link refused");
 fs::create_directories(target);uint64_t copied=0;TransferBuffer buffer;
 for(const auto&e:fs::recursive_directory_iterator(source)){
  if(cancel())throw std::runtime_error("Cancelled; verified partial practice copy retained");
  if(e.is_symlink())throw std::runtime_error("Source links unsupported");
  auto relative=entryRelative(e.path(),source);auto dest=target/relative;
  if(e.is_directory()){if(fs::is_symlink(dest))throw std::runtime_error("Destination link refused");fs::create_directories(dest);continue;}
  if(!e.is_regular_file())throw std::runtime_error("Unsupported source file");
  fs::create_directories(dest.parent_path());if(relative.generic_string()==skip){copied+=e.file_size();progress(copied,relative.generic_string());continue;}
  if(fs::exists(dest)){
   if(!fs::is_regular_file(dest)||fs::is_symlink(dest))throw std::runtime_error("Destination entry refused");
   auto before=ioTime();auto a=digest(e.path(),cancel),b=digest(dest,cancel);if(stats){stats->verifyUs+=ioTime()-before;stats->existingBytes+=a.size;}if(a.size!=b.size||a.crc!=b.crc)throw std::runtime_error("Existing practice file differs; refusing overwrite: "+relative.generic_string());
   copied+=a.size;progress(copied,relative.generic_string());continue;
  }
  auto temp=dest;temp+=".gzpart";if(fs::is_symlink(temp)||(fs::exists(temp)&&!fs::is_regular_file(temp)))throw std::runtime_error("Partial-file path refused");
  DirectFile in(e.path(),false),out(temp,true);Digest original;
  const uint64_t expectedSize=e.file_size();
  for(;;){if(cancel())throw std::runtime_error("Cancelled; partial practice file retained");auto before=ioTime();auto n=in.read(buffer.data(),buffer.size);if(stats)stats->readUs+=ioTime()-before;if(!n)break;before=ioTime();out.write(buffer.data(),n);if(stats){stats->writeUs+=ioTime()-before;stats->newBytes+=n;}if(fullReadback)original.crc=crc32(original.crc,(const Bytef*)buffer.data(),unsigned(n));original.size+=n;progress(copied+original.size,relative.generic_string());}
  out.flush();if(!out.close()||!in.close())throw std::runtime_error("Content copy close failed");
  if(original.size!=expectedSize||e.file_size()!=expectedSize||fs::file_size(temp)!=expectedSize)throw std::runtime_error("Content copy size verification failed");
  if(fullReadback){auto before=ioTime();auto checked=digest(temp,cancel);if(stats)stats->verifyUs+=ioTime()-before;if(original.size!=checked.size||original.crc!=checked.crc)throw std::runtime_error("Content read-back verification failed");}
  if(fs::exists(dest))throw std::runtime_error("Destination appeared during copy; refusing overwrite");
  fs::rename(temp,dest);copied+=original.size;progress(copied,relative.generic_string());
 }
 return copied;
}
}
