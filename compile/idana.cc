#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <string.h>

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

#include "FilePath.hh"
#include "HistManager.hh"
#include "DataProcess.hh"
#include "CountManager.hh"

using namespace std;

typedef struct
{
  Double_t iden_eve;
  time_t m_time;
} meas_t;

void idana()
{
  DataProcess dP;
  CountManager cM;

  dP.DataLoad();

  TFile *fout = new TFile("out.root", "RECREATE", "idana_results");

  HistManager::SetStyle();
  HistManager::NewHistObject();
  stringstream ss1;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /*Get data files path by using Class FilePath.*/
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  FilePath fp;
  fp.SetFilePath();
  fp.FileListMessenger();

  Int_t filenum = fp.GetFilenum();
  string filename[filenum];
  struct dirent **namelist = fp.GetNameList();
  for (Int_t i = 0; i < filenum; i++)
  {
    filename[i] = namelist[i]->d_name;
  }
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  struct stat st;

  for (Int_t iiii = 0; iiii < filenum; iiii++)
  {
    ss1.str("");

    ss1 << filename[iiii];
    ifstream aFile(ss1.str().c_str(), ios::binary);
    fp.LoadingMessenger(!aFile.is_open(), iiii);

    stat(ss1.str().c_str(), &st);
    time_t date = st.st_mtime;

    // a file read
    while (!aFile.eof())
    {
      dP.GetDatafromBinary(aFile);
      dP.SetADCch();
      dP.SetFlag();
      dP.CoinEveProcess(iiii);
      //  f << ss1.str().c_str() << "," << coinhist[39]->GetEntries() << endl;
    }
    cout << ss1.str() << " :(0,0) COUNT:" << cM.GetCount("ZERO") << endl;
    dP.WriteDeviation(date);
    cM.CountZero("ACCE");
    cM.CountZero("IDEA");
    printf("%d\n", CountManager::GetCount("ZERO"));
    HistManager::SetGrValue(iiii, iiii, CountManager::GetCount("ZERO"));
    HistManager::SetGrError(iiii, 0., sqrt((Double_t)(CountManager::GetCount("ZERO"))));
  }
  /////////////////////////////////////////////////////////////////////CREATE HISTGRAM
  HistManager::DrawHist();
  /*
    ////////////////////////////////////////////////////////////////////COIN HIST
    TCanvas *cvs1 = new TCanvas("cvs1", "My Canvas");
    cvs1->cd();
    cvs1->SetLogy();
    Double_t_t Entries[n];
    Double_t_t EntryErrors[n];
    Double_t_t EntryErrors2[n];

    for (Int_t i = 0; i < n; i++)
    {
      Entries[i] = 0;
      EntryErrors[i] = 0;
      EntryErrors2[i] = 0;
    }

    for (Int_t i = 0; i < n; i++)
    {
      hist[i]->Draw("");
      if (i == 0)
      {
        cvs1->PrInt_t("./3_pdf/hist.pdf(");
      }
      else if (i != 63)
      {
        cvs1->PrInt_t("./3_pdf/hist.pdf");
      }
      else
      {
        cvs1->PrInt_t("./3_pdf/hist.pdf)");
      }
      Entries[i] = coinhist[i]->GetEntries();
      EntryErrors[i] = sqrt(Entries[i]);
      cout << Entries[i] << endl;
    }

    ////////////////////////////////////////////////////////////////////////LOST HIST
    for (Int_t i = 0; i < 64; i++)
    {
      losthist[i]->Add(hist[i], coinhist[i], 1, -1);
    }
    TCanvas *cvs100 = new TCanvas("cvs100", "My Canvas");
    cvs100->cd();
    cvs100->SetLogy();
    for (Int_t i = 0; i < n; i++)
    {
      losthist[i]->Draw("");
      if (i == 0)
      {
        cvs100->PrInt_t("./3_pdf/lost.pdf(");
      }
      else if (i != 63)
      {
        cvs100->PrInt_t("./3_pdf/lost.pdf");
      }
      else
      {
        cvs100->PrInt_t("./3_pdf/lost.pdf)");
      }
    }

    ///////////////////////////////////////////////////////////////////////////VECTOR HIST
    TCanvas *cvs300 = new TCanvas("cvs300", "My Canvas");
    cvs300->cd();
    //    cvs300->SetLogy();
    for (Int_t i = 0; i < 31; i++)
    {

      vechist[i]->Draw("");

      if (i == 0)
      {
        cvs300->PrInt_t("./3_pdf/y_vec.pdf(");
      }
      else if (i != 30)
      {
        cvs300->PrInt_t("./3_pdf/y_vec.pdf");
      }
      else
      {
        cvs300->PrInt_t("./3_pdf/y_vec.pdf)");
      }
    }

    ////////////////////////////////////////////////////////////////////////////FIT COIN HIST
    TCanvas *cvs2 = new TCanvas("cvs2", "My Canvas");
    cvs2->cd();
    //  cvs2->SetLogy();
    Double_t_t means[n], CONSTANT[n], MEANS[n], SIGMA[n];
    Double_t_t MEANS_error[n];
    ofstream coin_para("./2_txt/fitting_parameter_coin.txt");
    coin_para << "means"
              << " "
              << "CONSTANT"
              << " "
              << "MEANS"
              << " "
              << "SIGMA"
              << " "
              << "MEANS_error" << endl;
    for (Int_t i = 0; i < n; i++)
    {
      TF1 *f1 = new TF1("f1", "gaus", 1000, 3100);
      coinhist[i]->Draw("");
      coinhist[i]->Fit("f1", "", "", 1000, 3100);
      //  coinhist[i]->GetYaxis()->SetRangeUser(0,10);
      f1->Draw("same");
      if (i == 0)
      {
        cvs2->PrInt_t("./3_pdf/fit_coinhist.pdf(");
      }
      else if (i != 63)
      {
        cvs2->PrInt_t("./3_pdf/fit_coinhist.pdf");
      }
      else
      {
        cvs2->PrInt_t("./3_pdf/fit_coinhist.pdf)");
      }
      means[i] = coinhist[i]->GetMean();
      CONSTANT[i] = f1->GetParameter(0);
      MEANS[i] = f1->GetParameter(1);
      MEANS_error[i] = f1->GetParError(1);
      SIGMA[i] = f1->GetParameter(2);
      coin_para << means[i] << " " << CONSTANT[i] << " " << MEANS[i] << " " << SIGMA[i] << " " << MEANS_error[i] << endl;
    }

    Int_t index = 64;
    Double_t_t x[n];
    Double_t_t ex[n];
    for (Int_t i = 0; i < n; i++)
    {
      x[i] = i;
      ex[i] = 0;
    }

    /////////////////////////////////////////////////////////////////////////EACH MPPC ENTRY
    TCanvas *cvs13 = new TCanvas("cvs13", "Entry number");
    TGraph *gr10 = new TGraphErrors(index, x, Entries, ex, EntryErrors); //それぞれのMPPC位置番号をX、エントリー数をY軸に取ったもの
    gr10->SetMarkerColor(kRed);
    gr10->Draw("APL");
    gr10->SetTitle("Each_Channel_entries(coincidence)");
    gr10->GetYaxis()->SetTitle("");
    gr10->GetXaxis()->SetTitle("MPPC_Channel_Number");
    gr10->SetMarkerSize(1);
    gr10->SetMarkerStyle(20);
    gr10->GetYaxis()->CenterTitle();
    gr10->GetXaxis()->CenterTitle();
    cvs13->PrInt_t("./4_gif/Each_MPPC_Entries.gif");

    //////////////////////////////////////////////////////////////////////////COIN 00
    TCanvas *cvs30 = new TCanvas("cvs30", "Entry 00");
    gr2->Draw("APL");
    gr2->SetTitle("entries(coincidence)");
    gr2->GetYaxis()->SetTitle("");
    gr2->GetXaxis()->SetTitle("number");
    gr2->SetMarkerSize(1);
    gr2->SetMarkerStyle(20);
    gr2->GetYaxis()->CenterTitle();
    gr2->GetXaxis()->CenterTitle();
    cvs30->PrInt_t("./4_gif/coin00.gif"); //pdf->gifに変更20190718

    //////////////////////////////////////////////////////////////////////////2Dhist
    TCanvas *cvs4 = new TCanvas("cvs4", "My Canvas");
    TH2D *image_ij = new TH2D("image_ij", "", 31, -15.5, 15.5, 31, -15.5, 15.5);

    for (Int_t i = 0; i < 31; i++)
    {
      for (Int_t j = 0; j < 31; j++)
      {
        image_ij->Fill(i - 15, j - 15, (Double_t)count_ij[i][j]);
        all_data << i << "   " << j << "   " << count_ij[i][j] << endl;
      }
    }
    image_ij->SetStats(0);
    image_ij->Draw("colz");
    cvs4->PrInt_t("./4_gif/Int_tensity_2D.gif");

    ofstream output("./2_txt/output.txt");
    for (Int_t i = 0; i < 31; i++)
    {
      for (Int_t j = 0; j < 31; j++)
      {
        output << i - 15 << " " << j - 15 << " " << count_ij[i][j] << endl;
      }
    }

    TCanvas *cvs8 = new TCanvas("cvs8", "My Canvas");
    TH2D *image_ij_err = new TH2D("image_ij_err", "", 31, -15.5, 15.5, 31, -15.5, 15.5);
    for (Int_t i = 0; i < 31; i++)
    {
      for (Int_t j = 0; j < 31; j++)
      {
        image_ij_err->Fill(i - 15, j - 15, (Double_t)sqrt(count_ij[i][j]) / (Double_t)count_ij[i][j]);
      }
    }
    image_ij_err->SetStats(0);
    image_ij_err->Draw("colz");
    cvs8->PrInt_t("./4_gif/Int_tensity_2D_error.gif");

    fout->Write();*/
}

void StandaloneApplication(Int_t argc, char **argv)
{
  idana();
}

Int_t main(Int_t argc, char **argv)
{
  TApplication app("ROOT Application", &argc, argv);
  StandaloneApplication(app.Argc(), app.Argv());
  app.Run();
  return 0;
}
