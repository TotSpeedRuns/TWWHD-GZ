#pragma once
#include <cstdint>
namespace gz {
constexpr uint64_t source=0x0005000010143600ULL;
constexpr uint64_t practice=0x0005000010575a01ULL;
static_assert(source!=practice);
enum class Action {Create,Launch,Refuse};
constexpr Action decide(bool installed,bool receipt,bool identity,bool partial) {
 if(installed)return receipt&&identity?Action::Launch:Action::Refuse;
 return receipt||partial?Action::Refuse:Action::Create;
}
}
