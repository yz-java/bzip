#include "bzip.h"

#define MAX_PATH 512

#ifdef WIN32
#define ACCESS(fileName,accessMode) _access(fileName,accessMode)
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define ACCESS(fileName,accessMode) access(fileName,accessMode)
#define MKDIR(path) mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

int32_t createDirectory(const std::string &directoryPath)
{
    uint32_t dirPathLen = directoryPath.length();
    if (dirPathLen > MAX_PATH)
    {
        return -1;
    }
    char tmpDirPath[MAX_PATH] = { 0 };
    for (uint32_t i = 0; i < dirPathLen; ++i)
    {
        tmpDirPath[i] = directoryPath[i];
        if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
        {
            if (ACCESS(tmpDirPath, 0) != 0)
            {
                int32_t ret = MKDIR(tmpDirPath);
                if (ret != 0)
                {
                    return ret;
                }
            }
        }
    }
    return 0;
}


int extract_currentfile(unzFile zfile,char * extractdirectory)
{
    unsigned int fileName_BufSize = 512;
    char *fileName_WithPath=new char[fileName_BufSize];
    char *p,*fileName_WithoutPath;
    unz_file_info64 zFileInfo;
    p = fileName_WithoutPath = fileName_WithPath;
    if (UNZ_OK != unzGetCurrentFileInfo64(zfile,
     &zFileInfo, fileName_WithPath,
      fileName_BufSize, NULL, 0, NULL, 0))
    {
        cout << "[ERROR] 获取当前文件信息失败" << endl;
        return -1;
    }
    char *temp = new char[fileName_BufSize];
    //修改fileName_WithPath，使得extractdirectory加在其前面
    strcpy(temp, extractdirectory);
    strcat(temp, fileName_WithPath);
    fileName_WithPath = temp;
    //判断当前文件是目录还是文件
    while ((*p) != '\0')
    {
        if (((*p) == '/') || ((*p) == '\\'))
            fileName_WithoutPath = p + 1;
        p++;
    }
    if (*fileName_WithoutPath == '\0')
    {
        cout << "[INFO] " << "成功读取当前目录:" << fileName_WithPath << endl;
        cout << "[INFO] " << "开始创建目录:" << fileName_WithPath << endl;
        //创建目录
        int err = createDirectory(fileName_WithPath);
        if (err != 0)
            cout << "[ERROR] " << "创建目录" 
            << fileName_WithPath << "失败" << endl;
        else
            cout << "[INFO] " << "成功创建目录" 
            << fileName_WithPath << endl;
    }
    else
    {
        cout << "[INFO] " << "成功读取当前文件:" 
        << fileName_WithoutPath << endl;
        cout << "[INFO] " << "开始解压当前文件:" 
        << fileName_WithoutPath << endl;
        //打开当前文件
        if (UNZ_OK != unzOpenCurrentFile(zfile))
        {
            //错误处理信息  
            cout <<"[ERROR] "<< "打开当前文件" << 
            fileName_WithoutPath << "失败！" << endl;
        }
        //定义一个fstream对象，用来写入文件
        fstream file;
        file.open(fileName_WithPath, ios_base::out | ios_base::binary);
        ZPOS64_T fileLength = zFileInfo.uncompressed_size;
        //定义一个字符串变量fileData，读取到的文件内容将保存在该变量中
        char *fileData = new char[fileLength];
        //解压缩文件  
        ZPOS64_T err = unzReadCurrentFile(zfile, (voidp)fileData, fileLength);
        if (err<0)
            cout << "[ERROR] " << "解压当前文件" 
            << fileName_WithoutPath << "失败！" << endl;
        else
            cout << "[INFO] " << "解压当前文件"  
            << fileName_WithoutPath << "成功！" << endl;
        file.write(fileData, fileLength);
        file.close();
        free(fileData);
    }
    return 0;
}

void EnumDirFiles(const string& dirPrefix,const string& dirName,vector<string>& vFiles)
{
    if (dirPrefix.empty() || dirName.empty())
        return;
    string dirNameTmp = dirName;
    string dirPre = dirPrefix;
 
    if (dirNameTmp.find_last_of("/") != dirNameTmp.length() - 1)
        dirNameTmp += "/";
    if (dirNameTmp[0] == '/')
        dirNameTmp = dirNameTmp.substr(1);
    if (dirPre.find_last_of("/") != dirPre.length() - 1)
        dirPre += "/";
 
    string path;
 
    path = dirPre + dirNameTmp;
 
 
    struct stat fileStat;
    DIR* pDir = opendir(path.c_str());
    if (!pDir) return;
 
    struct dirent* pDirEnt = NULL;
    while ( (pDirEnt = readdir(pDir)) != NULL )
    {
        if (strcmp(pDirEnt->d_name,".") == 0 || strcmp(pDirEnt->d_name,"..") == 0)
            continue;
 
        string tmpDir = dirPre + dirNameTmp + pDirEnt->d_name;
        if (stat(tmpDir.c_str(),&fileStat) != 0)
            continue;
 
        string innerDir = dirNameTmp + pDirEnt->d_name;
        if (fileStat.st_mode & S_IFDIR == S_IFDIR)
        {
            EnumDirFiles(dirPrefix,innerDir,vFiles);
            continue;
        }
 
        vFiles.push_back(innerDir);
    }
 
    if (pDir)
        closedir(pDir);
}
 
//压缩码流
int WriteInZipFile(zipFile zFile,const string& file)
{
    fstream f(file.c_str(),std::ios::binary | std::ios::in);
    f.seekg(0, std::ios::end);
    long size = f.tellg();
    f.seekg(0, std::ios::beg);
    if ( size <= 0 )
    {
        return zipWriteInFileInZip(zFile,NULL,0);
    }
    char* buf = new char[size];
    f.read(buf,size);
    int ret = zipWriteInFileInZip(zFile,buf,size);
    delete[] buf;
    return ret;
}
 
//zip 压缩
int zip(string srcDir, string zipFIlePath) {
    if (srcDir.find_last_of("/") == srcDir.length() - 1)
        srcDir = srcDir.substr(0,srcDir.length()-1);
 
    struct stat fileInfo;
    stat(srcDir.c_str(), &fileInfo);
    if (S_ISREG(fileInfo.st_mode)) {
        zipFile zFile = zipOpen(zipFIlePath.c_str(),APPEND_STATUS_CREATE);
        if (zFile == NULL) {
            cout<<"openfile failed"<<endl;
            return -1;
        }
        zip_fileinfo zFileInfo = { 0 };
        int ret = zipOpenNewFileInZip(zFile, srcDir.c_str(), &zFileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_BEST_COMPRESSION);
        if (ret != ZIP_OK) {
            cout<<"openfile in zip failed"<<endl;
            zipClose(zFile, NULL);
            return -1;
        }
        ret = WriteInZipFile(zFile,srcDir);
        if (ret != ZIP_OK) {
            cout<<"write in zip failed"<<endl;
            zipClose(zFile,NULL);
            return -1;
        }
        zipClose(zFile, NULL);
        cout<<"zip ok"<<endl;
    }
    else if (S_ISDIR(fileInfo.st_mode)) {
        size_t pos = srcDir.find_last_of("/");
        string dirName = srcDir.substr(pos + 1);
        string dirPrefix = srcDir.substr(0, pos);
 
        zipFile zFile = zipOpen(zipFIlePath.c_str(), APPEND_STATUS_CREATE);
        if (zFile == NULL) {
            cout<<"openfile failed"<<endl;
            return -1;
        }
 
        vector<string> vFiles;
        EnumDirFiles(dirPrefix, dirName, vFiles);
        vector<string>::iterator itF = vFiles.begin();
        for (;itF != vFiles.end(); ++itF) {
            zip_fileinfo zFileInfo = { 0 };
            int ret = zipOpenNewFileInZip(zFile,itF->c_str(),&zFileInfo,NULL,0,NULL,0,NULL,Z_DEFLATED, Z_BEST_COMPRESSION);
            if (ret != ZIP_OK) {
                cout<<"openfile in zip failed"<<endl;
                zipClose(zFile,NULL);
                return -1;
            }
            ret = WriteInZipFile(zFile, dirPrefix + "/"+ (*itF));
            if (ret != ZIP_OK) {
                cout<<"write in zip failed"<<endl;
                zipClose(zFile,NULL);
                return -1;
            }
        }
 
        zipClose(zFile,NULL);
        cout<<"zip ok"<<endl;
    }
    return 0;
}


int unzip(string zipFilePath, string targetDir){
    unzFile zfile;//定义一个unzFile类型的结构体zfile
    //定义zip文件的路径，可以使用相对路径
    //调用unzOpen64()函数打开zip文件
    zfile = unzOpen64(zipFilePath.c_str());
    if (zfile == NULL)
    {
        cout << zipFilePath << "[INFO] 打开压缩文件失败" << endl;
        return -1;
    }
    else
    {
        cout << "[INFO] 成功打开压缩文件" << endl;
    }
    unz_global_info64 zGlobalInfo;
    //unz_global_info64是一个结构体
    //其中最重要的是number_entry成员
    //这个变量表示了压缩文件中的所有文件数目（包括文件夹、
    //文件、以及子文件夹和子文件夹中的文件）
    //我们用这个变量来循环读取zip文件中的所有文件
    if (UNZ_OK != unzGetGlobalInfo64(zfile, &zGlobalInfo))
    //使用unzGetGlobalInfo64函数获取zip文件全局信息
    {
        cout << "[ERROR] 获取压缩文件全局信息失败" << endl;
        return -1;
    }
    //循环读取zip包中的文件，
    //在extract_currentfile函数中
    //进行文件解压、创建、写入、保存等操作
    for (int i = 0; i < zGlobalInfo.number_entry; i++)
    {
        //extract_currentfile函数的第二个参数指将文件解压到哪里
        targetDir+="/";
        int err = extract_currentfile(zfile, (char *)targetDir.c_str());
        //关闭当前文件
        unzCloseCurrentFile(zfile);
        //使指针指向下一个文件
        unzGoToNextFile(zfile);
    }
    //关闭压缩文件
    unzClose(zfile);
    return 0;
}