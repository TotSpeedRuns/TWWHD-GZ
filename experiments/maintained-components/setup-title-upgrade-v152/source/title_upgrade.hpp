#pragma once
#include <exception>
#include <stdexcept>

// The Ops implementation owns three fixed paths in one checked Layout folder.
// No unknown file is deleted or overwritten. A legacy backup is retained.
namespace gztitle {
enum class Slot { Live, Stage, Backup };
enum class Kind { Missing, Legacy, Current, Unknown };
struct State { Kind live, stage, backup; };

template<class Ops> State inspect(Ops& ops) {
    const State s{ops.probe(Slot::Live), ops.probe(Slot::Stage), ops.probe(Slot::Backup)};
    if (s.live == Kind::Unknown || (s.stage != Kind::Missing && s.stage != Kind::Current) ||
        (s.backup != Kind::Missing && s.backup != Kind::Legacy))
        throw std::runtime_error("Unknown title upgrade file; all files retained");
    return s;
}

inline bool owned(const State& s, bool allowKnownPrior) {
    if (s.live == Kind::Current) return true;
    if (!allowKnownPrior) return false;
    if (s.live == Kind::Legacy && s.backup == Kind::Missing) return true;
    // A prior verified Live -> Backup rename may have completed before exit.
    // The exact legacy backup authenticates this missing-live retry state.
    return s.live == Kind::Missing && s.backup == Kind::Legacy;
}

inline bool committed(const State& s) {
    return s.live == Kind::Current && s.stage == Kind::Missing && s.backup == Kind::Legacy;
}

template<class Ops> bool upgrade(Ops& ops) {
    State s = inspect(ops);
    if (!owned(s, true)) throw std::runtime_error("Title upgrade identity differs; all files retained");
    if (s.live == Kind::Current) return false;
    ops.checkpoint();
    if (s.stage == Kind::Missing) ops.buildStage();
    s = inspect(ops);
    if (!owned(s, true) || s.live == Kind::Current || s.stage != Kind::Current)
        throw std::runtime_error("Staged title identity differs; all files retained");
    ops.flushVolume();
    s = inspect(ops);
    if (!owned(s, true) || s.live == Kind::Current || s.stage != Kind::Current)
        throw std::runtime_error("Title identity changed before replacement; all files retained");
    // Cancellation is accepted before either rename. No cancellation callback
    // runs within the bounded rename / rollback span.
    ops.checkpoint();
    if (s.live == Kind::Legacy) {
        if (s.backup != Kind::Missing)
            throw std::runtime_error("Title backup already exists; all files retained");
        ops.rename(Slot::Live, Slot::Backup);
    }
    try {
        s = inspect(ops);
        if (s.live != Kind::Missing || s.stage != Kind::Current || s.backup != Kind::Legacy)
            throw std::runtime_error("Title replacement state differs; all files retained");
        ops.rename(Slot::Stage, Slot::Live);
        if (!committed(inspect(ops)))
            throw std::runtime_error("Installed title identity differs; backup retained");
        ops.flushVolume();
        if (!committed(inspect(ops)))
            throw std::runtime_error("Installed title identity changed after flush; backup retained");
    } catch (...) {
        const auto failure = std::current_exception();
        // Roll back only the exact pre-second-rename state. Unknown state or
        // a completed replacement is retained for checked retry, not guessed.
        try {
            s = inspect(ops);
            if (s.live == Kind::Missing && s.stage == Kind::Current && s.backup == Kind::Legacy) {
                ops.rename(Slot::Backup, Slot::Live);
                ops.flushVolume();
                s = inspect(ops);
                if (s.live != Kind::Legacy || s.stage != Kind::Current || s.backup != Kind::Missing)
                    throw std::runtime_error("Title rollback identity differs; files retained");
            }
        } catch (...) {
            throw std::runtime_error("Title replacement stopped; backup/state retained for checked retry");
        }
        std::rethrow_exception(failure);
    }
    return true;
}
}
