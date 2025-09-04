// get_attr.h
//	code to extract the xattr data from the extended attribute structure
//      bool get_attr(const std::string& attr_name, const std::string& file, bool useHex, bool quiet, std::string& result)
//	input:
//	    saves the value of the xattr with key attr_name in result. use useHex = true to print output in hex (useful for binary values)
//	    the file parameter is the file or directory to which extended attribute belongs.
//	    the script assumes that the attr_name extended attribute is present on the file. if not, you'll get an error, unless called with quiet = true.
//	    a very efficient way of checking whether the attribute is present in the file is with getfattr
//	    (look at the find_attr and listtags scripts, which uses this mechanism)
//	output:
//	    the code returns the value of the attribute and a result code
//	    if the file does not contain the attribute, the script prints nothing (with quiet) or an error (without quiet)
//	    prints a msg on stderr when the input is not according to expectation (parse error)
//	note:
//	    the location of the attribute's value is not related to the location of the attribute key string itself, or so investigation revealed
//	    (they are not always following each other). reverse-engineering learnt that the attribute key string is preceded by a string length byte and 10 index bytes
//	    comprising of 4 bytes file offset to the value data, 4 bytes length of the value data and two bytes 0x00.

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

const char TAB = '\t';
const char LF  = '\n';

static void shift(int& argc, char**& argv, int pos = 1)
{
    if(pos < argc) {
        for(int i=pos; i<argc; ++i)
            argv[i] = argv[i+1];
        --argc;
    }
}

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}

static bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

static std::string dirname(const std::string& path)
{
	std::string p = path;
	while (endsWith(p,"/")) p = p.substr(0, p.length()-1);
	size_t found = p.find_last_of("/");
	if (found == std::string::npos) return ".";
	return p.substr(0,found);
}

static std::string basename(const std::string& path)
{
	std::string p = path;
	while (endsWith(p,"/")) p = p.substr(0, p.length()-1);
	size_t found = p.find_last_of("/");
	if (found == std::string::npos) return p;
	return p.substr(found+1);
}

static std::string tohex(const char* str, size_t len)
{
	static const char* hexchr = "0123456789abcdef"; // we use lower case a-f
	std::string hex;
	for (int n=0; n<len; ++n) { unsigned char ch = str[n]; hex += hexchr[ch/16]; hex += hexchr[ch%16]; }
	return hex;
}
static std::string tohex(const std::string& str)
{
	return tohex(str.c_str(), str.length());
}

static uint64_t fromhex(const char* str, size_t len)
{
	static bool once = true;
	static unsigned char chrhex[256];
	if (once) {
		for(int n=0; n<256; ++n) chrhex[n] = 0;
		for(int n='0'; n<='9'; ++n) chrhex[n] = (n-'0');
		for(int n='a'; n<='f'; ++n) chrhex[n] = (n-'a'+10); // we use lower case a-f
		once=false;
	}
	uint64_t result = 0;
	for (int n=0; n<len; ++n) result = result*16 + chrhex[str[n]];
	return result;
}
static uint64_t fromhex(const std::string& str)
{
	return fromhex(str.c_str(), str.length());
}

#include <sys/xattr.h>
#define XATTR_SIZE 10000

static bool get_attr(const std::string& attr, const std::string& fname, bool useHex, bool quiet, std::string& result)
{
     char value[XATTR_SIZE];
     ssize_t valueLen;
     std::string attrname=std::string("user.DosStream.") + attr + ":$DATA";

             valueLen = getxattr(fname.c_str(), attrname.c_str(), value, XATTR_SIZE);
             if (valueLen == -1) {
		 return false;
             } else if (!useHex) {
                 result = value;
             } else {
		     result = tohex(value, valueLen-1);
             }

	return true;
}

// EOF
