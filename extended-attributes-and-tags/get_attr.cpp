// get_attr.cpp
//	program to interpret a binary xattr stream and extract the data
//	input:
//		get_attr [-x] [-q] attr_name file
//		prints the value of the xattr with key attr_name of file. use -x to print output in hex (useful for binary values)
//		the script assumes that the attr_name extended attribute is present in the file. if not, you'll get an error, unless called with -q (quiet).
//		a very efficient way of finding files and checking whether the attribute is present in the file is used in the find_attr and listtags scripts.
//	output:
//		the script prints the value of the attribute
//		if the file does not contain the attribute, the script prints nothing (with -q) or an error (without -q)
//		prints a msg on stderr when the input is not according to expectation (parse error)
//
//	compile with: g++ -o get_attr get_attr.cpp -lstdc++

#include "get_attr.h"

int main(int argc, char** argv)
{
	bool error = false;

	// process options
	bool useHex = false, quiet = false;
	for(int i=1 ; i<argc ; ) {
		if(std::string(argv[i]) == "-x") {
			useHex = true;
			shift(argc,argv,i);
		}
		else if(std::string(argv[i]) == "-q") {
			quiet = true;
			shift(argc,argv,i);
		}
		else if(argv[i][0] == '-') {
			std::cerr << "unknown option: " << argv[i] << LF;
			error = true;
			shift(argc,argv,i);
		}
		else
			++i;
	}

	// get the input params - these should be the only args left
	if (error || argc != 3) {
		std::cerr << "usage: " << argv[0] << " [-x] [-q] attr_name file" << LF;
		return 1;
	}
	
	std::string result;
	if (! get_attr(argv[1], argv[2], useHex, quiet, result)) return 1;
	std::cout << result;
	return 0;	
}

// EOF
