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
    static void SetGrValue(int point, int x, int y);
    static void SetGrError(int point, double ex, double ey);
    // Getter
    // static TH1F *GetCoinhist() { return coinhist; }
    // static TH1F *GetHist() { return hist; }
    // static TH1F *GetLosthist() { return losthist; }
    // static TH1F *GetVechist() { return vechist; }
    // static TH2F *GetCoinchhist() { return coinchhist; }
    // static TGraphErrors *GetGr() { return gr; }
    // Draw and Save
    static void DrawHist();
    static void DrawCoinhist();
    static void DrawLosthist();
    static void DrawVachist();
    static void DrawCoinchhist();
    static void DrawGr();

public:
    static TH1F *coinhist[CH];
    static TH1F *hist[CH];
    static TH1F *losthist[CH];
    static TH1F *vechist[CH];
    static TH2F *coinchhist;
    static TGraphErrors *gr;
};

#endif