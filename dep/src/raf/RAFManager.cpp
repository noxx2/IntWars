#include "RAFManager.h"
#include "tinydir.h"
#include "stdafx.h"


#ifdef _WIN32
#include <windows.h>
#include<Winreg.h>
#endif
using namespace std;

RAFManager* RAFManager::instance = 0;

bool RAFManager::init(const string& rootDirectory) {
   tinydir_dir dir;

   if(tinydir_open_sorted(&dir, rootDirectory.c_str()) == -1) {
      return false;
   }

   for (int i = 0; i < dir.n_files; i++)
   {
      tinydir_file file;
      tinydir_readfile_n(&dir, &file, i);

      if (!file.is_dir || strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) {
         continue;
      }

      tinydir_dir subDir;
      tinydir_open_sorted(&subDir, (rootDirectory + '/' + file.name).c_str());
      
      for (int j = 0; j < subDir.n_files; j++)
      {
         tinydir_file subFile;
         tinydir_readfile_n(&subDir, &subFile, j);
      
         string filename(subFile.name);
         string check = ".raf";
         
         if(filename.length() >= 5 && filename.find(check) && filename[filename.length()-1] == 'f') {
            string rafPath = rootDirectory + '/' + file.name + '/' + filename;
            RAFFile* raf = new RAFFile(rafPath);
            files.push_back(raf);
         }
      }
      
      tinydir_close(&subDir);
   }
   
   printf("Loaded %lu RAF files\n", files.size());

   tinydir_close(&dir);
   
   return true;
}

bool RAFManager::readFile(const std::string& path, vector<unsigned char>& toFill) {
   for(vector<RAFFile*>::iterator it = files.begin(); it != files.end(); ++it) {
      RAFFile* f = *it;
      if(f->readFile(path, toFill)) {
         return true;
      }
   }
   
   return false;
}
std::string RAFManager::findGameBasePath()
{
	printf("Searching for LoL base path...\n");
#ifdef _WIN32
	HKEY hKey;
	std::vector<string> strKeyPathCU, strKeyPathLM;
	strKeyPathCU.push_back("SOFTWARE\\RIOT GAMES\\RADS");
	strKeyPathCU.push_back("SOFTWARE\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\Wow6432Node\\RIOT GAMES\\RADS");
	strKeyPathCU.push_back("SOFTWARE\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\RIOT GAMES\\RADS");
	strKeyPathCU.push_back("SOFTWARE\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\RIOT GAMES\\RADS");
	strKeyPathCU.push_back("SOFTWARE\\RIOT GAMES\\RADS");

	strKeyPathLM.push_back("Software\\Wow6432Node\\Riot Games\\RADS");
	strKeyPathLM.push_back("SOFTWARE\\RIOT GAMES\\RADS");

	string strKeyName = "LOCALROOTFOLDER";
	DWORD dwValueType;
	TCHAR byteValue[100];
	DWORD dwValueSize;


	//Check CURRENT_USER keys
	for(int i=0; i< strKeyPathCU.size();i++)
	{
		if( RegOpenKeyExA(HKEY_CURRENT_USER, strKeyPathCU[i].c_str(), 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS )
		{
			continue;
		}

		if( RegQueryValueExA(hKey, strKeyName.c_str(), NULL, &dwValueType, (LPBYTE)byteValue, &dwValueSize) != ERROR_SUCCESS )
		{
			continue;
		}

		string sValue(byteValue);
		sValue += "/projects/lol_game_client/";
		printf("Found base path in %s\n",sValue.c_str());

		return sValue;
	}

	//Check LOCAL_MACHINE keys

	for(int i=0; i< strKeyPathLM.size();i++)
	{
		if( RegOpenKeyExA(HKEY_CURRENT_USER, strKeyPathLM[i].c_str(), 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS )
		{
			continue;
		}

		if( RegQueryValueExA(hKey, strKeyName.c_str(), NULL, &dwValueType, (LPBYTE)byteValue, &dwValueSize) != ERROR_SUCCESS )
		{
			continue;
		}

		std::string sValue(byteValue);
		sValue += "/projects/lol_game_client/";

		printf("Found base path in %s\n",sValue.c_str());
		return sValue;
	}

	printf("Couldnt find League of Legends game path or unable to read Registry keys\n");
	return "";

#else
	return "";
#endif
}