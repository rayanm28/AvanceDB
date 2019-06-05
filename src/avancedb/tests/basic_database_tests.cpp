/*
 *  AvanceDB - an in-memory database similar to Apache CouchDB
 *  Copyright (C) 2015-2017 Ripcord Software
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <stdlib.h> 
#include <vector>
#include <cstring>

#include <boost/format.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <time.h>
#include <cmath>
#include "libscriptobject_gason.h"
#include "script_object_factory.h"
#include "script_array_factory.h"
#include "script_array_vector_source.h"

#include "libhttpserver.h"

#include "../databases.h"
#include "../database.h"
#include "../rest_exceptions.h"
#include "../post_all_documents_options.h"

using std::vector;
using namespace std;


 int DOCCOUNT;
 /*************Get Random STR**********************/
    string gen_random(const int len) {
        string s("");   
        srand(1);
        for (int i = 0; i < len; ++i) {
            s+=rand()%25+97;
        }
      return s;
    }
    /***********************************************/
class BasicDatabaseTests : public ::testing::Test {
protected:
    BasicDatabaseTests() {

    }
    
    static void SetUpTestCase() {
        auto dbName = "BasicDatabaseTests";
        databases_.AddDatabase(dbName);
        db_ = databases_.GetDatabase(dbName);
        
        ifstream dataFile("6MKeys.csv");

        string linebuffer;

        if(!dataFile.is_open()) {
            cout << "Cannot open input file.\n";
          }
        unsigned long int i=0;
        std::string json = R"({"docs":[)";
        while (dataFile && getline(dataFile, linebuffer)){
        bool continueFlag=false;
		if (linebuffer.length() == 0)continue;
          stringstream lineStream(linebuffer);
             string token;
            getline(lineStream,token,'\t');
            if(token.length()<8)continue;
            for(int k=0;k<token.length();++k){
            if(token.at(k)<' ' || token.at(k)>'~' || token.at(k)=='\"' ||  token.at(k)=='\\' ){continueFlag=true;break;}
            }
            if(continueFlag)continue;
            if (i > 0) {
                json += ',';
            }
            auto id=token;
            json += MakeDocJson(id);
            i++;
        }
        DOCCOUNT=i;

        json += R"(]})";
        
       std::vector<char> buffer{json.cbegin(), json.cend()};
        buffer.push_back('\0');
        rs::scriptobject::ScriptObjectJsonSource source(buffer.data()); 
        auto obj = rs::scriptobject::ScriptObjectFactory::CreateObject(source, false);
        docs_ = obj->getArray("docs");
    }
    
    virtual void SetUp() {
        
    }
    
    virtual void TearDown() {
        
    }
  
    static std::string MakeDocId(unsigned id) {
        return (boost::format("%08u") % id).str();

    }

    static std::string MakeDocJson(const std::string& id) {
        return MakeDocJson(id.c_str());
    }
    
    static std::string MakeDocJson(const char* id) {
        auto json = (boost::format(R"({"_id":"%s"})") % id).str();
        return json;
    }

    static bool ValidateRevision(unsigned num, document_ptr doc) {
        return ValidateRevision(num, doc->getRev());
    }
    
    static bool ValidateRevision(unsigned num, const std::string& rev) {
        return ValidateRevision(num, rev.c_str());
    }
    
    static bool ValidateRevision(unsigned num, const char* rev) {
        auto valid = false;
        
        if (rev) {
            auto len = std::strlen(rev);
            if (len == 34) {
                valid = rev[0] == ('0' + num) && rev[1] == '-';
            }
        }
        
        return valid;
    }
    
    static Databases databases_;
    static database_ptr db_;
    static script_array_ptr docs_;
    static unsigned updateSequence_;
};

Databases BasicDatabaseTests::databases_;
database_ptr BasicDatabaseTests::db_;
script_array_ptr BasicDatabaseTests::docs_;
unsigned BasicDatabaseTests::updateSequence_ = 0;

TEST_F(BasicDatabaseTests, test2) {    
  

     srand(1);
     vector<string> documentslookup;
    
std::chrono::steady_clock::time_point start_set_doc = std::chrono::steady_clock::now();
    for(int i=0;i<DOCCOUNT;++i){
       auto obj = docs_->getObject(i);
       auto id = obj->getString("_id");
        if(rand()%2){
        auto doc = db_->SetDocument(id, obj);
        }
        documentslookup.push_back(id);
    }
std::chrono::steady_clock::time_point finish_set_doc = std::chrono::steady_clock::now();
std::chrono::duration<long double> elapsedTime_set_doc=std::chrono::duration_cast<std::chrono::nanoseconds>(finish_set_doc - start_set_doc);
	cout<<"set_doc_Time: " << elapsedTime_set_doc.count() << endl;
	cout<<"****************"<< endl;

for(int loop_num=1;loop_num<=20;loop_num+=2){
  int docs_size=documentslookup.size();
  unsigned int search_length=docs_size*10*loop_num;

std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    for(unsigned int i=0;i<search_length;++i){
               auto doctmp=db_->GetDocument(documentslookup[i%docs_size].c_str(),false);
         }

    /*for(unsigned int i=0;i<search_length;++i){
               auto doctmp=db_->GetDocument(documentslookup[rand()%docs_size].c_str(),false);
         }*/

        std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();
        std::chrono::duration<long double> elapsedTime=std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
	cout<<"search_length: "<< search_length<< endl;        
	cout<<"get_doc_Time: " << elapsedTime.count() << endl;


}
}

