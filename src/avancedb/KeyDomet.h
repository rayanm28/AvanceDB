/*
 * KeyDomet.h
 *
 *  Created on: 15 Jan 2018
 *      Author: hkh148
 */

#ifndef KEYDOMET_H_
#define KEYDOMET_H_
#include <iostream>
#include <string>
#include <cstring>

using std::string;

namespace keydomet {

template<int8_t SIZE>
struct KeyPrefixType {
	static_assert(SIZE != 32 && SIZE != 64, "Only 32 or 64 bit prefixes are supported");
};

template<>
struct KeyPrefixType<32> {
	typedef int32_t type;
};
template<>
struct KeyPrefixType<64> {
	typedef int64_t type;
};

//typedef typename KeyPrefixType<SIZE>::type PrefixType;

template<int8_t SIZE>
class KeyPrefix {
public:
	typedef typename KeyPrefixType<SIZE>::type PrefixType;
public:
	PrefixType prefix;
public:
	explicit KeyPrefix() : prefix(0){

	}

	explicit KeyPrefix(const string& str) {
		prefix = ((PrefixType) str[3] << 24) | ((PrefixType) str[2] << 16)
				| ((PrefixType) str[1] << 8) | (PrefixType) str[0];
		if (SIZE == 64)
			prefix |= ((PrefixType) str[7] << 56) | ((PrefixType) str[6] << 48)
					| ((PrefixType) str[5] << 40) | ((PrefixType) str[4] << 32);
	}
	explicit KeyPrefix(const char* str) :
			KeyPrefix(string(str)) {
	}

	KeyPrefix(const KeyPrefix& another_prefix) :
			prefix(another_prefix.prefix) {
	}

	const PrefixType& getPrefix() const {
		return prefix;
	}
	bool operator<(const KeyPrefix& another_prefix) const{
		return prefix < another_prefix.prefix;
	}
	bool operator==(const KeyPrefix& another_prefix) const {
		return prefix == another_prefix.prefix;
	}
	~KeyPrefix(){}
};

#ifndef LIKELY
#   define LIKELY(condition) __builtin_expect(static_cast<bool>(condition), 1)
#   define UNLIKELY(condition) __builtin_expect(static_cast<bool>(condition), 0)
#endif
#define LESS_HINT(c) LIKELY(c)
#define EQ_HINT(c) LIKELY(c)

constexpr int8_t Size = 64;
class KeyDomet {
public:
	KeyPrefix<Size> prefix;
	string str;
public:

	KeyDomet() : prefix() , str(""){}
	KeyDomet(string&& str) :
			prefix(str), str(std::move(str)) {
	}
	KeyDomet(KeyDomet&& other) :
			prefix(other.prefix), str(std::move(other.str)) {
	}

    KeyDomet(const string& str) :
			prefix(str), str(str) {
	}
	KeyDomet(const char* str) :
			KeyDomet(string(str)) {
	}
	KeyDomet(const KeyDomet& another_keydomet) :
			prefix(another_keydomet.prefix), str(another_keydomet.str) {
	}
	KeyDomet& operator=(const KeyDomet& another_keydomet){
		str = another_keydomet.str;
		prefix = another_keydomet.prefix;
		return *this;
	}
	KeyDomet& operator=(KeyDomet&& another_keydomet){
			str = std::move(another_keydomet.str);
			prefix = another_keydomet.prefix;
			return *this;
		}
	bool operator<(const KeyDomet& another_keydomet) const{
		if(prefix == another_keydomet.prefix){
			return strcmp(str.data() + 8,another_keydomet.str.data() + 8) < 0;
		} else {
			return prefix < another_keydomet.prefix;
		}
	}
	bool operator>(const KeyDomet& another_keydomet) const{
		return another_keydomet < (*this);
	}

	bool operator==(const KeyDomet& another_keydomet) const{
        if (this->prefix == another_keydomet.prefix) {
            return strcmp(str.data() + 8, another_keydomet.str.data() + 8) == 0;
        } else {
            return false;
        }
	}

	const KeyPrefix<Size>& getPrefix() const{
		return prefix;
	}

	const string& getString() const {
		return str;
	}
	~KeyDomet(){}

};



struct KeyDometHasher
{
    size_t operator()(const KeyDomet& k) const
    {
        return k.getPrefix().getPrefix();
    }
};

struct KeyDometComparator
{
    bool operator()(const KeyDomet& lhs, const KeyDomet& rhs) const
    {
        return lhs == rhs;
    }
};

} /* namespace keydomet */

#endif /* KEYDOMET_H_ */
