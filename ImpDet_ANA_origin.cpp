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

typedef struct
{
  time_t n_time;
  double n_pressure;
  double n_temperature;
  double n_solargeo;
}pres_t;

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
double b_pressure=1013.;//基準とする気圧[hpa]
double b_temp=10.;//基準となる気温[℃]
//double factor_pre=0.124; //補正の値[%/hPa]
double factor_pre=18.68; //補正の値[cph/hPa]

double factor_temp=0.147;//[%/℃]
double factor_sol=10.93;//[%/GV]

void ImpDet_ANA(){
  cout<<"Please Select number"<<endl;
  cout<<"1.See coincidence event"<<endl;
  cout<<"2.correct some effect"<<endl;

  Int_t value;
  cin>>value;
  cout<<value<<endl;

  std::ofstream all_data("all_data.txt");
  std::ofstream data_00("00_data.txt");
  std::ofstream all_data_ch("all_data_ch.txt");


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

  std::ifstream acc_inf("acceptance.txt");
  for(int i=0; i<31; i++)
  {
     for(int j=0; j<31; j++)
      {
          int x=0;
          int y=0;
          acc_inf >> x >> y >> place_acc[x+15][y+15];
      }
  }
  acc_inf.close();

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

    std::ifstream config("config.txt");//config.txtから読み込み
    for(Int_t i=0;i<n;i++)
      {
	config >>esrch[i]>>place[i];//config.txtから読み込む
      }
    config.close();


    Double_t ped[n],pe1[n],pe2[n],pe3[n],thresh_ch[n],gain[n];
    for(Int_t i=0;i<n;i++)
      {
	ped[i]=0;
	thresh_ch[i]=0;
      }
     std::ifstream four_peak("four_peak.txt");//ペデスタル、3つのピーク、threshold。解析ファイルで作ったところ
//     std::ifstream thresh("threshould.txt");
      for(Int_t i=0;i<n;i++)
      {
//	    thresh >> thresh_ch[i];
//	four_peak>>ped[i]>>pe1[i]>>pe2[i]>>pe3[i]>>dummy;//読み込む
	four_peak>>ped[i]>>pe1[i]>>pe2[i]>>gain[i];//読み込む
//	    thresh_ch[i]=ped[i]+((pe2[i]-ped[i])/2.)*3.5;
	    thresh_ch[i]=ped[i]+gain[i]*2.5;
//	    thresh_ch[i]=ped[i];
/*if(i==40){
	    thresh_ch[i]=ped[i]+gain[i]*1.5;
	      }
*/
/*	if(!(ped[i]<pe1[i] && pe1[i]<pe2[i] && pe2[i]<pe3[i]))
	  {
//	    thresh_ch[i]=1000;
	    thresh_ch[i]=950;
	  }//ピークサーチでうまくいかなかった場合ここで個別に指定する
*/	//cout<<thresh_ch[i]<<endl;
      }



      // Double_t ped[64];
/*    std::ifstream pedestal("pedestal_position.txt");
    for(Int_t i=0;i<n;i++)
      {
	//	pedestal>>dummy>>ped[i];
//	thresh_ch[i]=1000;
      }
    //    thresh_ch[5]=1400;
*/



if(value==1)
{
////////////////////////////////////////////////////////////////
///////////////////////   graph   //////////////////////
///////////////////////////////////////////////////////////////

   TH1F *coinhist[n];//64個のMPPCのコインシデンスを取った波高データのヒストグラム
   TH1F *hist[n];//64個のMPPCのコインシデンスを取った波高データのヒストグラム
   TH1F *losthist[n];//64個のMPPCのコインシデンスを取った波高データのヒストグラム
   TH1F *phist[n];//64個のMPPCのコインシデンスを取った波高データのヒストグラム


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

	std::ostringstream _pname;
	_pname<<"chp"<<i;
	std::string phist_name = _pname.str();
	phist[i]=new TH1F(phist_name.c_str(),phist_name.c_str(),4096,calib1(0,gain[i],ped[i]),calib1(4095,gain[i],ped[i]));

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
	vechist[i]=new TH1F(ahist_name.c_str(),ahist_name.c_str(),4096,calib1(0,90,780),calib1(4095,90,780));
	 }


TH2D *test2d1 = new TH2D("test2d1","",16,-0.5,15.5,16,15.5,31.5);
TH2D *test2d2 = new TH2D("test2d2","",16,31.5,47.5,16,47.5,63.5);
TH2D *the0_2d1 = new TH2D("the0_2d1","",16,-0.5,15.5,16,15.5,31.5);
TH2D *the0_2d2 = new TH2D("the0_2d2","",16,31.5,47.5,16,47.5,63.5);
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
ofstream dev("deviation.txt");

    while((entry=readdir(dp))!=NULL)
      {
	string tmp;
	tmp=entry -> d_name;//読み込んだものの名前を記憶(おそらく拡張子以外のところ)
	cout<<tmp<<endl;
	if(StrString(tmp.c_str(),".dat"))
	  {
	    filename[filenum]=tmp;

	    cout<<tmp<<" "<<filenum<<" "<<filename[filenum]<<endl;
	    filenum++;
	  }
      }


//	 int vec_count[filenum];


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
//		char bu;
	//	char bu2=ss1.str().c_str();
// cout<<"2:"<<ss1.str()<<"loading"<<endl;
	    cout<<iiii<<": "<<ss1.str()<<" loading "<<endl;
//	    strncpy(bu,bu2+18,3);
	//	cout<<bu<<endl;



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
			phist[place[i]]->Fill(calib1(buffer,gain[place[i]],ped[place[i]]));
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
	     //   cout<<val<<endl;
	     // int buffermem[65];
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
			 // cout<<buffer<<endl;
if(i<32)
{
			buffermem2[place[i]]=buffer;
			hist[place[i]+32]->Fill(buffer);
			phist[place[i]+32]->Fill(calib1(buffer,gain[place[i]+32],ped[place[i]+32]));
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if(e_count1!=e_count2)
		{
		cout<<"event data is not match"<<endl;
		cout<<"mu-PSD1 evene num: "<<e_count1<<"mu-PSD2 event num: "<<e_count2<<endl;
		}





/*
	Int_t buffermem[65];//他のループ内でも使いたいから
	while(!hoge.eof())
	  {
	    UInt_t val;//datファイル内の情報を格納
	    hoge.read((char*)&val,sizeof(int));//一個目のヘッダーを読み込む
	    //   cout<<val<<endl;
	    // int buffermem[65];
	    if(val==0xffffea0c)//この形式が0xffff0xea0cの場合
	      {
		hoge.read((char*)&val,sizeof(int));//次の行を読み込む
		for(int i=0;i<65;++i)
		  {
		    hoge.read((char*)&val,sizeof(int));//65回読み込む意味はヘッダー一行読んだ後MPPC64個分のデータを読み込む
		    if(i>0)//
		      {
			int buffer=val & 0xffff;//語尾についているのは何?=後ろの2byteを読み込む
			// cout<<buffer<<endl;
			buffermem[place[i-1]]=buffer;
			//			hist[place[i-1]]->Fill(buffer);
		      }
		  }
*/
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

		//		    cout<<maxADCch1<<endl;
		if(maxADCch1>thresh_ch[maxch1])
		  {
	//		flag1=true;
		    // cout<<maxch1<<" "<<maxch1_2nd<<endl;

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
		    x_vec = (63-maxch4) - (31-maxch2) + 15;
		    y_vec = (maxch3-32) - maxch1 + 15;
		    coinhist[maxch1]->Fill(maxADCch1);
		    coinhist[maxch2]->Fill(maxADCch2);
		    coinhist[maxch3]->Fill(maxADCch3);
		    coinhist[maxch4]->Fill(maxADCch4);
		    count_ij[x_vec][y_vec]++;
			iden_eve++;
if(x_vec==15 && y_vec==15)
{
//			vec_count[(unsigned char)bu]++;
			counter++;
data_00<<maxch1<<"  "<<maxch2<<"  "<<maxch3<<"  "<<maxch4<<endl;
}
all_data_ch<<maxch1<<"  "<<maxch2<<"  "<<maxch3<<"  "<<maxch4<<endl;
if(x_vec==15)
{
/*vechist[y_vec]->Fill(maxADCch1);
vechist[y_vec]->Fill(maxADCch2);
vechist[y_vec]->Fill(maxADCch3);
vechist[y_vec]->Fill(maxADCch4);
*/

vechist[y_vec]->Fill(calib1(maxADCch1,gain[maxch1],ped[maxch1]));
vechist[y_vec]->Fill(calib1(maxADCch2,gain[maxch2],ped[maxch2]));
vechist[y_vec]->Fill(calib1(maxADCch3,gain[maxch3],ped[maxch3]));
vechist[y_vec]->Fill(calib1(maxADCch4,gain[maxch4],ped[maxch4]));


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
		  test2d1 -> Fill(maxch1,maxch2);
         test2d2 -> Fill(maxch3,maxch4);

		if(abs(maxch2-(maxch4-32))==0 && abs(maxch1-(maxch3-32))==0) //<= x で、どこまで離れていてもOKか判別
          {
//            the0_2d1 -> Fill(47-maxch2,15-maxch1);
//            the0_2d2 -> Fill(111-maxch4,79-maxch3);
         the0_2d1 -> Fill(maxch1,maxch2);
         the0_2d2 -> Fill(maxch3,maxch4);

          }

		    /*		     if(maxch1==4 ||maxch1==6)
		     {
				cout<<maxADCch1<<" "<<buffermem[7]<<" "<<buffermem[5]<<endl;
				}*/


		    /* if(maxch1==5)

		      {

			//	cout<<maxch2<<endl;
			a<<maxADCch1<<" "<<maxch3<<" "<<maxch4<<" "<<maxch2<<endl;
			}*/

		  }




	  }///////while()
      cout<<ss1.str()<<"   "<<counter<<endl;
  dev<<ctime(&date)<<"  "<<iden_eve<<"  "<<trigger<<"  "<<counter<<endl;
trigger=0;
iden_eve=0;
		gr2->SetPoint(iiii,iiii,counter);
		gr2->SetPointError(iiii,0,sqrt((double)counter));
//		gr2->SetPoint((unsigned char)bu,(unsigend char)bu,counter[(unsigned char)bu]);
	//	gr2->SetPointError((unsigned char)bu,0,sqrt((double)counter[(unsigned char)bu]));

      }//fileopen


    TCanvas *cvs1=new TCanvas("cvs1","My Canvas");
      cvs1->cd();
      Double_t Entries[n];//それぞれのエントリー数
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
	  coinhist[i]->Draw("");
	  if(i==0)
	  {
	    cvs1->Print("coincident_hist64.pdf(");
	  }
	  else if(i!=63)
	    {
	      cvs1->Print("coincident_hist64.pdf");
	  }
	  else
	    {
	      cvs1->Print("coincident_hist64.pdf)");

	    }//64個のヒストを1個のPDFに表示する方法
	  Entries[i]=coinhist[i]->GetEntries();
	  EntryErrors[i]=sqrt(Entries[i]);
	  EntryErrors2[i]=sqrt(entry_eff[i]);
	  cout<<Entries[i]<<endl;
	}
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
	      cvs100->Print("lost.pdf(");
	    }
	  else if(i!=63)
	    {
	      cvs100->Print("lost.pdf");
	    }
	  else
	    {
	      cvs100->Print("lost.pdf)");
	    }
	}

      TCanvas *cvs200=new TCanvas("cvs200","My Canvas");
      cvs200->cd();
      cvs200->SetLogy();
      for(Int_t i=0;i<n;i++ )
	{
	  phist[i]->Draw("");
	  if(i==0)
	    {
	      cvs200->Print("photon.pdf(");
	    }
	  else if(i!=63)
	    {
	      cvs200->Print("photon.pdf");
	    }
	  else
	    {
	      cvs200->Print("photon.pdf)");
	    }
	}
//////////
/*
int re=1;
 Double_t peak_posi2[31];
ofstream vecp("vector_peak.txt");
 for(Int_t ci=0;ci<31;ci++)//1つ1つのMPPCのピークを見つける
   {
     TSpectrum *s2 =new TSpectrum(1);//4つピークを見つける
     Double_t *xpeaks2=s2->GetPositionX();//座標取得
     // Search<<i<<"\t"<<*xpeaks<<endl;
     Int_t the_number_of_peak2,bin2;
     Double_t source_hist2[4096],dest_hist2[4096];
     Float_t fPositionX2[100],fPositionY2[100];
     Float_t pede_sear2[n];
     vechist[ci]->Draw("");
	  vechist[ci]->Rebin(re);

     for(Int_t i=0;i<4096;i++)
       {
	 source_hist2[i]=vechist[ci]->GetBinContent(i+1);//それぞれのヒストグラムですべてのビン情報を取得
       }

     the_number_of_peak2=s2->SearchHighRes(source_hist2,dest_hist2,4096,50,1,kTRUE,3,kTRUE,3);//元のヒストのビン情報、スムージングした後のビン情報、見たいとこまでのビンの数、シグマ、閾値、スムージングのパラメータ

     for(int i=0;i<the_number_of_peak2;i++)
       {
	 peak_posi2[ci]=xpeaks2[i];
	 bin2=1+Int_t(peak_posi2[ci]+0.5);

	 fPositionX2[i] = vechist[ci]->GetBinCenter(bin2);
	 fPositionY2[i] = vechist[ci]->GetBinContent(bin2);//ピーク位置の高さを取得

       }

//     pede_sear2[ci]=xpeaks2[0];
     pede_sear2[ci]=fPositionX2[0];

//     pe_3_sear2[ci]=xpeaks2[3];
//     pe_edge[ci]=pede_sear2[ci]+(pe_3_sear2[ci]-pe_1_sear2[ci])/2.*(rough_pe);

     vecp<<ci<<"   "<<pede_sear2[ci]<<endl;//ファイルに書き出す

     TPolyMarker * pm2 = (TPolyMarker*)vechist[ci]->GetListOfFunctions()->FindObject("TPolyMarker");//マーカーをつけるやーつ

     if (pm2) {

       vechist[ci]->GetListOfFunctions()->Remove(pm2);

       delete pm2;
     }

     pm2 = new TPolyMarker(the_number_of_peak2, fPositionX2, fPositionY2);
     vechist[ci]->GetListOfFunctions()->Add(pm2);
     pm2->SetMarkerStyle(23);
     pm2->SetMarkerColor(kRed);
     pm2->SetMarkerSize(1.3);
   }
*/


//////////
      TCanvas *cvs300=new TCanvas("cvs300","My Canvas");
      cvs300->cd();
  //    cvs300->SetLogy();
      for(Int_t i=0;i<31;i++ )
	{

	  vechist[i]->Draw("");

	  if(i==0)
	    {
	      cvs300->Print("y_vec.pdf(");
	    }
	  else if(i!=30)
	    {
	      cvs300->Print("y_vec.pdf");
	    }
	  else
	    {
	      cvs300->Print("y_vec.pdf)");
	    }
	}

      TCanvas *cvs2=new TCanvas("cvs2","My Canvas");
      cvs2->cd();
      //  cvs2->SetLogy();
      Double_t CONSTANT[n],MEANS[n],SIGMA[n];
      Double_t MEANS_error[n];
      std::ofstream coin_para("fitting_parameter_coin.txt");
      for(Int_t i=0;i<n;i++ )
	{
	  TF1 *f1=new TF1("f1","gaus",1000,3100);
	  coinhist[i]->Draw("");
	  coinhist[i]->Fit("f1","","",1000,3100);
	//  coinhist[i]->GetYaxis()->SetRangeUser(0,10);
	  f1->Draw("same");
	  if(i==0)
	    {
	      cvs2->Print("fit_coinhist.pdf(");
	    }
	  else if(i!=63)
	    {
	      cvs2->Print("fit_coinhist.pdf");
	    }
	  else
	    {
	      cvs2->Print("fit_coinhist.pdf)");
	    }
	  CONSTANT[i]=f1->GetParameter(0);
	  MEANS[i]=f1->GetParameter(1);
	  MEANS_error[i]=f1->GetParError(1);
	  SIGMA[i]=f1->GetParameter(2);
	  coin_para<<CONSTANT[i]<<" "<<MEANS[i]<<" "<<SIGMA[i]<<" "<<MEANS_error[i]<<endl;
	}

      Int_t index=64;
      Double_t x[n];
      Double_t ex[n];
      for(Int_t i=0;i<n;i++)
	{
	  x[i]=i;
	  ex[i]=0;
	}

      TCanvas *cvs3=new TCanvas("cvs3","Entry number");//here
      TGraph *gr1=new TGraphErrors(index,x,entry_eff,ex,EntryErrors2);//それぞれのMPPC位置番号をX、エントリー数をY軸に取ったもの
      gr1->Draw("APL");
      gr1->SetTitle("Each_Channel_entries(coincidence)");
      gr1->GetYaxis()->SetTitle("");
      gr1->GetXaxis()->SetTitle("MPPC_Channel_Number");
      gr1->SetMarkerSize(1);
      gr1->SetMarkerStyle(20);
      gr1->GetYaxis()->CenterTitle();
      gr1->GetXaxis()->CenterTitle();
      cvs3->Print("Each_MPPC_Entries.pdf");

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
      cvs13->Print("Each_MPPC_Entries_2.gif");//pdf->gifに変更20190718

      TCanvas *cvs30=new TCanvas("cvs30","Entry 00");
      gr2->Draw("APL");
      gr2->SetTitle("entries(coincidence)");
      gr2->GetYaxis()->SetTitle("");
      gr2->GetXaxis()->SetTitle("number");
      gr2->SetMarkerSize(1);
      gr2->SetMarkerStyle(20);
      gr2->GetYaxis()->CenterTitle();
      gr2->GetXaxis()->CenterTitle();
      cvs30->Print("coin00.gif");//pdf->gifに変更20190718


////////////////////////////////////////////////////////////////////////////////
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
  cvs4->Print("Intensity_2D.gif");

  std::ofstream output("output.txt");
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
  cvs8->Print("Intensity_2D_error.gif");

/*とりあえずオフにしておく20190718
 TCanvas *cvs5=new TCanvas("cvs5","My Canvas");
 cvs5->cd();
 cvs5->SetLogy();
 Double_t peak_posi[n];
 Double_t pe_edge[n];
 Int_t rough_pe=7;//ノイズの下限値としたいp.eの位置
 std::ofstream four_peak_2;
 four_peak_2.open("four_peak_coin.txt");//1～3p.eを見つける
*/
/*
 for(Int_t ci=0;ci<64;ci++)//1つ1つのMPPCのピークを見つける
   {
     TSpectrum *s1 =new TSpectrum(1);//4つピークを見つける
     Double_t *xpeaks=s1->GetPositionX();//座標取得
     // Search<<i<<"\t"<<*xpeaks<<endl;
     Int_t the_number_of_peak,bin;
     Double_t source_hist[4096],dest_hist[4096];
     Float_t fPositionX[100],fPositionY[100];
     Float_t pede_sear[n];
     Float_t pe_1_sear[n];
     Float_t pe_2_sear[n];
     Float_t pe_3_sear[n];
     // Float_t pe_4_sear[n];
     coinhist[ci]->Draw("");
//     coinhist[ci]->Rebin(4);

     for(Int_t i=0;i<4096;i++)
       {
	 source_hist[i]=coinhist[ci]->GetBinContent(i+1);//それぞれのヒストグラムですべてのビン情報を取得
       }

     the_number_of_peak=s1->SearchHighRes(source_hist,dest_hist,1300,20,0.001,kFALSE,3,kFALSE,5);//元のヒストのビン情報、スムージングした後のビン情報、見たいとこまでのビンの数、シグマ、閾値、スムージングのパラメータ

     for(int i=0;i<the_number_of_peak;i++)
       {
	 peak_posi[ci]=xpeaks[i];
	 bin=1+Int_t(peak_posi[ci]+0.5);

	 fPositionX[i] = coinhist[ci]->GetBinCenter(bin);
	 fPositionY[i] = coinhist[ci]->GetBinContent(bin);//ピーク位置の高さを取得

       }
     pede_sear[ci]=xpeaks[0];
//  coinhist[ci]->Rebin(8);
//     pe_1_sear[ci]=xpeaks[1];
//     pe_2_sear[ci]=xpeaks[2];
//     pe_3_sear[ci]=xpeaks[3];
//     pe_edge[ci]=pede_sear[ci]+(pede_sear[ci]-pe_1_sear[ci])/2.;

//     four_peak<<pede_sear[ci]<<" "<<pe_1_sear[ci]<<" "<<pe_2_sear[ci]<<" "<<pe_3_sear[ci]<<" "<<pe_edge[ci]<<endl;//ファイルに書き出す
//     four_peak_2<<pede_sear[ci]<<" "<<pe_1_sear[ci]<<" "<<pe_2_sear[ci]<<" "<<pe_edge[ci]<<endl;//ファイルに書き出す
//     four_peak_2<<pede_sear[ci]<<" "<<pe_1_sear[ci]<<" "<<pe_2_sear[ci]<<" "<<pe_edge[ci]<<endl;//ファイルに書き出す

  TF1 *f2[4];

    for(Int_t i=0;i<4;i++)
      {
	std::ostringstream _fname;
	_fname<<"f2"<<i;
	std::string fit_name = _fname.str();
	f2[i]=new TF1(fit_name.c_str(),"gaus");
f2[i]->SetParameter(2,30);
      }

//coinhist[ci]->Fit("f20","+","",xpeaks[0]-35,xpeaks[0]+35);
coinhist[ci]->Fit("f20","+","",fPositionX[0]-35,fPositionX[0]+35);
f2[0]->Draw("same");
four_peak_2<< f2[0]->GetParameter(1)<<"  ";

coinhist[ci]->Fit("f21","+","",f2[0]->GetParameter(1)+45,f2[0]->GetParameter(1)+115);

f2[1]->Draw("same");
four_peak_2<< f2[1]->GetParameter(1)<<"  ";

coinhist[ci]->Fit("f22","+","",f2[1]->GetParameter(1)+45,f2[1]->GetParameter(1)+115);
f2[2]->Draw("same");
four_peak_2<< f2[2]->GetParameter(1)<<"  ";

coinhist[ci]->Fit("f23","+","",f2[2]->GetParameter(1)+45,f2[2]->GetParameter(1)+115);
f2[3]->Draw("same");
 four_peak_2<< f2[3]->GetParameter(1)<<endl;

     TPolyMarker * pm = (TPolyMarker*)coinhist[ci]->GetListOfFunctions()->FindObject("TPolyMarker");//マーカーをつけるやーつ

     if (pm) {

       coinhist[ci]->GetListOfFunctions()->Remove(pm);

       delete pm;
     }

     pm = new TPolyMarker(the_number_of_peak, fPositionX, fPositionY);
     coinhist[ci]->GetListOfFunctions()->Add(pm);
     pm->SetMarkerStyle(23);
     pm->SetMarkerColor(kRed);
     pm->SetMarkerSize(1.3);
     if(ci==0)
       {
         cvs5->Print("peak_search_coin.pdf(");
       }
     else if(ci!=64)
       {
         cvs5->Print("peak_search_coin.pdf");
       }

   }
         cvs5->Print("peak_search_coin.pdf)");
*/
/*
TFile *file=new TFile("hist.root","RECREATE");
hist[7]->Write();
coinhist[7]->Write();
*/
}///////////////////////////////////////////////////////////////////////////////value==1 END

////////////////////////////////////////////////////////////////////////////////value==2 START
else if(value==2)
{

  struct stat statBuf;

    if (stat("analysis_data", &statBuf) == 0)
        {
        cout<<"ディレクトリが存在します"<<endl;
 //       return;
        }
    else
    mkdir("analysis_data",0777);



    stringstream ss1;

    const char* path="./";//文字列型
    DIR *dp;//ファイル一覧を取得
    dirent* entry;//direntとは実行が成功したら0,失敗したら-1を返す
    string filename[4096];
    int filenum=0;
    dp = opendir(path);//path以下のやつを全部開く


ifstream pre_file("pressure.txt");//行数を調べたい(配列のため)
ofstream devi("devi.txt");


int n_line=0;
	 while(!pre_file.eof())
   {
     string dum;
     getline(pre_file, dum);
     n_line++;
   }
pre_file.close();

////////////////大気圧データ保存
pres_t *pres;
size_t elm_size=sizeof(pres_t);
pres=(pres_t*) malloc(n_line * elm_size);//配列の動的確保
////////////////



char buff[100];
char buff2[100];
time_t  cal_time;

ifstream pre_file2("pressure.txt");

int pre_n=0;

while(!pre_file2.eof())
{
  pre_file2>>buff>>buff2>>pres[pre_n].n_pressure>>pres[pre_n].n_temperature>>pres[pre_n].n_solargeo;//ここでの太陽磁場はまだ中性子[cpm]
//  pre_file2>>buff>>buff2>>pres[pre_n].n_pressure;//ここでの太陽磁場はまだ中性子[cpm]
//  cout<<buff<<"   "<<buff2<<"  "<<pres[pre_n].n_pressure<<endl;
  cal_time = get_caltime(buff, buff2);
  if(cal_time == -1)
           printf("変換失敗\n");
   else  pres[pre_n].n_time=cal_time;//時間に変換
  pre_n++;
}
pre_file2.close();


    while((entry=readdir(dp))!=NULL)
      {
	string tmp;
	tmp=entry -> d_name;//読み込んだものの名前を記憶(おそらく拡張子以外のところ)
	cout<<tmp<<endl;
	if(StrString(tmp.c_str(),".dat"))
	  {
	    filename[filenum]=tmp;

	    cout<<tmp<<" "<<filenum<<" "<<filename[filenum]<<endl;
	    filenum++;
	  }
      }

///////////////////////測定データ保存
//////////////////////
meas_t *meas;
elm_size=sizeof(meas_t);
meas=(meas_t*) malloc(filenum * elm_size);//配列の動的確保

//////////////////////補正後データ保存
meas_t *meas_corr;
elm_size=sizeof(meas_t);
meas_corr=(meas_t*) malloc(filenum * elm_size);//配列の動的確保
/////////////////////

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
  //		char bu;
  	//	char bu2=ss1.str().c_str();
  // cout<<"2:"<<ss1.str()<<"loading"<<endl;
  	    cout<<iiii<<": "<<ss1.str()<<" loading "<<endl;
  //	    strncpy(bu,bu2+18,3);
  	//	cout<<bu<<endl;
  	  }

  	 int eas_s1=0; //easiroc識別番号
  	 int eas_s2=0; //easiroc識別番号
  	 int e_count1=0; //イベントカウンター
  	 int e_count2=0; //イベントカウンター
  	 int buffermem1[64];
  	 int buffermem2[64];
  	int iden_counter=0;
  	int read_count=0;

    stringstream cfile;
    cfile<<"./analysis_data/correct"<<ss1.str().c_str();//保存するデータのパス
    ofstream ofst(cfile.str().c_str());//分割ファイルのデータを保存
    stat(ss1.str().c_str(), &st);
    time_t date=st.st_mtime;//ファイルの更新時間を取得
    ofst<<ctime(&date)<<date<<endl;
    meas[iiii].m_time=date;

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
  	     //   cout<<val<<endl;
  	     // int buffermem[65];
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
  			 // cout<<buffer<<endl;
  if(i<32)
  {
  			buffermem2[place[i]]=buffer;
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
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

  		//		    cout<<maxADCch1<<endl;
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
  		    x_vec = (63-maxch4) - (31-maxch2) + 15;
  		    y_vec = (maxch3-32) - maxch1 + 15;
  		    count_ij[x_vec][y_vec]++;
          iden_counter++;
  ofst<<maxch1<<"  "<<maxch2<<"  "<<maxch3<<"  "<<maxch4<<endl;
  		  }

  	  }///////while()
      ofst.close();
      meas[iiii].iden_eve=(double)iden_counter;
      cout<<ss1.str()<<"   "<<iden_counter<<endl;
      iden_counter=0.;
    }//fileopen全てのファイル読み込み終わり

////////////////////////////////////////////////////////////////////////////////////
////////////                  補正        //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

///////////時間で比較し、補正する
for(int i=0; i<filenum; i++)
    {
      bool flagt=false;
      time_t diftime=meas[i].m_time-1800;//更新時間が分割ファイルの最後のため1800秒引く
      for(int j=0; j<n_line-1; j++)
        {
          if(difftime(diftime,pres[j].n_time)<0 && flagt==false)
            {
              flagt=true;
              if(abs(difftime(diftime,pres[j].n_time))>abs(difftime(diftime,pres[j-1].n_time)))//時間が近いほうの圧力を使用
                {
//                  meas_corr[i].iden_eve=meas[i].iden_eve*(1+((pres[j-1].n_pressure-b_pressure)*factor_pre/100.));
                  meas_corr[i].iden_eve=meas[i].iden_eve+(pres[j-1].n_pressure-b_pressure)*factor_pre;

//                  meas_corr[i].iden_eve=meas[i].iden_eve*(1+((pres[j-1].n_pressure-b_pressure)*factor_pre/100.))*(1+((pres[j-1].n_temperature-b_temp)*factor_temp/100.));

                  	cout<<meas[i].iden_eve<<"   "<<meas_corr[i].iden_eve<<"  "<<pres[j-1].n_pressure<<endl;
                }
              else if(abs(difftime(diftime,pres[j].n_time))<abs(difftime(diftime,pres[j-1].n_time)))//上に同じ
                {
//                  meas_corr[i].iden_eve=meas[i].iden_eve*(1+((pres[j].n_pressure-b_pressure)*factor_pre/100.));
                  meas_corr[i].iden_eve=meas[i].iden_eve+(pres[j].n_pressure-b_pressure)*factor_pre;

 //                   meas_corr[i].iden_eve=meas[i].iden_eve*(1+((pres[j-1].n_pressure-b_pressure)*factor_pre/100.))*(1+((pres[j-1].n_temperature-b_temp)*factor_temp/100.));
                  	cout<<meas[i].iden_eve<<"   "<<meas_corr[i].iden_eve<<"  "<<pres[j].n_pressure<<endl;
                }
                else
                cout<<"       correct error      "<<endl;
                meas_corr[i].m_time=meas[i].m_time;
                  	devi<<meas[i].m_time<<"  "<<meas[i].iden_eve<<"   "<<meas_corr[i].iden_eve<<"  "<<pres[j].n_pressure<<"  "<<pres[j].n_temperature<<endl;

            }

        }
    }

///////////

/////////////////////////////////////////並び替え
qsort(meas, filenum , sizeof(meas_t) ,compare_meas_time);//並び替えqsort(対象のポインタ,配列要素の個数,配列要素のサイズ,比較関数)

qsort(meas_corr, filenum , sizeof(meas_t) ,compare_meas_time);//並び替えqsort(対象のポインタ,配列要素の個数,配列要素のサイズ,比較関数)
/////////////////////////////////////////

/////////////////////////////////////////グラフ作成
TGraphErrors *gr1=new TGraphErrors;
TGraphErrors *gr2=new TGraphErrors;
TGraphErrors *gr3=new TGraphErrors;

time_t b_time=meas[0].m_time;//b_time=base_time
for(int i=0; i<filenum; i++)
{
  gr1->SetPoint(i,meas[i].m_time-b_time,meas[i].iden_eve);
  //gr1->SetPointError(i,0,sqrt(meas[i].iden_eve));

  gr2->SetPoint(i,meas_corr[i].m_time-b_time,meas_corr[i].iden_eve);
  //gr2->SetPointError(i,0,sqrt(meas_corr[i].iden_eve));
  }
b_time=pres[0].n_time;
for(int i=0; i<n_line-1; i++)
gr3->SetPoint(i,pres[i].n_time-b_time,pres[i].n_pressure);

TCanvas *cvs30=new TCanvas("cvs30","identified event");
gr1->Draw("APL");
gr1->SetTitle("identified_event");
gr1->GetYaxis()->SetTitle("count[-]");
gr1->GetXaxis()->SetTitle("time[s]");
gr1->SetMarkerSize(0.7);
gr1->SetMarkerStyle(20);
gr1->SetMarkerColor(kRed);
gr1->GetYaxis()->CenterTitle();
gr1->GetXaxis()->CenterTitle();

gr2->Draw("PL");
gr2->SetTitle("identified_event");
gr2->GetYaxis()->SetTitle("count[-]");
gr2->GetXaxis()->SetTitle("time[s]");
gr2->SetMarkerSize(0.7);
gr2->SetMarkerStyle(20);
gr2->SetMarkerColor(kBlue);

cvs30->Print("identified_event.pdf");


TCanvas *cvs0=new TCanvas("cvs0","preessure");
gr3->Draw("APL");
gr3->SetTitle("pressure");
gr3->GetYaxis()->SetTitle("pressure[hPa]");
gr3->GetXaxis()->SetTitle("time[s]");
gr3->SetMarkerSize(0.5);
gr3->SetMarkerStyle(20);
gr3->SetMarkerColor(kRed);
gr3->GetYaxis()->CenterTitle();
gr3->GetXaxis()->CenterTitle();

cvs0->Print("pressure.pdf");


/////////////////////////////////////////

}///////////////////////////////////////////////////////////////////////////////value==2 END

else
{
cout<<" change value "<<endl;
}
}

////////////////////////////////////////////////////////////////////////////////関数の定義
time_t  get_caltime(char *tstr, char *tstr2)//時間変換
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
