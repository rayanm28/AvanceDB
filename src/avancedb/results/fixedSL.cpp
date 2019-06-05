#include <iostream>
#include <fstream>
#include <stdlib.h> 
#include <sstream>
using namespace std;


int main(int argc, char* argv[]){
    if(argc!=3){std::cout<<"Bad Arguments";return 1;}
    int fixedLen=stoi(argv[1]);
    ifstream file1(argv[2]);
    if(!file1.is_open()){cout<<"can't open file" << argv[2]<<endl;return 1;}
    string arg1(argv[2]);
    ofstream resFile(arg1.substr(0,(arg1.length()-10))+"Fixed"+".csv");

    string linebuffer1,token,fcnt,nfcnt,elTime,found;
    getline(file1, linebuffer1);
    
     while (file1 && getline(file1, linebuffer1)){
           
        int searchLength=0;
        stringstream lineStream1(linebuffer1);

        getline(lineStream1,token,'\t');
        getline(lineStream1,fcnt,'\t');
        getline(lineStream1,nfcnt,'\t'); 
        getline(lineStream1,elTime,'\t');
        getline(lineStream1,found,'\t');  
        searchLength=stoi(fcnt)+stoi(nfcnt);
        if((searchLength==fixedLen) && (found.compare("F")==0)){
            resFile<< token << "\t"<<fcnt<<"\t"<<nfcnt<<"\t"<<elTime<<"\t"<<found<< endl;
        }
    }

    return 0;
}
