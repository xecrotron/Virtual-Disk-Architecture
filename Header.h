#include <bits/stdc++.h>
using namespace std;

#define BackupStorageCapacity 256
#define BlockSize 16

class CvFile {
	
	string fileName;
	
	string fileContent;
	
	public:

		void setFileName(string fileName);

		string getFileName();

		void setFileContent(string fileContent);

		string getFileContent();

};

class Version {

	public:

		int version_number;

		int created_at;

		bool is_deleted;

		vector<int> blocks;

		Version(int _version_number, int _created_at);

};

class BackupFile {

	public:

		CvFile file;

		vector<Version> versions;

};

class DataBackup {

	int capacity;

	int block_size;

	vector<vector<char>> backup_storage;

	set<int> available_blocks;
	
	map<string, BackupFile> backup_data;

	public:

		int initializeVirtualDisk(int _capacity = BackupStorageCapacity, int _block_size = BlockSize);

		// dumps the current usage stats
		void displayVirtualDisk(); 

		// returns 0 if success; -1 if error
		int backup(CvFile file, int refTime); 
		
		// show latest state of catalog info of files and their properties including chunk mapping refTime : -1 - indicates latest data	refTime : non-negative value indicates a point in time 
		void dumpCatalog(int refTime);

		int restore(CvFile file, int refTime);

		// mark f1 as deleted as of t10.
		int markDeleted(CvFile file, int refTime); 

		int pruneOlderVersionsOlderThan(CvFile file, int refTime);

};
