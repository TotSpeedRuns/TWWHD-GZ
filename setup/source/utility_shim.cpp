#include "setup.hpp"
#include <utility/file.hpp>
#include <utility/path.hpp>
#include <fstream>
#include <iterator>
namespace Utility {
 bool isRoot(const fspath& p){auto s=p.string();return s.ends_with(":")||s.ends_with(":/");}
 int getFileContents(const fspath& p,std::string& out,bool){std::ifstream f(p,std::ios::binary);if(!f)return 1;out.assign(std::istreambuf_iterator<char>(f),{});return f.bad()?1:0;}
 int getFileContents(const fspath& p,std::stringstream& out){std::ifstream f(p,std::ios::binary);if(!f)return 1;out<<f.rdbuf();return f.bad()?1:0;}
 fspath get_temp_dir(){return workspace;}
}
