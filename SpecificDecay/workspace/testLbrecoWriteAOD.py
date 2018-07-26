#!/usr/bin/env cmsRun
import FWCore.ParameterSet.Config as cms

process = cms.Process("bphAnalysis")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1000) )
process.load("Configuration.Geometry.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")

process.CandidateSelectedTracks = cms.EDProducer( "ConcreteChargedCandidateProducer",
                #src=cms.InputTag("oniaSelectedTracks::RECO"),
                src=cms.InputTag("generalTracks::RECO"),
                particleType=cms.string('pi+')
)

from PhysicsTools.PatAlgos.producersLayer1.genericParticleProducer_cfi import patGenericParticles
process.patSelectedTracks = patGenericParticles.clone(src=cms.InputTag("CandidateSelectedTracks"))

process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(False) )
process.options.allowUnscheduled = cms.untracked.bool(True)

process.source = cms.Source("PoolSource",fileNames = cms.untracked.vstring(
'file:///home/ltsai/Data/CMSSW_9_4_0/default/0294A5EC-3CED-E711-97F1-001E677925F0.root'
))
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')
process.GlobalTag = GlobalTag(process.GlobalTag, '94X_dataRun2_ReReco_EOY17_v1', '')

#from BPHAnalysis.SpecificDecay.LbrecoSelectForWrite_cfi import recoSelect
from BPHAnalysis.SpecificDecay.newSelectForWrite_cfi import recoSelect


# loaded default PAT sequence
process.load('PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff')
process.load('PhysicsTools.PatAlgos.selectionLayer1.selectedPatCandidates_cff')
process.load('PhysicsTools.PatAlgos.cleaningLayer1.cleanPatCandidates_cff')

#process.selectedPatMuons.src = 
process.selectedPatMuons.cut = cms.string('muonID(\"TMOneStationTight\")'
    ' && abs(innerTrack.dxy) < 0.3'
    ' && abs(innerTrack.dz)  < 20.'
    ' && innerTrack.hitPattern.trackerLayersWithMeasurement > 5'
    ' && innerTrack.hitPattern.pixelLayersWithMeasurement > 0'
    ' && innerTrack.quality(\"highPurity\")'
)

#make patTracks
from PhysicsTools.PatAlgos.tools.trackTools import makeTrackCandidates
makeTrackCandidates(process,
    label        = 'TrackCands',                  # output collection
    tracks       = cms.InputTag('generalTracks'), # input track collection
    particleType = 'pi+',                         # particle type (for assigning a mass)
    preselection = 'pt > 0.7',                    # preselection cut on candidates
    selection    = 'pt > 0.7',                    # selection on PAT Layer 1 objects
    isolation    = {},                            # isolations to use (set to {} for None)
    isoDeposits  = [],
    mcAs         = None                           # replicate MC match as the one used for Muons
)
process.patTrackCands.embedTrack = True

# remove MC dependence
from PhysicsTools.PatAlgos.tools.coreTools import *
removeMCMatching(process, names=['All'], outputModules=[] )

HLTName='HLT_DoubleMu4_JpsiTrk_Displaced_v*'
from HLTrigger.HLTfilters.hltHighLevel_cfi import hltHighLevel
process.hltHighLevel= hltHighLevel.clone(HLTPaths = cms.vstring(HLTName))


process.lbWriteSpecificDecay = cms.EDProducer('lbWriteSpecificDecay',

# the label used in calling data
    bsPointLabel = cms.string('offlineBeamSpot::RECO'),
    pVertexLabel = cms.string('offlinePrimaryVertices::RECO'),
    gpCandsLabel = cms.string('patSelectedTracks'),
    #patMuonLabel = cms.string('selectedPatMuons'),
    #ccCandsLabel = cms.string('onia2MuMuPAT::RECO'),
    dedxHrmLabel = cms.string('dedxHarmonic2::RECO'),
    #dedxPLHLabel = cms.string('dedxPixelHarmonic2::RECO'),

# the label of output product
    oniaName      = cms.string('oniaFitted'),
    TkTkName      = cms.string('TkTkFitted'),
    LbToTkTkName  = cms.string('LbToTkTkFitted'),
    #Lam0Name      = cms.string('Lam0Fitted'),
    #LbToLam0Name  = cms.string('LbToLam0Fitted'),
    writeVertex   = cms.bool( True ),
    writeMomentum = cms.bool( True ),
    recoSelect    = cms.VPSet(recoSelect)
)

process.out = cms.OutputModule(
    "PoolOutputModule",
    #fileName = cms.untracked.string('recoWriteSpecificDecay.root'),
    #fileName = cms.untracked.string('recoWriteSDecay.mcLbToJPsipK_13TeV_noPU.root'),
    fileName = cms.untracked.string('recoWriteSDecay.2017testData_13TeV.root'),
    outputCommands = cms.untracked.vstring(
        "drop *",
        "keep *_lbWriteSpecificDecay_*_bphAnalysis",
        "keep *_offlineBeamSpot_*_RECO",
        "keep *_offlinePrimaryVertices_*_RECO",
        #"keep *_genParticles__HLT"
    )
)

process.p = cms.Path(
    process.hltHighLevel *
    process.CandidateSelectedTracks *
    process.patSelectedTracks*
    process.lbWriteSpecificDecay
)

process.e = cms.EndPath(process.out)
