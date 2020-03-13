#include <iostream>
#include <cstring>

#include <RtypesCore.h>

#include "CountManager.hh"

using namespace std;

// static menber
Int_t CountManager::readCnt_ = 0;
Int_t CountManager::acceptCnt_ = 0;
Int_t CountManager::idealEveCnt_ = 0;
Int_t CountManager::zeroVecCnt_ = 0;

CountManager::CountManager()
{
}

CountManager::~CountManager()
{
}

void CountManager::CountUp(const char *str)
{
    if (strcmp(str, "READ") == 0)
        readCnt_++;
    if (strcmp(str, "ACCE") == 0)
        acceptCnt_++;
    if (strcmp(str, "IDEA") == 0)
        idealEveCnt_++;
    if (strcmp(str, "ZERO") == 0)
        zeroVecCnt_++;
    // else
    //     cout << "Write correct arg in CountManager::CountUp()" << endl;
}

void CountManager::CountZero(const char *str)
{
    if (strcmp(str, "READ") == 0)
        readCnt_ = 0;
    if (strcmp(str, "ACCE") == 0)
        acceptCnt_ = 0;
    if (strcmp(str, "IDEA") == 0)
        idealEveCnt_ = 0;
    if (strcmp(str, "ZERO") == 0)
        zeroVecCnt_ = 0;
    // else
    //     cout << "Write correct arg in CountManager::CountZero()" << endl;
}

Int_t CountManager::GetCount(const char *str)
{
    if (strcmp(str, "READ") == 0)
    {
        return readCnt_;
    }
    if (strcmp(str, "ACCE") == 0)
    {
        return acceptCnt_;
    }
    if (strcmp(str, "IDEA") == 0)
    {
        return idealEveCnt_;
    }
    if (strcmp(str, "ZERO") == 0)
    {
        return zeroVecCnt_;
    }
    // else
    //     cout << "Write correct arg in CoutnManager::GetCount()" << endl;
    return 0;
}
