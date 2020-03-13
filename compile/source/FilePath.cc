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

#include <RtypesCore.h>

#include "FilePath.hh"

using namespace std;

// Declaration static variable
Int_t FilePath::filenum_;

// constructor
FilePath::FilePath(){
}

// destractor
FilePath::~FilePath(){
    for(Int_t i=0;i<filenum_;i++){free(namelist_[i]);}
    free(namelist_);
}

//  Option Method
Int_t FilePath::scanExtract(const struct dirent* dir)
{
    regex regex(".*dat.*");
     return (regex_search(dir->d_name,regex));
}

//  Set Method
void FilePath::SetFilePath(){
    filenum_ = scandir("./",&namelist_,scanExtract,versionsort);
    if (filenum_ < 0) {perror("scandir");}
    return;
}

//  GetMethod
Int_t FilePath::GetFilenum(){
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
    cout << "----------Below file will be loaded---------" << endl;

    for(Int_t i=0;i<filenum_;i++){
        cout << i << ": " << namelist_[i]->d_name << endl;
    }
    cout << "----------------File List END---------------" << endl
         << endl;
}

void FilePath::LoadingMessenger(bool bin, Int_t i){
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
