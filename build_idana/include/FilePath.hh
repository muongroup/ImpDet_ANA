#ifndef FilePath_h
#define FilePath_h 1

#include <string>
#include <RtypesCore.h>

using namespace std;
class FilePath
{
public:
    FilePath();
    ~FilePath();

public:
    // option
    static Int_t scanExtract(const struct dirent *dir);
    // set method
    void SetFilePath();
    // get method
    Int_t GetFilenum();
    struct dirent **GetNameList();
    // messenger
    void FileListMessenger();
    void LoadingMessenger(bool,Int_t);
    // date
    static string GetFileTime(string filename,const char* type);

protected:
    static Int_t filenum_;
    struct dirent **namelist_;
};
#endif