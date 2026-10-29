// Included after titleHash/logPhase and before ownedIdentity in main.cpp.
static constexpr const char* legacyTitleHash="718f8650532ff94d82db347bb9185858fd39b8492a9a91b6f3beb5108f1df819";
static constexpr const char* originalTitleHash="06aaa05d996605d379b04e7ab1dfb9592d8d739c7c13e97bcd23049d33f29c5a";
static constexpr const char* titleRecipeHash="d8bcb9e4987adaf1771a7bf7be3739be37bef14fbb94c5f00566c50ff9ba93e6";
static void flushPracticeVolume(const std::string& volume);

// Only small checked title/recipe files use this non-cancelling digest. It is
// required after a rename so a user cancellation cannot interrupt rollback.
static std::string checkedTitleHash(const fs::path& path){
 const auto state=fs::symlink_status(path);
 if(!fs::is_regular_file(state)||fs::is_symlink(state)||fs::file_size(path)>32U*1024U*1024U)throw std::runtime_error("Title transaction file refused");
 std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Cannot read title transaction file");
 std::vector<char> data(256*1024);SHA256 hash;
 while(in){in.read(data.data(),data.size());hash.add(data.data(),in.gcount());}
 if(!in.eof())throw std::runtime_error("Title transaction read failed");
 return hashHex(hash.getHash());
}

struct InstalledTitleUpgrade {
 std::string destination,original,recipe,volume,base;
 InstalledTitleUpgrade(const std::string& d,const std::string& o="",const std::string& r="",const std::string& v="",const std::string& b=""):destination(d),original(o),recipe(r),volume(v),base(b){
  if(!fs::path(destination).lexically_normal().string().ends_with("/usr/title/00050000/10575a01"))throw std::runtime_error("Title upgrade storage identity differs");
 }
 fs::path path(gztitle::Slot slot)const{
  const std::string layout=destination+"/content/Common/Layout/";
  if(slot==gztitle::Slot::Live)return layout+"Title_00.szs";
  if(slot==gztitle::Slot::Stage)return layout+"Title_00-v151-029987c7d8d9.upload";
  return layout+"Title_00-before-v151-718f8650532f.szs.bak";
 }
 gztitle::Kind probe(gztitle::Slot slot)const{
  const auto p=path(slot);std::error_code error;const auto state=fs::symlink_status(p,error);
  if(state.type()==fs::file_type::not_found)return gztitle::Kind::Missing;
  if(error||!fs::is_regular_file(state)||fs::is_symlink(state))throw std::runtime_error("Title transaction entry refused");
  const auto digest=checkedTitleHash(p);
  if(digest==titleHash)return gztitle::Kind::Current;
  if(digest==legacyTitleHash)return gztitle::Kind::Legacy;
  return gztitle::Kind::Unknown;
 }
 void checkpoint()const{if(cancelRequested.load())throw std::runtime_error("Cancelled before title replacement; files retained");}
 void flushVolume()const{if(volume.empty())throw std::runtime_error("Title transaction volume missing");flushPracticeVolume(volume);}
 void rename(gztitle::Slot from,gztitle::Slot to)const{
  // Never rely on platform rename-overwrite behaviour.
  const auto expected=from==gztitle::Slot::Stage?gztitle::Kind::Current:gztitle::Kind::Legacy;
  if(probe(from)!=expected)throw std::runtime_error("Title rename source differs; files retained");
  if(probe(to)!=gztitle::Kind::Missing)throw std::runtime_error("Title rename target already exists; refusing overwrite");
  fs::rename(path(from),path(to));
 }
 void buildStage()const{
  checkpoint();if(original.empty()||recipe.empty()||base.empty())throw std::runtime_error("Title upgrade source binding missing");
  if(probe(gztitle::Slot::Stage)!=gztitle::Kind::Missing)throw std::runtime_error("Title stage already exists; refusing overwrite");
  const auto source=original+"/content/Common/Layout/Title_00.szs";
  if(checkedTitleHash(source)!=originalTitleHash||checkedTitleHash(recipe)!=titleRecipeHash)throw std::runtime_error("Original title or selected title recipe differs");
  fs::create_directories(workspace);const auto candidate=workspace+"/Title_00-v151-original-only.szs";
  if(fs::is_symlink(candidate)||(fs::exists(candidate)&&!fs::is_regular_file(candidate)))throw std::runtime_error("SD title candidate entry refused");
  writeFile(candidate,readFile(source));patchTitleArchive(candidate,recipe);
  if(checkedTitleHash(candidate)!=titleHash)throw std::runtime_error("SD title candidate differs");
  checkpoint();const auto fixed=readFile(candidate);
  gzcopy::DirectFile out(path(gztitle::Slot::Stage),true);out.write((const char*)fixed.data(),fixed.size());out.flush();
  if(!out.close()||checkedTitleHash(path(gztitle::Slot::Stage))!=titleHash)throw std::runtime_error("Staged title close/readback failed; stage retained");
 }
};

static bool installedTitleIdentity(const std::string& destination,bool allowKnownPrior){
 InstalledTitleUpgrade ops(destination);return gztitle::owned(gztitle::inspect(ops),allowKnownPrior);
}
static void upgradeInstalledTitle(const std::string& destination,const std::string& original,const std::string& recipe,const std::string& volume,const std::string& base){
 InstalledTitleUpgrade ops(destination,original,recipe,volume,base);
 if(gztitle::upgrade(ops))logPhase(base,"Native title layout upgraded and verified; prior title retained in same-volume backup");
}
