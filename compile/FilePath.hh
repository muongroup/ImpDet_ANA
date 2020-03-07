#ifndef FilePath_h
#define FilePath_h 1

class FilePath
{
public:
    FilePath();
    ~FilePath();

public:
    // option
    static int scanExtract(const struct dirent *dir);
    // set method
    void SetFilePath();
    // get method
    int GetFilenum();
    struct dirent **GetNameList();
    // messenger
    void FilePathMessenger();

protected:
    static int filenum_;
    struct dirent **namelist_;
};
#endif