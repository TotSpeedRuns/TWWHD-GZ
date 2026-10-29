#include "source/title_upgrade.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <string>

using gztitle::Kind;
using gztitle::Slot;
struct Ops {
    std::array<Kind,3> files;
    std::string failBefore, failAfter;
    unsigned builds=0, renames=0, flushes=0, checkpoints=0;
    static unsigned index(Slot s){return static_cast<unsigned>(s);}
    void event(const std::string& name, bool after=false) {
        auto& failure=after?failAfter:failBefore;
        if(failure==name){failure.clear();throw std::runtime_error("injected "+name);}
    }
    Kind probe(Slot s)const{return files[index(s)];}
    void checkpoint(){++checkpoints;event("checkpoint"+std::to_string(checkpoints));}
    void flushVolume(){++flushes;const auto key="flush"+std::to_string(flushes);event(key);event(key,true);}
    void buildStage(){++builds;event("build");assert(files[1]==Kind::Missing);files[1]=Kind::Current;event("build",true);}
    void rename(Slot from,Slot to){
        ++renames;const auto key="rename"+std::to_string(renames);event(key);
        assert(files[index(to)]==Kind::Missing);
        assert(files[index(from)]==(from==Slot::Stage?Kind::Current:Kind::Legacy));
        files[index(to)]=files[index(from)];files[index(from)]=Kind::Missing;event(key,true);
    }
};
static Ops make(Kind live,Kind stage=Kind::Missing,Kind backup=Kind::Missing){return {{live,stage,backup},{},{},0,0,0,0};}
template<class T> static bool throws(T& ops){try{(void)gztitle::upgrade(ops);return false;}catch(const std::runtime_error&){return true;}}
template<class T> static void success(T& ops){(void)gztitle::upgrade(ops);assert(ops.files[0]==Kind::Current);}

int main(){
    unsigned cases=0;
    // The exact helper, not a second model, evaluates every file-kind tuple.
    const Kind kinds[]={Kind::Missing,Kind::Legacy,Kind::Current,Kind::Unknown};
    for(auto live:kinds)for(auto stage:kinds)for(auto backup:kinds){
        Ops ops=make(live,stage,backup);const auto before=ops.files;
        const bool structural=(live!=Kind::Unknown)&&(stage==Kind::Missing||stage==Kind::Current)&&(backup==Kind::Missing||backup==Kind::Legacy);
        const bool eligible=structural&&(live==Kind::Current||(live==Kind::Legacy&&backup==Kind::Missing)||(live==Kind::Missing&&backup==Kind::Legacy));
        if(eligible){success(ops);assert(ops.files[2]==(live==Kind::Current?backup:Kind::Legacy));}
        else{assert(throws(ops));assert(ops.files==before);assert(ops.builds==0&&ops.renames==0&&ops.flushes==0);}
        ++cases;
    }
    // Strict final identity never accepts a legacy or missing live title.
    for(auto live:kinds)for(auto stage:kinds)for(auto backup:kinds){
        const gztitle::State s{live,stage,backup};
        assert(gztitle::owned(s,false)==(live==Kind::Current));++cases;
    }
    for(const std::string point:{"build","flush1","rename1","rename2","flush2","checkpoint1","checkpoint2"}){
        Ops ops=make(Kind::Legacy);ops.failBefore=point;assert(throws(ops));
        // Every injected failure preserves an exact legacy copy or a fully
        // installed current title. The helper never erases a prior archive.
        assert(ops.files[0]==Kind::Legacy||ops.files[2]==Kind::Legacy);
        success(ops);++cases;
    }
    for(const std::string point:{"build","flush1","rename1","rename2","flush2"}){
        Ops ops=make(Kind::Legacy);ops.failAfter=point;assert(throws(ops));
        assert(ops.files[0]==Kind::Legacy||ops.files[2]==Kind::Legacy);
        success(ops);++cases;
    }
    // Failed promotion and failed rollback retain Missing/Current/Legacy,
    // which a later exact retry can complete without another first rename.
    {Ops ops=make(Kind::Legacy);ops.failBefore="rename2";ops.failAfter="rename3";assert(throws(ops));assert(ops.files[0]==Kind::Legacy);success(ops);++cases;}
    // One failure at promotion plus rollback-before failure needs two faults.
    struct RollbackOps:Ops {
        bool promotion=true,rollback=true;
        void rename(Slot from,Slot to){
            if(from==Slot::Stage&&promotion){promotion=false;throw std::runtime_error("promotion failed");}
            if(from==Slot::Backup&&rollback){rollback=false;throw std::runtime_error("rollback failed");}
            Ops::rename(from,to);
        }
    };
    {RollbackOps ops;ops.files={Kind::Legacy,Kind::Missing,Kind::Missing};assert(throws(ops));
     assert(ops.files[0]==Kind::Missing&&ops.files[1]==Kind::Current&&ops.files[2]==Kind::Legacy);
     (void)gztitle::upgrade(ops);assert(gztitle::committed(gztitle::inspect(ops)));++cases;}
    std::cout<<"title_upgrade actual-header cases: "<<cases<<" passed\n";
}
