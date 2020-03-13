#include <iostream>
#include <fstream>

#include <RtypesCore.h>

#include "CountManager.hh"
#include "HistManager.hh"
#include "DataProcess.hh"

using namespace std;

class CountManager;
class HistManager;

DataProcess::DataProcess() : easCH_{0}, detCH_{0}, ped_{0}, thresh_{0},
                             easID_1_(0), easID_2_(0), eveCnt_1_(0), eveCnt_2_(0),
                             chHolder_{0}, xVec_(0), yVec_(0), vecCnt_{0},
                             maxADCch_{0}, maxADCch_2nd_{0}, maxCH_{0}, maxCH_2nd_{0}
{

    dev = new ofstream("./2_txt/deviation.txt");
    data00 = new ofstream("./2_txt/zero_data.txt");
    allData = new ofstream("./2_txt/all_data.txt");
    allDataCH = new ofstream("./2_txt/all_data_ch.txt");
    coinPara = new ofstream("./2_txt/fitting_parameter_coin.txt");
    vecOut = new ofstream("./2_txt/output.txt");
}

DataProcess::~DataProcess()
{
}

void DataProcess::DataLoad()
{
    ifstream config("./1_info/config.txt");
    // ifstream pedestal("./1_info/pedestal.txt");
    ifstream threshould("./1_info/threshould.txt");
    for (Int_t i = 0; i < CH; i++)
    {
        config >> easCH_[i] >> detCH_[i];
        // pedestal >> ped_[i];
        threshould >> thresh_[i];
    }
    config.close();
    // pedestal.close();
    threshould.close();
}

void DataProcess::FillHist(ifstream &aFile)
{
    UInt_t val;

    aFile.read((char *)&val, sizeof(Int_t));

    if (val == 0xffffea0c)
    {
        aFile.read((char *)&val, sizeof(Int_t)); //Header2 : reserved Number of word(16bit)
        aFile.read((char *)&val, sizeof(Int_t)); //Header3 : Eventcounter(12bit) : easiroc識別番号(1 or 2 下位16bit)
        easID_1_ = (val & 0xffff);
        eveCnt_1_ = (val >> 16);

        for (Int_t i = 0; i < CH; i++)
        {
            if (easID_1_ == 1)
            {
                aFile.read((char *)&val, sizeof(Int_t));
                Int_t buffer = val & 0xffff;

                if (i < CH / 2)
                {
                    HistManager::SetHist(detCH_[i], buffer);
                }
            }
            else
            {
                cout << "Data is not correct (mu-PSD1)" << endl;
            }
        }
    }

    aFile.read((char *)&val, sizeof(Int_t));
    if (val == 0xffffea0c)
    {
        aFile.read((char *)&val, sizeof(Int_t)); //Header2 : reserved Number of word(16bit)
        aFile.read((char *)&val, sizeof(Int_t)); //Header3 : Eventcounter(12bit) : easiroc識別番号(1 or 2 下位16bit)

        easID_2_ = (val & 0xffff);

        eveCnt_2_ = (val >> 16);

        for (Int_t i = 0; i < CH; i++)
        {
            if (easID_2_ == 2)
            {
                aFile.read((char *)&val, sizeof(Int_t));
                Int_t buffer = val & 0xffff;
                if (i < CH / 2)
                {
                    HistManager::SetHist(detCH_[i + (CH / 2)], buffer);
                }
            }
            else
            {
                cout << "Data is not correct (mu-PSD2)" << endl;
            }
        }
    }
}

void DataProcess::SetPedestal()
{
    for (Int_t i = 0; i < CH; i++)
    {
        ped_[i] = HistManager::GetMaxHistBin(i);
    }
}
void DataProcess::GetDatafromBinary(ifstream &aFile)
{
    UInt_t val;

    aFile.read((char *)&val, sizeof(Int_t));
    CountManager::CountUp("READ");

    if (val == 0xffffea0c)
    {
        aFile.read((char *)&val, sizeof(Int_t)); //Header2 : reserved Number of word(16bit)
        aFile.read((char *)&val, sizeof(Int_t)); //Header3 : Eventcounter(12bit) : easiroc識別番号(1 or 2 下位16bit)
        easID_1_ = (val & 0xffff);
        eveCnt_1_ = (val >> 16);

        CountManager::CountUp("ACCE");
        for (Int_t i = 0; i < CH; i++)
        {
            if (easID_1_ == 1)
            {
                aFile.read((char *)&val, sizeof(Int_t));
                Int_t buffer = val & 0xffff;

                if (i < CH / 2)
                {
                    chHolder_[detCH_[i]] = buffer;
                    HistManager::SetHist(detCH_[i], buffer);
                }
            }
            else
            {
                cout << "Data is not correct (mu-PSD1)" << endl;
            }
        }
    }
    else
    {
        cout << "Header is not correct (mu-PSD1) READ COUNT:" << CountManager::GetCount("READ") << endl;
    }

    aFile.read((char *)&val, sizeof(Int_t));

    CountManager::CountUp("READ");

    if (val == 0xffffea0c)
    {
        aFile.read((char *)&val, sizeof(Int_t)); //Header2 : reserved Number of word(16bit)
        aFile.read((char *)&val, sizeof(Int_t)); //Header3 : Eventcounter(12bit) : easiroc識別番号(1 or 2 下位16bit)

        easID_2_ = (val & 0xffff);

        eveCnt_2_ = (val >> 16);
        CountManager::CountUp("ACCE");

        for (Int_t i = 0; i < CH; i++)
        {
            if (easID_2_ == 2)
            {
                aFile.read((char *)&val, sizeof(Int_t));
                Int_t buffer = val & 0xffff;
                if (i < CH / 2)
                {
                    chHolder_[detCH_[i + (CH / 2)]] = buffer;
                    HistManager::SetHist(detCH_[i + (CH / 2)], buffer);
                }
            }
            else
            {
                cout << "Data is not correct (mu-PSD2)" << endl;
            }
        }
    }
    else
    {
        cout << "Header is not correct (mu-PSD2) READ COUNT:" << CountManager::GetCount("READ") << endl;
    }

    if (eveCnt_1_ != eveCnt_2_)
    {
        cout << "event data is not match" << endl;
        cout << "mu-PSD1 event num " << eveCnt_1_ << endl;
        cout << "mu-PSD2 event num " << eveCnt_2_ << endl;
    }
}

void DataProcess::SetADCch()
{
    for (Int_t i = 0; i < SHEETNUM; i++)
    {
        maxADCch_[i] = 0;
        maxADCch_2nd_[i] = 0;
        maxCH_[i] = 0;
        maxCH_2nd_[i] = 0;
        for (Int_t j = 0; j < CH / 4; j++)
        {
            Int_t index = j + i * (CH / 4);

            if (chHolder_[index] - ped_[index] > maxADCch_[i])
            {
                maxADCch_2nd_[i] = maxADCch_[i];
                maxCH_2nd_[i] = maxCH_[i];
                maxADCch_[i] = chHolder_[index] - ped_[index];
                maxCH_[i] = index;
            }
        }
        maxADCch_[i] += ped_[maxCH_[i]];
    }
}

void DataProcess::SetFlag()
{
    for (Int_t i = 0; i < SHEETNUM; i++)
    {
        flag_[i] = false;
        if (maxADCch_[i] > thresh_[maxCH_[i]])
        {
            if (maxADCch_2nd_[i] > thresh_[maxCH_[i]] && abs(maxCH_[i] - maxCH_2nd_[i]) > 1)
            {
                flag_[i] = false;
            }
            else
            {
                flag_[i] = true;
            }
        }
    }
}

void DataProcess::CoinEveProcess(Int_t iiii)
{
    if (flag_[0] && flag_[1] && flag_[2] && flag_[3])
    {
        xVec_ = (maxCH_[1] - 16) - (maxCH_[3] - 48) + 15;
        yVec_ = (maxCH_[2] - 32) - maxCH_[0] + 15;

        // printf("CH %d ADC %d\n", maxCH_[0], maxADCch_[0]);
        // printf("CH %d ADC %d\n", maxCH_[1], maxADCch_[1]);
        // printf("CH %d ADC %d\n", maxCH_[2], maxADCch_[2]);
        // printf("CH %d ADC %d\n", maxCH_[3], maxADCch_[3]);
        HistManager::SetCoinhist(maxCH_[0], maxADCch_[0]);
        HistManager::SetCoinhist(maxCH_[1], maxADCch_[1]);
        HistManager::SetCoinhist(maxCH_[2], maxADCch_[2]);
        HistManager::SetCoinhist(maxCH_[3], maxADCch_[3]);

        vecCnt_[xVec_][yVec_]++;

        CountManager::CountUp("IDEA");
        if (xVec_ == 15 && yVec_ == 15)
        {
            CountManager::CountUp("ZERO");
            *data00 << maxCH_[0] << " " << maxCH_[1] << " " << maxCH_[2] << " " << maxCH_[3] << endl;
        }
        *allDataCH << maxCH_[0] << " " << maxCH_[1] << " " << maxCH_[2] << " " << maxCH_[3] << endl;

        for (Int_t i = 0; i < SHEETNUM; i++)
        {
            if (xVec_ == 15)
            {
                HistManager::SetVechist(yVec_, maxADCch_[i]);
            }
            HistManager::SetCoinchhist(maxCH_[i], maxADCch_[i]);
        }
    }
}

void DataProcess::WriteDeviation(time_t date)
{
    *dev << ctime(&date) << "  "
         << CountManager::GetCount("IDEA") << "  "
         << CountManager::GetCount("ACCE") << "  "
         << CountManager::GetCount("ZERO") << endl;
}

void DataProcess::WriteFitPara()
{
    Double_t means[CH];
    Double_t CONSTANT[CH];
    Double_t MEANS[CH];
    Double_t SIGMA[CH];
    Double_t MEANS_error[CH];
    memcpy(means, HistManager::means, sizeof(HistManager::means));
    memcpy(CONSTANT, HistManager::CONSTANT, sizeof(HistManager::CONSTANT));
    memcpy(MEANS, HistManager::MEANS, sizeof(HistManager::MEANS));
    memcpy(SIGMA, HistManager::SIGMA, sizeof(HistManager::SIGMA));
    memcpy(MEANS_error, HistManager::MEANS_error, sizeof(HistManager::MEANS_error));

    *coinPara << "means"
              << " "
              << "CONSTANT"
              << " "
              << "MEANS"
              << " "
              << "SIGMA"
              << " "
              << "MEANS_error" << endl;
    for (Int_t i = 0; i < CH; i++)
    {
        *coinPara << means[i] << " " << CONSTANT[i] << " " << MEANS[i] << " " << SIGMA[i] << " " << MEANS_error[i] << endl;
    }
}

void DataProcess::VecCountProcess()
{
    for (Int_t i = 0; i < VEC; i++)
    {
        for (Int_t j = 0; j < VEC; j++)
        {
            HistManager::SetVecplot(i - 15, j - 15, (Double_t)vecCnt_[i][j]);
            *vecOut << i - 15 << "  " << j - 15 << "  " << vecCnt_[i][j] << endl;
        }
    }
}