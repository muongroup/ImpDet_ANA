#ifndef AdvancedAnalysis_h
#define AdvancedAnalysis_h 1

#include <string.h>
#include "DataProcess.hh"

using namespace std;

class AdvancedAnalysis
{
    public:
        AdvancedAnalysis();
        ~AdvancedAnalysis();

    public:
        static void CountUpVecCountTmp(Int_t x, Int_t y) { vecCntTmp_[x][y]++; };
        void ResetVecCountTmp() {memset(vecCntTmp_, 0, sizeof(vecCntTmp_)); };
        void CreateOutput(Int_t iiii, string *filename, Int_t interval, Int_t x1,Int_t x2,Int_t y1, Int_t y2);
        void PixelsCountIntegrator(Int_t x1, Int_t x2, Int_t y1, Int_t y2);
        void DrawpixelsGr();

    protected:
        Int_t Interval_;
        Int_t GrPoint_;
        Int_t PixelsCnt_;
        bool IntervalOutFlag_;
        static Int_t vecCntTmp_[VEC][VEC];
        TGraphErrors *pixelsGr;
        ofstream *intervalOut;
};

#endif