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
#include "TFile.h"

typedef struct
{
  double iden_eve;
  time_t m_time;
}meas_t;

bool StrString(const char *s1,const char *s2);
double calib1(double ch, double gain, double pedestal);
time_t  get_caltime(char *tstr, char *tstr2);
int compare_meas_time(const void *a, const void *b);

int timespan=3600.; //[sec]分割ファイルの時間間隔

void ImpDet_ANA(){

  std::ofstream all_data("./2_txt/all_data.txt");
  std::ofstream data_00("./2_txt/00_data.txt");
  std::ofstream all_data_ch("./2_txt/all_data_ch.txt");

  TFile *fout = new TFile("out.root", "RECREATE","idana_results");

  gStyle->SetPalette(1);
  gStyle->SetOptFit(0001);

  const int n=64;//チャンネルの数。どの検出器でも対応できるようにここで個数を決める。
  Int_t place[n];//位置番号
  Int_t esrch[n];//EASIROCモジュールが認識している位置番号
  Int_t count_ij[31][31];
  Int_t iden_eve=0;

  Int_t x_vec=0;
  Int_t y_vec=0;

  double entry_eff[n];
  for(Int_t i=0;i<n;i++)
  {
    place[i]=0;
    esrch[i]=0;
    entry_eff[i]=0.;
  }

  double place_eff[31][31]; //place番号xの差+15, place番号yの差+15の検出効率
  double place_acc[31][31]; //place番号xの差+15, place番号yの差+15のacceptance

  for(Int_t i=0;i<31;i++)
  {
    for(Int_t j=0;j<31;j++)
    {
      count_ij[i][j] = 0;
      place_eff[i][j] = 0.;
      place_acc[i][j] = 0.;

    }
  }

  for(int i=0; i<16; i++)
  {
    for(int j=16; j<32; j++)
    {
      for(int k=32; k<48; k++)
      {
        for(int l=48; l<64; l++)
        {
          x_vec = (63-l) - (31-j) + 15;
          y_vec = (k-32) - i + 15;
          place_eff[x_vec][y_vec] += 0.00390625; //組み合わせひとつにつき、1/(16*16)の効率を足す
        }
      }
    }
  }

  std::ifstream config("./1_info/config.txt");//config.txtから読み込み
  for(Int_t i=0;i<n;i++)
  {
    config >>esrch[i]>>place[i];//config.txtから読み込む
  }
  config.close();


  Double_t ped[n],thresh_ch[n];
  std::ifstream pedestal("./1_info/pedestal.txt");
  std::ifstream threshould("./1_info/threshould.txt");
  for(Int_t i=0;i<n;i++)
  {
    threshould >> thresh_ch[i];
    pedestal   >> ped[i];
  }


  ////////////////////////////////////////////////////////////////
  ///////////////////////   graph   //////////////////////
  ///////////////////////////////////////////////////////////////

  TH1F *coinhist[n];//64個のMPPCのコインシデンスを取った波高データのヒストグラム
  TH1F *hist[n];//64個のMPPCのコインシデンスを取った波高データのヒストグラム
  TH1F *losthist[n];//64個のMPPCのコインシデンスを取った波高データのヒストグラム

  for(Int_t i=0;i<n;i++)
  {
    std::ostringstream _cname;
    _cname<<"cch"<<i;
    std::string chhist_name = _cname.str();
    coinhist[i]=new TH1F(chhist_name.c_str(),chhist_name.c_str(),4096,0,4096);

    std::ostringstream _name;
    _name<<"ch"<<i;
    std::string hist_name = _name.str();
    hist[i]=new TH1F(hist_name.c_str(),hist_name.c_str(),4096,0,4096);

    std::ostringstream _lostname;
    _lostname<<"lostch"<<i;
    std::string losthist_name = _lostname.str();
    losthist[i]=new TH1F(losthist_name.c_str(),losthist_name.c_str(),4096,0,4096);
  }


  TH1F *vechist[31];//64個のMPPCのコインシデンスを取った波高データのヒストグラム
  for(Int_t i=0;i<31;i++)
  {
    std::ostringstream _aname;
    _aname<<"vec"<<i;
    std::string ahist_name = _aname.str();
    vechist[i]=new TH1F(ahist_name.c_str(),ahist_name.c_str(),4096,0,4096);
  }

  TH2F *coinchhist = new TH2F("coinchhist","",64,0-0.5,64-0.5,3000,500,3500);
  TGraphErrors *gr2=new TGraphErrors;//それぞれのMPPC位置番号をX、エントリー数をY軸に取ったもの

  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  stringstream ss1;

  const char* path="./";//文字列型
  DIR *dp;//ファイル一覧を取得
  dirent* entry;//direntとは実行が成功したら0,失敗したら-1を返す
  string filename[4096];
  int filenum=0;
  dp = opendir(path);//path以下のやつを全部開く
  ofstream dev("./2_txt/deviation.txt");
  
  int d=0; int dd=0; int ddd=0; //桁ごとのカウント数
  size_t len=0; // .dat0ファイルの文字列の長さを格納
  while((entry=readdir(dp))!=NULL)
  {
    string tmp;
    tmp=entry -> d_name;

//    cout<<tmp<<endl;

    if(StrString(tmp.c_str(),".dat"))
    { 
      if(d==0){len=strlen(tmp.c_str());} //.dat0 ファイルの文字列の長さを取得
//      cout<<"len="<<len<<endl;
//      cout<<"strlen="<<strlen(tmp.c_str())<<endl;
      if     (strlen(tmp.c_str())==len)  {filename[d]      =tmp;   d++;}
      else if(strlen(tmp.c_str())==len+1){filename[dd+10]  =tmp;  dd++;}
      else if(strlen(tmp.c_str())==len+2){filename[ddd+100]=tmp; ddd++;}
      else                      {cout<<"loaded wrong file or more than 999 files"<<endl; return;}
//      filename[filenum]=tmp;

//      cout<<tmp<<" "<<filenum<<" "<<filename[filenum]<<endl;
//      filenum++;
    }
  }
  
  filenum = d + dd + ddd;
  if(filenum==0){cout<<" No file loaded."<<endl<<endl; return;}
  else          {cout<<filenum<<" files loaded."<<endl<<endl;}

  for(int i=0;i<filenum; i++){cout<<i<<" "<<filename[i]<<endl;}

  struct stat st;
  double sfile[filenum];

//  ofstream f("39ch_entries.txt");

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
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////    mu-PSD1    /////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      hoge.read((char*)&val,sizeof(int));//一個目のヘッダーを読み込む
      //   cout<<val<<endl;
      // int buffermem[65];
      read_count++;
      if(val==0xffffea0c)//この形式が0xffff0xea0cの場合
      {
        hoge.read((char*)&val,sizeof(int));//次の行を読み込む(Header2 : reserved Number of word(16bit))
        hoge.read((char*)&val,sizeof(int));//Header3 : Eventcounter(12bit) : easiroc識別番号(1 or 2 下位16bit)//65回読み込む意味はヘッダー一行読んだ後MPPC64個
        eas_s1=(val & 0xffff);//(下位16bit演算)
        e_count1=(val >> 16);//(16bit右シフト)
        trigger++;
        for(int i=0;i<64;++i)
        {
          if(eas_s1==1)
          {
            hoge.read((char*)&val,sizeof(int));
            int buffer=val & 0xffff;//語尾についているのは何?=後ろの2byteを読み込む
            // cout<<buffer<<endl;
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


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////    mu-PSD2    /////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

      if(e_count1!=e_count2)
      {
        cout<<"event data is not match"<<endl;
        cout<<"mu-PSD1 evene num: "<<e_count1<<"mu-PSD2 event num: "<<e_count2<<endl;
      }

      Int_t maxADCch1=0;//1シート目に一番波高の高いMPPCのADC-pedestal値
      Int_t maxADCch1_2nd=0;//1シート目に2番波高の高いMPPCのADC-pedestal値
      Int_t maxch1=0;	//1シート目の一番波高の高い位置番号
      Int_t maxch1_2nd=0;//1シート目の2番目に波高の高い位置番号
      bool flag1=false;

      for(Int_t j=0;j<16;j++)
      {
        if(buffermem1[j]-ped[j]>maxADCch1)
        {
          maxADCch1_2nd=maxADCch1;
          maxch1_2nd=maxch1;
          maxADCch1=buffermem1[j]-ped[j];
          maxch1=j;
        }//1シートのなかで一番波高の高いMPPCのADC-Pedestalの値を探す
      }
      maxADCch1+=ped[maxch1];//引いていたペデスタル値を直してADC値に

      if(maxADCch1>thresh_ch[maxch1])
      {
        if(maxADCch1_2nd>thresh_ch[maxch1_2nd] && abs(maxch1 - maxch1_2nd) > 1)
        {
          flag1=false;
        }
        else
        {
          flag1=true;
        }
      }

      Int_t maxADCch2=0;//2シート目の一番波高の高いMPPCのADC-pedestal値
      Int_t maxADCch2_2nd=0;//2シート目の2番目に波高の高いMPPCのADC-pedestal値
      Int_t maxch2=0;//2シート目の一番波高の高い位置番号
      Int_t maxch2_2nd=0;//2シート目の2番目に波高の高い位置番号
      bool flag2=false;//
      for(Int_t j=16;j<32;j++)
      {
        if(buffermem1[j]-ped[j]>maxADCch2)
        {
          maxADCch2_2nd=maxADCch2;
          maxch2_2nd=maxch2;
          maxADCch2=buffermem1[j]-ped[j];
          maxch2=j;
        }
      }

      maxADCch2+=ped[maxch2];

      if(maxADCch2>thresh_ch[maxch2])
      {
        //			flag2=true;
        if(maxADCch2_2nd>thresh_ch[maxch2_2nd] && abs(maxch2-maxch2_2nd)>1)
        {
          // cout<<maxADCch2<<endl;
          flag2=false;
        }
        else
        {
          flag2=true;
        }
      }



      Int_t maxADCch3=0;
      Int_t maxADCch3_2nd=0;
      Int_t maxch3=0;
      Int_t maxch3_2nd=0;
      bool flag3=false;
      for(Int_t j=32;j<48;j++)
      {
        if(buffermem2[j-32]-ped[j]>maxADCch3)
        {
          maxADCch3_2nd=maxADCch3;
          maxch3_2nd=maxch3;
          maxADCch3=buffermem2[j-32]-ped[j];
          maxch3=j;
        }
      }

      maxADCch3+=ped[maxch3];
      if(maxADCch3>thresh_ch[maxch3])
      {
        //			flag3=true;
        //     cout<<maxADCch3<<endl;
        if(maxADCch3_2nd>thresh_ch[maxch3_2nd] && abs(maxch3 - maxch3_2nd)>1)
        {
          flag3=false;
        }
        else
        {
          flag3=true;
        }
      }
      Int_t maxADCch4=0;
      Int_t maxADCch4_2nd=0;
      Int_t maxch4=0;
      Int_t maxch4_2nd=0;
      bool flag4=false;


      for(Int_t j=48;j<64;j++)
      {
        if(buffermem2[j-32]-ped[j]>maxADCch4)
        {
          maxADCch4_2nd=maxADCch4;
          maxch4_2nd=maxch4;
          maxADCch4=buffermem2[j-32]-ped[j];
          maxch4=j;
        }
      }

      maxADCch4+=ped[maxch4];
      if(maxADCch4>thresh_ch[maxch4])
      {
        //			flag4=true;
        // cout<<maxADCch4<<endl;
        if(maxADCch4_2nd>thresh_ch[maxch4_2nd] && abs(maxch4 - maxch4_2nd)>1)
        {
          flag4=false;
        }
        else
        {
          flag4=true;
        }
      }



      if(flag1 && flag2 && flag3 && flag4)//全部でフラグが立つ、つまり全部の層を通過したと判断した場合にプロット
      {
        x_vec = (maxch2-16) - (maxch4-48) +15;
        y_vec = (maxch3-32) -  maxch1     +15;
        coinhist[maxch1]->Fill(maxADCch1);
        coinhist[maxch2]->Fill(maxADCch2);
        coinhist[maxch3]->Fill(maxADCch3);
        coinhist[maxch4]->Fill(maxADCch4);
        count_ij[x_vec][y_vec]++;
        iden_eve++;
        if(x_vec==15 && y_vec==15)
        {
          counter++;
          data_00<<maxch1<<"  "<<maxch2<<"  "<<maxch3<<"  "<<maxch4<<endl;
        }
        all_data_ch<<maxch1<<"  "<<maxch2<<"  "<<maxch3<<"  "<<maxch4<<endl;
        if(x_vec==15)
        {
          vechist[y_vec]->Fill(maxADCch1);
          vechist[y_vec]->Fill(maxADCch2);
          vechist[y_vec]->Fill(maxADCch3);
          vechist[y_vec]->Fill(maxADCch4);
        }

        coinchhist -> Fill(maxch1,maxADCch1);
        coinchhist -> Fill(maxch2,maxADCch2);
        coinchhist -> Fill(maxch3,maxADCch3);
        coinchhist -> Fill(maxch4,maxADCch4);

        entry_eff[maxch1]+=1./(3600.*130.*place_eff[x_vec][y_vec]*place_acc[x_vec][y_vec]);
        entry_eff[maxch2]+=1./(3600.*130.*place_eff[x_vec][y_vec]*place_acc[x_vec][y_vec]);
        entry_eff[maxch3]+=1./(3600.*130.*place_eff[x_vec][y_vec]*place_acc[x_vec][y_vec]);
        entry_eff[maxch4]+=1./(3600.*130.*place_eff[x_vec][y_vec]*place_acc[x_vec][y_vec]);
        /*
           entry_eff[maxch1]+=1./place_acc[x_vec][y_vec];
           entry_eff[maxch2]+=1./place_acc[x_vec][y_vec];
           entry_eff[maxch3]+=1./place_acc[x_vec][y_vec];
           entry_eff[maxch4]+=1./place_acc[x_vec][y_vec];
         */
      }
    }

  cout<<ss1.str()<<"   "<<counter<<endl;
  dev<<ctime(&date)<<"  "<<iden_eve<<"  "<<trigger<<"  "<<counter<<endl;
  trigger=0;
  iden_eve=0;

  gr2->SetPoint(iiii,iiii,counter);
  gr2->SetPointError(iiii,0,sqrt((double)counter));

//  f << ss1.str().c_str() << "," << coinhist[39]->GetEntries() << endl;
}
  /////////////////////////////////////////////////////////////////////CREATE HISTGRAM

  ////////////////////////////////////////////////////////////////////COIN HIST
  TCanvas *cvs1=new TCanvas("cvs1","My Canvas");
  cvs1->cd();
  cvs1->SetLogy();
  Double_t Entries[n];
  Double_t EntryErrors[n];
  Double_t EntryErrors2[n];

  for(Int_t i=0;i<n;i++)
  {
    Entries[i]=0;
    EntryErrors[i]=0;
    EntryErrors2[i]=0;
  }

  for(Int_t i=0;i<n;i++)
  {
    hist[i]->Draw("");
    if(i==0)
    {
      cvs1->Print("./3_pdf/hist.pdf(");
    }
    else if(i!=63)
    {
      cvs1->Print("./3_pdf/hist.pdf");
    }
    else
    {
      cvs1->Print("./3_pdf/hist.pdf)");
    }
    Entries[i]=coinhist[i]->GetEntries();
    EntryErrors[i]=sqrt(Entries[i]);
    EntryErrors2[i]=sqrt(entry_eff[i]);
    cout<<Entries[i]<<endl;
  }

////////////////////////////////////////////////////////////////////////LOST HIST
  for(int i=0; i<64; i++)
  {
    losthist[i]->Add(hist[i],coinhist[i],1,-1);
  }
  TCanvas *cvs100=new TCanvas("cvs100","My Canvas");
  cvs100->cd();
  cvs100->SetLogy();
  for(Int_t i=0;i<n;i++ )
  {
    losthist[i]->Draw("");
    if(i==0)
    {
      cvs100->Print("./3_pdf/lost.pdf(");
    }
    else if(i!=63)
    {
      cvs100->Print("./3_pdf/lost.pdf");
    }
    else
    {
      cvs100->Print("./3_pdf/lost.pdf)");
    }
  }

  ///////////////////////////////////////////////////////////////////////////VECTOR HIST
  TCanvas *cvs300=new TCanvas("cvs300","My Canvas");
  cvs300->cd();
  //    cvs300->SetLogy();
  for(Int_t i=0;i<31;i++ )
  {

    vechist[i]->Draw("");

    if(i==0)
    {
      cvs300->Print("./3_pdf/y_vec.pdf(");
    }
    else if(i!=30)
    {
      cvs300->Print("./3_pdf/y_vec.pdf");
    }
    else
    {
      cvs300->Print("./3_pdf/y_vec.pdf)");
    }
  }

  ////////////////////////////////////////////////////////////////////////////FIT COIN HIST
  TCanvas *cvs2=new TCanvas("cvs2","My Canvas");
  cvs2->cd();
  //  cvs2->SetLogy();
  Double_t means[n],CONSTANT[n],MEANS[n],SIGMA[n];
  Double_t MEANS_error[n];
  std::ofstream coin_para("./2_txt/fitting_parameter_coin.txt");
  coin_para<<"means"<<" "<<"CONSTANT"<<" "<<"MEANS"<<" "<<"SIGMA"<<" "<<"MEANS_error"<<endl;
  for(Int_t i=0;i<n;i++ )
  {
    TF1 *f1=new TF1("f1","gaus",1000,3100);
    coinhist[i]->Draw("");
    coinhist[i]->Fit("f1","","",1000,3100);
    //  coinhist[i]->GetYaxis()->SetRangeUser(0,10);
    f1->Draw("same");
    if(i==0)
    {
      cvs2->Print("./3_pdf/fit_coinhist.pdf(");
    }
    else if(i!=63)
    {
      cvs2->Print("./3_pdf/fit_coinhist.pdf");
    }
    else
    {
      cvs2->Print("./3_pdf/fit_coinhist.pdf)");
    }
    means[i]=coinhist[i]->GetMean();
    CONSTANT[i]=f1->GetParameter(0);
    MEANS[i]=f1->GetParameter(1);
    MEANS_error[i]=f1->GetParError(1);
    SIGMA[i]=f1->GetParameter(2);
    coin_para<<means[i]<<" "<<CONSTANT[i]<<" "<<MEANS[i]<<" "<<SIGMA[i]<<" "<<MEANS_error[i]<<endl;
  }

  Int_t index=64;
  Double_t x[n];
  Double_t ex[n];
  for(Int_t i=0;i<n;i++)
  {
    x[i]=i;
    ex[i]=0;
  }

  /////////////////////////////////////////////////////////////////////////EACH MPPC ENTRY
  TCanvas *cvs13=new TCanvas("cvs13","Entry number");
  TGraph *gr10=new TGraphErrors(index,x,Entries,ex,EntryErrors);//それぞれのMPPC位置番号をX、エントリー数をY軸に取ったもの
  gr10->SetMarkerColor(kRed);
  gr10->Draw("APL");
  gr10->SetTitle("Each_Channel_entries(coincidence)");
  gr10->GetYaxis()->SetTitle("");
  gr10->GetXaxis()->SetTitle("MPPC_Channel_Number");
  gr10->SetMarkerSize(1);
  gr10->SetMarkerStyle(20);
  gr10->GetYaxis()->CenterTitle();
  gr10->GetXaxis()->CenterTitle();
  cvs13->Print("./4_gif/Each_MPPC_Entries.gif");

  //////////////////////////////////////////////////////////////////////////COIN 00
  TCanvas *cvs30=new TCanvas("cvs30","Entry 00");
  gr2->Draw("APL");
  gr2->SetTitle("entries(coincidence)");
  gr2->GetYaxis()->SetTitle("");
  gr2->GetXaxis()->SetTitle("number");
  gr2->SetMarkerSize(1);
  gr2->SetMarkerStyle(20);
  gr2->GetYaxis()->CenterTitle();
  gr2->GetXaxis()->CenterTitle();
  cvs30->Print("./4_gif/coin00.gif");//pdf->gifに変更20190718

  //////////////////////////////////////////////////////////////////////////2Dhist
  TCanvas *cvs4 = new TCanvas("cvs4","My Canvas");
  TH2D *image_ij = new TH2D("image_ij","",31,-15.5,15.5,31,-15.5,15.5);

  for(Int_t i=0; i<31;i++)
  {
    for(Int_t j=0;j<31;j++)
    {
      image_ij ->Fill(i-15,j-15,(double)count_ij[i][j]);
      all_data<<i<<"   "<<j<<"   "<<count_ij[i][j]<<endl;
    }
  }
  image_ij->SetStats(0);
  image_ij->Draw("colz");
  cvs4->Print("./4_gif/Intensity_2D.gif");

  std::ofstream output("./2_txt/output.txt");
  for(Int_t i=0;i<31;i++)
  {
    for(Int_t j=0;j<31;j++)
    {
      output<<i-15<<" "<<j-15<<" "<<count_ij[i][j]<<endl; //add endl 190628
    }
  }

  TCanvas *cvs8 = new TCanvas("cvs8","My Canvas");
  TH2D *image_ij_err = new TH2D("image_ij_err","",31,-15.5,15.5,31,-15.5,15.5);
  for(Int_t i=0; i<31;i++)
  {
    for(Int_t j=0;j<31;j++)
    {
      image_ij_err ->Fill(i-15,j-15,(double)sqrt(count_ij[i][j])/(double)count_ij[i][j]);
    }
  }
  image_ij_err->SetStats(0);
  image_ij_err->Draw("colz");
  cvs8->Print("./4_gif/Intensity_2D_error.gif");

  fout->Write();
}
////////////////////////////////////////////////////////////////////////////////関数の定義
time_t  get_caltime(char *tstr, char *tstr2)
{
  char    buf[20];
  char    buf2[20];
  time_t  cal_time;

  struct tm
    work_tm;

  /*        if(strlen(tstr) != 14)
            return(-1);*/
  strncpy(buf, tstr, 4);
  buf[4] = '\0';
  work_tm.tm_year = atoi(buf) - 1900;//年
  strncpy(buf, tstr+5, 2);
  buf[2] = '\0';
  work_tm.tm_mon = atoi(buf) - 1;//月
  strncpy(buf, tstr+5+3, 2);
  work_tm.tm_mday = atoi(buf);//日

  if(strlen(tstr2) == 4)
  {
    strncpy(buf2, tstr2, 1);
    work_tm.tm_hour = atoi(buf2);
    strncpy(buf2, tstr2+2, 2);
    work_tm.tm_min = atoi(buf2);
  }
  else if(strlen(tstr2) == 5)
  {
    strncpy(buf2, tstr2, 2);
    work_tm.tm_hour = atoi(buf2);
    strncpy(buf2, tstr2+3, 2);
    work_tm.tm_min = atoi(buf2);
  }

  work_tm.tm_sec = 00;
  work_tm.tm_isdst = -1;
  if((cal_time = mktime(&work_tm)) == -1){
    return(-1);
  }
  return(cal_time);
}

int compare_meas_time(const void *a, const void *b) {
  meas_t *A = ( meas_t *)a;
  meas_t *B = ( meas_t *)b;
  return A->m_time - B->m_time;
}

double calib1(double ch, double gain, double pede)
{
  double a=(ch-pede)/gain;
  return a;
}



bool StrString(const char*s1,const char *s2)//サブルーチンみたいなもの
{
  int n;
  n=strlen(s2);

  while(1){
    s1=strchr(s1,s2[0]);

    if(s1==NULL)

      return false;

    if(strncmp(s1,s2,n)==0)
      return true;
    s1++;
  }
}
