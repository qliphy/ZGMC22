// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Event.hh"
#include "Rivet/Math/LorentzTrans.hh"
#include "Rivet/Particle.hh"
#include "Rivet/Projections/ChargedLeptons.hh"
#include "Rivet/Projections/DressedLeptons.hh"
#include "Rivet/Projections/FastJets.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/IdentifiedFinalState.hh"
#include "Rivet/Projections/MissingMomentum.hh"
#include "Rivet/Projections/PromptFinalState.hh"
#include "Rivet/Projections/VetoedFinalState.hh"
#include "Rivet/Projections/InvisibleFinalState.hh"

namespace Rivet {

  /// @brief Z(ll)y cross-section at 13 TeV
  class MyZG : public Analysis {
  public:

    /// Constructor
    DEFAULT_RIVET_ANALYSIS_CTOR(MyZG);

    /// @name Analysis methods
    //@{
    /// Book histograms and initialise projections before the run
    void init() {

    FinalState fs;
    declare(fs, "FinalState");


      // Prompt photons
      const PromptFinalState photon_fs(Cuts::abspid == PID::PHOTON && Cuts::pT > 30*GeV && Cuts::abseta < 2.5);
      declare(photon_fs, "Photons");

      // Prompt leptons
      const PromptFinalState bareelectron_fs = Cuts::abspid == PID::ELECTRON;
      const PromptFinalState baremuon_fs = Cuts::abspid == PID::MUON;

      // Dressed leptons
      const FinalState allphoton_fs(Cuts::abspid == PID::PHOTON); // photons used for lepton dressing
      const Cut leptoncut = Cuts::pT > 25*GeV && Cuts::abseta < 2.5; // to be cut further: PT>30GeV
      const DressedLeptons dressedelectron_fs(allphoton_fs, bareelectron_fs, 0.1, leptoncut, true); // use *all* photons for lepton dressing
      const DressedLeptons dressedmuon_fs(allphoton_fs, baremuon_fs, 0.1, leptoncut, true); // use *all* photons for lepton dressing

      declare(dressedelectron_fs, "Electrons");
      declare(dressedmuon_fs, "Muons");
      
      // FS excluding the leading photon
      VetoedFinalState vfs;
      vfs.addVetoOnThisFinalState(photon_fs);
      vfs.addVetoOnThisFinalState(dressedmuon_fs);
      vfs.addVetoOnThisFinalState(InvisibleFinalState());
      declare(vfs, "isolatedFS");

    vfs.vetoNeutrinos();
    FastJets fastjets(vfs, FastJets::ANTIKT, 0.4);
    declare(fastjets, "Jets");


      // Histograms
      book(_hist_EgammaT,     2, 1, 1); // dSigma / dE^gamma_T 
      book(_hist_etagamma,    3, 1, 1);
      book(_hist_mZgamma,     4, 1, 1); // dSigma / dm^{Zgamma}
      book(_hist_EZgammaT,    5, 1, 1);
      book(_hist_dPhiZgamma,  6, 1, 1);
      book(_hist_ETbyMZgamma, 7, 1, 1);
    }


/// Perform the per-event analysis
   void analyze(const Event& event) {
     // Get objects
     vector<DressedLepton> electrons = apply<DressedLeptons>(event, "Electrons").dressedLeptons();
     vector<DressedLepton> muons = apply<DressedLeptons>(event, "Muons").dressedLeptons();
     const Particles& photons = apply<PromptFinalState>(event, "Photons").particlesByPt();

     if (photons.empty())  vetoEvent;
     if (electrons.size() < 2 && muons.size() < 2)  vetoEvent;
     vector<DressedLepton> lep;
     // Sort the dressed leptons by pt
     if (electrons.size() >= 2) {
       lep.push_back(electrons[0]);
       lep.push_back(electrons[1]);
     } else {
       lep.push_back(muons[0]);
       lep.push_back(muons[1]);
     }
     if(lep[0].Et() < 30)  vetoEvent;
     if(lep[1].Et() < 30)  vetoEvent;

     double mll = (lep[0].momentum() + lep[1].momentum()).mass();
     if (mll < 40*GeV) vetoEvent;

     vector<Particle> selectedPh;
     Particles fs = apply<VetoedFinalState>(event, "isolatedFS").particles();
     for (const Particle& ph : photons){

       // check photon isolation
       //double coneEnergy(0.0);
       //for (const Particle& p : fs) {
       //  if ( deltaR(ph, p) < 0.2 )  coneEnergy += p.Et();
       //}
       //if (coneEnergy / ph.Et() > 0.07 )  continue;

       if (deltaR(ph, lep[0]) < 0.4) continue;
       if (deltaR(ph, lep[1]) < 0.4) continue;
       selectedPh.push_back(ph);
     }


     if(selectedPh.size()<1) vetoEvent;
     double mlly = (lep[0].momentum() + lep[1].momentum() + selectedPh[0].momentum()).mass();
     //if(mll + mlly <= 182*GeV) vetoEvent;
     double ptlly = (lep[0].momentum() + lep[1].momentum() + selectedPh[0].momentum()).pT();
     double dphilly = deltaPhi((lep[0].momentum() + lep[1].momentum()).phi(), selectedPh[0].momentum().phi());


////--------------Smooth isolation, in refer to CMS_2021_I1978840-------------------------------------------
      // Here we build a list of particles to cluster jets, to be used in the photon isolation.
      // The selection of particles that we want to veto from this isolation sum is non-trivial:
      // the leading and subleading pT lepton, the leading pT photon that has DeltaR > 0.4 from the (sub)leading pT
      // leptons, the invisibles  and any tau decay products. Therefore, the selection is done
      // here instead of in the initialise method.

      //??const Particles invisibles = apply<FinalState>(event, "Invisibles").particlesByPt();

      //Particles finalparts_iso = apply<FinalState>(event, "FinalState").particles();
      Particles filtered_iso;
      //for (Particle const& p : finalparts_iso) {
      for (Particle const& p : fs) {
        bool veto_particle = false;
        if (p.genParticle() == lep[0].genParticle() || p.genParticle() == lep[1].genParticle() || p.genParticle() == selectedPh[0].genParticle() || p.fromTau()) {
          veto_particle = true;
        }

       //?? if(invisibles.size() > 0) {
       //   for (auto const& inv : invisibles) {
       //     if (p.genParticle() == inv.genParticle()) {
       //       veto_particle = true;
       //     } 
       //   }
       // }
       //  

        if (!veto_particle) {
          filtered_iso.push_back(p);
        }
      }
      auto proj = getProjection<FastJets>("Jets");
      proj.reset();
      proj.calc(filtered_iso);
      auto jets_iso = proj.jets();

      bool p0_frixione = true;
      double frixione_sum = 0.;
      double photon_iso_dr_ = 0.4;

      // Apply Frixione isolation to the photon:
      auto jparts = sortBy(jets_iso, [&](Jet const& part1, Jet const& part2) {
        return deltaR(part1, selectedPh[0]) < deltaR(part2, selectedPh[0]);
      });

      for (auto const& ip : jparts) {
        double dr = deltaR(ip, selectedPh[0]);
        if (dr >= photon_iso_dr_) {
          break;
        }
        frixione_sum += ip.pt();
        if (frixione_sum > (selectedPh[0].pt() * ((1. - std::cos(dr)) / (1. - std::cos(photon_iso_dr_))))) {
          p0_frixione = false;
        }
      }

      if(!p0_frixione) vetoEvent;
//------------------------------------------------------------------





     // Fill plots
     _hist_EgammaT->fill(selectedPh[0].pT()/GeV);
     _hist_etagamma->fill(selectedPh[0].abseta());
     _hist_mZgamma->fill(mlly/GeV);
     _hist_EZgammaT->fill(ptlly/GeV);
     _hist_dPhiZgamma->fill(dphilly/pi);
     _hist_ETbyMZgamma->fill(ptlly/mlly);
   } // end of analysis

   /// Normalise histograms etc., after the run
   void finalize() {
      const double sf = crossSection()/femtobarn/sumOfWeights();
      scale(_hist_EgammaT, sf);
      scale(_hist_etagamma, sf);
      scale(_hist_mZgamma, sf);
      scale(_hist_EZgammaT, sf);
      scale(_hist_dPhiZgamma, sf/pi);
      scale(_hist_ETbyMZgamma, sf);
   }


  protected:

    // Data members like post-cuts event weight counters go here
    size_t _mode;

  private:

    /// Histograms
    Histo1DPtr _hist_EgammaT;
    Histo1DPtr _hist_etagamma;
    Histo1DPtr _hist_mZgamma;
    Histo1DPtr _hist_EZgammaT;
    Histo1DPtr _hist_dPhiZgamma;
    Histo1DPtr _hist_ETbyMZgamma;
  }; 

   // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(MyZG);

}
