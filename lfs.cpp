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

unsigned int imap[IMAP_BLOCKS*BLOCK_SIZE];
unsigned int checkpoint[40];
int current_segment = 0;
int current_block = 0;
int current_position = 0;
int inumber = 0;
char segment[SEGMENT_SIZE];
char segment_summary[SEGMENT_BLOCKS][2];
char segment_status[64] = {0}; // 0 is clean 1 is dirty

//initializing new list of inum
/*void initInumList(){
	for(int i = 0; i < 40; i++){
		inumList[i] = new inum();
	}
}*/
string filemap[40];
void import(string filename, string lfs){
	/*bool exist = false;
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
	}*/
	//check that lfs name isn't larger than 255 bytes
	if(lfs.length() > 255){
		cout << "The name of the file should not exceed 255 bytes" << endl;
	}
	//part of check for file in filemap
	else{
		inode newNode;
		//copying filename to new inode
		for(int i = 0; i < lfs.length(); i++){
			newNode.name[i] = lfs[i];
		}
		newNode.name[lfs.length()] = '\0';
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
		fstream filemap("DRIVE/FILEMAP", ios::binary | ios::ate);
		filemap.seekp(inumber * BLOCK_SIZE_FILEMAP);
		const char temp[1] = {1};
		filemap.write(temp, 1); //1 indicates the block is valid
		filemap.write(lfs.c_str(), lfs.length()+1);
		filemap.close();
		//WRITING INODE
		segment_summary[current_block][0] = inumber;
		inumber++;
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
}

void remove(string lfs){
	int inum = -1;
	fstream filemap("DRIVE/FILEMAP", ios::binary | ios::ate);
	for(int i=0; i<TOTAL_FILES; i++){
		filemap.seekg(i*BLOCK_SIZE_FILEMAP);
		char status[1];
		filemap.read(status, 1);
		if(status[0] == 1){
			char temp[BLOCK_SIZE_FILEMAP-1];
			filemap.read(temp, BLOCK_SIZE_FILEMAP-1);
			string name=temp;
			if(name == lfs){
				filemap.close();
				inum = i;
				break;
			}
		}
	}
	if(inum == -1){
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
	int inum = -1;
	fstream filemap("DRIVE/FILEMAP", ios::binary | ios::ate);
	for(int i=0; i<TOTAL_FILES; i++){
		filemap.seekg(i*BLOCK_SIZE_FILEMAP);
		char status[1];
		filemap.read(status, 1);
		if(status[0] == 1){
			char temp[BLOCK_SIZE_FILEMAP-1];
			filemap.read(temp, BLOCK_SIZE_FILEMAP-1);
			string name(temp);
			if(name == lfs){
				filemap.close();
				inumber = i;
			}
		}
		break;
	}
	if(inum == -1){
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
			cout << "The expected file is out of bounds" << endl;
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
			fstream file("DRIVE/SEGMENT"+to_string(seg_num), ios::binary | ios::ate);
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

void list(){
	//may need to add a way to get file size
	cout << "Current Segment: " << current_segment << endl;
	ifstream in("DRIVE/FILEMAP");
	for(int i = 0; i < 10240; i++){
		in.seekg(i*256);
		char status[1];
		in.read(status, 1);
		if(status[0] == 1){
			char name[255];
			in.read(name, 255);
			cout << name << endl;
		}
	}
	in.close();
}

void clean(string value){
/*	int val = stoi(value);
	char target_segments[val];
	int num = 0;
	unsigned int cleanInfo[1024][2];
	unsigned int cleanedUpSeg[1024*1024];
	unsigned int next = 0;
	int current_target_seg = 0;
	unsigned int targetSum[1024][2];
	char targetSeg[SEGMENT_SIZE];

	for(int i = 0; i < 64 && num < val; i++){
		if(segment_status[i] == 1){
			target_segments[num] = i;
			num++;
		}
	}

	if(num == 0){
		cout << "All segments are clean" << endl;
		return;
	}

	cout << num << " dirty segments were found of the " << val << " total being asked to be cleaned" << endl;
	
	for(int j = 0; j < 1024; j++){
		cleanInfo[j][0] = (unsigned int) -1;
		cleanInfo[j][1] = (unsigned int) -1;
	}

	for(int k = 0; k < num; k++){
		current_target_seg = target_segments[0];
		segment_status[k] = 0;
		if(current_target_seg == current_segment){
			memcpy(targetSum, segment_summary, SEGMENT_BLOCKS * BLOCK_SIZE);
			memcpy(targetSeg, segment, SEGMENT_SIZE);
		}
		else{
			char sumBuf[1024*1024];
			fstream segfile("DRIVE/SEGMENT"+current_target_seg, ios::binary | ios::ate);
			segfile.read(targetSeg, SEGMENT_SIZE);
			segfile.read(sumBuf, 1024*1024);
			memcpy(targetSum, sumBuf, 1024*1024);
			segFile.close();
		}
		for(int h = 0; h < 1024; h++){
			unsigned int tempInum = targetSum[h][0];
			unsigned int tempBnum = targetSum[h][1];
			if(tempInum != (unsigned int) -1 && tempBnum != (unsigned int) -1){
				
			
*/	
}

void exit(){
	//writing out checkpoint
	fstream check("DRIVE/CHECKPOINT", ios::binary | ios::ate);
	char temp[160];
	memcpy(temp, checkpoint, 160);
	check.write(temp, 160);
	check.write(segment_status, current_segment); //remembers status of all segments and current one in use for next time
	check.close();
	//writing out current segment
	fstream seg("DRIVE/SEGMENT"+to_string(current_segment), ios::binary | ios::ate);
	seg.write(segment, 1024*1024);
	seg.write(reinterpret_cast<const char*>(&segment_summary), 1024*1024);
	seg.close();
}

int main(int argc, char* argv[]){
	bool state = true;
	string response;
	string temp1;
	string temp2;
	string temp3;
	//add way to find the next clean segment
	while(state){
		cout << "Pick a following command: IMPORT, REMOVE, DISPLAY, LIST, CLEAN, EXIT" << endl;
		cin >> response;
		if(response == "IMPORT"){
			cout << "Enter a filename" << endl;
			cin >> temp1;
			cout << "Enter a name for the lfs" << endl;
			cin >> temp2;
			import(temp1, temp2);
		}
		else if(response == "REMOVE"){
			cout << "Enter the name of the lfs to remove" << endl;
			cin >> temp1;
			remove(temp1);
		}
		else if(response == "DISPLAY"){
			cout << "Enter the name of the lfs to display" << endl;
			cin >> temp1;
			cout << "Enter the amount to display" << endl;
			cin >> temp2;
			cout << "Enter the starting byte for the display to begin at" << endl;
			cin>> temp3;
			display(temp1, temp2, temp3);
		}
		else if(response == "LIST"){
			list();
		}
		else if(response == "CLEAN"){
			cout << "Enter the number of segments you wish to clean" << endl;
			cin >> temp1;
			clean(temp1);
		}
		else if(response == "EXIT"){
			exit();
			state = false;
		}
		else{
			cout << "Invalid Response" << endl;
		}
	}
	return 0;
}




