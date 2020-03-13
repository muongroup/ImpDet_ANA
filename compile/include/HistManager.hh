#ifndef HistManager_h
#define HistManager_h 1

#include <TStyle.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TROOT.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TFile.h>
#include <TApplication.h>
#include <TObject.h>
#include <RtypesCore.h>

#include "DataProcess.hh"

class HistManager
{
public:
    HistManager();
    ~HistManager();

public:
    static void SetStyle();
    static void NewHistObject();
    // Setter
    static void SetCoinhist(int ch, int value);
    static void SetHist(int ch, int value);
    static void SetLosthist(int ch, int value);
    static void SetVechist(int ch, int value);
    static void SetCoinchhist(int x, int y);
    static void SetVecplot(int x, int y,Double_t z);
    static void SetGrValue(int point, int x, int y);
    static void SetGrError(int point, double ex, double ey);
    // Getter
    // static TH1F *GetCoinhist() { return coinhist; }
    // static TH1F *GetHist() { return hist; }
    // static TH1F *GetLosthist() { return losthist; }
    // static TH1F *GetVechist() { return vechist; }
    // static TH2F *GetCoinchhist() { return coinchhist; }
    // static TGraphErrors *GetGr() { return gr; }
    // static Double_t Getmeans() { return means[CH]; };
    // static Double_t GetCONSTANT() { return CONSTANT[CH]; };
    // static Double_t GetMEANS() { return MEANS[CH]; };
    // static Double_t GetSIGMA() { return SIGMA[CH]; };
    // static Double_t GetMEANS_error() { return MEANS_error[CH]; };

    // Draw and Save
    static void DrawHist();
    static void DrawCoinhist();
    static void DrawLosthist();
    static void DrawVechist();
    static void DrawCoinchhist();
    static void DrawZeroEveGr();
    static void DrawEachChEveGr();
    static void Draw2Dhist();

protected:
    static TH1F *hist[CH];
    static TH1F *coinhist[CH];
    static TH1F *losthist[CH];
    static TH1F *vechist[CH];
    static TH2F *coinchhist;
    static TH2D *vecplot;
    static TH2D *vecploterr;
    static TGraphErrors *gr;
    static TGraphErrors *gr2;

public:
    // Fitting Parameter
    static Double_t means[CH];
    static Double_t CONSTANT[CH];
    static Double_t MEANS[CH];
    static Double_t SIGMA[CH];
    static Double_t MEANS_error[CH];
};

#endif