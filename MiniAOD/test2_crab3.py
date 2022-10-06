from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName   = 'Rivet-v3'
config.General.transferLogs = True

config.section_("JobType")
config.JobType.pluginName  = 'Analysis'
#config.JobType.generator = 'lhe'
#config.JobType.inputFiles = ['LOWVA.lhe']
# Name of the CMSSW configuration file
config.JobType.psetName    = 'Rivet-on-MiniAOD.py'


config.section_("Data")
# This string determines the primary dataset of the newly-produced outputs.
# For instance, this dataset will be named /CrabTestSingleMu/something/USER
config.Data.inputDataset = '/ZGToLLG_01J_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIISummer20UL18MiniAODv2-106X_upgrade2018_realistic_v16_L1v1-v1/MINIAODSIM'
#config.Data.primaryDataset = 'Bulk'
#config.Data.inputDBS = 'phys03'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.totalUnits = 20
config.Data.publication = False

# This string is used to construct the output dataset name
#config.Data.outputPrimaryDataset  = 'test-GENSIM-v1'

config.section_("Site")
# Where the output files will be transmitted to
config.Site.storageSite = 'T2_CH_CERN'
