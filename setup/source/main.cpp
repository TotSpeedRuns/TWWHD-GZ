#include "setup_revision.hpp"
#include "setup_progress.hpp"
#include "setup.hpp"
#include "install_policy.hpp"
#include "storage_install_check.hpp"
#include "setup_mode.hpp"
#include "fast_copy.hpp"
#include <sysapp/launch.h>
#include <coreinit/ios.h>
#include <mocha/mocha.h>
#include <whb/proc.h>
#include <whb/log_console.h>
#include <whb/log.h>
#include <whb/sdcard.h>
#include <vpad/input.h>
#include <padscore/kpad.h>
#include "setup_input.hpp"
#include <coreinit/mcp.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <coreinit/filesystem_fsa.h>
#include <coreinit/screen.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/cache.h>
#include <libs/tinyxml2.hpp>
#include <libs/hashing.hpp>
#include <nuspack/packer.hpp>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <cstring>
#include <cstdio>
#include <proc_ui/procui.h>
#include <coreinit/foreground.h>
namespace fs=std::filesystem;
std::string workspace;
static std::string status="Ready",failure;static std::mutex statusLock;static std::atomic<bool> busy{false},done{false};
static std::atomic<bool> cancelRequested{false},launchRequested{false};
static int selectedStorage=0;static bool fullReadback=false;
alignas(64) static OSThread thread;alignas(64) static char stack[512*1024];static bool threadCreated=false;
static bool threadJoined=false;
void report(const std::string& text){std::lock_guard<std::mutex> lock(statusLock);status=text;}
Bytes readFile(const std::string& path){std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Cannot read "+path);in.seekg(0,std::ios::end);auto n=in.tellg();if(n<0||n>128*1024*1024)throw std::runtime_error("File too large "+path);in.seekg(0);Bytes b(size_t(n),0);if(!in.read((char*)b.data(),b.size()))throw std::runtime_error("Short read "+path);return b;}
void writeFile(const std::string& path,const Bytes& b){const std::string allowed=std::string(WHBGetSdCardMountPath())+"/wwhdgz-practice-fast-v47/";if(!fs::path(path).lexically_normal().string().starts_with(allowed))throw std::runtime_error("Refusing write outside practice SD workspace");std::ofstream out(path,std::ios::binary|std::ios::trunc);if(!out||!out.write((const char*)b.data(),b.size()))throw std::runtime_error("Write failed "+path);out.close();if(!out)throw std::runtime_error("Close failed "+path);}
std::string fileHash(const std::string& path){std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Cannot verify "+path);std::vector<char> b(256*1024);SHA256 h;while(in){if(cancelRequested.load())throw std::runtime_error("Cancelled during identity verification");in.read(b.data(),b.size());h.add(b.data(),in.gcount());}if(!in.eof())throw std::runtime_error("Read error "+path);return hashHex(h.getHash());}
static void copyTree(const fs::path& from,const fs::path& to){
 fs::create_directories(to);
 for(const auto& e:fs::directory_iterator(from)){
  if(cancelRequested.load())throw std::runtime_error("Cancelled. Original game unchanged; partial SD folder retained.");
  const auto dest=to/e.path().filename();if(e.is_symlink())throw std::runtime_error("Unsupported source link");
  if(e.is_directory()){copyTree(e.path(),dest);continue;}if(!e.is_regular_file())throw std::runtime_error("Unsupported source entry");
  report("Copy: "+e.path().filename().string());std::ifstream in(e.path(),std::ios::binary);std::ofstream out(dest,std::ios::binary);if(!in||!out)throw std::runtime_error("Copy open failed");std::vector<char> b(256*1024);SHA256 h;
  while(in){if(cancelRequested.load())throw std::runtime_error("Cancelled during channel staging");in.read(b.data(),b.size());auto n=in.gcount();if(n){out.write(b.data(),n);h.add(b.data(),n);if(!out)throw std::runtime_error("SD full/write failed");}}
  if(!in.eof())throw std::runtime_error("Source read error");out.close();if(!out||fileHash(dest.string())!=hashHex(h.getHash()))throw std::runtime_error("SD copy verification failed");
 }
}
static void editXML(const std::string& path,bool meta){
 tinyxml2::XMLDocument doc;if(LoadXML(doc,path)!=tinyxml2::XML_SUCCESS)throw std::runtime_error("Invalid game XML");auto root=doc.RootElement();if(!root||!root->FirstChildElement("title_id"))throw std::runtime_error("Game XML title missing");
 root->FirstChildElement("title_id")->SetText("0005000010575a01");
 if(auto group=root->FirstChildElement("group_id"))group->SetText("0000575a");
 if(meta)for(auto e=root->FirstChildElement();e;e=e->NextSiblingElement()){std::string name=e->Name();if(name.starts_with("longname_")||name.starts_with("shortname_"))e->SetText("TWWHD-GZ");}
 tinyxml2::XMLPrinter printer;doc.Print(&printer);Bytes b(printer.CStr(),printer.CStr()+printer.CStrSize()-1);writeFile(path,b);
}
struct SecretWipe{void* p;size_t n;~SecretWipe(){volatile unsigned char* b=(volatile unsigned char*)p;while(n--)*b++=0;}};

static constexpr const char* previousPracticeHash="e9207a52684a1aaddf2a4a428028a204bf1b893334eb7266b977c11c3ec714d5";
static constexpr const char* metadataOnlyHash="93e33229aff0f7e187a6dd6351510e77d46e1c8bda7d5910516412d6cef351c3";
static constexpr const char* sectionOnlyHash="d9c3f06dec5d6fe92612340f1f4bd8037162b4db80d924d186da6bc74d19b84c";
static constexpr const char* importGroupHash="2fe659c38b0a67796ac5a21d50c21366105ed30ff223847f25e8dc308ad71aa1";
static constexpr const char* literalRepairHash="41636ea8ec5fa8db73051d5d1eeb66dc2c18d1d14bba8bab0935d7cee40a6447";
static constexpr const char* fontRepairHash="bfd4dc5da64547c46821f90eb9737395bd7e5831abec232f814f5abf929f02c6";
static constexpr const char* coachingRepairHash="6904a6b284a671c91f064ed1aae572f47e5b46e1e272c681870dd6c17896bf56";
static constexpr const char* viewerRepairHash="50d86b9f05a523c1a0dccdd6305692601af9ae014ee0dc976d9e6713156b5b10";
static constexpr const char* menuRepairHash="e5a63ed15ad8c947cce131cc81500ba11b9563a2f044f1e0822f9f459defefaa";
static constexpr const char* mssRepairHash="b0c9c2edc60d063fffc0b686d0d1c3537c4f08bdf56ab7e9c4a5a6251880c7a5";
static constexpr const char* v96RepairHash="68cfe726f1f6766340a4988f35cf44096f9bfa02aaccc5690d81cb0f77774f99";
static constexpr const char* v97RepairHash="5d0b8dbfe7eed4eaf6516238846eb7ab11acd8c62ec54271ba201afd28660bee";
static constexpr const char* v100RepairHash="1041237c922612b16f8c319482a4d1482ea11c99f18730bfd12c94d2d34fe7a6";
static constexpr const char* v102RepairHash="87a78b55b4b9a10d4ad0cdf9d6160f035c359c6ee7c9c3557c5f0079c2e4fdc2";
static constexpr const char* v106RepairHash="84b8f5a825ed2c52f163fe10bf182946367eb11a83516e1c85b20c0828d70e21";
static constexpr const char* v107RepairHash="3238f54daad89382a684ee591cca1d41aae882541ef48b20aea24cf57751c2fc";
static constexpr const char* v108RepairHash="b7495d2abe6cf635abfd0c78abf635e9bd5940c440632b47677888e715ff1548";
static constexpr const char* v108NativeRepairHash="6972fbcc82a629c076db2306788902889ae1fbf1b3e4716b0b71693a88230acd";
static constexpr const char* v109RepairHash="2ef33bd4bcba534151836369d31b3a709f55169bf99a56dbfd2561a1320a76b7";
static constexpr const char* v109NativeRepairHash="a67e0f43da012e79362712b2c892e351b82c167c865f18e6665af0fdfa7ec6a0";
static constexpr const char* v112RepairHash="aeb10cffc654d216ce7475b361e7e01e0b114191e8ea1ae535d7390683858269";
static constexpr const char* v112NativeRepairHash="6f01efbf9815200aaa1ad107b4f7ea4a1e0712bb7ccfae993f61444c33dcfbc0";
static constexpr const char* v113RepairHash="aaeca7c2f8394c90f4541fb798cfc6f6b639254aaaec8900674ff64711555e2b";
static constexpr const char* v113NativeRepairHash="76f05eed484d63743e9639206bd562d25cbfc58a6c8eaa5b850c697b3a1e0c13";
static constexpr const char* v114RepairHash="83a41ac171ccbf7f776c79135180dd883426318a355e2e181bf2b3de82386f30";
static constexpr const char* v114NativeRepairHash="22b9917cd35cc36f52ab39d15bc422ee9a20597236bf74b68ce9db81a398e724";
static constexpr const char* v115RepairHash="e10f4e549e5d95763aac726b0582bcc128875d7cb59cd81aedfe6d9a7f54e96d";
static constexpr const char* v115NativeRepairHash="2e46e16949ad6c0a7ec4fd86254a7342168e9ce3efe54612213aeb3a685f43eb";
static constexpr const char* v116RepairHash="da24f3dbcf75c6353cf73b49a3d506fc1e395b3d328150801d6ef88926398cc9";
static constexpr const char* v116NativeRepairHash="a8d96c6d1ffe3ffc404680873730a8038b1fc9c278f7d43369527f953cb66aa2";
static constexpr const char* v117RepairHash="9dcdb0d87f2e4f3c9296d63c35f2bb4ad72a0fecd0e004a46f63844d7af01dee";
static constexpr const char* v117NativeRepairHash="9fa9c31e6a12a29046697dd0cb6f6d6bbbcea01bb4dc48f6fe402f0abc2bb2f7";
static constexpr const char* v119RepairHash="c848552a27ff02cbd7d6bf8a9ace58308ad533053ae5403a85ce917f19840e5c";
static constexpr const char* v119NativeRepairHash="e0bfb2f4c3fe1a77b04db9e4bbc0277b6be08fd9e292912ee1c0bb5385818efa";
static constexpr const char* v120RepairHash="ecf8fee6f0975d17f0e0aaf64e7b9f821d93e595f19de99a15e27fe7b98e22aa";
static constexpr const char* v120NativeRepairHash="c62f3c086f54e6bca46c0b91a8eb6791829e39a5a281ca9ea4632ed221611773";
static constexpr const char* v121RepairHash="8c09c37c17ef0a8a77853427de048b40ec4cb7e70ce4464d31ed9227fa577037";
static constexpr const char* v121NativeRepairHash="24102885e9c50d6d9d45f678b5f8a59d6258a0ec59eab4a79b7cc8514b6fe0f4";
static constexpr const char* v122RepairHash="79a49d607066ef1b41c3edf4e608729a6b19a52b7cbf842b6e759ef2a1bc7af5";
static constexpr const char* v122NativeRepairHash="4b3d18625925a90b166c06096944989b052e1d7ac71ddca62b35c94c35f63a95";
static constexpr const char* v123RepairHash="becd2dc9af42400690ae3e2812d165e86bfbaa8caddd3dc78790a9e83b7259e9";
static constexpr const char* v123NativeRepairHash="8d1f1b448b4113539e78bd71249388873d5a246260c1827322cdaa3a4d960537";
static constexpr const char* v124RepairHash="2f2b0e674ade7191ca8d7ddecbd460b6d793b7b08448a58eb278419ca6babbfc";
static constexpr const char* v124NativeRepairHash="b0e1b9b4ba9c3049a2b6d0626864d7b3a1bc1ab0c98ceb0ae1245823de736aef";
static constexpr const char* v111RepairHash="e2e9192a3981312f2cfa7d2d63edc5df9c61e7292db8ec2d31df3518252c26f2";
static constexpr const char* v111NativeRepairHash="9014cf0e2a185f12a4e8e876b0aa9e329379f74daf9e0ec65df490de3db44678";
static constexpr const char* v110RepairHash="a3fd36817c21428f1b0ef407d436f20fdb956bf30fb95b939307b495c024a0d4";
static constexpr const char* v110NativeRepairHash="c5dae702bad31a0926ad34cba3a33dce2e9cb5ae5373b03e0f1cdf1bcc0d54ad";
static constexpr const char* v101RepairHash="0cb39144906d526cffd1faa2a44592d18af89a28673f460fd8a6c2fd337e9091";
static constexpr const char* v75RepairHash="ec48e5c44990b53e37d918050481bb834f4cbacdf1fa3f0e37f91c20adf6ab47";
static constexpr const char* v74bRepairHash="10302c7e6eb7f93a99b61fd7f1ca9b778111c533d62bccb9ca36dff23c9a4684";
static constexpr const char* v74RepairHash="f37eb240023ca7b95b01806a1e6dd19439d0586f5a60067d2d69709ad1488c4f";
static constexpr const char* v73RepairHash="72af4e9ff69ce2175d0911d1249088d70b155052af5d0f3466e3424f7fbd345b";
static constexpr const char* v72RepairHash="c889fedde585e7e0e6df42041e8f2036e6326dc6fe832a184589460de70f35fb";
static constexpr const char* v71RepairHash="24803bb52a97e7feed83bdbe9aa8dab57706b0aacf0a3fd6f63733760edaf445";
static constexpr const char* v70RepairHash="4429885d16ce1a399ffcfe3f4596b07b72622a3e754dd4d1fe79bcb45c771919";
static constexpr const char* v69RepairHash="6a8851468d9e3fee2e16b4855957762627231482e054c8a1358162214f465119";
static constexpr const char* v66RepairHash="d37a467ef462bd754dffb2dc412dc9146f9e23e91e7a298422e5b60a0c14ff7b";
static constexpr const char* practiceHash="24cf85b01722444e3303f14821b1856400120f4b5e6fef66eef3ee9bfe0cbc4b";
static constexpr const char* oldIconHash="898e6d480b7deae68cc9e329f2633a560423e22d4ed839ab2984abdd4eb43dd6";
static constexpr const char* iconHash="a7f68cdf3181b1d08866e966a9d9018c85a73e4aa8f7caa787f058ac92075b80";
static std::string receiptText(){return (std::string("TWWHD-GZ ")+setupRevision+"\n0005000010575a01\n")+practiceHash+"\n"+iconHash+"\n";}
static bool lookup(uint64_t title,MCPTitleListType& out){
 int h=MCP_Open();if(h<0)throw std::runtime_error("Cannot check installed titles");
 int count=MCP_TitleCount(h);if(count<0||count>4096){MCP_Close(h);throw std::runtime_error("Cannot enumerate installed titles");}
 size_t size=std::max(1,count)*sizeof(MCPTitleListType);auto list=(MCPTitleListType*)MEMAllocFromDefaultHeapEx(size,64);
 if(!list){MCP_Close(h);throw std::runtime_error("Title list allocation failed");}
 uint32_t n=0;auto err=MCP_TitleList(h,&n,list,size);bool found=false;
 if(err>=0&&n<=uint32_t(count)){for(uint32_t i=0;i<n;++i)if(list[i].titleId==title){out=list[i];found=true;break;}}
 MEMFreeToDefaultHeap(list);MCP_Close(h);if(err<0||n>uint32_t(count))throw std::runtime_error("Title enumeration failed");return found;
}
static std::string mountTitle(const MCPTitleListType& info,std::string& volume){
 std::string p=info.path;if(p.starts_with("/vol/storage_mlc01/"))volume="storage_mlc01";else if(p.starts_with("/vol/storage_usb01/"))volume="storage_usb01";else throw std::runtime_error("Unsupported title storage");
 if(Mocha_MountFS(volume.c_str(),nullptr,("/vol/"+volume).c_str())!=MOCHA_RESULT_SUCCESS)throw std::runtime_error("Cannot mount title storage");
 return volume+":"+p.substr(5+volume.size());
}
static std::atomic<bool> installFinished{false};static std::atomic<int32_t> installResult{0};
static void installCallback(IOSError result,void*){installResult.store(int32_t(result));installFinished.store(true);}
static void installPractice(const std::string& sdPath,MCPInstallTarget target){
 MCPTitleListType existing{};if(lookup(gz::practice,existing))throw std::runtime_error("Practice title already exists; refusing overwrite");
 int h=MCP_Open();if(h<0)throw std::runtime_error("Cannot open installer");
 try{
  alignas(64) char path[FS_MAX_PATH]{};if(sdPath.size()>=sizeof(path))throw std::runtime_error("Install path too long");std::memcpy(path,sdPath.c_str(),sdPath.size()+1);
  alignas(64) MCPInstallInfo info{};checkInstallResult("MCP_InstallGetInfo",int32_t(MCP_InstallGetInfo(h,path,&info)),target);
  if(*reinterpret_cast<uint64_t*>(&info)!=gz::practice)throw std::runtime_error("Install package title identity mismatch");
  checkInstallResult("MCP_InstallSetTargetDevice",int32_t(MCP_InstallSetTargetDevice(h,target)),target);
  checkInstallResult("MCP_InstallSetTargetUsb",int32_t(MCP_InstallSetTargetUsb(h,target)),target);
  MCPInstallTarget actualTarget{};checkInstallResult("MCP_InstallGetTargetDevice",int32_t(MCP_InstallGetTargetDevice(h,&actualTarget)),target);
  if(actualTarget!=target)throw std::runtime_error("Installer target differs from selected storage; stopped before install");
  alignas(64) IOSVec vec{};vec.vaddr=path;vec.len=sizeof(path);
  installFinished.store(false);installResult.store(0);
  checkInstallResult("IOS_IoctlvAsync install",int32_t(IOS_IoctlvAsync(h,0x81,1,0,&vec,installCallback,nullptr)),target);
  while(!installFinished.load()){
   alignas(64) MCPInstallProgress p{};auto progressResult=MCP_InstallGetProgress(h,&p);
   const auto suffix=cancelRequested.load()?"; cancel pending until native installer returns":"";
   if(progressResult<0)report(installErrorText("MCP_InstallGetProgress",int32_t(progressResult),int(target))+suffix);
   else if(p.sizeTotal)report("Installing practice copy: "+std::to_string(unsigned(std::min(100.0,100.0*p.sizeProgress/p.sizeTotal)))+"%"+suffix);
   else report(std::string("Native installer working; waiting for size/progress")+suffix);
   OSSleepTicks(OSMillisecondsToTicks(250));
  }
  checkInstallResult("Practice install callback",installResult.load(),target);
 }catch(...){MCP_Close(h);throw;}MCP_Close(h);
}


static constexpr const char* titleHash="718f8650532ff94d82db347bb9185858fd39b8492a9a91b6f3beb5108f1df819";
static std::string fastReceipt(){return receiptText()+titleHash+"\nFAST47\n";}
static void logPhase(const std::string& base,const std::string& line){report(line);if(fs::exists(base)){std::ofstream f(base+"/setup-log.txt",std::ios::app);f<<OSTicksToMilliseconds(OSGetTime())<<" ms "<<line<<"\n";}}
struct CopyTimingLog{
 std::string base;gzcopy::TransferStats stats;
 ~CopyTimingLog(){logPhase(base,"Copy timing: source read "+std::to_string(stats.readUs/1000)+" ms, destination write "+std::to_string(stats.writeUs/1000)+" ms, completed verification "+std::to_string(stats.verifyUs/1000)+" ms, newly written "+std::to_string(stats.newBytes)+" bytes, checked existing "+std::to_string(stats.existingBytes)+" bytes");}
};
static uint64_t freeSpace(const char* path){auto fsa=FSAAddClient(nullptr);if(fsa<0)throw std::runtime_error("Cannot check storage free space");uint64_t n=0;auto result=FSAGetFreeSpaceSize(fsa,path,&n);FSADelClient(fsa);if(result!=FS_ERROR_OK)throw std::runtime_error(std::string("Storage unavailable: ")+path);return n;}
static bool receiptMatches(const std::string& path,const std::string& expected){if(!fs::exists(path))return false;auto b=readFile(path);return std::string(b.begin(),b.end())==expected;}
static bool ownedIdentity(const std::string& path,bool allowKnownPrior=false){auto normal=fs::path(path).lexically_normal().string();if(!normal.ends_with("/usr/title/00050000/10575a01"))throw std::runtime_error("Practice storage path identity mismatch");auto icon=fileHash(path+"/meta/iconTex.tga"),rpx=fileHash(path+"/code/cking.rpx");return (rpx==practiceHash||(allowKnownPrior&&(rpx==v96RepairHash||rpx==v97RepairHash||rpx==v100RepairHash||rpx==v102RepairHash||rpx==v106RepairHash||rpx==v107RepairHash||rpx==v108RepairHash||rpx==v108NativeRepairHash||rpx==v109RepairHash||rpx==v109NativeRepairHash||rpx==v112RepairHash||rpx==v112NativeRepairHash||rpx==v113RepairHash||rpx==v113NativeRepairHash||rpx==v114RepairHash||rpx==v114NativeRepairHash||rpx==v115RepairHash||rpx==v115NativeRepairHash||rpx==v116RepairHash||rpx==v116NativeRepairHash||rpx==v117RepairHash||rpx==v117NativeRepairHash||rpx==v119RepairHash||rpx==v119NativeRepairHash||rpx==v120RepairHash||rpx==v120NativeRepairHash||rpx==v121RepairHash||rpx==v121NativeRepairHash||rpx==v122RepairHash||rpx==v122NativeRepairHash||rpx==v123RepairHash||rpx==v123NativeRepairHash||rpx==v124RepairHash||rpx==v124NativeRepairHash||rpx==v111RepairHash||rpx==v111NativeRepairHash||rpx==v110RepairHash||rpx==v110NativeRepairHash||rpx==v101RepairHash||rpx==v75RepairHash||rpx==v74bRepairHash||rpx==v74RepairHash||rpx==v73RepairHash||rpx==v72RepairHash||rpx==v71RepairHash||rpx==v70RepairHash||rpx==v69RepairHash||rpx==v66RepairHash||rpx==previousPracticeHash||rpx==metadataOnlyHash||rpx==sectionOnlyHash||rpx==importGroupHash||rpx==literalRepairHash||rpx==fontRepairHash||rpx==coachingRepairHash||rpx==menuRepairHash||rpx==mssRepairHash||rpx==viewerRepairHash)))&&(icon==iconHash||(allowKnownPrior&&icon==oldIconHash))&&fileHash(path+"/content/Common/Layout/Title_00.szs")==titleHash;}
static void flushPracticeVolume(const std::string& volume){
 auto client=FSAAddClient(nullptr);if(client<0)throw std::runtime_error("Cannot open durable storage check");
 if(Mocha_UnlockFSClientEx(client)!=MOCHA_RESULT_SUCCESS){FSADelClient(client);throw std::runtime_error("Cannot unlock durable storage check");}
 auto result=FSAFlushVolume(client,("/vol/"+volume).c_str());FSADelClient(client);
 if(result!=FS_ERROR_OK)throw std::runtime_error("Practice volume flush failed: "+std::to_string(int(result)));
}
static void verifyFreshContent(const std::string& original,const std::string& volume,const std::string& rawTitle){
 if(Mocha_MountFS("gzverify",nullptr,("/vol/"+volume).c_str())!=MOCHA_RESULT_SUCCESS)throw std::runtime_error("Cannot mount independent practice verification client");
 try{const auto root=fs::path("gzverify:"+rawTitle.substr(5+volume.size())+"/content");uint64_t files=0;
  for(const auto& e:fs::recursive_directory_iterator(original+"/content")){if(cancelRequested.load())throw std::runtime_error("Cancelled during persistent content check");if(!e.is_regular_file())continue;auto relative=gzcopy::entryRelative(e.path(),original+"/content");auto dest=root/relative;
   if(!fs::is_regular_file(dest)||fs::is_symlink(dest))throw std::runtime_error("Persisted practice file missing: "+relative.generic_string());
   if(relative.generic_string()!="Common/Layout/Title_00.szs"&&fs::file_size(dest)!=e.file_size())throw std::runtime_error("Persisted practice file size differs: "+relative.generic_string());++files;
  }report("Independent client checked "+std::to_string(files)+" persisted content files");
 }catch(...){Mocha_UnmountFS("gzverify");throw;}Mocha_UnmountFS("gzverify");
}

static int prepare(int choice,const char**){bool mocha=false;std::string sourceVolume,destVolume;bool sourceMounted=false,destMounted=false;
 const std::string sd=WHBGetSdCardMountPath();std::string base=sd+"/wwhdgz-practice-fast-v47";std::string output=sd+"/install/TWWHD-GZ-fast-v47";workspace=base+"/work";
 try{
  if(choice!=0&&choice!=1)throw std::runtime_error("Choose internal memory or USB");
  report("Initializing Aroma/Mocha access...");
  if(Mocha_InitLibrary()!=MOCHA_RESULT_SUCCESS)throw std::runtime_error("Aroma/Mocha unavailable");mocha=true;
  report("Checking installed practice title...");
  alignas(64) MCPTitleListType existing{};bool installed=lookup(gz::practice,existing);std::string destination;bool resume=false,recoverReceipt=false;
  if(installed){report("Mounting existing practice storage...");destination=mountTitle(existing,destVolume);destMounted=true;if(!ownedIdentity(destination,true))throw std::runtime_error("Existing practice identity differs; refusing overwrite");
   // Older completion receipts did not establish reboot persistence. Check
   // actual files again; never launch solely on an old receipt.
   // A lost SD receipt is not evidence of a different installed title. The
   // private title path and all three patched payload hashes passed above.
   // Recovery still verifies every existing bulk file and never overwrites it.
   recoverReceipt=!receiptMatches(base+"/PENDING.txt",fastReceipt());resume=true;
   if((choice==0)!=(destVolume=="storage_mlc01"))throw std::runtime_error("Resume must use the existing practice storage device");
  }else if(fs::exists(base)||fs::exists(output)){
   auto attempt=chooseFreshSetupAttempt(base,output,[](const std::string& p){return fs::exists(p);});
   base=attempt.first;output=attempt.second;workspace=base+"/work";
   report("Prior SD attempt retained; using fresh retry staging folder");
  }
  report("Checking installed EUR original game...");
  alignas(64) MCPTitleListType source{};if(!lookup(gz::source,source))throw std::runtime_error("Installed EUR base game not found");
  std::string original=source.path;if(original.starts_with("/vol/storage_mlc01/"))sourceVolume="storage_mlc01";else if(original.starts_with("/vol/storage_usb01/"))sourceVolume="storage_usb01";else throw std::runtime_error("Unsupported original-game storage");
  if(sourceVolume!=destVolume||!destMounted){if(Mocha_MountFS(sourceVolume.c_str(),nullptr,("/vol/"+sourceVolume).c_str())!=MOCHA_RESULT_SUCCESS)throw std::runtime_error("Cannot mount original-game storage");sourceMounted=true;}
  report("Checking original game version and executable...");
  original=sourceVolume+":"+original.substr(5+sourceVolume.size());
  auto xmlBytes=readFile(original+"/code/app.xml");tinyxml2::XMLDocument xml;if(xml.Parse((const char*)xmlBytes.data(),xmlBytes.size())!=tinyxml2::XML_SUCCESS)throw std::runtime_error("Source metadata invalid");auto node=xml.RootElement();auto v=node?node->FirstChildElement("title_version"):nullptr;if(!v||!v->GetText()||std::string(v->GetText())!="0000")throw std::runtime_error("EUR digital base v0 required");
  if(fileHash(original+"/code/cking.rpx")!="f9f461738949a09481dc1a31c01ad27db813c4c6058fdd7d015624a4146bbf0b")throw std::runtime_error("Original executable not supported");
  if(recoverReceipt){
   fs::create_directories(workspace);
   logPhase(base,"Recovering missing/mismatched SD receipt: private title path, practice RPX, icon, title archive and original RPX verified");
   logPhase(base,"Recovery checks existing content without overwriting; copies only missing files; no packaging or installation");
   // Preserve any previous receipt for diagnosis. Only the completion receipt
   // below is issued, after the entire checked content copy succeeds.
  }
  if(installed&&fileHash(destination+"/code/cking.rpx")!=practiceHash){
   const auto priorHash=fileHash(destination+"/code/cking.rpx");
   if(priorHash!=v96RepairHash&&priorHash!=v97RepairHash&&priorHash!=v100RepairHash&&priorHash!=v102RepairHash&&priorHash!=v106RepairHash&&priorHash!=v107RepairHash&&priorHash!=v108RepairHash&&priorHash!=v108NativeRepairHash&&priorHash!=v109RepairHash&&priorHash!=v109NativeRepairHash&&priorHash!=v112RepairHash&&priorHash!=v112NativeRepairHash&&priorHash!=v113RepairHash&&priorHash!=v113NativeRepairHash&&priorHash!=v114RepairHash&&priorHash!=v114NativeRepairHash&&priorHash!=v115RepairHash&&priorHash!=v115NativeRepairHash&&priorHash!=v116RepairHash&&priorHash!=v116NativeRepairHash&&priorHash!=v117RepairHash&&priorHash!=v117NativeRepairHash&&priorHash!=v119RepairHash&&priorHash!=v119NativeRepairHash&&priorHash!=v120RepairHash&&priorHash!=v120NativeRepairHash&&priorHash!=v121RepairHash&&priorHash!=v121NativeRepairHash&&priorHash!=v122RepairHash&&priorHash!=v122NativeRepairHash&&priorHash!=v123RepairHash&&priorHash!=v123NativeRepairHash&&priorHash!=v124RepairHash&&priorHash!=v124NativeRepairHash&&priorHash!=v111RepairHash&&priorHash!=v111NativeRepairHash&&priorHash!=v110RepairHash&&priorHash!=v110NativeRepairHash&&priorHash!=v101RepairHash&&priorHash!=v75RepairHash&&priorHash!=v74bRepairHash&&priorHash!=v74RepairHash&&priorHash!=v73RepairHash&&priorHash!=v72RepairHash&&priorHash!=v71RepairHash&&priorHash!=v70RepairHash&&priorHash!=v69RepairHash&&priorHash!=v66RepairHash&&priorHash!=previousPracticeHash&&priorHash!=metadataOnlyHash&&priorHash!=sectionOnlyHash&&priorHash!=importGroupHash&&priorHash!=literalRepairHash&&priorHash!=fontRepairHash&&priorHash!=coachingRepairHash&&priorHash!=menuRepairHash&&priorHash!=mssRepairHash&&priorHash!=viewerRepairHash)throw std::runtime_error("Unknown private RPX; refusing repair");
   fs::create_directories(workspace);logPhase(base,"Updating practice tools; original game and practice saves preserved");
   const auto backup=base+"/cking-before-loader-layout-"+priorHash.substr(0,12)+".rpx";
   if(!fs::exists(backup))writeFile(backup,readFile(destination+"/code/cking.rpx"));
   if(fileHash(backup)!=priorHash)throw std::runtime_error("RPX backup differs; refusing repair");
   const auto candidate=workspace+"/cking-front-metadata.rpx";
   writeFile(candidate,readFile(original+"/code/cking.rpx"));patchExecutable(candidate,sd+"/wiiu/apps/GZSetup/practice.gzd");
   auto fixed=readFile(candidate);const auto staged=destination+"/code/cking-front-metadata.upload";
   const auto retained=destination+"/code/cking-before-loader-layout-"+priorHash.substr(0,12)+".rpx.bak";
   if(fs::exists(staged)){if(fileHash(staged)!=practiceHash)throw std::runtime_error("Unknown staged executable; refusing overwrite");}
   else{gzcopy::DirectFile out(staged,true);out.write((const char*)fixed.data(),fixed.size());out.flush();if(!out.close())throw std::runtime_error("Staged RPX close failed");}
   if(fileHash(staged)!=practiceHash)throw std::runtime_error("Staged RPX identity differs");
   if(fs::exists(retained))throw std::runtime_error("Existing RPX repair backup retained; review before retry");
   fs::rename(destination+"/code/cking.rpx",retained);
   try{fs::rename(staged,destination+"/code/cking.rpx");}catch(...){fs::rename(retained,destination+"/code/cking.rpx");throw;}
   flushPracticeVolume(destVolume);if(fileHash(destination+"/code/cking.rpx")!=practiceHash)throw std::runtime_error("Installed RPX identity differs after repair");
   logPhase(base,"RPX metadata repair committed; original private executable retained in backups");
  }
  if(installed&&fileHash(destination+"/meta/iconTex.tga")==oldIconHash){
   fs::create_directories(workspace);logPhase(base,"Repairing known top-down icon to native bottom-up TGA encoding; preserving artwork");
   const auto backup=base+"/icon-before-native-encoding.tga";auto old=readFile(destination+"/meta/iconTex.tga");
   if(!fs::exists(backup))writeFile(backup,old);else if(fileHash(backup)!=oldIconHash)throw std::runtime_error("Icon backup differs; refusing repair");
   auto fixed=readFile(sd+"/wiiu/apps/GZSetup/selected-icon.tga");if(hashHex([&](){SHA256 h;h.add(fixed.data(),fixed.size());return h.getHash();}())!=iconHash)throw std::runtime_error("Native icon resource identity differs");
   gzcopy::DirectFile out(destination+"/meta/iconTex.tga",true);out.write((const char*)fixed.data(),fixed.size());out.flush();if(!out.close()||fileHash(destination+"/meta/iconTex.tga")!=iconHash)throw std::runtime_error("Native icon repair verification failed");
   flushPracticeVolume(destVolume);logPhase(base,"Native icon encoding repaired and volume flushed");
  }
  logPhase(base,"Detecting setup mode from installed practice identity and actual content");
  const bool complete=installed&&gzsetup::contentComplete(original+"/content",destination+"/content",
    [&](const fs::path& p){return gzcopy::entryRelative(p,original+"/content");},[]{return cancelRequested.load();});
  const auto mode=gzsetup::choose(installed,complete,fullReadback);
  logPhase(base,mode==gzsetup::Mode::Create?"First setup: create separate practice copy":mode==gzsetup::Mode::Repair?"Repair/update: existing content complete; no bulk copy or packaging":"Resume: verify existing content and copy missing files");
  if(mode==gzsetup::Mode::Repair){
   if(!ownedIdentity(destination))throw std::runtime_error("Repaired private identity check failed");
   flushPracticeVolume(destVolume);
   alignas(64) MCPTitleListType checkedTitle{};if(!lookup(gz::practice,checkedTitle))throw std::runtime_error("Private registration missing");
   verifyFreshContent(original,destVolume,std::string(checkedTitle.path));
   auto receipt=fastReceipt();writeFile(base+"/INSTALLED.txt",Bytes(receipt.begin(),receipt.end()));
   logPhase(base,"Repair/update complete: executable verified, content present, storage flushed. No bulk copy or packaging.");done.store(true);
  }else{
  auto scanBytes=[&](const std::string& path,const std::string& label){
   logPhase(base,"Scanning "+label+" files; B: cancel...");auto last=OSGetTime();uint64_t files=0;
   auto bytes=gzcopy::bytes(path,[]{return cancelRequested.load();},[&](const gzcopy::ScanProgress& p){
    files=p.files;auto now=OSGetTime();if(OSTicksToMilliseconds(now-last)>=250){
     last=now;report("Scanning "+label+": "+std::to_string(p.files)+" files, "+std::to_string(p.bytes/(1024*1024))+" MiB counted; B: cancel");
    }
   });
   logPhase(base,"Scan complete: "+label+", "+std::to_string(files)+" files");return bytes;
  };
  uint64_t contentBytes=scanBytes(original+"/content","game content");const char* target=choice==0?"/vol/storage_mlc01":"/vol/storage_usb01";
  uint64_t remaining=contentBytes;
  if(resume)for(const auto&e:fs::recursive_directory_iterator(original+"/content")){if(e.is_regular_file()){auto dest=fs::path(destination+"/content")/gzcopy::entryRelative(e.path(),original+"/content");if(fs::is_regular_file(dest)&&!fs::is_symlink(dest))remaining-=std::min<uint64_t>(remaining,std::min<uint64_t>(e.file_size(),fs::file_size(dest)));}}
  if(cancelRequested.load())throw std::runtime_error("Cancelled before storage check");
  logPhase(base,"Checking selected practice storage free space...");
  if(freeSpace(target)<remaining+128ULL*1024*1024)throw std::runtime_error("Not enough free space on selected practice storage");
  if(!resume){
   uint64_t stageBytes=scanBytes(original+"/code","code")+scanBytes(original+"/meta","metadata")+fs::file_size(original+"/content/Common/Layout/Title_00.szs");
   if(cancelRequested.load())throw std::runtime_error("Cancelled before SD space check");
   logPhase(base,"Checking SD staging free space...");
   if(freeSpace("/vol/external01")<stageBytes*2+128ULL*1024*1024)throw std::runtime_error("Not enough SD space for small channel package");
   fs::create_directories(workspace);logPhase(base,choice==0?"Install target: internal memory (NAND)":"Install target: USB");
   const std::string game=base+"/game";logPhase(base,"Copying small code/meta channel files to SD");copyTree(original+"/code",game+"/code");copyTree(original+"/meta",game+"/meta");
   fs::create_directories(game+"/content/Common/Layout");writeFile(game+"/content/Common/Layout/Title_00.szs",readFile(original+"/content/Common/Layout/Title_00.szs"));
   patchExecutable(game+"/code/cking.rpx","fs:/vol/external01/wiiu/apps/GZSetup/practice.gzd");patchTitleArchive(game+"/content/Common/Layout/Title_00.szs","fs:/vol/external01/wiiu/apps/GZSetup/title.gzt");
   fs::remove(game+"/code/title.fst");fs::remove(game+"/code/title.tmd");editXML(game+"/code/app.xml",false);editXML(game+"/meta/meta.xml",true);
   writeFile(game+"/meta/iconTex.tga",readFile("fs:/vol/external01/wiiu/apps/GZSetup/selected-icon.tga"));
   if(fileHash(game+"/code/cking.rpx")!=practiceHash||fileHash(game+"/meta/iconTex.tga")!=iconHash||fileHash(game+"/content/Common/Layout/Title_00.szs")!=titleHash)throw std::runtime_error("Small channel payload verification failed");
   if(cancelRequested.load())throw std::runtime_error("Cancelled before channel packaging");logPhase(base,"Packaging small channel only; bulk game content is skipped");
   alignas(64) WiiUConsoleOTP otp{};SecretWipe otpWipe{&otp,sizeof(otp)};if(Mocha_ReadOTP(&otp)!=MOCHA_RESULT_SUCCESS)throw std::runtime_error("Cannot obtain console packaging key");Key key{};SecretWipe keyWipe{key.data(),key.size()};std::copy_n(otp.wiiUBank.wiiUCommonKey,16,key.begin());
   if(createPackage(game,output,defaultEncryptionKey,key)!=PackError::NONE)throw std::runtime_error("Small package creation failed");
   if(!fs::exists(output+"/title.tmd")||!fs::exists(output+"/title.tik"))throw std::runtime_error("Small package incomplete");
   if(cancelRequested.load())throw std::runtime_error("Cancelled after packaging, before installation");
   auto pending=fastReceipt();writeFile(base+"/PENDING.txt",Bytes(pending.begin(),pending.end()));
   logPhase(base,"Installing small separate channel");installPractice("/vol/app_sd/install/"+fs::path(output).filename().string(),choice==0?MCP_INSTALL_TARGET_MLC:MCP_INSTALL_TARGET_USB);
   alignas(64) MCPTitleListType practice{};if(!lookup(gz::practice,practice))throw std::runtime_error("New practice channel not found");
   requireInstalledStorage(std::string(practice.path),choice);
   std::string expectedVolume=choice==0?"storage_mlc01":"storage_usb01";
   if(sourceMounted&&sourceVolume==expectedVolume){destination=expectedVolume+":"+std::string(practice.path).substr(5+expectedVolume.size());destVolume=expectedVolume;}
   else{destination=mountTitle(practice,destVolume);destMounted=true;}
   if(!ownedIdentity(destination))throw std::runtime_error("Installed small channel verification failed");
  }
  logPhase(base,fullReadback?"Full mode: CRC32 read-back of every new file":"Fast mode: checked writes, close and exact sizes; no bulk read-back");
  if(resume)logPhase(base,"Resuming: existing files still get source/destination CRC checks");
  CopyTimingLog timing{base,{}};const auto start=OSGetTime();auto last=start;
  gzcopy::copy(original+"/content",destination+"/content","Common/Layout/Title_00.szs",[](){return cancelRequested.load();},[&](uint64_t n,const std::string&){auto now=OSGetTime();if(OSTicksToMilliseconds(now-last)>=500){last=now;auto ms=std::max<uint64_t>(1,OSTicksToMilliseconds(now-start));char speed[32],io[32];std::snprintf(speed,sizeof(speed),"%.1f",double(n)*1000.0/(1024.0*1024.0*double(ms)));auto transferUs=timing.stats.readUs+timing.stats.writeUs;std::snprintf(io,sizeof(io),"%.1f",transferUs?double(timing.stats.newBytes)*1000000.0/(1024.0*1024.0*double(transferUs)):0.0);report(setupProgress(n,contentBytes)+" Content: "+std::to_string(n/(1024*1024))+" / "+std::to_string(contentBytes/(1024*1024))+" MiB; overall "+speed+" MiB/s; read+write I/O only "+io+" MiB/s");}},fullReadback,&timing.stats);
  if(!ownedIdentity(destination)||fileHash(original+"/code/cking.rpx")!="f9f461738949a09481dc1a31c01ad27db813c4c6058fdd7d015624a4146bbf0b")throw std::runtime_error("Final practice/original verification failed");
  logPhase(base,"Flushing practice volume before completion");flushPracticeVolume(destVolume);
  alignas(64) MCPTitleListType finalTitle{};if(!lookup(gz::practice,finalTitle))throw std::runtime_error("Practice registration missing at completion");
  logPhase(base,"Checking every content file through an independent filesystem client");verifyFreshContent(original,destVolume,std::string(finalTitle.path));
  auto receipt=fastReceipt();writeFile(base+"/INSTALLED.txt",Bytes(receipt.begin(),receipt.end()));logPhase(base,"Completed flushed content copy in "+std::to_string(OSTicksToMilliseconds(OSGetTime()-start)/1000)+" seconds; independent client size/path checks passed. Reboot persistence and launch require hardware confirmation.");done.store(true);
  }
 }catch(const std::exception&e){logPhase(base,std::string("Setup stopped: ")+e.what());std::lock_guard<std::mutex> lock(statusLock);failure=e.what();}
 if(destMounted){logPhase(base,"Cleanup: unmount practice storage begin");auto result=Mocha_UnmountFS(destVolume.c_str());logPhase(base,"Cleanup: unmount practice storage returned "+std::to_string(int(result)));}
 if(sourceMounted){logPhase(base,"Cleanup: unmount original read-only storage begin");auto result=Mocha_UnmountFS(sourceVolume.c_str());logPhase(base,"Cleanup: unmount original storage returned "+std::to_string(int(result)));}
 if(mocha){logPhase(base,"Cleanup: Mocha shutdown begin");auto result=Mocha_DeInitLibrary();logPhase(base,"Cleanup: Mocha shutdown returned "+std::to_string(int(result)));}
 logPhase(base,"Cleanup: worker finished");if(done.load())report("Copy and flush complete. B: return to Menu; launch TWWHD-GZ separately.");busy.store(false);return 0;
}

static void joinWorker(){if(threadCreated&&!threadJoined){cancelRequested.store(true);OSJoinThread(&thread,nullptr);threadJoined=true;}}
static bool sdMounted=false,hasForeground=true;
static std::string lifecyclePath;
static void cleanStorageBeforeExit(){
 cancelRequested.store(true);joinWorker();
 if(sdMounted){logPhase(lifecyclePath,"v58: worker joined; unmounting SD before foreground/ProcUI release");WHBUnmountSdCard();sdMounted=false;}
}
static uint32_t releasedCallback(void*){cleanStorageBeforeExit();hasForeground=false;return 0;}
static uint32_t exitingCallback(void*){cleanStorageBeforeExit();return 0;}
static uint32_t acquiredCallback(void*){hasForeground=true;if(!sdMounted)sdMounted=WHBMountSdCard();return 0;}
int main(int,char**){
 // Use the SDK lifecycle, including HBL/Mii Maker return handling. The SDK
 // console allocates screen buffers in foreground MEM1 and releases/reacquires
 // them through ProcUI callbacks; no MEM2 OSScreen buffers survive HOME.
 WHBProcInit();WHBLogConsoleInit();WHBLogConsoleSetColor(0x102030ff);
 // Startup recovery: avoid unverified padscore initialization; GamePad-only helper.
 ProcUIRegisterCallback(PROCUI_CALLBACK_RELEASE,releasedCallback,nullptr,50);
 ProcUIRegisterCallback(PROCUI_CALLBACK_EXIT,exitingCallback,nullptr,50);
 ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE,acquiredCallback,nullptr,50);
 sdMounted=WHBMountSdCard();
 if(sdMounted)lifecyclePath=std::string(WHBGetSdCardMountPath())+"/wwhdgz-practice-fast-v47";
 else report("SD card mount failed. Reinsert SD and relaunch; setup not started.");
 std::vector<std::string> lastLines;
 OSTime workerStarted=0;
 bool wasForeground=true,menuRequested=false,returnAfterCancel=false;
 while(WHBProcIsRunning()){
  if(!hasForeground){wasForeground=false;OSSleepTicks(OSMillisecondsToTicks(50));continue;}
  if(menuRequested){OSSleepTicks(OSMillisecondsToTicks(50));continue;}
  if(!wasForeground){lastLines.clear();wasForeground=true;}
  if(returnAfterCancel&&!busy.load()){cleanStorageBeforeExit();menuRequested=true;SYSLaunchMenu();continue;}
  const auto input=readSetupInput();
  const uint32_t trigger=input.trigger;
  if(trigger){
   if(!busy.load()&&!threadCreated&&(trigger&(VPAD_BUTTON_UP|VPAD_BUTTON_DOWN)))selectedStorage=1-selectedStorage;
   if(!busy.load()&&!threadCreated&&(trigger&VPAD_BUTTON_X))fullReadback=!fullReadback;
   if(trigger&VPAD_BUTTON_B){
    if(busy.load()){returnAfterCancel=true;cancelRequested.store(true);report("Cancelling; waiting for current I/O, then returning to Menu...");}
    else{cleanStorageBeforeExit();menuRequested=true;SYSLaunchMenu();continue;}
   }
   if(sdMounted&&!busy.load()&&!threadCreated&&!done.load()&&(trigger&VPAD_BUTTON_A)){
    report("Starting setup worker...");workerStarted=OSGetTime();busy.store(true);
    if(!OSCreateThread(&thread,prepare,selectedStorage,nullptr,stack+sizeof(stack),sizeof(stack),16,OS_THREAD_ATTRIB_AFFINITY_CPU2)){busy.store(false);report("Cannot start setup worker");}
    else{threadCreated=true;OSResumeThread(&thread);}
   }
  }
  std::vector<std::string> lines={std::string("GZ Setup ")+setupRevision+" - setup / repair",selectedStorage==0?"Practice storage: Internal memory (NAND)":"Practice storage: USB","Up/Down: storage. A: setup / repair.","Auto-detects new, existing or incomplete copy.",fullReadback?"X: Full CRC read-back verification":"X: Fast mode (checked writes/sizes)","Existing complete copy: quick update only.","Separate practice game and saves.","B: cancel; B after completion: return."};
  if(busy.load()){auto seconds=OSTicksToMilliseconds(OSGetTime()-workerStarted)/1000;lines.push_back(std::string(cancelRequested.load()?"Cancel pending: ":"Working: ")+std::to_string(seconds)+" seconds");}
  lines.push_back(std::string("SD: ")+(sdMounted?"mounted":"NOT mounted"));
  lines.push_back(input.gamepad?"Input: GamePad connected":input.pro?"Input: Pro Controller connected":"Input: connect GamePad (startup recovery)");
  {std::lock_guard<std::mutex> lock(statusLock);lines.push_back(status);if(!failure.empty())lines.push_back(failure);}
  if(lines!=lastLines){
   std::vector<std::string> consoleLines;
   for(const auto& line:lines)for(size_t pos=0;pos<line.size();pos+=64)consoleLines.push_back(line.substr(pos,64));
   // Replace the SDK's sixteen-line rolling log with one complete UI frame.
   if(consoleLines.size()>16)consoleLines.erase(consoleLines.begin()+1,consoleLines.begin()+1+(consoleLines.size()-16));
   consoleLines.resize(16);
   for(const auto& line:consoleLines)WHBLogPrintf("%s",line.c_str());
   WHBLogConsoleDraw();lastLines=lines;
  }
  OSSleepTicks(OSMillisecondsToTicks(50));
 }
 cleanStorageBeforeExit();WHBLogConsoleFree();if(!menuRequested)WHBProcShutdown();return 0;
}
