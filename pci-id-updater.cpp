#include <bits/stdint-uintn.h>
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

static const char options[] = "j:";
static const char help_msg[] = 
"Usage: pci-id-upator <input-pci.ids-file> <updated-pci.ids-file> <switches>\n"
"-f <json-format-pci-ids>\n"
"-i <vendor>:[<device>:<device-name>:[<subvendor>:<subdevice>:<subsystem_name]]\n"
;

#define PCI_ID_NAME_MAX 100
#define PCI_ID_MAX 0xffff

#define PCI_ID_DEVICE_KEY "device"
#define PCI_ID_VENDOR_KEY "vendor"
#define PCI_ID_SDEVICE_KEY "sdevice"
#define PCI_ID_SVENDOR_KEY "svendor"

#define PCI_ID_DEVICE_NAME_KEY "device_name"
#define PCI_ID_VENDOR_NAME_KEY "vendor_name"
#define PCI_ID_SUBSYSTEM_NAME_KEY "subsystem_name"


/*
template<class T, class C = std::vector<T>, class P = std::less<typename C::value_type> >
struct heapq :std::priority_queue<T,C,P> {
    using priority_queue<T,C,P>::priority_queue;
    typename C::iterator begin() { return std::priority_queue<T, C, P>::c.begin(); }
    typename C::iterator end() { return std::priority_queue<T, C, P>::c.end(); }
};

*/
template<class T, class C = std::vector<T>, class P = std::less<typename C::value_type> >
struct pQueue :
   std::priority_queue<T,C,P> {
   typename C::iterator begin() { return std::priority_queue<T, C, P>::c.begin(); }
   typename C::iterator end() { return std::priority_queue<T, C, P>::c.end(); }
};

class vendor_id;
class device_id;
struct sub_id{
    uint16_t vid, did;
    std::string name;

    sub_id(uint16_t vid, uint16_t did, std::string name)
    : vid(vid),did(did),name(name){}

};
class Compare{
    public:
        bool operator()(sub_id &s1, sub_id &s2){
            if(s1.vid < s2.vid){
                return true;
            }else if (s1.vid = s2.vid ){
                if(s1.did <= s2.vid)
                    return true;
                else
                    return false;
            }else{
                return false;
            }
        }
        bool operator()(device_id &s1, device_id &s2)
        {
            if(s1.getID() <= s2.getID()){
                return true;
            }
            return false;
        }
        bool operator()(vendor_id &s1, vendor_id &s2)
        {
            if(s1.getID() <= s2.getID()){
                return true;
            }
            return false;
        }

};
class device_id{
    private:
        pQueue <sub_id, std::vector<sub_id>, Compare> subsys;
        std::string name;
        int id;
    public:
        device_id(int id, std::string &&name): id(id),name(name){}
        inline int getNumOfSubSys(){return this->subsys.size();}
        inline int getID(){return this->id;}
        inline std::string &getName(){return this->name;}
        void insert(sub_id &&id){
            subsys.push(id);
        }
        void insert(sub_id &id){
            subsys.push(id);
        }


        bool inSubSys(int vid, int did){
            for(struct sub_id sid: subsys)
                if(sid.did == did && sid.vid == vid)
                    return true;
            return false;
        }





};
class vendor_id{
    private:
        pQueue <device_id, std::vector<device_id>, Compare> devs;
        std::string name;
        int id;
    public:
        vendor_id(int id, std::string &&name):
            name(name),id(id){}

        inline int getNumOfDevs(){return this->devs.size();}
        inline int getID(){return this->id;}
        inline std::string &getName(){return this->name;}

        void insert(device_id &&id){
            devs.push(id);
        }
        void insert(device_id &id){
            devs.push(id);
        }



        device_id &getDevice(int id){
            for(device_id &d: this->devs)
                if(d.getID() == id)
                    return d;
        }
        inline pQueue<device_id,std::vector<device_id>, Compare> &getDevices(){return devs;}


        bool inDevs(int did){
            for(device_id d:devs)
                if(d.getID() == did)
                    return true;
            return false;
        }




};
// kinda like builder
class pci_ids{
    private:
        pQueue <vendor_id, std::vector<vendor_id>, Compare> vens;
    public:
        pci_ids(): vens(){}
        bool inVens(int vid){
            for(vendor_id v: this->vens)
                if(v.getID() == vid)
                    return true;
            return false;
        }
        vendor_id &getVendor(int id){
            for(vendor_id &v: this->vens)
                if(v.getID() == id)
                    return v;
        }
        void fill(vendor_id &vid, char *error){
            // Case 1 - if the vendor id wasnt filled; cant create a entry
            if(vid.getID() == -1)
                return;
            else if(!inVens(vid.getID())){
                this->vens.push(vid);
                return;
            }
        }

};

class VendorIDBuilder{
    private:
        int vendor, device, svendor, sdevice;
        char *vendor_name, *device_name, *subsystem_name;
    

    public:
        VendorIDBuilder(): vendor(-1),device(-1),svendor(-1),
                           sdevice(-1),vendor_name(NULL),
                           device_name(NULL), subsystem_name(NULL){}

        VendorIDBuilder &append(char * key, char *val){
            if(!strcmp(key, PCI_ID_VENDOR_NAME_KEY)){
                this->vendor_name = val; 
            }else if(!strcmp(key, PCI_ID_DEVICE_NAME_KEY)){
                this->device_name = val;
            }else if(!strcmp(key, PCI_ID_SUBSYSTEM_NAME_KEY)){
                this->subsystem_name = val;
            }
            return *this;
        }
        VendorIDBuilder &append(char * key, int val){
            if(!strcmp(key, PCI_ID_VENDOR_KEY)){
                this->vendor = val;
            }else if (!strcmp(key, PCI_ID_DEVICE_KEY)){
                this->device = val;
            }else if (!strcmp(key, PCI_ID_SVENDOR_KEY)){
                this->svendor = val;
            }else if (!strcmp(key, PCI_ID_SDEVICE_KEY)){
                this->sdevice = val;
            }
            return *this;
        }
        vendor_id &build(){
            vendor_id v = vendor_id(vendor, vendor_name);
            device_id d = device_id(device, device_name);
            sub_id s = sub_id(svendor, sdevice, subsystem_name);
            v.insert(d);
            d.insert(s);

            return v;
        }
        void clear(){
            this->vendor = -1;
            this->device = -1;
            this->svendor = -1;
            this->sdevice = -1;
            this->vendor_name = NULL;
            this->device_name= NULL;
            this->subsystem_name = NULL;
        }



};
class JSONFileParser{
    private:
        static VendorIDBuilder builder;
        JSONFileParser();
        
        static json_value *get_json_array_file(const char *json_file, char *error)
        {

            int fd, f_len;
            char *file_mm;
            json_value *json_data; //aray
            if((fd=open(json_file, O_RDONLY)) < 0){
                sprintf(error, "parse_json_array_file: could not open %s\n", json_file);
                return NULL;
            }else if((f_len=lseek(fd, 0, SEEK_END)) < 0){
                sprintf(error, "parse_json_array_file: lseek error on %s\n", json_file);
                return NULL;
            }else if((file_mm=(char *)mmap(0, f_len, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED){
                sprintf(error, "parse_json_array_file: Not able to memory map your file submitted\n");
                return NULL;
            }else if(!(json_data = json_parse_ex(0, file_mm, f_len, error))){
                return NULL;
            }

            if(json_data->type != json_array){
                sprintf(error, "Not the correct format");
                return NULL;
            }
            cleanup:
                //if(close(fd) < 0){
                    
            return json_data;
        }
        static bool isValidID(int id){
            if(id < 0 || id > PCI_ID_MAX)
                return false;
            return true;
        }






    public:

        static int parse_json_array_file(const char *json_file, pci_ids &ids, char *error){

            // Step 1 - get the json_array
            json_value *json_ids;
            int int_val;
            
            if(!(json_ids=get_json_array_file(json_file, error))){
                return 0;
            }
            for(auto id : json_ids->u.array){
                //json_object_entry * id = json_ids->u.array.values[]
                if(id->type != json_object){
                    sprintf(error, "parse_json_array_file: an element in the array isnt an object type\n");
                    return 0;
                }
                builder.clear();
                for(auto pair: id->u.object){
                   if(pair.value->type != json_string){
                        sprintf(error, "Parse_json_array_file: a field value in an object value is not a string\n");
                        return 0;
                    }
                    char *key = pair.name;
                    char *val = pair.value->u.string.ptr;
                    if(!(int_val=strtol(val, NULL, 16))){ // also what if its 0 how do check if its not numbers
                        if(strcmp(key, PCI_ID_VENDOR_NAME_KEY) && strcmp(key, PCI_ID_DEVICE_NAME_KEY) 
                        && strcmp(key, PCI_ID_SUBSYSTEM_NAME_KEY)){
                            sprintf(error, "iparse_json_array_file: invalid key type\n");
                            continue;
                        }
                        builder.append(key, val);

                    // See if id is valid
                    }else if(!isValidID(int_val)){
                        sprintf(error, "parse_json_array_file: not a valid id value\n");
                        continue;
                    // see if the keys are valid coressonding to ids
                    }else if(strcmp(key, PCI_ID_VENDOR_KEY) &&  strcmp(key, PCI_ID_DEVICE_KEY)
                          && strcmp(key, PCI_ID_SVENDOR_KEY) && strcmp(key, PCI_ID_SDEVICE_KEY)){
                        sprintf(error, "parse_json_array_file: not a key\n");
                    }
                    builder.append(key,val);
                }// after num of fields
                vendor_id &v = builder.build();
                ids.fill(v, error);

            }// num of objects
            return 1;
        }

       

};








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