#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>

#define MAX_SEGMENTS 64
#define SEGMENT_SIZE 1048576
#define SEGMENT_BLOCKS 1024
#define BLOCK_SIZE 1024
#define IMAP_BLOCKS 40
#define MAX_FILESIZE 131072 //128KB
#define TOTAL_DATABLOCKS 128
#define TOTAL_FILES 10240 //10KB
#define BLOCK_SIZE_FILEMAP 256

using namespace std;

struct inode{
	string name; //max size of 128 characters
	int size;
	int dblock[128];
	char dummy[384];
	inode(){
		this -> size = 0;
	}
};
//struct for inum so it can point to an inode
struct inum{
	int num;
	inode* nodeptr;
	inum(){
		this -> num = -1;
		this -> nodeptr = nullptr;
	}
};

vector<inode*> imap;
inum* inumList[40];
//initializing new list of inum
void initInumList(){
	for(int i = 0; i < 40; i++){
		inumList[i] = new inum();
	}
}
string filemap[40];
void import(string filename, string lfs){
	bool exist = false;
	//check for if file exists in filemap
	for(int i = 0; i < 40; i++){
		if(filename == filemap[i]){
			exist = true;
			break;
		}
	}
	//check if there's space in the imap
	if(imap.size() == IMAP_BLOCKS){
		cout << "The imap is currently full" << endl;
	}
	//check that lfs name isn't larger than 128 bytes
	else if(lfs.length() > 128){
		cout << "The name of the file should not exceed 128 bytes" << endl;
	}
	//part of check for file in filemap
	else if(exist == true){
		cout << "This lfs name is already in use" << endl;
	}
	else{
		inode* node = new inode();
		node -> name = lfs;
		ifstream file(filename, ios::binary | ios::ate);
		int fileLength = file.tellg();
		int charCount = 0;
		char buffer[BLOCK_SIZE] = {0};
		file.open(filename);
		for(int i = 0; i < fileLength; i++){
			if(charCount == BLOCK_SIZE || i == fileLength-1){
				//save and empty out buffer contents
				memset(buffer, 0, sizeof(buffer));
				charCount = 0;
			}
			file.get(buffer[i]);
			charCount++;
		}
		//looking for next available inode, one with a num of -1
		for(int i = 0; i < 40; i++){
			if(inumList[i] -> num == -1){
				inumList[i] -> num = i;
				inumList[i] -> nodeptr = node;
				filemap[i] = filename;
				break;
			}
		}

		node -> size = fileLength;
		imap.push_back(node); //adding inode to vector of inodes in use
	}
}

void remove(string lfs){
	//looking for the inode with the same lfs name
	for(int i = 0; i < imap.size(); i++){
		if(imap[i] -> name == lfs){
			for(int j = 0; j < 40; j++){
				//finding inum that corresponds to inode
				if(inumList[j] -> nodeptr -> name == lfs){
					filemap[inumList[j] -> num] = "empty";
					inumList[j] -> nodeptr = nullptr;
					inumList[j] -> num = -1;
				}
			}
			imap.erase(imap.begin()+i);
			break;
		}
	}
	//print statement in the event no match was found
	cout << "That lfs file does not exist" << endl;
}

int main(int argc, char* argv[]){
	return 0;
}




