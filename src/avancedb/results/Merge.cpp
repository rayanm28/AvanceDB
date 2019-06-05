#include <iostream>
#include <fstream>
#include <stdlib.h> 
#include <sstream>
using namespace std;

int main(int argc, char *argv[]) {

    if(argc!=3){std::cout<<"Bad Arguments";return 1;}
    
    ifstream file1(argv[1]);
    if(!file1.is_open()){cout<<"can't open file" << argv[1]<<endl;return 1;}
    ifstream file2(argv[2]);    
    if(!file2.is_open()){cout<<"can't open file" << argv[2]<<endl;return 1;}
    string arg1=string(argv[1]);
    ofstream resFile(arg1.substr(0,(arg1.length()-4))+"Merged"+".csv");

    string linebuffer1,linebuffer2,token,foundToken1,foundToken2;
    
   
     while (file1 && file2 && getline(file1, linebuffer1) && getline(file2, linebuffer2)){
        stringstream lineStream1(linebuffer1),lineStream2(linebuffer2);

        for(int i=0;i<3;++i){
            getline(lineStream1,token,'\t');    
            resFile<< token << "\t";
        }
        getline(lineStream1,foundToken1,'\t'); 
        getline(lineStream2,token,'\t'); 
        getline(lineStream2,token,'\t');   
        resFile<< token << "\t";
        getline(lineStream2,foundToken2,'\t');
        if(foundToken2.compare(foundToken1)!=0){std::cout<<token <<"Not Matched F/NF"; return 1;}
        resFile<< foundToken1 << endl;
    }
    return 0;
}
