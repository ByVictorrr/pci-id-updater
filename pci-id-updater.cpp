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


template<class T, class C = std::vector<T>, class P = std::less<typename C::value_type> >
struct heapq :std::priority_queue<T,C,P> {
    using priority_queue<T,C,P>::priority_queue;
    typename C::iterator begin() { return std::priority_queue<T, C, P>::c.begin(); }
    typename C::iterator end() { return std::priority_queue<T, C, P>::c.end(); }
};

// model

bool checkID(int id){
    if(id < 0 || id > 0xffff)
        return false;
    return true;
}

struct pci_id{
    int vendor, device, svendor, sdevice;
};


// list of maps <ven_id->{dev_id struct}>
// <dev_id->{subsystem_id struct}



struct sub_id{
    uint16_t vid, did;
    std::string name;

    sub_id(uint16_t vid, uint16_t did, std::string name)
    : vid(vid),did(did),name(name){}

};
class device_id{
    private:
        heapq <sub_id, std::vector<sub_id>> subsys;
        std::string name;
        uint16_t id;
    public:
        device_id(uint16_t id, std::string &&name): id(id),name(name){}
        inline int getNumOfSubSys(){return this->subsys.size();}
        inline uint16_t getID(){return this->id;}
        inline std::string &getName(){return this->name;}
        void insert(sub_id &&id){
            subsys.push(id);
        }
        bool inSubSys(uint16_t vid, uint16_t did){
            for(struct sub_id sid: subsys)
                if(sid.did == did && sid.vid == vid)
                    return true;
            return false;
        }





};
class vendor_id{
    private:
        heapq <device_id, std::vector<device_id>> devs;
        std::string name;
        uint16_t id;
    public:
        vendor_id(int id, std::string &&name):
            name(name),id(id){}

        inline int getNumOfDevs(){return this->devs.size();}
        inline uint16_t getID(){return this->id;}
        inline std::string &getName(){return this->name;}

        void insert(device_id &&id){
            devs.push(id);
        }
        device_id &getDevice(int id){
            for(device_id &d: this->devs)
                if(d.getID() == id)
                    return d;
        }


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
        heapq <vendor_id> vens;
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
                void fill(const struct pci_id_model *model, char *error){
            if(!model)
                return;
            // Case 1 - no vendor dont fill, maybe print
            if(!isValidID(model->vendor)){
                return;
            // Case 2 - if vendor isnt in the queue, create a new one and push
            }else if(!inVens(model->vendor)){
                this->vens.push(vendor_id(model->vendor, model->vendor_name));
            }
            // It doesnt matter if vendor is new just inserted
            vendor_id &vtmp=getVendor(model->vendor);
            // Case 3 - no device, then dont fill 
            if(!isValidID(model->device)){
                return;
            // Case 4 - if device isnt in queue, create a new one and push
            }else if(!vtmp.inDevs(model->device)){
                    vtmp.insert(device_id(model->device, model->device_name));
            }
            // Grab newly insert or not newly inserted
            device_id &dtmp = vtmp.getDevice(model->device);
            if(!isValidID(model->svendor) || !isValidID(model->sdevice)){
                return;
            }else if(!dtmp.inSubSys(model->svendor,model->sdevice)){
                dtmp.insert(sub_id(model->svendor, model->sdevice, model->subsystem_name));
            }else{
                // print already in list
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
        VendorIDBuilder builder;
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
         bool isValidID(int id){
            if(id < 0 || id > PCI_ID_MAX)
                return false;
            return true;
        }






    public:
        int parse_json_array_file(const char *json_file, pci_ids &ids, char *error){

            // Step 1 - get the json_array
            json_value *json_ids;
            int int_val;
            
            if(!(json_ids=get_json_array_file(json_file, error))){
                return -1;
            }
            for(auto id : json_ids->u.array){
                //json_object_entry * id = json_ids->u.array.values[]
                if(id->type != json_object){
                    sprintf(error, "parse_json_array_file: an element in the array isnt an object type\n");
                    return -1;
                }
                builder.clear();
                for(auto pair: id->u.object){
                   if(pair.value->type != json_string){
                        sprintf(error, "Parse_json_array_file: a field value in an object value is not a string\n");
                        return -1;
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
                builder.build();

            }// num of objects
        }

       

};

class ModelBuilder{
    private:

    public:
        j

};







//const char *usage = ""
int main(int argc, char **argv){
    int opt, i, j;
    json_value *json_ids; //aray
    struct _json_value * *field; 
    char error[100];
    int arr_len, num_fields;
    struct pci_id_model temp;
    pci_ids ids;

    memset(error, 0, 100);
    memset(&temp, 0, sizeof(temp));

    while((opt=getopt(argc, argv, options)) != -1){

        switch (opt) {
        case 'f':
            // start parsing the json file (requirements )
            for(i=0; i < arr_len; i++){
                json_object_entry * id = json_ids[i]->u.object.values;
                num_fields = json_ids[i]->u.object.length;
                // insert head
                for(j=0; j < num_fields; j++){
                    char *field_name = id[j].name;
                    char * field_val_str = id[j].value->u.string.ptr;
                    int field_val;
                    // check val type before  ()
                    if(!(field_val=strtol(field_val_str, NULL, 16))){ // also what if its 0 how do check if its not numbers
                        // could be description
                        // change to regex (all hex or digits)
                        if(!strcmp(field_name, "vendor_desc")){
                            temp.vendor_name = field_val_str;
                        }else if(!strcmp(field_name, "device_desc")){
                            temp.device_name = field_val_str;
                        }else if(!strcmp(field_name, "subsystem_desc")){
                            temp.subsystem_name = field_val_str; 
                        }else{
                            // not correct format
                        }
                    }else{
                        // change to regex (all hex or digits)
                        if(!strcmp(field_name, "vendor")){
                            temp.vendor = field_val;
                        }else if (!strcmp(field_name, "device")){
                            temp.device = field_val;
                        }else if (!strcmp(field_name, "svendor")){
                            temp.svendor = field_val;
                        }else if (!strcmp(field_name, "sdevice")){
                            temp.sdevice = field_val;
                        }else{
                            // not correect format; please check how the input  
                        }


                    }


                }// after num of fields
                ids.fill(&temp, error);
                memset(&temp, 0, sizeof(temp)); // also init ints -1


            }// after num of arrays
            // Step 1 - open pci.ids and new pci.ids file
            std::ofstream ofile(argv[2]);
            auto &f = std::istream("text.h");
            std::istream ifile(argv[1]);
            std::string line;
            // Step 2 - read by line and parse and write to new file
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