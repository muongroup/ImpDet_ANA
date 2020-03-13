#ifndef FilePath_h
#define FilePath_h 1

#include <RtypesCore.h>

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

protected:
    static Int_t filenum_;
    struct dirent **namelist_;
};
#endif