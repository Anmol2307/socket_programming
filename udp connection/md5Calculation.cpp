#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
using namespace std;


string getMd5sum(string filePath){

	unsigned char c[MD5_DIGEST_LENGTH];
    //char filename[]=filePath.c_str();
    int i;
    FILE *inFile = fopen (filePath.c_str(), "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filePath.c_str());
        exit(0);
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    char ch[5];
    string s;
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) {sprintf(ch, "%02x", c[i]); s+=ch;}
    //printf (" %s\n", filename);
    fclose (inFile);
    return 	s;
}

int hexToInt(char c){
	if(c >= '0' && c <= '9') return c-'0';
	else return c-'a'+10;
}

int bigModulo(string str, int N){
	int	l = str.length();
	if(l == 1) return hexToInt(str[0])%N;
	return (((bigModulo(str.substr(0,l-1), N) % N) * 16%N) + hexToInt(str[l-1])%N)%N;
}

int main()
{
    cout<<getMd5sum("../ticket.pdf")<<endl;
    char ch[10] = "50";
    cout<<bigModulo(ch, 9)<<endl;
    return 0;
}
