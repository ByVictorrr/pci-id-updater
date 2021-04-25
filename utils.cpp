#include "pci-id-updater.h"


VendorIDBuilder &VendorIDBuilder::append(char * key, char *val){
    if(!strcmp(key, PCI_ID_VENDOR_NAME_KEY)){
        this->vendor_name = val; 
    }else if(!strcmp(key, PCI_ID_DEVICE_NAME_KEY)){
        this->device_name = val;
    }else if(!strcmp(key, PCI_ID_SUBSYSTEM_NAME_KEY)){
        this->subsystem_name = val;
    }
    return *this;
}

VendorIDBuilder &VendorIDBuilder::append(char * key, int val){
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
vendor_id &VendorIDBuilder::build(){
    vendor_id v = vendor_id(this->vendor, this->vendor_name);
    device_id d = device_id(this->device, this->device_name);
    sub_id s = sub_id(this->svendor, this->sdevice, this->subsystem_name);
    v.insert(d);
    d.insert(s);

    return v;
}
void VendorIDBuilder::clear(){
    this->vendor = -1;
    this->device = -1;
    this->svendor = -1;
    this->sdevice = -1;
    this->vendor_name = NULL;
    this->device_name= NULL;
    this->subsystem_name = NULL;
}




json_value *JSONFileParser::get_json_array_file(const char *json_file, char *error)
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
    }else if(!(json_data = json_parse_ex(NULL, file_mm, f_len, error))){
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


int JSONFileParser::parse_json_array_file(const char *json_file, pci_ids &ids, char *error){

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



