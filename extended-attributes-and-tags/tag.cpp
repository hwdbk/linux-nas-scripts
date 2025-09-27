// tag.cpp
//
// program to interpret binary xattr stream and extract the tags (labels) from the com.apple.metadata:_kMDItemUserTags bplist structure
// input:
//	   usage: tag options file|directory
//	   the script assumes that the com.apple.metadata:_kMDItemUserTags extended attribute is present in the file, so it is wise to grep first before calling this script
//	   	look at the listtags script on how to do this efficiently.
//	   if you know which tag you're looking for (e.g. "Red"),
//	   	look at the mk_tag_links script on how to do this efficiently.
// output:
//	   prints the Finder tags (user tags and Finder labels) associated with file, mimicking the 'tag' program as written in Objective-C by jdberry (https://github.com/jdberry/tag)
//	   without arguments, tag uses options 'tag -l -n -G' (-lnG, --list --name --no-garrulous), e.g. "/Volumes/share/file_or_directory\tComplete,Green,Orange,Red"
//	   at the moment, only listing (-l, --list) is implemented with the default -lnG, or -N (--no-name) and -g (--garrulous) options
//	   if the file does not contain tags (empty com.apple.metadata:_kMDItemUserTags bplist), the program prints nothing.
//	   prints a msg on stderr when the input is not according to expectation (parse error).
// note:
//	   the formatting of the com.apple.metadata:_kMDItemUserTags varies considerably, depending which application wrote the extended attributes (tag, Finder) or
//	   whether the list is empty or not (no com.apple.metadata:_kMDItemUserTags at all or empty bplist).
//	   the bplist format itself is perfectly explained in https://medium.com/@karaiskc/understanding-apples-binary-property-list-format-281e6da00dbd

#include "get_attr.h"

#include <sys/stat.h>
#include <getopt.h>
#include <locale>
#include <codecvt>
#include <list>

// string (utf8) -> u16string -> wstring
static std::wstring utf8_to_utf16(const std::string& utf8)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert; 
	std::u16string utf16 = convert.from_bytes(utf8);
	std::wstring wstr(utf16.begin(), utf16.end());
	return wstr;
}

// wstring -> u16string -> string (utf8)
static std::string utf16_to_utf8(const std::wstring& utf16)
{
	std::u16string u16str(utf16.begin(), utf16.end());
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert; 
	std::string utf8 = convert.to_bytes(u16str);
	return utf8;
}

static std::string read(std::string& str, int n)
{
	std::string result = str.substr(0,n);
	str = str.substr(n);
	return result;
}

static bool is_directory( const char *path ) {
	struct stat path_stat;
	if ( stat(path, &path_stat) != 0 )
		return false; // error, e.g. does not exist -> it's not a directory
	return ( S_ISDIR(path_stat.st_mode) != 0 );
}

static void usage(int argc, char** argv)
{
	std::cerr << "usage: tag [-hlnNgGtTp0] file|directory" << LF;
//	for (int n=0 ; n<argc ; ++n) std::cerr << n << "=" << argv[n] << LF; // DEBUG
}

int main(int argc, char** argv)
{
	// syntax
	if (argc == 0) { usage(argc,argv); return 1; }
	
	// options (-lnGt is default if no options are given)
	bool optList = true, optName = true, optGarrulous = false, optTags = true, optSlash = false, optNul = false;
	struct option longopts[] = { // long to short option mapping
		{ "help",         no_argument, NULL, 'h' },
		{ "list",         no_argument, NULL, 'l' },
		{ "name",         no_argument, NULL, 'n' },
		{ "no-name",      no_argument, NULL, 'N' },
		{ "garrulous",    no_argument, NULL, 'g' },
		{ "no-garrulous", no_argument, NULL, 'G' },
		{ "tags",         no_argument, NULL, 't' },
		{ "no-tags",      no_argument, NULL, 'T' },
		{ "slash",        no_argument, NULL, 'p' },
		{ "nul",          no_argument, NULL, '0' },
		{ NULL, 0, NULL, 0 }
	};
	int c = 0;
	while ( (c = getopt_long(argc, argv, "hlnNgGtTp0", longopts, NULL)) != -1 ) {
		switch ( c ) { // short option mapping
			case 'l': optList      = true;  break;
			case 'n': optName      = true;  break;
			case 'N': optName      = false; break;
			case 'g': optGarrulous = true;  break;
			case 'G': optGarrulous = false; break;
			case 't': optTags      = true;  break;
			case 'T': optTags      = false; break;
			case 'p': optSlash     = true;  break;
			case '0': optNul       = true;  break;
			case 'h': case '?': default: usage(argc,argv); return 1;
		}
	}
	if (::optind >= argc) { usage(argc,argv); return 1; } // no file|dir arguments were given
	for ( ; ::optind < argc; ++::optind ) { // the remaining args must be file|dir params
		std::string file = argv[::optind];
		// get the extended attributes binary bplist blob in hex
		// note that the bplist can be there, but empty - this happens when a tag was added and later removed in the Finder
		std::string hex;
		if (! get_attr("com.apple.metadata:_kMDItemUserTags", file, true, true, hex)) return 1;
	
		// collect the tags in list
		std::list<std::string> tags;
		// (try to) read "bplist" - this SHOULD be at this position in the file or the offset referencing didn't work
		std::string bplist = read(hex, 12);
		if (bplist != tohex("bplist")) { std::cerr << "tag: error in " << file << ":bplist not found (found 0x" << bplist << ")" << LF ; return 1; }
		
		std::string x,k,l,t;
		x = read(hex, 4); // read 4 hex digits (2 bytes) - this is the version number, usually "00" (but "14" and "18" has also been reported)
		// at this point, we're at the array
		x = read(hex, 1); // this should be the '0xAk' (array marker) - com.apple.metadata:_kMDItemUserTags is encoded as a bplist array of strings
		if ( x != "a" ) { std::cerr << "tag: error in " << file << ": array marker not found (found '" << x << "')" << LF; return 1; }
		k = read(hex, 1); // this should be the number of elements
		if ( k == "f" ) {
			// multi-byte array length: 0x1t kk [kk ...]
			t = read(hex, 1); if ( t != "1" ) { std::cerr << "tag: error in " << file << ": unexpected data in multi-byte array length parameter (" << t << ")" << LF; return 1; }
			t = read(hex, 1);
			k = read(hex, 2*(1<<fromhex(t))); // the 4 bits after '1' defines how may bytes we need to describe the length: 2^t bytes
		}
		int kk = fromhex(k); // hex to decimal
	//	std::cerr << "- '" << x << "' " << kk << LF;
		x = read(hex, 2*kk); // skip the object refs
		// at this point, we're at the actual tag strings. these are preceded by 0x5l or 0x6l length byte(s)
		while ( kk>0 ) {
			x = read(hex, 1);
			l = read(hex, 1);
	//		std::cerr << "-- " << x << l << LF;
			if (! ( x == "5" || x == "6" ) ) { std::cerr << "tag: error in " << file << ": string marker not found (found 0x" << x << l << ")" << LF; return 1; }
			if ( l == "f" ) {
				// multi-byte string length: 0x1t kk [kk ...]
				t = read(hex, 1) ; if ( t != "1" ) { std::cerr << "tag: error in " << file << ": unexpected data in multi-byte string length parameter (" << t << ")"; return 1; }
				t = read(hex, 1);
	//			std::cerr << "--- 1" << t << LF;
				l = read(hex, 2*(1<<fromhex(t))); // the 4 bits after '1' defines how may bytes we need to describe the length: 2^t bytes
			}
	//		std::cerr << "---- " << l << LF;
			int ll=fromhex(l); // hex to decimal
			if ( x == "5" ) { // regular ASCII string. note that as soon as you use a UTF-8 character, the string becomes UTF-16 ($x is 6)
				std::string tag;
				while ( ll>0 ) {
					// read the string but ignore the "\n<digit>" at the end, if found
					x = read(hex, 2); if ( x == "0a" && ll == 2 ) { x = read(hex, 2) ; break; }
					tag += static_cast<char>(fromhex(x));
					--ll;
				}
				tags.push_back(tag);
			}
			else if ( x == "6" ) { // UTF-16 string (output as UTF-8)
				std::wstring tag;
				while ( ll>0 ) {
					// read the string but ignore the "\n<digit>" at the end, if found
					x = read(hex, 4); if ( x == "000a" && ll == 2 ) { x = read(hex, 4) ; break; }
					tag += static_cast<wchar_t>(fromhex(x));
					--ll;
				}
				tags.push_back(utf16_to_utf8(tag));
			}
			--kk;
		}
		// output tags
		if ( tags.size() > 0 ) { // don't print (empty) output on empty sets
			if ( optName ) { // print the file|dir name/path
				std::cout << file << ( optSlash && is_directory(file.c_str()) ? "/" : "" ) << ( optNul ? '\0' : (optGarrulous || ! optTags ? '\n' : '\t') );
			}
			if ( optTags ) { // print the tags
				for( std::list<std::string>::iterator t = tags.begin(); t != tags.end(); ++t) {
					if ( t != tags.begin() ) std::cout << ( optNul ? '\0' : (optGarrulous ? '\n' : ',') );
					std::cout << *t;
				}
				std::cout << ( optNul ? '\0' : '\n' );
			}
		}
	}
}

//EOF
