#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <vector>
#include <bitset>
#include <cstring>

/*
	This rlencode is mainly about two part,one argument and two arguments; I would like to outline 
	whole structure from big view.

	In order to deal with the big file, I set buff size as 4kb.However the key issue here is that how
	to deal the connection between the content of current buffer and the next buffer. I use the divide
	the buffer content as firstHalf and lastHalf to deal with it.I would show it detaily below. In 
	order to present them clearly, I use 3 variables:

	lastHalf: means the last consecutive sequence from last character in the buffer content
	firstHalf: means the remaining content except the lastHalf
	target: means the content we should processed... target = lastHalf + beforeHalf(normally)

		eg:  ...aaaccc|caabbcccc|cccddd...
			=>lastHalf:    ccc
			=>firstHalf:   caabb
			=>target:	   ccccaabb

	For example:
		Assume buffer size = 5
		===>plainText: aacbb	|bbbdd	|dddef	|ffff

		===>lastHalf: ""    bb       dd       f   ffff
		=>beforeHalf: aac         bbb     ddde    ""
		=>target:     aac    bbbbb    ddddde    ffffff

	As for how to deal with the count num, I list them below

	[0, 127]==> 		 1 _ _ _ _ _ _ _|
	[128, 16383]==>		 1 _ _ _ _ _ _ _ |1 _ _ _ _ _ _ _|
	[16384, 2097151]==>  1 _ _ _ _ _ _ _ |1 _ _ _ _ _ _ _|1 _ _ _ _ _ _ _|
	[2097152, 268435455] 1 _ _ _ _ _ _ _ |1 _ _ _ _ _ _ _|1 _ _ _ _ _ _ _|1 _ _ _ _ _ _ _|

===>One argument: when received one param, that means we should output debug info.
		The method of lastHalf and firstHalf can separate the content wisely, because it can ensure
		that there is no connection between the consecutive parts.

		When I get the target content, I process it and output to the screen when it reach 50 000
		characters in order to make it more efficiency...


===>Two argument: when received two params,that means we should write the binaryString or plainText
		to dest file.
		This method is almost same as one argument method.However, the last buffer content can be a
		litter tricky to deal with.In order to control the memory under 20M, I use several flags to 
		moniter them. When it reach max length, It would be ouput or write to file and then clear...

*/

// Initialise global variables

using namespace std;

int maxSize = 500000;
int consecCount = 0;
int maxMemLength = 500000;
char consecChar = ' ';
bool overMemLengthFlag = false;

bool oneParam = false;
bool twoParam = false;

string firstHalf = "";
string lastHalf = "";
string target = "";
string encode = "";
string binaryString = "";

ofstream fout;

// Get the binary code string specified by the count num
// eg: if count = 5  
//		==> 1000 0101
void binaryCode(unsigned int num)
{	
	int index;
	bitset<8> oneBset;
	// within one byte
	if(num <= 127){
		oneBset = num;
		oneBset.set(7);
		binaryString += (char)(oneBset.to_ulong());

	// within 2 bytea, means we can use 14 bits to store the count
	}else if(num >= 128 && num <= 16383){
		bitset<14> twoBset;
		twoBset = num;
		oneBset.reset();
		oneBset.set(7);
		for(index = 13; index > 6; index--){
			if(twoBset.test(index)){
				oneBset.set(index-7);
			}
		}
		binaryString += (char)(oneBset.to_ulong());

		oneBset.reset();
		oneBset.set(7);
		for(index = 6; index >= 0; index--){
			if(twoBset.test(index)){
				oneBset.set(index);
			}
		}
		binaryString += (char)(oneBset.to_ulong());

	// within 3 bytea, means we can use 21 bits to store the count
	}else if(num >= 16384 && num <= 2097151){
		bitset<21> threeBset;
		threeBset = num;
		oneBset.reset();
		oneBset.set(7);
		for(index = 20; index > 13; index--){
			if(threeBset.test(index)){
				oneBset.set(index - 14);
			}
		}
		binaryString += (char)(oneBset.to_ulong());

		oneBset.reset();
		oneBset.set(7);
		for(index = 13; index > 6; index--){
			if(threeBset.test(index)){
				oneBset.set(index-7);
			}
		}
		binaryString += (char)(oneBset.to_ulong());

		oneBset.reset();
		oneBset.set(7);
		for(index = 6; index >= 0; index--){
			if(threeBset.test(index)){
				oneBset.set(index);
			}
		}
		binaryString += (char)(oneBset.to_ulong());

	// within 4 bytea, means we can use 28 bits to store the count
	}else if(num >= 2097152 && num <= 268435455){
		bitset<28> fourBset;
		fourBset = num;
		oneBset.reset();
		oneBset.set(7);

		for(index = 27; index > 20; index--){
			if(fourBset.test(index)){
				oneBset.set(index - 21);
			}
		}
		binaryString += (char)(oneBset.to_ulong());

		oneBset.reset();
		oneBset.set(7);
		for(index = 20; index > 13; index--){
			if(fourBset.test(index)){
				oneBset.set(index - 14);
			}
		}
		binaryString += (char)(oneBset.to_ulong());

		oneBset.reset();
		oneBset.set(7);
		for(index = 13; index > 6; index--){
			if(fourBset.test(index)){
				oneBset.set(index-7);
			}
		}
		binaryString += (char)(oneBset.to_ulong());

		oneBset.reset();
		oneBset.set(7);
		for(index = 6; index >= 0; index--){
			if(fourBset.test(index)){
				oneBset.set(index);
			}
		}
		binaryString += (char)(oneBset.to_ulong());
	}
}

// comress the plainText and output the debug info
// specified by the one argument or two arguments
void compress(string& plainText)
{
	int index = 0;
	int count = 0;
	int tempIndex = 0;
	char ch;

	while(index < plainText.length()){

		if(oneParam) encode += plainText.at(index);
		if(twoParam) binaryString += plainText.at(index);
		count = 0;
		tempIndex = index;
		while(tempIndex < plainText.length()){
			if(plainText.at(tempIndex) != plainText.at(index)){
				break;
			}
			tempIndex++;
			count++;
		}
		if(count > 2){
			if(oneParam) encode += "[" + to_string(count - 3) + "]";
			if(twoParam) binaryCode(count - 3);
			index = tempIndex;
		}else{
			index++;
		}
	}

	// Note that I set the maxSize threshold
	// so that it can save memory
	if(oneParam){
		if(encode.length() >= maxSize){
			cout << encode;
			encode = "";
		}
	}else if(twoParam){
		fout.write(binaryString.c_str(), sizeof(char) * (binaryString.size()));
		binaryString = "";
	}
}

// deal with the last buffer content
// because it may not be fulled with 
// content.I should deal with it separately
void dealWith()
{
	if(oneParam){
		encode += consecChar;
		encode += "[" + to_string(consecCount - 3) + "]";
	}else if(twoParam){
		binaryString += consecChar;
		binaryCode(consecCount - 3);
	} 
}

// this functoin can ensure that there would no
// connection between current buffer content and
// next buffer content.
void process(string &original)
{
	int consecutiveLen = 0;
	int length = original.length();
	int index = length - 1;

	if(length == 0) return;

	char lastCharacter = original.at(length - 1);
	while(index >= 0){
		if(original.at(index) == lastCharacter){
			consecutiveLen++;
			index--;
		}else break;
	}

	firstHalf = original.substr(0, length - consecutiveLen);

	// if the firstHalf is empty
	if(firstHalf == ""){
		// if the target is not emtpy...
		if(target != ""){
			// if the target characters is same as buffer content, concatenate them...
			if(target.at(target.length() - 1) == original.at(0)){

				// Here, I check whether it exceeed the memory size
				if(target.length() >= maxMemLength){
					overMemLengthFlag = true;
					consecChar = target.at(target.length() - 1);
					consecCount += original.length();
				}else{
					target += original;
				}

			// if the target character is different from buffer content
			// compress them target directly...
			}else{

				// check whether it exceeds the max memory size...
				if(overMemLengthFlag){
					consecCount += target.length();
					dealWith();
					overMemLengthFlag = false;
					consecChar = ' ';
					consecCount = 0;

					target = "";
					lastHalf = original.substr(index + 1, consecutiveLen);
					target = lastHalf;

				}else{
					compress(target);
					target = "";
					lastHalf = original.substr(index + 1, consecutiveLen);
					target = lastHalf;

				}
			}
		// if the target is empty, just make it as the lastHalf
		}else{
			lastHalf = original.substr(index + 1, consecutiveLen);
			target = lastHalf;
		}
	// if the firstHalf is not emtpy
	}else{
		// check whether it exceeds the max memory size...
		if(overMemLengthFlag){
			if(firstHalf.at(0) != consecChar){

				consecCount += target.length();
				dealWith();
				compress(firstHalf);

			}else{
				int len = 0;
				while(len < firstHalf.length()){
					if(firstHalf.at(len) == consecChar) len++;
					else break;
				}
				consecCount += len;
				consecCount += target.length();
				dealWith();

				if(len != firstHalf.length()){
					string firstHalfSubstring = firstHalf.substr(len, firstHalf.length());
					compress(firstHalfSubstring);
				}
			}

			// reset the relevant variables when it exceeds memory size...
			overMemLengthFlag = false;
			consecChar = ' ';
			consecCount = 0;

		}else{
			target += firstHalf;
			compress(target);
		}

		// reset the target, and make it as the current lastHalf
		target = "";
		lastHalf = original.substr(index + 1, consecutiveLen);
		target = lastHalf;
	}
}

int main(int argc, char const *argv[])
{

	// set params properly
	ifstream in(argv[1]);
	if(argc == 2){
		oneParam = true;
	}else if(argc == 3){
		twoParam = true;
		fout.open(argv[2], ios::binary);
	}else{
		return -1;
	}

	// memset buffer when it used...
	char buf[1024 * 4 + 1];
	memset(buf, 0, sizeof(buf));

	if(! in.is_open()){
		cout << "Error..." << endl;
	}

	// read content by using buffer
	while(! in.eof()){
		// must be attention! set the last bit '\0', otherwise garbage
		in.read(buf, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';

		string original(buf);

		process(original);
		memset(buf, 0, sizeof(buf));
	}

	// check whether it exceeds max memory size...
	if(overMemLengthFlag){
		consecCount += target.length();
		dealWith();
		overMemLengthFlag = false;
		consecChar = ' ';
		consecCount = 0;
		target = "";
	}

	// compress the last target
	compress(target);

	if(oneParam){
		cout << encode;
	}

	in.close();
	if(twoParam){
		fout.close();
	}
	return 0;
}

