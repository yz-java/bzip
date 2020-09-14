#include "zlib.h"
#include "unzip.h"
#include "zip.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>
using namespace std;


int zip(string srcDir, string zipFIlePath);

int unzip(string zipFilePath, string targetDir);