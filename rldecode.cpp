#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <vector>
#include <bitset>
#include <cstring>

/*
	This rldecode would be much more like rlencode, just the reverse version of encode;
	I would like to outline whole structure from big view.

	Similarly, I use the firstString and lastString to deal with connections between the current
	buffer content and next buffer content(they are same as rlencode!).

	For example(* denote ascii which highest bit is 1):
		Assume buffer size = 5
		===>plainText:   aabc*|**dde*|ffggd|*ab
		===>lastHalf:  ""   c*    e*     d   b
		=>beforeHalf:     aab  **dd  ffgg  *a
		=>target:      aab  c***dd e*ffgg d*a  b

	and then deal with the target string, that is traverse the target, if find a special
	character(*), I would convert it to bitset and then compute the actual count.

	For example: 
		target: aab*c
		1. I traverse target, if it's common character, just add it to output string
		2. If I find special character(*), I convert it into bitset, and then compute the
		   acutal num from it.Just 120, then I repeated the character for 120 times...
*/



// Initialise global variables
using namespace std;

bool flag = true;
bool oneParam = false;
bool twoParam = false;

int maxSize = 500000;
string beforeString = "";
string lastString = "";
string debugInfo = "";
ofstream fout;

// recover the actual number by given the special character like *
void recover(vector<bitset<8> > &vec, string &plainText, char ch)
{
	int i, j;
	int num = 0;
	int length = vec.size();
	for(i = length - 1; i >= 0; i--){
		for(j = 0; j < 7; j++){
			if(vec[i].test(j)){
				num += 1 << ((length - 1 - i) * 7 + j);
			}
		}
	}

	// if oneParam, combine the output format using []
	if(oneParam){
		debugInfo += string(1, ch) + "[" + to_string(num) + "]";	
	// if twoParam, I need to check the memory size first
	// and then combine the plaintext	
	}else if(twoParam){
		num += 3;

		//I check wether it exceeds memory size here
		if(num >= maxSize){
			int count = num / maxSize;
			int remainder = num % maxSize;
			for(i = 0; i < count; i++){
				for(j = 0; j < maxSize; j++){
					plainText += ch;
				}
				fout.write(plainText.c_str(), sizeof(char) * (plainText.size()));
				plainText = "";
			}

			for(i = 0; i < remainder; i++){
				plainText += ch;
			}
		}else{
			for(i = 0; i < num; i++){
					plainText += ch;
			}
		}
	}
}


// combine the plainText based on the encodeText
void combine(string &encodeText, string &plainText)
{
	if(encodeText == "") return;

	int temp;
	int i = 0;
	bitset<8> bset;
	vector<bitset<8> > vec;

	// traverse encodeText
	while(i < encodeText.length()){
		temp = i+1;
		if(temp < encodeText.length()){

			bset = encodeText.at(temp);
			if(bset.test(7)){
				while(temp < encodeText.length()){
					bset = encodeText.at(temp);

					// Note that, I used vector to store the 
					// consecutive special character
					if(bset.test(7)) vec.push_back(bset);
					else break;
					temp++;
				}
				// recover the number which represent by the special character
				recover(vec, plainText, encodeText.at(i));
				i = temp;
				vec.clear();
			}else{
				if(oneParam){
					debugInfo += encodeText.at(i);
				}else if(twoParam){
					plainText += encodeText.at(i);
				}
				i++;
			}

		}else{
			if(oneParam){
				debugInfo += encodeText.at(i);
			}else if(twoParam){
				plainText += encodeText.at(i);
			}
			i++;
		}
	}

	// if oneParam, check the length first and then output directly...
	if(oneParam){
		if(debugInfo.length() >= maxSize){
			cout << debugInfo;
			debugInfo = "";
		}
	// if twoParam, write to dest file directly...
	}else if(twoParam){
		fout.write(plainText.c_str(), sizeof(char) * (plainText.size()));
		plainText = "";
	}

}

// the core function of the whole decode
// 1. It would generate the target string(ensure no connection between consecutive buffer)
// 2. Check whether it exceeds memory size, if it does, do some special precedure below
// 3. Control the whole function here, make them work properly...
void process(string &encode, string &original, string &plainText)
{	
	if(encode == "") return;

	bitset<8> bset;
	int length = 1;
	int index = encode.length() - 1;

	char lastCharacter = encode.at(index);
	bset = lastCharacter;

	// if length of encode > 1
	if(encode.length() > 1){
		index--;
		// find the special character here
		if(bset.test(7)){
			while(index >= 0){
				bset = encode.at(index);
				if(bset.test(7)){
					length++;
					index--;
				}else{
					break;
				}
			}
			// if the encode length < 1024, note that I set the buffer size as 1025.
			if(encode.length() < 1024){
				if(index == -1){
					beforeString = encode.substr(0, encode.length());
					original = lastString + beforeString;
					lastString = "";
				}else{

					beforeString = encode.substr(0, encode.length() - length - 1);
					original = lastString + beforeString;
					lastString = encode.substr(index, length + 1);	
				}

			// if the encode length equal to the buffer size
			}else{
				beforeString = encode.substr(0, encode.length() - length - 1);
				original = lastString + beforeString;
				lastString = encode.substr(index, length + 1);	
			}
		// find the normal character
		}else{
			while(index >= 0){
				if(lastCharacter == encode.at(index)){
					length++;
					index--;
				}else{
					break;
				}
			}

			beforeString = encode.substr(0, encode.length() - length);
			original = lastString + beforeString;
			lastString = encode.substr(index+1, length);
		}
	}else if(encode.length() == 1){
		original = lastString + lastCharacter;
		lastString = "";
	}

	// combine the plainText here
	combine(original, plainText);
}

int main(int argc, char const *argv[])
{
	ifstream in(argv[1]);

	string original = "";
	string plainText = "";

	// set params properly
	if(argc == 2){
		oneParam = true;
	}else if(argc == 3){
		twoParam = true;
		fout.open(argv[2]);
	}else{
		return -1;
	}

	char buf[1025];
	// memset buffer when it used...
	memset(buf, 0, sizeof(buf));
	if(! in.is_open()){
		cout << "Error..." << endl;
	}

	// read content by using buffer
	while(! in.eof()){
		// must be attention! set the last bit '\0', otherwise garbage
		in.read(buf, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';
		string encode(buf);
		process(encode, original, plainText);
		memset(buf, 0, sizeof(buf));
	}
	
	// combine the lastString to the plainText
	combine(lastString, plainText);

	// if oneParam, output the last plainText
	if(oneParam) cout << debugInfo;

	in.close();
	if(twoParam){
		fout.close();
	}

	return 0;
}
