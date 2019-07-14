#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <sys/stat.h>

using namespace std;

int main(int argc, char* argv[]){
	mkdir("./DRIVE", 0700);
	//creating the segment files in drive
	ofstream out1;
	for(int i = 0; i < 64; i++){
		out1.open("DRIVE/SEGMENT"+to_string(i));
		for(int j = 0; j < 1048576; j++){
			char init[1] = {0};
			out1.write(init, 1);
		}
		for(int h = 0; h < 1024; h++){
			unsigned int init = 0;
			out1.write(reinterpret_cast<const char*>(&init), 4);
		}
		out1.close();
	}
	//create checkpoint
	ofstream out2;
	out2.open("DRIVE/CHECKPOINT");
	for(int i = 0; i < 40; i++){
		unsigned int init = 0;
		out2.write(reinterpret_cast<const char*>(&init), 4);
	}
	for(int i = 0; i < 64; i++){
		char init[1] = {0};
		out2.write(init, 1);
	}
	out2.close();
	ofstream out3;
	out3.open("DRIVE/FILEMAP");
	for(int i = 0; i < 10240*256; i++){
		char init[1] = {0};
		out3.write(init, 1);
	}
	out3.close();
	cout << ".DRIVE was created successfully" << endl;
	return 0;
}




	
