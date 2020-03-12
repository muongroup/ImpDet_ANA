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
    if (strcmp(str, "READ"))
        readCnt_++;
    else if (strcmp(str, "ACCE"))
        acceptCnt_++;
    else if (strcmp(str, "IDEA"))
        idealEveCnt_++;
    else if (strcmp(str, "ZERO"))
        zeroVecCnt_++;
    else
        cout << "Write correct arg in CountManager::CountUp()" << endl;
}

void CountManager::CountZero(const char *str)
{
    if (strcmp(str, "READ"))
        readCnt_ = 0;
    else if (strcmp(str, "ACCE"))
        acceptCnt_ = 0;
    else if (strcmp(str, "IDEA"))
        idealEveCnt_ = 0;
    else if (strcmp(str, "ZERO"))
        zeroVecCnt_ = 0;
    else
        cout << "Write correct arg in CountManager::CountZero()" << endl;
}

Int_t CountManager::GetCount(const char *str)
{
    if (strcmp(str, "READ"))
        return readCnt_;
    else if (strcmp(str, "ACCE"))
        return acceptCnt_;
    else if (strcmp(str, "IDEA"))
        return idealEveCnt_;
    else if (strcmp(str, "ZERO"))
        return zeroVecCnt_;
    else
        cout << "Write correct arg in CoutnManager::GetCount()" << endl;
    return 0;
}
