#include "Header.h"
#include "Utils.cpp"

void CvFile::setFileName(string fileName) {

	this->fileName = fileName;

}

string CvFile::getFileName() {

	return fileName;

}

void CvFile::setFileContent(string fileContent) {

	this->fileContent = fileContent;

}

string CvFile::getFileContent() {

	return fileContent;

}

Version::Version(int _version_number, int _created_at) {

	version_number = _version_number;
	created_at = _created_at;
	is_deleted = false;

}

// Initialize the size of virtual disk and keep track of available blocks
int DataBackup::initializeVirtualDisk(int _capacity, int _block_size) {

	capacity = _capacity / _block_size;
	block_size = _block_size;

	backup_storage.clear();
	backup_storage.resize(capacity, vector<char>(block_size, ' '));

	for (int i = 0; i < capacity; i++) {
		available_blocks.insert(i);
	}

	return 0;
}

// Store the file on virtual disk
// Returns 1 if backup is successfull otherwise 0
int DataBackup::backup(CvFile file, int refTime) {

	string file_name = file.getFileName();
	string file_content = file.getFileContent();

	// If available space is not enough to fit the file contents return 0
	if (available_blocks.size() * block_size < file_content.size()) {
		cout << file_name << " backup unsuccessfull!!!" << endl << endl;
		return 0;
	}

	BackupFile backup_file;
	if (backup_data.count(file_name)) {
		backup_file = backup_data[file_name];
	}
	else {
		backup_file.file = file;
	}

	int version_number = backup_file.versions.size();
	Version version(version_number, refTime);

	int idx = 0;
	vector<int> blocks;

	for (auto block: available_blocks) {

		blocks.push_back(block);

		for (int i = 0; i < block_size && idx < file_content.size(); i++) {
			backup_storage[block][i] = file_content[idx++];
		}

		if (idx == file_content.size()) {
			break;
		}
	}

	for (auto block: blocks) {
		available_blocks.erase(block);
	}

	version.blocks = blocks;

	backup_file.versions.push_back(version);
	
	backup_data[file_name] = backup_file;

	cout << file_name << " backup successfull!!!" << endl << endl;

	return 1;
} 

// Display the virtual disk info - chunks used / free etc.
void DataBackup::displayVirtualDisk() {

	cout << "--- Storage Info ---" << endl;
	for (int i = 0; i < capacity; i++) {
		cout << "Block " << i << ": ";
		for (int j = 0; j < block_size; j++) {
			cout << backup_storage[i][j];
		}
		cout << endl;
	}
	cout << endl;

	return ;
}

// 	Show latest state of catalog info of files and their properties including chunk mapping 
//	refTime : -1 - indicates latest data
//	refTime : non-negative value indicates a point in time 
void DataBackup::dumpCatalog(int refTime) {

	if (refTime == -1) {
		cout << "--- Latest catalog --- " << endl;

		for (auto files: backup_data) {
			if (files.second.versions.back().is_deleted) {
				continue;
			}

			cout << "File name: " << files.second.file.getFileName() << endl;
			cout << "Version: " << files.second.versions.size() - 1 << endl;
			cout << "Created at: " << files.second.versions.back().created_at << endl;
			cout << "Blocks allocated: ";
			for (auto blocks: files.second.versions.back().blocks) {
				cout << blocks << " ";
			}
			cout << endl;
			cout << endl;
		}
	}
	else {
		cout << "--- Catalog at time - " << refTime << " ---" << endl;
		for (auto files: backup_data) {
			int idx = binarySearch(files.second.versions, refTime) - 1;

			if (idx < 0 || files.second.versions[idx].is_deleted) {
				continue;
			}

			cout << "File name: " << files.second.file.getFileName() << endl;
			cout << "Version: " << idx << endl;
			cout << "Created at: " << files.second.versions[idx].created_at << endl;
			cout << "Blocks allocated: ";
			for (auto blocks: files.second.versions[idx].blocks) {
				cout << blocks << " ";
			}
			cout << endl;
			cout << endl;
		}
	}

	return ;
}

//	This will print file content of given files version closest to mentioned time stamp (nearest backed up version)
int DataBackup::restore(CvFile file, int refTime) {

	cout << "--- Restoring ---" << endl;

	string file_name = file.getFileName();

	if (refTime == -1) {
		if (backup_data[file_name].versions.back().is_deleted) {
			cout << "File does not exist." << endl << endl;
			return 0;
		}

		cout << "File name: " << file_name << endl;
		cout << "Version: " << backup_data[file_name].versions.size() - 1 << endl;
		cout << "Created at: " << backup_data[file_name].versions.back().created_at << endl;
		cout << "File content: ";
		for (auto block: backup_data[file_name].versions.back().blocks) {
			for (auto data: backup_storage[block]) {
				cout << data;
			}
		}
		cout << endl;
		cout << endl;
	}
	else {
		int idx = binarySearch(backup_data[file_name].versions, refTime) - 1;

		if (idx < 0 || backup_data[file_name].versions[idx].is_deleted) {
			cout << "File does not exist." << endl << endl;
			return 0;
		}
		
		cout << "File name: " << file_name << endl;
		cout << "Version: " << idx << endl;
		cout << "Created at: " << backup_data[file_name].versions[idx].created_at << endl;
		cout << "File content: ";
		for (auto block: backup_data[file_name].versions[idx].blocks) {
			for (auto data: backup_storage[block]) {
				cout << data;
			}
		}
		cout << endl;
		cout << endl;
	}

	return 1;
}

// Delete the file and return if successfull
int DataBackup::markDeleted(CvFile file, int refTime) {

	string file_name = file.getFileName();

	if (backup_data.count(file_name) == 0) {
		cout << "File does not exist!!!" << endl;
		return 0;
	}

	BackupFile deletedFile = backup_data[file_name];
	Version version(deletedFile.versions.size(), refTime);

	version.is_deleted = true;

	deletedFile.versions.push_back(version);
	backup_data[file_name] = deletedFile;

	cout << file_name << " deleted!!!" << endl << endl;

	return 0;
}

// Prune the versions older than the given time
int DataBackup::pruneOlderVersionsOlderThan(CvFile file, int refTime) {

	string file_name = file.getFileName();
	vector<Version> newVersions;
	for (auto version: backup_data[file_name].versions) {
		if (version.created_at >= refTime) {
			newVersions.push_back(version);
			continue;
		}

		for (auto block: version.blocks) {
			available_blocks.insert(block);
			for (int i = 0; i < block_size; i++) {
				backup_storage[block][i] = ' ';
			}
		}
	}

	backup_data[file_name].versions = newVersions;

	cout << "Successfully pruned older versions!!!" << endl << endl;

	return 0;
}

int main(int argc, char* argv[]) {
	
	// Sample files with different versions

	string F1_V0 ="This is F1 v0 -backed up at t0"; 	//	30
	string F1_V1 ="This is F1 v1 -backed up at t5"; 	//	30
	string F1_V2 ="This is F1 v2 -backed up at t10";	//	31

	string F2_V0 ="This is F2 v0 -backed up at t2"; 	//	30
	string F2_V1 ="This is F2 v1 -backed up at t5"; 	//	30
	string F2_V2 ="This is a longer F2 v2 -backed up at t10"; 	//	39

	string F3_V0 ="This is F3 v0 -backed up at t5. This file tells the story of a simple backup"; 		//	76
	string F3_V1 ="This is F3 v1 -backed up at t10. This file tells the story of how a simple backup and restore app can be used to demo version support in action"; 	 //	 143
	
	// Backup class object
	DataBackup myDB;

	// virtual disk of size 256 bytes and block size of 16 bytes
	myDB.initializeVirtualDisk(256, 16); 

	
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////   Phase 1 - Simple backup, Restore and printing of data structures 	///////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	// Backup file 1
	CvFile file_1;
	file_1.setFileName("File 1");

	file_1.setFileContent(F1_V0); 
	myDB.backup(file_1, 0);

	file_1.setFileContent(F1_V1); 
	myDB.backup(file_1, 5);

	file_1.setFileContent(F1_V2); 
	myDB.backup(file_1, 10);

	// Backup file 2
	CvFile file_2;
	file_2.setFileName("File 2");

	file_2.setFileContent(F2_V0);
	myDB.backup(file_2, 2);

	file_2.setFileContent(F2_V1);
	myDB.backup(file_2, 5);
		
	// Display contents of disk
	myDB.displayVirtualDisk();	

	// Display latest catalog
	myDB.dumpCatalog(-1); 

	// Display catalog at t=0
	myDB.dumpCatalog(0);

	// Restore
	myDB.restore(file_1, -1);
	myDB.restore(file_2, 4);
	myDB.restore(file_2, 5);

	
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////   Phase 2 - more backups - version support 	///////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Backup file 3
	CvFile file_3;
	file_3.setFileName("File 3");

	file_3.setFileContent(F3_V0);
	myDB.backup(file_3, 5);

	// Unsuccessfull as storage is not available
	file_3.setFileContent(F3_V1);
	myDB.backup(file_3, 10);

	// Display latest catalog
	myDB.dumpCatalog(-1);

	// Display the virtual disk
	myDB.displayVirtualDisk();

	
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////   Phase 3 restoring versions 	///////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	// Restoring the file version present at t=7
	myDB.restore(file_1, 7);	// Should be F1 v1?

	// Restoring the file version present at t=3
	myDB.restore(file_1, 3);	// F1 v0?

	
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////   Phase 4 - Marking a deleted file and restoring versions 	///////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	// Mark f1 as deleted as of t15.
	myDB.markDeleted(file_1, 15); 

	myDB.restore(file_1, -1);	//	No file found

	myDB.restore(file_1, 11);	//	F1 v2?

	
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////   Phase 5 - Pruning of versions and reclaiming space 	///////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	// Display latest catalog
	myDB.dumpCatalog(-1); 

	// Pruning versions before t=3
	myDB.pruneOlderVersionsOlderThan(file_1, 3);

	// Display contents of disk
	myDB.displayVirtualDisk();	

	myDB.restore(file_1, 3);

	// Backup a longer version of F2 to check non contiguous allocation
	file_2.setFileContent(F2_V2);
	myDB.backup(file_2, 10);
	
	// Display contents of disk
	myDB.displayVirtualDisk();	

	// Display latest catalog
	myDB.dumpCatalog(-1); 

	return 0;
}
