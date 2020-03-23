#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include <TApplication.h>
#include <TGraphErrors.h>
#include <TGraph2D.h>
#include <TObject.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TGaxis.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>

#include "FilePath.hh"
#include "HistManager.hh"
#include "DataProcess.hh"
#include "CountManager.hh"
#include "AdvancedAnalysis.hh"

using namespace std;

void idana()
{
  DataProcess dP;
  AdvancedAnalysis aA;

  dP.DataLoad();

  TFile *fout = new TFile("out.root", "RECREATE", "idana_results");

  HistManager::SetStyle();
  HistManager::NewHistObject();

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

  // read each file
  for (Int_t iiii = 0; iiii < filenum; iiii++)
  {
    CountManager::CountZero("ZERO");
    CountManager::CountZero("READ");
    CountManager::CountZero("ACCE");

    string fileName;
    string time;
    fileName = filename[iiii];
    ifstream aFile(fileName.c_str(), ios::binary);
    fp.LoadingMessenger(!aFile.is_open(), iiii);
    time = fp.GetFileTime(fileName,"TEXT");

    // read a file
    while (!aFile.eof())
    {
      dP.FillHist(aFile);
    }
    dP.SetPedestal();

    // back to first of a file
    aFile.clear();
    aFile.seekg(0);

    // read a file
    while (!aFile.eof())
    {
      dP.GetDatafromBinary(aFile);
      dP.SetADCch();
      dP.SetFlag();
      dP.CoinEveProcess(iiii);
    }

    aA.CreateOutput(iiii, filename, 2, -5, 2, -2, 3);

    cout << fileName << " :(0,0) COUNT:" << CountManager::GetCount("ZERO") << endl;
    dP.WriteDeviation(time);
    CountManager::CountZero("ACCE");
    CountManager::CountZero("IDEA");
    HistManager::SetGrValue(iiii, iiii, CountManager::GetCount("ZERO"));
    HistManager::SetGrError(iiii, 0., sqrt((Double_t)(CountManager::GetCount("ZERO"))));
  }

  // create hist and graph
  HistManager::DrawHist();
  HistManager::DrawCoinhist();
  dP.WriteFitPara();
  HistManager::DrawLosthist();
  HistManager::DrawVechist();
  HistManager::DrawCoinchhist();
  HistManager::DrawZeroEveGr();
  HistManager::DrawEachChEveGr();
  dP.VecCountProcess();
  HistManager::Draw2Dhist();
  aA.DrawpixelsGr();
  fout->Write();
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
