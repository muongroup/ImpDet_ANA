#include <sstream>

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
#include "HistManager.hh"

using namespace std;

// const Int_t CH = 64;
// const Int_t VEC = 31;
// const Int_t SHEETNUM = 4;

TH1F *HistManager::coinhist[CH] = {};
TH1F *HistManager::hist[CH] = {};
TH1F *HistManager::losthist[CH] = {};
TH1F *HistManager::vechist[CH] = {};
TH2F *HistManager::coinchhist = 0;
TGraphErrors *HistManager::gr = 0;

HistManager::HistManager()
{
}

HistManager::~HistManager()
{
}

void HistManager::SetStyle()
{
    gStyle->SetPalette(1);
    gStyle->SetOptFit(0001);
}

void HistManager::NewHistObject()
{
    for (Int_t i = 0; i < CH; i++)
    {
        ostringstream _cname;
        _cname << "cch" << i;
        string chhist_name = _cname.str();
        coinhist[i] = new TH1F(chhist_name.c_str(), chhist_name.c_str(), 4096, 0, 4096);

        ostringstream _name;
        _name << "ch" << i;
        string hist_name = _name.str();
        hist[i] = new TH1F(hist_name.c_str(), hist_name.c_str(), 4096, 0, 4096);

        ostringstream _lostname;
        _lostname << "lostch" << i;
        string losthist_name = _lostname.str();
        losthist[i] = new TH1F(losthist_name.c_str(), losthist_name.c_str(), 4096, 0, 4096);
    }

    for (Int_t i = 0; i < VEC; i++)
    {
        ostringstream _aname;
        _aname << "vec" << i;
        string ahist_name = _aname.str();
        vechist[i] = new TH1F(ahist_name.c_str(), ahist_name.c_str(), 4096, 0, 4096);
    }

    coinchhist = new TH2F("coinchhist", "", 64, 0 - 0.5, 64 - 0.5, 3000, 500, 3500);
    gr = new TGraphErrors;
}

void HistManager::SetCoinhist(int ch,int value){
    coinhist[ch]->Fill(value);
}
void HistManager::SetHist(int ch,int value){
    hist[ch]->Fill(value);
}
void HistManager::SetLosthist(int ch,int value){
    losthist[ch]->Fill(value);
}
void HistManager::SetVechist(int ch,int value){
    vechist[ch]->Fill(value);
}
void HistManager::SetCoinchhist(int x, int y){
    coinchhist->Fill(x, y);
}
void HistManager::SetGrValue(int point, int x, int y){
    gr->SetPoint(point, x, y);
}
void HistManager::SetGrError(int point, double ex, double ey){
    gr->SetPointError(point, ex, ey);
}

void HistManager::DrawHist(){
    TCanvas *cvs1 = new TCanvas("cvs1", "hist");
    cvs1->cd();
    cvs1->SetLogy();
    for (Int_t i = 0; i < CH; i++)
    {
        hist[i]->Draw("");
        if (i == 0)
        {
            cvs1->Print("./3_pdf/hist.pdf(");
        }
        else if (i != 63)
        {
            cvs1->Print("./3_pdf/hist.pdf");
        }
        else
        {
            cvs1->Print("./3_pdf/hist.pdf)");
        }
    }
}