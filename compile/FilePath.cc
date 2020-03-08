/*
This Class search data files in the current directory.
*/
#include <iostream>
#include <string>
#include <regex>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "FilePath.hh"

using namespace std;

// Declaration static variable
int FilePath::filenum_;

// constructor
FilePath::FilePath(){
}

// destractor
FilePath::~FilePath(){
    for(int i=0;i<filenum_;i++){free(namelist_[i]);}
    free(namelist_);
}

//  Option Method
int FilePath::scanExtract(const struct dirent* dir)
{
    regex regex(".*dat.*");
     return (regex_search(dir->d_name,regex));
}

//  Set Method
void FilePath::SetFilePath(){
    int i=0;
    filenum_ = scandir("./",&namelist_,scanExtract,versionsort);
    if (filenum_ < 0)
        perror("scandir");
    else {
        while (i < filenum_) {
            printf("%s\n", namelist_[i]->d_name);
            i++;
        }
    }
    return;
}

//  GetMethod
int FilePath::GetFilenum(){
    return filenum_;
}

struct dirent** FilePath::GetNameList(){
    return namelist_;
}

// Mesenger
void FilePath::FileListMessenger(){
    if(filenum_==0){
        cout << "No file loaded." << endl;
        return;
    }

    for(int i=0;i<filenum_;i++){
        cout << "----------Below file will be loaded---------" << endl;
        cout << i << ": " << namelist_[i]->d_name << endl;
        cout << "----------------File List END---------------" << endl << endl;
    }
}

void FilePath::LoadingMessenger(bool bin, int i){
    if(bin)
    {
        cout << "----------------NO FILE--------------------" << endl;
        cout << "--------------EXIT PROGRAM-----------------" << endl;
        exit(0);
    }
    else
    {
        cout << "LODING-- " << i << ": " << namelist_[i]->d_name << endl;
    }
    return;
}
