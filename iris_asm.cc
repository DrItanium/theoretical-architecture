#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include "asm_interact.h"

void usage(char* arg0);

int main(int argc, char* argv[]) {
	FILE* input = 0;
	std::string line("v.obj");
	std::string target;

	std::ostream* output = 0;
	bool closeOutput = false,
		 closeInput = false,
		 errorfree = true;
   int last = argc - 1,
   	   i = 0;
   /* make sure these are properly initialized */
   last = argc - 1;
   errorfree = 1;
   i = 0;
   if(argc > 1) {
      for(i = 1; errorfree && (i < last); ++i) {
		 std::string tmpline(argv[i]);
         if(tmpline.size() == 2 && tmpline[0] == '-') {
            switch(tmpline[1]) {
				case 't':
					++i;
					target = argv[i];
					break;
			   case 'o':
			   		++i;
			   		line = argv[i];
			   		break;
               case 'h':
               default:
                  errorfree = false;
                  break;
            }
         } else {
            errorfree = false;
            break;
         }
      }
      if(errorfree) {
         if(i == last) {
			std::string tline(argv[last]);
			if(tline.size() == 1 && tline[0] == '-') {
				input = stdin;
				closeInput = false;
			} else if (tline.size() >= 1) {
				if ((input = fopen(tline.c_str(), "r")) != NULL) {
					closeInput = true;
				} else {
					std::cerr << "Couldn't open " << tline << " for reading!" << std::endl;
					exit(1);
				}
			}
            /* open the output */
            if(line.size() == 1 && line[0] == '-') {
			   output = &std::cout; 
			   closeOutput = false;
            } else {
			   output = new std::ofstream(line.c_str(), std::ofstream::out | std::ofstream::binary);
			   closeOutput = true;
            }
         } else {
			 std::cerr << "no file provided" << std::endl;
         }
      } else if (target.empty()) {
		std::cerr << "No target provided!" << std::endl;
	  }
   }
   if(output && input) {
	   iris::parseAssembly(target, input, output);
	  if (closeOutput) {
	  	static_cast<std::ofstream*>(output)->close();
	  	delete output;
	  	output = 0;
	  }
	  if(closeInput) {
	  	fclose(input);
		input = 0;
	  }
   } else {
      usage(argv[0]);
   }
}

void usage(char* arg0) {
	std::cerr << "usage: " << arg0 << " [-o <file>] <file>" << std::endl;
}