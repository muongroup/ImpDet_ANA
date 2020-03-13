#ifndef CountManager_h
#define CountManager_h 1

#include <RtypesCore.h>

class CountManager
{
public:
    CountManager();
    ~CountManager();

public:
    //these methods use this poInt_ter.
    //process count
    static void CountUp(const char *);
    static void CountZero(const char *);
    //get count
    static Int_t GetCount(const char *);

protected:
    static Int_t readCnt_;
    static Int_t acceptCnt_;
    static Int_t idealEveCnt_;
    static Int_t zeroVecCnt_;
};

#endif