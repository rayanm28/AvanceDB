#include <iostream>
#include <fstream>
#include <stdlib.h> 
#include <sstream>
#include <vector>
#include <math.h>   
using namespace std;


int main(int argc, char* argv[]){
    if(argc!=2){std::cout<<"Bad Arguments";return 1;}
    
    ifstream file1(argv[1]);
    if(!file1.is_open()){cout<<"can't open file" << argv[1]<<endl;return 1;}
    string arg1=string(argv[1]);
    ofstream resFile;
    resFile.open(arg1.substr(0,(arg1.length()-9))+"hist"+".csv", std::ios_base::app);
    vector<int> usedKeyDometcnt(10,0);
    vector<long double> usedKeyDometElTime(10,0.0);
    string linebuffer1,token,fcnt,nfcnt,elTime,found;
    unsigned int sum=0;
    long double sumET=0.0;
     while (file1 && getline(file1, linebuffer1)){

        int searchLength=0;
        stringstream lineStream1(linebuffer1);

        getline(lineStream1,token,'\t');
        getline(lineStream1,fcnt,'\t');
        getline(lineStream1,nfcnt,'\t'); 
        getline(lineStream1,elTime,'\t');
        getline(lineStream1,found,'\t');  
        
        int used_count=stoi(fcnt),not_used_count=stoi(nfcnt),index=0;
        if(used_count!=0 || not_used_count!=0){
        int avg=(int)floor(10*(used_count/(double)(used_count+not_used_count)));
        index=(avg==10)?9:avg;
        }
        usedKeyDometcnt[index]++;
        sum++;
        usedKeyDometElTime[index]+=stold(elTime);
        
    }
        resFile<< "[SL:" << stoi(fcnt)+stoi(nfcnt)<<"]\t";

    for(int i=0;i<usedKeyDometcnt.size();++i){
        resFile<<100*(usedKeyDometcnt[i]/(double)sum)<<"\t";
    }
        resFile<<endl;
    resFile<< "[ET:" << stoi(fcnt)+stoi(nfcnt)<<"]\t";
    for(int i=0;i<usedKeyDometElTime.size();++i){
        if(usedKeyDometcnt[i]==0){continue;}
        sumET+=(usedKeyDometElTime[i]/(double)usedKeyDometcnt[i]);
    }

  /*  for(int i=0;i<usedKeyDometElTime.size();++i){
        if(usedKeyDometcnt[i]==0){resFile<< "0\t"; continue;}
        resFile<< 100*((usedKeyDometElTime[i]/(double)usedKeyDometcnt[i])/(long double)sumET) <<"\t";
    }*/

        for(int i=0;i<usedKeyDometElTime.size();++i){
        if(usedKeyDometcnt[i]==0){resFile<< "0\t"; continue;}
        resFile<< (1000000*(usedKeyDometElTime[i]/(double)usedKeyDometcnt[i])) <<"\t";
    }
    
        resFile<<endl;
    return 0;
}
