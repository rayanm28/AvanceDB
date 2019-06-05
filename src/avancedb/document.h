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

#ifndef RS_AVANCEDB_DOCUMENT_H
#define RS_AVANCEDB_DOCUMENT_H

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <map>
#include <string>
#include <cstring>
#include <vector>

#include "types.h"
#include "document_attachment.h"
#include "KeyDomet.h"
using namespace keydomet;

//static int used_count=0;
//static int not_used_count=0;

class Document final : public boost::enable_shared_from_this<Document>, private boost::noncopyable {
public:
    class Less {
    public:
        bool operator()(const document_ptr& a, const document_ptr& b) {
               return a->id_ < b->id_;
        }
    };
    
    class Equal {
    public:
        bool operator()(const document_ptr& a, const document_ptr& b) {
               return a->id_ == b->id_;
        }
    };
    
    class Compare {
    public:      
        Compare(const char* id) :idK(std::move(string(id))) {
        }
        int operator()(const document_ptr& doc) {
           int64_t diff=idK.prefix.prefix-doc->id_.prefix.prefix;
           if (diff!=0) {
                //used_count++;
            return ((int*)&diff)[1]|1;
            }
                //not_used_count++;
              return strcmp(idK.str.data() + 8, doc->id_.str.data() + 8);

        }
        KeyDomet idK;
    };
    
    static document_ptr Create(const char* id, script_object_ptr obj, sequence_type seqNum, bool incrementRev = true);
    
    const char* getId() const;
    std::uint64_t getIdHash() const;
    static std::uint64_t getIdHash(const char*);
    const char* getRev() const;
    sequence_type getUpdateSequence() const;
    
    const script_object_ptr getObject() const;

    document_attachment_ptr getAttachment(const char* name, bool includeBody);
    std::vector<document_attachment_ptr> getAttachments();


private:
    
    friend document_ptr boost::make_shared<document_ptr::element_type>(script_object_ptr&, sequence_type&);

    Document(script_object_ptr obj, sequence_type seqNum);
    
    static bool ValidateHashField(const char*);
    
    script_object_ptr obj_;
    const KeyDomet id_;
    const char* rev_;
    const sequence_type seqNum_;
};

#endif /* RS_AVANCEDB_DOCUMENT_H */

