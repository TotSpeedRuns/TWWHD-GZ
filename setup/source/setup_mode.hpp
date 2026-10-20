#pragma once
#include <filesystem>
#include <functional>
#include <stdexcept>
namespace gzsetup {
enum class Mode { Create, Repair, Resume };
constexpr Mode choose(bool installed, bool complete, bool fullVerification=false) {
 return !installed ? Mode::Create : (complete&&!fullVerification ? Mode::Repair : Mode::Resume);
}
inline bool contentComplete(const std::filesystem::path& source,
 const std::filesystem::path& target,
 const std::function<std::filesystem::path(const std::filesystem::path&)>& relative,
 const std::function<bool()>& cancelled) {
 namespace fs=std::filesystem;
 bool complete=true;
 for(const auto& e:fs::recursive_directory_iterator(source)) {
  if(cancelled())throw std::runtime_error("Cancelled during setup mode detection");
  if(e.is_symlink())throw std::runtime_error("Source links unsupported");
  auto name=relative(e.path());auto dest=target/name;
  if(fs::is_symlink(dest))throw std::runtime_error("Practice content link refused");
  if(e.is_directory())continue;
  if(!e.is_regular_file())throw std::runtime_error("Unsupported source file");
  if(!fs::exists(dest)){complete=false;continue;}
  if(!fs::is_regular_file(dest))throw std::runtime_error("Practice content entry refused");
  if(name.generic_string()!="Common/Layout/Title_00.szs" && fs::file_size(dest)!=e.file_size())
   throw std::runtime_error("Existing practice file size differs; refusing overwrite: "+name.generic_string());
 }
 return complete;
}
}
