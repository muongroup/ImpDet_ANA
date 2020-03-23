#include <iostream>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TAxis.h>

#include "FilePath.hh"
#include "AdvancedAnalysis.hh"

using namespace std;

Int_t AdvancedAnalysis::vecCntTmp_[VEC][VEC] = {};

AdvancedAnalysis::AdvancedAnalysis() : Interval_(0),
                                       GrPoint_(0),
                                       PixelsCnt_(0),
                                       IntervalOutFlag_(0)
{
    pixelsGr = new TGraphErrors();
    intervalOut = new ofstream("2_txt/Interval.txt");
}

AdvancedAnalysis::~AdvancedAnalysis() {}

void AdvancedAnalysis::CreateOutput(Int_t iiii, string *filename, Int_t interval, Int_t x1, Int_t x2, Int_t y1, Int_t y2)
{
    if (IntervalOutFlag_ == 0)
    {
        *intervalOut << "Integrated Pixels (x1,x2,y1,y2) = "
                     << "(" << x1 << "," << x2 << "," << y1 << "," << y2 << ")"
                     << endl
                     << endl;
        *intervalOut << setw(16) << "start"
                     << " "
                     << setw(16) << "end"
                     << " "
                     << setw(6) << "count"
                     << endl;
        IntervalOutFlag_ = true;
    }

    if (interval <= 0)
    {
        cout << "You should set an interger > 0 as interval." << endl;
    }
    //set interval as intergre > 0.
    if (Interval_ == interval - 1)
    {
        string outFile, InterOutStart, InterOutEnd;
        if (interval == 1)
        {
            InterOutStart = FilePath::GetFileTime(filename[iiii], "EXCEL");
            InterOutEnd = FilePath::GetFileTime(filename[iiii], "EXCEL");
            outFile = "./2_txt/" + FilePath::GetFileTime(filename[iiii], "TEXT") + ".txt";
            cout << "<========= Create  " << outFile << endl;
        }
        else
        {
            InterOutStart = FilePath::GetFileTime(filename[iiii - (interval - 1)], "EXCEL");
            InterOutEnd = FilePath::GetFileTime(filename[iiii], "EXCEL");
            string startFile, endFile;
            startFile = FilePath::GetFileTime(filename[iiii - (interval - 1)], "TEXT");
            endFile = FilePath::GetFileTime(filename[iiii], "TEXT");
            outFile = "./2_txt/" + startFile + "_" + endFile + ".txt";
            cout << "<========= Create  " << outFile << endl;
        }
        ofstream output(outFile.c_str());
        for (Int_t i = 0; i < VEC; i++)
        {
            for (Int_t j = 0; j < VEC; j++)
            {
                output << setw(3) << i - 15 << "  "
                       << setw(3) << j - 15 << "  "
                       << vecCntTmp_[i][j] << endl;
            }
        }

        PixelsCountIntegrator(x1, x2, y1, y2);
        pixelsGr->SetPoint(GrPoint_, GrPoint_, PixelsCnt_);
        pixelsGr->SetPointError(GrPoint_, 0, sqrt((Double_t)PixelsCnt_));

        *intervalOut << setw(16) << InterOutStart << " "
                     << setw(16) << InterOutEnd << " "
                     << setw(6) << PixelsCnt_ << endl;

        ResetVecCountTmp();
        GrPoint_++;
        PixelsCnt_ = 0;
        Interval_ = 0;
    }
    else
    {
        Interval_++;
    }
    return;
}

void AdvancedAnalysis::PixelsCountIntegrator(Int_t x1, Int_t x2, Int_t y1, Int_t y2)
{
    for (Int_t i = x1; i <= x2; i++)
    {
        for (Int_t j = y1; j <= y2; j++)
        {
            PixelsCnt_ += vecCntTmp_[i + 15][j + 15];
        }
    }
    cout << "<========= Pixels Count = " << PixelsCnt_ << endl;
}

void AdvancedAnalysis::DrawpixelsGr()
{
    TCanvas *cvs10 = new TCanvas("cvs10", "pixels count graph");
    cvs10->cd();
    pixelsGr->SetName("pixelsGr");
    pixelsGr->SetTitle("Integrated Pixels Counts for an Interval");
    pixelsGr->SetMarkerColor(kRed);
    pixelsGr->SetMarkerStyle(20);
    pixelsGr->SetMarkerSize(1);
    pixelsGr->GetXaxis()->SetRange(-0.5, GrPoint_);
    pixelsGr->Draw("APL");
}