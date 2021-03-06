/*
 * Copyright (c) 2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Anthony Gutierrez
 */

/* @file
 * Implementation of a bi-mode branch predictor
 */

#include "cpu/pred/gshare.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"

GShareBP::GShareBP(const GShareBPParams *params)
    : BPredUnit(params),
      globalHistoryRegs(params->numThreads, 0),
      globalHistoryBits(params->globalHistoryBits),
      predictorSets(params->predictorSets),
      ctrBits(params->ctrBits)
{

    addressBitMask = predictorSets - 1;
    historyRegisterMask = (1 << globalHistoryBits) - 1;

    DPRINTF(BPUinfo,"Used GShareBP with:\n");
    DPRINTF(BPUinfo,"ctrBits = %d\n");
    threadCounters.resize(params->numThreads);
    for ( int i = 0; i < params->numThreads; i++ ) {
        threadCounters[i].resize(predictorSets);
        for ( int j = 0; j < predictorSets; j++ )
            threadCounters[i][j].setBits(ctrBits);
    }

    if (  params->faultEnabled &&
          params->faultField == 3 &&
          params->faultTickEnd == -1) {
        if ( params->faultEntry >= predictorSets )
          fatal("BP: FaultEntry exceeds"
              "dimension of the saturating counter array");
        for ( int i = 0; i < params->numThreads; i++ )
          threadCounters[i][params->faultEntry].setFaulted(
            params->faultBitPosition,params->faultStuckBit);
    }


}

/*
 * For an unconditional branch we set its history such that
 * everything is set to taken. I.e., its choice predictor
 * chooses the taken array and the taken array predicts taken.
 */
void
GShareBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryRegs[tid];
    history->finalPred = true;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, true);
}

void
GShareBP::squash(ThreadID tid, void *bpHistory)
{
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryRegs[tid] = history->globalHistoryReg;

    delete history;
}

/*
 * Here we lookup the actual branch prediction. We use the PC to
 * identify the bias of a particular branch, which is based on the
 * prediction in the choice array. A hash of the global history
 * register and a branch's PC is used to index into both the taken
 * and not-taken predictors, which both present a prediction. The
 * choice array's prediction is used to select between the two
 * direction predictors for the final branch prediction.
 */

unsigned
GShareBP::indexCompute(ThreadID tid, Addr branchAddr,
    unsigned globalHistoryReg ){

    unsigned index = branchAddr >> instShiftAmt;

    for ( int i = 0; i < sizeof(Addr)/globalHistoryBits; i++)
        index ^= (globalHistoryReg & historyRegisterMask)
          << i*globalHistoryBits;

    return index & addressBitMask;
}

bool
GShareBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{

    unsigned index = indexCompute(tid,branchAddr,globalHistoryRegs[tid]);

    bool finalPrediction = threadCounters[tid][index].read() >>
                                                    (ctrBits-1);

    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryRegs[tid];
    history->finalPred = finalPrediction;
    bpHistory = static_cast<void*>(history);

    updateGlobalHistReg(tid, finalPrediction);
    return finalPrediction;
}

void
GShareBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    globalHistoryRegs[tid] &= (historyRegisterMask & ~ULL(1));
}

void
GShareBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed)
{
    if (bpHistory) {

        BPHistory *history = static_cast<BPHistory*>(bpHistory);

        unsigned index =
          indexCompute(tid,branchAddr,history->globalHistoryReg);

        if ( taken )
            threadCounters[tid][index].increment();
        else
            threadCounters[tid][index].decrement();

        if (squashed) {
            if (taken) {
                globalHistoryRegs[tid] = (history->globalHistoryReg << 1) | 1;
            } else {
                globalHistoryRegs[tid] = (history->globalHistoryReg << 1);
            }
            globalHistoryRegs[tid] &= historyRegisterMask;
        } else {
            delete history;
        }
    }
}

void
GShareBP::retireSquashed(ThreadID tid, void *bp_history)
{
    BPHistory *history = static_cast<BPHistory*>(bp_history);
    delete history;
}

unsigned
GShareBP::getGHR(ThreadID tid, void *bp_history) const
{
    return static_cast<BPHistory*>(bp_history)->globalHistoryReg;
}

void
GShareBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
    globalHistoryRegs[tid] = taken ? (globalHistoryRegs[tid] << 1) | 1 :
                               (globalHistoryRegs[tid] << 1);
    globalHistoryRegs[tid] &= historyRegisterMask;
}

GShareBP*
GShareBPParams::create()
{
    return new GShareBP(this);
}
