# Copyright (c) 2012 Mark D. Hill and David A. Wood
# Copyright (c) 2015 The University of Wisconsin
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Nilay Vaish and Dibakar Gope

from m5.SimObject import SimObject
from m5.params import *

class BranchPredictor(SimObject):
    type = 'BranchPredictor'
    cxx_class = 'BPredUnit'
    cxx_header = "cpu/pred/bpred_unit.hh"
    abstract = True

    numThreads = Param.Unsigned(1, "Number of threads")
    BTBEntries = Param.Unsigned(1024, "Number of BTB entries")
    BTBTagSize = Param.Unsigned(20, "Size of the BTB tags, in bits")
    RASSize = Param.Unsigned(16, "RAS size")
    instShiftAmt = Param.Unsigned(2, "Number of bits to shift instructions by")

    faultEnabled = Param.Bool(False, "Whether the fault is enabled or not")
    faultLabel = Param.String(None, "Injected fault label")
    faultStuckBit = Param.Unsigned(0, "Faulted value")
    faultField = Param.Unsigned(0, "0 = Tag, 1 = Target, 2 = Valid")
    faultEntry = Param.Unsigned(0, "The entry where inject the fault")
    faultBitPosition = Param.Unsigned(0, "The bit target of the fault")
    faultTickBegin = Param.Int64(0, "Fault begin time")
    faultTickEnd = Param.Int64(-1, "Fault end time")
    faultEnd = Param.Bool(False, "It is the end event of a transient fault")

    controlFaultEnabled = Param.Bool(False,
        "Whether the control fault is enabled or not")
    controlFaultTriggerDescriptor = Param.String(None,
        "Description of the trigger for the injected control fault")
    controlFaultActionDescriptor = Param.String(None,
        "Description of the action for the  injected control fault")

    useIndirect = Param.Bool(False, "Use indirect branch predictor")
    indirectHashGHR = Param.Bool(True, "Hash branch predictor GHR")
    indirectHashTargets = Param.Bool(True, "Hash path history targets")
    indirectSets = Param.Unsigned(256, "Cache sets for indirect predictor")
    indirectWays = Param.Unsigned(2, "Ways for indirect predictor")
    indirectTagSize = Param.Unsigned(16, "Indirect target cache tag bits")
    indirectPathLength = Param.Unsigned(3,
        "Previous indirect targets to use for path history")



class LocalBP(BranchPredictor):
    type = 'LocalBP'
    cxx_class = 'LocalBP'
    cxx_header = "cpu/pred/2bit_local.hh"

    localPredictorSize = Param.Unsigned(1024, "Size of local predictor")
    localCtrBits = Param.Unsigned(2, "Bits per counter")


class TournamentBP(BranchPredictor):
    type = 'TournamentBP'
    cxx_class = 'TournamentBP'
    cxx_header = "cpu/pred/tournament.hh"

    localPredictorSize = Param.Unsigned(2048, "Size of local predictor")
    localCtrBits = Param.Unsigned(2, "Bits per counter")
    localHistoryTableSize = Param.Unsigned(2048, "size of local history table")
    globalPredictorSize = Param.Unsigned(8192, "Size of global predictor")
    globalCtrBits = Param.Unsigned(2, "Bits per counter")
    choicePredictorSize = Param.Unsigned(8192, "Size of choice predictor")
    choiceCtrBits = Param.Unsigned(2, "Bits of choice counters")


class BiModalBP(BranchPredictor):
    type = 'BiModalBP'
    cxx_class = 'BiModalBP'
    cxx_header = "cpu/pred/bi_modal.hh"

    historyBits = Param.Unsigned(4,
        "Number of bit of the global history register")
    predictorSize = Param.Unsigned(1024, "Size of global predictor")
    ctrBits = Param.Unsigned(2, "Bits per counter")


class BiModeBP(BranchPredictor):
    type = 'BiModeBP'
    cxx_class = 'BiModeBP'
    cxx_header = "cpu/pred/bi_mode.hh"

    globalPredictorSize = Param.Unsigned(8192, "Size of global predictor")
    globalCtrBits = Param.Unsigned(2, "Bits per counter")
    choicePredictorSize = Param.Unsigned(8192, "Size of choice predictor")
    choiceCtrBits = Param.Unsigned(2, "Bits of choice counters")

class GShareBP(BranchPredictor):
    type = 'GShareBP'
    cxx_class = 'GShareBP'
    cxx_header = "cpu/pred/gshare.hh"

    globalHistoryBits = Param.Unsigned(6,
        "Number of bit of the global history register")
    ctrBits = Param.Unsigned(2, "Bits per counter")
    predictorSets = Param.Unsigned(1024,
        "Size of global predictor")
