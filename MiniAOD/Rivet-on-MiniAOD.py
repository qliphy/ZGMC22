import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('analysis')
options.parseArguments()

process = cms.Process('TEST')

# import of standard configurations
process.load('FWCore.MessageService.MessageLogger_cfi')
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.MessageLogger.cerr.FwkReport.reportEvery = 50
# Input source
process.source = cms.Source("PoolSource",
    fileNames  = cms.untracked.vstring(options.inputFiles),
#    fileNames = cms.untracked.vstring(
#     '/store/mc/RunIISummer20UL18MiniAODv2/ZGToLLG_01J_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8/MINIAODSIM/106X_upgrade2018_realistic_v16_L1v1-v1/00000/0FB611DB-DE9A-0E4D-9A91-A1CBFD85AE6B.root'
#    )
)
process.options = cms.untracked.PSet(
)

process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
process.load("GeneratorInterface.RivetInterface.mergedGenParticles_cfi")
process.load("GeneratorInterface.RivetInterface.genParticles2HepMC_cfi")
process.mergedGenParticles = cms.EDProducer("MergedGenParticleProducer",
    inputPruned = cms.InputTag("prunedGenParticles"),
    inputPacked = cms.InputTag("packedGenParticles"),
)
process.genParticles2HepMC.genParticles = cms.InputTag("mergedGenParticles")


#process.output = cms.OutputModule("PoolOutputModule",
#    fileName = cms.untracked.string( 
#        'file:out.root'
#    ),
#)
#process.p = cms.Path(process.mergedGenParticles*process.genParticles2HepMC)
#process.out_step     = cms.EndPath(process.output)


process.load('GeneratorInterface.RivetInterface.rivetAnalyzer_cfi')
process.rivetAnalyzer.AnalysisNames = cms.vstring('MyZG')
process.rivetAnalyzer.OutputFile = cms.string('mcfile.yoda')
process.rivetAnalyzer.HepMCCollection = cms.InputTag('genParticles2HepMC:unsmeared')
process.rivetAnalyzer.UseExternalWeight = cms.bool(True)
#process.rivetAnalyzer.useLHEweights = True
#process.rivetAnalyzer.CrossSection = cms.double(178.4)

process.p = cms.Path(process.mergedGenParticles*process.genParticles2HepMC*process.rivetAnalyzer)


