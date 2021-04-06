#include "zlib.h"
#include "unzip.h"
#include "zip.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <functional>
using namespace std;

int zip(string srcDir, string zipFIlePath);

int zip(string srcDir, string zipFIlePath,function<void(int totalNum,int currentNum)> zipCallBack);

int unzip(string zipFilePath, string targetDir);

int unzip(string zipFilePath, string targetDir,function<void(int totalNum,int currentNum)> unzipCallBack);

int getZipFileNums(string zipFilePath);