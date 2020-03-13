#ifndef DataProcess_h
#define DataProcess_h 1

#include <fstream>
#include <time.h>

#include <RtypesCore.h>

using namespace std;

const Int_t CH = 64;
const Int_t VEC = 31;
const Int_t SHEETNUM = 4;

class CountManager;

class DataProcess
{
public:
    DataProcess();
    ~DataProcess();

public:
    void DataLoad();
    void GetDatafromBinary(ifstream &);
    void SetADCch();
    void SetFlag();
    void CoinEveProcess(Int_t);
    void WriteDeviation(time_t date);
    void WriteFitPara();
    void VecCountProcess();

protected:
    // output file
    ofstream *dev;
    ofstream *data00;
    ofstream *allData;
    ofstream *allDataCH;
    ofstream *coinPara;
    ofstream *vecOut;
    // load from txt
    Int_t easCH_[CH];
    Int_t detCH_[CH];
    Double_t ped_[CH];
    Double_t thresh_[CH];
    // header date
    Int_t easID_1_;
    Int_t easID_2_;
    Int_t eveCnt_1_;
    Int_t eveCnt_2_;
    // body
    Int_t chHolder_[CH];
    Int_t xVec_;
    Int_t yVec_;
    Int_t vecCnt_[VEC][VEC];
    // flag
    Int_t maxADCch_[SHEETNUM];     //set by SetADCch()
    Int_t maxADCch_2nd_[SHEETNUM]; //set by SetADCch()
    Int_t maxCH_[SHEETNUM];        //set by SetADCch()
    Int_t maxCH_2nd_[SHEETNUM];    //set by SetADCch()
    bool flag_[SHEETNUM];
};

#endif