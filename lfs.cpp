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
	char name[255];
	int size; //in bytes
	int dblock[128];
	char dummy[384];
};
//struct for inum so it can point to an inode
/*struct inum{
	int num;
	inode* nodeptr;
	inum(){
		this -> num = -1;
		this -> nodeptr = nullptr;
	}
};*/

unsigned int imap[IMAP_BLOCKS*BLOCK_SIZE];
//inum* inumList[40];
int current_segment = 0;
int current_block = 0;
int current_position = 0;
char segment[SEGMENT_SIZE];
char segment_summary[SEGMENT_BLOCKS][2];
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
	else if(lfs.length() > 255){
		cout << "The name of the file should not exceed 255 bytes" << endl;
	}
	//part of check for file in filemap
	else if(exist == true){
		cout << "This lfs name is already in use" << endl;
	}
	else{
		inode newNode;
		//copying filename to new inode
		for(int i = 0; i < lfs.length(); i++){
			newNode.name[i] = lfs[i];
		}
		newNode.name[lfs.length()] = '\0';
		int inumber = imap.size() + 1;
		int size = 0; //in bytes
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
				size++;
				memcpy(&segment[current_block * BLOCK_SIZE], buffer, fileLength);
			}
			file.get(buffer[i]);
			charCount++;
		}
		newNode.size = size;
		//FILEMAP UPDATE
		fstream filemap("DRIVE/FILEMAP" ios::binary | ios::ate);
		filemap.seekp(inumber * BLOCK_SIZE_FILEMAP);
		filemap.write(1, 1); //1 indicates the block is valid
		filemap.write(lfs.c_str(), lfs.length()+1);
		filemap.close();
		//WRITING INODE
		segment_summary[current_block][0] = inumber;
		segment_summary[current_block][1] = 1; //indicates valid and in use
		current_block++;
		//IMAP UPDATE
		if(current_block == SEGMENT_BLOCKS){
			//write out a full segment
			fstream seg("DRIVE/SEGMENT"+to_string(current_segment));
			seg.write(segment, SEGMENT_SIZE);
			seg.close();
			current_segment++;
			current_block = 0;
		}
		imap[inumber] = current_position;
		current_position = (size*1024)+ charCount;
		
	}
	/*else{
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
	}*/
}

void remove(string lfs){
	int inumber = -1;
	fstream filemap("DRIVE/FILEMAP", ios::binary | ios::ate);
	for(int i=0; i<TOTAL_FILES; i++){
		filemap.seekg(i*BLOCK_SIZE_FILEMAP);
		char status[1];
		filemap.read(status, 1);
		if(status[0] == 1){
			char temp[BLOCK_SIZE_FILEMAP-1];
			filemap.read(temp, BLOCK_SIZE_FILEMAP-1);
			string name(temp);
			if(name == lfs);
				filemap.close();
				inumber = i;
			}
		}
		break;
	}
	if(inumber == -1){
		cout << "Filename does not exist" << endl;
		return;
	}
	else{
		fstream filemap("DRIVE/FILEMAP", ios::binary | ios::ate);
		filemap.seekp(inumber*BLOCK_SIZE_FILEMAP);
		filemap.write(0, 1); //marking this particular area in filemap no longer usable
		filemap.close();
		//may need to update the imap
	}
}

void display(string lfs, string number, string startbyte){
	int inumber = -1;
	fstream filemap("DRIVE/FILEMAP", ios::binary | ios::ate);
	for(int i=0; i<TOTAL_FILES; i++){
		filemap.seekg(i*BLOCK_SIZE_FILEMAP);
		char status[1];
		filemap.read(status, 1);
		if(status[0] == 1){
			char temp[BLOCK_SIZE_FILEMAP-1];
			filemap.read(temp, BLOCK_SIZE_FILEMAP-1);
			string name(temp);
			if(name == lfs);
				filemap.close();
				inumber = i;
			}
		}
		break;
	}
	if(inumber == -1){
		cout << "Filename does not exist" << endl;
		return;
	}
	//finding the inode
	inode temp;
	unsigned int block_pos = imap[inumber];
	unsigned int seg_num = (block_pos/1024)+1;
	unsigned int pos = (block_pos%1024)*1024;
	if(block_pos != (unsigned int) -1){
		if(seg_num == MAX_SEGMENTS){
			cout << "The expected file is out of bounds" << end;
			return;
		}
		fstream segfile("DRIVE/SEGMENT"+to_string(seg_num), ios::binary | ios::ate);
		segfile.seekg(pos);
		char buf[1024];
		segfile.read(buf, 1024);
		memcpy(&temp, buf, sizeof(inode));
		segfile.close();
		int starting_point = stoi(startbyte);
		int ending_point = stoi(number)+starting_point;
		if(starting_point > temp.size){
			cout << "Start point is larger than the content size" << endl;
			return;
		}
		if(ending_point > temp.size){
			ending_point = temp.size;
		}
		bool start = true;
		for(int i = starting_point/1024; i < ending_point/1024; i++){
			if(start == true){
				pos = pos+starting_point;
				start = false;
			}
			char buffer[1024];
			fstream file("DRIVE/SEGMENT"+to_string(seg_num). ios::binary | ios::ate);
			file.seekg(pos);
			file.read(buffer, 1024);
			file.close();
			for(int j = 0; j < 1024; j++){
				cout << buffer[i];
			}
		}
		cout << endl;
		return;
	}
	cout << "No block position was found" << endl;
}	

int main(int argc, char* argv[]){
	return 0;
}




