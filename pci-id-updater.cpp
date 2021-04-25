#include <iostream>
#include <fstream>
#include <map>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <array>
#include <queue>
#include <regex>
#include "json.h"
#include "pci-id-updater.h"

static const char options[] = "j:";
static const char help_msg[] = 
"Usage: pci-id-upator <input-pci.ids-file> <updated-pci.ids-file> <switches>\n"
"-f <json-format-pci-ids>\n"
"-i <vendor>:[<device>:<device-name>:[<subvendor>:<subdevice>:<subsystem_name]]\n"
;


//const char *usage = ""
int main(int argc, char **argv){
    int opt, i, j;
    json_value *json_ids; //aray
    struct _json_value * *field; 
    char error[100];
    int arr_len, num_fields;
    pci_ids ids;

    memset(error, 0, 100);

    while((opt=getopt(argc, argv, options)) != -1){

        switch (opt) {
        case 'f':
            

            /*
            std::ofstream ofile(argv[2]);
            auto &f = std::istream("text.h");
            std::istream ifile(argv[1]);
            std::string line;
            */

            if(!JSONFileParser::parse_json_array_file(optarg,ids,error)){
                fprintf(stderr, error);
                return EXIT_FAILURE;
            }
            // Step 1 - open pci.ids and new pci.ids file
            // Step 2 - read by line and parse and write to new file
            /*
            if(ifile.is_open()){
                while(getline(ifile,line)){
                    // parser dont read anything if # 
                    std::regex re("\t{0,2}[0-9a-fA-F]{4}[0-9a-fA-F]{0,1}\s*");
                    if(line[0] == '#'){
                        //ofile.write(line);
                    }else{
                        // check to seee if the line meets a regex 
                        //regex

                    }


                }
                //ifile.close();
                */
           break; 
        case 'i':
            break;
        default:
            //print options
            printf(help_msg);
            return -1;
            break;

        }
    }
}