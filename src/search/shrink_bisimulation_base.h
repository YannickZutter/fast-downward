#ifndef SHRINK_BISIMULATION_BASE_H
#define SHRINK_BISIMULATION_BASE_H
#include "shrink_strategy.h"
#include <vector>

/* A base class for shrink strategies that create a bisimulation
   (ShrinkBisimulation and ShrinkDFP). Its main purpose is to provide
   the are_bisimilar()-method.
*/

class ShrinkBisimulationBase : public ShrinkStrategy {
public:
    ShrinkBisimulationBase();
    virtual ~ShrinkBisimulationBase();
    virtual void shrink(Abstraction &abs, int threshold, bool force = false) const = 0;

    virtual bool is_bisimulation() const = 0;
    virtual bool has_memory_limit() const = 0;
    virtual bool is_dfp() const = 0;

    virtual std::string description() const = 0;
protected:
    bool are_bisimilar(
        const vector<pair<int, int> > &succ_sig1,
        const vector<pair<int, int> > &succ_sig2,
        bool greedy_bisim,
        const vector<int> &group_to_h,
        int source_h_1, int source_h_2) const;
};

// TODO: document purpose of Signatures
typedef std::vector<std::pair<int, int> > SuccessorSignature;

struct Signature {
    int h;
    int group;
    SuccessorSignature succ_signature;
    int state;

    Signature(int h_, int group_, const SuccessorSignature &succ_signature_,
              int state_)
        : h(h_), group(group_), succ_signature(succ_signature_), state(state_) {
    }

    bool matches(const Signature &other) const {
        return h == other.h && group == other.group && succ_signature
               == other.succ_signature;
    }

    bool operator<(const Signature &other) const {
        if (h != other.h)
            return h < other.h;
        if (group != other.group)
            return group < other.group;
        if (succ_signature != other.succ_signature)
            return succ_signature < other.succ_signature;
        return state < other.state;
    }
};


#endif
