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
#include "TStyle.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TCanvas.h"
#include "TGaxis.h"
#include "TROOT.h"
#include "TGraphErrors.h"
#include "TF1.h"

bool StrString(const char *s1,const char *s2);

void ImpDet_pedestal(){

  gStyle->SetPalette(1);
  gStyle->SetOptFit(0001);

  const int n=64;
  Int_t place[n];//検出器番号
  Int_t esrch[n];//EASIROC番号
  Int_t count_ij[31][31];

  Int_t x_vec=0;
  Int_t y_vec=0;

  for(Int_t i=0;i<n;i++)
  {
    place[i]=0;
    esrch[i]=0;
  }


  std::ifstream config("./1_info/config.txt");//config.txtから読み込み
  for(Int_t i=0;i<n;i++)
  {
    config >>esrch[i]>>place[i];//config.txtから読み込む
  }
  config.close();

  ///////////////////////////////////////////////////////////////define histgram

  TH1F *hist[n];

  for(Int_t i=0;i<n;i++)
  {
    std::ostringstream _name;
    _name<<"ch"<<i;
    std::string hist_name = _name.str();
    hist[i]=new TH1F(hist_name.c_str(),hist_name.c_str(),4096,0,4096);
  }

  ///////////////////////////////////////////////////////////////Include .dat file
  stringstream ss1;
  const char* path="./";
  DIR *dp;
  dirent* entry;
  string filename[4096];
  int filenum=0;
  dp = opendir(path);

  while((entry=readdir(dp))!=NULL)
  {
    string tmp;
    tmp=entry -> d_name;
    cout<<tmp<<endl;
    if(StrString(tmp.c_str(),".dat"))
    {
      filename[filenum]=tmp;
      cout<<tmp<<" "<<filenum<<" "<<filename[filenum]<<endl;
      filenum++;
    }
  }

  struct stat st;
  double sfile[filenum];

  for(Int_t iiii=0;iiii<filenum;iiii++)
  {
    ss1.str("");

    ss1<<filename[iiii];
    std::ifstream hoge(ss1.str().c_str(),std::ios::binary);
    if(!hoge.is_open())
    {
      cout<<"no file"<<endl;
      return;
    }
    else
    {
      cout<<iiii<<": "<<ss1.str()<<" loading "<<endl;
    }

    int eas_s1=0; //easiroc識別番号
    int eas_s2=0; //easiroc識別番号
    int e_count1=0; //イベントカウンター
    int e_count2=0; //イベントカウンター
    int buffermem1[64];
    int buffermem2[64];
    int counter=0;
    int read_count=0;
    int trigger=0;

    stat(ss1.str().c_str(), &st);
    time_t date=st.st_mtime;

    for(int i=0; i<64; i++)
    {
      buffermem1[i]=0;
      buffermem2[i]=0;
    }

    while(!hoge.eof())
    {
      UInt_t val;//datファイル内の情報を格納

//////////////////////////////////////////////////////////////////////////////////////////mu-PSD1
      hoge.read((char*)&val,sizeof(int));//一個目のヘッダーを読み込む
      read_count++;
      if(val==0xffffea0c)//この形式が0xffff0xea0cの場合
      {
        hoge.read((char*)&val,sizeof(int));//次の行を読み込む(Header2 : reserved Number of word(16bit))
        hoge.read((char*)&val,sizeof(int));//Header3 : Eventcounter(12bit) : easiroc識別番号(1 or 2 下位16bit)
        eas_s1=(val & 0xffff);//(下位16bit演算)
        e_count1=(val >> 16);//(16bit右シフト)
        trigger++;
        for(int i=0;i<64;++i)
        {
          if(eas_s1==1)
          {
            hoge.read((char*)&val,sizeof(int));
            int buffer=val & 0xffff;//後ろの2byteを読み込む
            if(i<32)
            {
              buffermem1[place[i]]=buffer;
              hist[place[i]]->Fill(buffer);
            }
          }
          else
          {
            cout<<"data is not correct (mu-PSD1)"<<endl;
          }
        }
      }
      else
      {
        cout<<"Header is not correct (mu-PSD1)"<<read_count<<endl;
      }

      //////////////////////////////////////////////////////////////////////////////////////////mu-PSD2
      hoge.read((char*)&val,sizeof(int));//一個目のヘッダーを読み込む
      read_count++;
      if(val==0xffffea0c)//この形式が0xffff0xea0cの場合
      {
        hoge.read((char*)&val,sizeof(int));//次の行を読み込む(Header2 : reserved Number of word(16bit))
        hoge.read((char*)&val,sizeof(int));//Header3 : Eventcounter(12bit) : easiroc識別番号(1 or 2 下位16bit)//65回読み込む意味はヘッダー一行読んだ後MPPC64個
        eas_s2=(val & 0xffff);//(下位16bit演算)
        e_count2=(val >> 16);//(16bit右シフト)

        for(int i=0;i<64;++i)
        {
          if(eas_s2==2)
          {
            hoge.read((char*)&val,sizeof(int));
            int buffer=val & 0xffff;//語尾についているのは何?=後ろの2byteを読み込む
            if(i<32)
            {
              buffermem2[place[i]]=buffer;
              hist[place[i]+32]->Fill(buffer);
            }
          }
          else
          {
            cout<<"data is not correct (mu-PSD2)"<<endl;
          }
        }
      }
      else
      {
        cout<<"Header is not correct (mu-PSD2)"<<read_count<<endl;
      }
    }///////while()
  }//for
/////////////////////////////////////////////////////////////////////////////////pedestal
  Double_t ped[n];
  ofstream pedestal("./1_info/pedestal.txt");
  for(Int_t i=0; i<n; i++)
  {
    ped[i]=hist[i]->GetMaximumBin();
    pedestal << ped[i] << endl;
  }
  
}//main


////////////////////////////////////////////////////////////////////////////////関数の定義
bool StrString(const char*s1,const char *s2)//サブルーチンみたいなもの
{
  int n;
  n=strlen(s2);//strlenは文字列の長さを返す

  while(1){
    s1=strchr(s1,s2[0]);//s1の最初からs2[0]の文字を探す。その文字がなければNULLを返す

    if(s1==NULL)

      return false;

    if(strncmp(s1,s2,n)==0)//s1とs2を前からn文字分比べる、s1=s2の場合0を返す
      return true;
    s1++;
  }
}
