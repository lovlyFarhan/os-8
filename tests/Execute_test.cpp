#include "os/Execute.h"
#include <stdio.h>

int main(const int /*argc*/,const char*const /*argv*/[]) {
	std::string	result,test;

	exec::execute("ls /", result, 4096);
	for(size_t i= 1; i < 384; ++i) {
		exec::execute("ls /", test, i);
		if(test != result) {
			printf("FAILED on blockSize=%ld\n", i);
		}
	}
	return 0;
}
