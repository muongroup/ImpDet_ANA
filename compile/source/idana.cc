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

void idana()
{
  DataProcess dP;

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

  stringstream ss1;
  struct stat st;

  for (Int_t iiii = 0; iiii < filenum; iiii++)
  {
    CountManager::CountZero("ZERO");
    CountManager::CountZero("READ");
    CountManager::CountZero("ACCE");

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
    }
    cout << ss1.str() << " :(0,0) COUNT:" << CountManager::GetCount("ZERO") << endl;
    dP.WriteDeviation(date);
    CountManager::CountZero("ACCE");
    CountManager::CountZero("IDEA");
    HistManager::SetGrValue(iiii, iiii, CountManager::GetCount("ZERO"));
    HistManager::SetGrError(iiii, 0., sqrt((Double_t)(CountManager::GetCount("ZERO"))));
  }
 ///////////////////////////////CREATE HISTGRAM
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
