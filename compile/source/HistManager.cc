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
TH2D *HistManager::vecplot = 0;
TH2D *HistManager::vecploterr = 0;
TGraphErrors *HistManager::gr = 0;
TGraphErrors *HistManager::gr2 = 0;

Double_t HistManager::means[CH] = {};
Double_t HistManager::CONSTANT[CH] = {};
Double_t HistManager::MEANS[CH] = {};
Double_t HistManager::SIGMA[CH] = {};
Double_t HistManager::MEANS_error[CH] = {};

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
    vecplot = new TH2D("vecplot", "", 31, -15.5, 15.5, 31, -15.5, 15.5);
    vecploterr = new TH2D("vecploterr", "", 31, -15.5, 15.5, 31, -15.5, 15.5);
    gr = new TGraphErrors;
    gr2 = new TGraphErrors;
}

void HistManager::SetCoinhist(int ch, int value)
{
    coinhist[ch]->Fill(value);
}
void HistManager::SetHist(int ch, int value)
{
    hist[ch]->Fill(value);
}
void HistManager::SetLosthist(int ch, int value)
{
    losthist[ch]->Fill(value);
}
void HistManager::SetVechist(int ch, int value)
{
    vechist[ch]->Fill(value);
}
void HistManager::SetCoinchhist(int x, int y)
{
    coinchhist->Fill(x, y);
}

void HistManager::SetVecplot(int x, int y, Double_t z)
{
    Double_t ze;
    vecplot->Fill(x, y, z);
    ze = sqrt(z) / z;
    vecploterr->Fill(x, y, ze);
}

void HistManager::SetGrValue(int point, int x, int y)
{
    gr->SetPoint(point, x, y);
}
void HistManager::SetGrError(int point, double ex, double ey)
{
    gr->SetPointError(point, ex, ey);
}

void HistManager::DrawHist()
{
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

void HistManager::DrawCoinhist()
{
    TCanvas *cvs2 = new TCanvas("cvs2", "coinhist");
    cvs2->cd();
    //  cvs2->SetLogy();
    // Double_t_t means[CH], CONSTANT[CH], MEANS[CH], SIGMA[CH];
    // Double_t_t MEANS_error[CH];
    // ofstream coin_para("./2_txt/fitting_parameter_coin.txt");
    // coin_para << "means"
    //           << " "
    //           << "CONSTANT"
    //           << " "
    //           << "MEANS"
    //           << " "
    //           << "SIGMA"
    //           << " "
    //           << "MEANS_error" << endl;
    for (Int_t i = 0; i < CH; i++)
    {
        TF1 *f1 = new TF1("f1", "gaus", 1000, 3100);
        coinhist[i]->Draw("");
        coinhist[i]->Fit("f1", "", "", 1000, 3100);
        //  coinhist[i]->GetYaxis()->SetRangeUser(0,10);
        f1->Draw("same");
        if (i == 0)
        {
            cvs2->Print("./3_pdf/fit_coinhist.pdf(");
        }
        else if (i != 63)
        {
            cvs2->Print("./3_pdf/fit_coinhist.pdf");
        }
        else
        {
            cvs2->Print("./3_pdf/fit_coinhist.pdf)");
        }
        means[i] = coinhist[i]->GetMean();
        CONSTANT[i] = f1->GetParameter(0);
        MEANS[i] = f1->GetParameter(1);
        MEANS_error[i] = f1->GetParError(1);
        SIGMA[i] = f1->GetParameter(2);
        // cout << means[i] << " " << CONSTANT[i] << " " << MEANS[i] << " " << SIGMA[i] << " " << MEANS_error[i] << endl;
    }
}

void HistManager::DrawLosthist()
{
    for (Int_t i = 0; i < CH; i++)
    {
        losthist[i]->Add(hist[i], coinhist[i], 1, -1);
    }
    TCanvas *cvs3 = new TCanvas("cvs3", "losthist");
    cvs3->cd();
    cvs3->SetLogy();
    for (Int_t i = 0; i < CH; i++)
    {
        losthist[i]->Draw("");
        if (i == 0)
        {
            cvs3->Print("./3_pdf/lost.pdf(");
        }
        else if (i != 63)
        {
            cvs3->Print("./3_pdf/lost.pdf");
        }
        else
        {
            cvs3->Print("./3_pdf/lost.pdf)");
        }
    }
}

void HistManager::DrawVechist()
{
    TCanvas *cvs4 = new TCanvas("cvs4", "vechist");
    cvs4->cd();
    for (Int_t i = 0; i < VEC; i++)
    {

        vechist[i]->Draw("");

        if (i == 0)
        {
            cvs4->Print("./3_pdf/y_vec.pdf(");
        }
        else if (i != 30)
        {
            cvs4->Print("./3_pdf/y_vec.pdf");
        }
        else
        {
            cvs4->Print("./3_pdf/y_vec.pdf)");
        }
    }
}

void HistManager::DrawCoinchhist()
{
    TCanvas *cvs5 = new TCanvas("cvs5", "coinchhist");
    cvs5->cd();
    coinchhist->Draw("COLZ");
}

void HistManager::DrawZeroEveGr()
{
    TCanvas *cvs6 = new TCanvas("cvs6", "Entry 00");
    gr->SetTitle("entries(coincidence)");
    gr->GetYaxis()->SetTitle("");
    gr->GetXaxis()->SetTitle("number");
    gr->SetMarkerSize(1);
    gr->SetMarkerStyle(20);
    gr->GetYaxis()->CenterTitle();
    gr->GetXaxis()->CenterTitle();
    gr->Draw("APL");
    cvs6->Print("./4_gif/coin00.gif");
}

void HistManager::DrawEachChEveGr()
{
    TCanvas *cvs7 = new TCanvas("cvs7", "Each Entry");
    cvs7->cd();
    Double_t Entries[CH];
    Double_t EntriesErr[CH];
    for (Int_t i = 0; i < CH; i++)
    {
        Entries[i] = coinhist[i]->GetEntries();
        EntriesErr[i] = sqrt(Entries[i]);
        gr2->SetPoint(i, i, Entries[i]);
        gr2->SetPointError(i, 0, EntriesErr[i]);
    }
    gr2->SetMarkerColor(kRed);
    gr2->SetTitle("Each_Channel_entries(coincidence)");
    gr2->GetYaxis()->SetTitle("");
    gr2->GetXaxis()->SetTitle("MPPC_Channel_Number");
    gr2->SetMarkerSize(1);
    gr2->SetMarkerStyle(20);
    gr2->GetYaxis()->CenterTitle();
    gr2->GetXaxis()->CenterTitle();
    gr2->Draw("APL");
    cvs7->Print("./4_gif/Each_MPPC_Entries.gif");
}

void HistManager::Draw2Dhist()
{
    TCanvas *cvs8 = new TCanvas("cvs8", "vector plot");
    cvs8->cd();
    vecplot->SetStats(0);
    vecplot->Draw("COLZ");
    cvs8->Print("./4_gif/Int_tensity_2D.gif");

    TCanvas *cvs9 = new TCanvas("cvs9", "vector plot error");
    cvs9->cd();
    vecploterr->SetStats(0);
    vecploterr->Draw("COLZ");
    cvs9->Print("./4_gif/Int_tensity_2D_error.gif");
}

Int_t HistManager::GetMaxHistBin(Int_t i)
{
    Int_t max;
    max = hist[i]->GetMaximumBin();
    return max;
}