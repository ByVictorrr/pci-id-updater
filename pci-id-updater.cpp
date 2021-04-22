#include <bits/stdint-uintn.h>
#include <map>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "json.h"

static const char options[] = "j:";
static const char help_msg[] = 
"Usage: pci-id-upator <input-pci.ids-file> <updated-pci.ids-file> <switches>\n"
"-f <json-format-pci-ids>\n"
"-i <vendor>:[<device>:<device-name>:[<subvendor>:<subdevice>:<subsystem_name]]\n"
;

#define PCI_ID_NAME_MAX 100

// model
struct pci_id{
    int vendor, device, svendor, sdevice;
    char *vendor_name, *device_name, *subsystem_name;
};

// list of maps <ven_id->{dev_id struct}>
// <dev_id->{subsystem_id struct}


// map<uint16_t, map<struct pci_id_device, > map 
class id_map{
    private:
        std::map<uint16_t, std::string*> ven_map, dev_map;
        std::map<std::pair<uint16_t, uint16_t>, std::string*> subsys_map;
        std::map<uint16_t, uint16_t> ven_to_dev; 
        std::map<uint16_t, std::pair<uint16_t, uint16_t>> dev_to_sub;
    public:
        int insert(struct pci_id *model, char *error)
        {
            if(model->vendor > 0xffff || model->vendor < 0){
                return;
            }
            ven_map.insert(std::pair<uint16_t, std::string>(id, name));

        }



};




struct _json_value **parse_json_array_file(const char *json_file, int *arr_len, char *error)
{

    int fd, f_len;
    char *file_mm;
    json_value *json_data; //aray
    struct _json_value ** json_ids;
    if((fd=open(json_file, O_RDONLY)) < 0){
        return NULL;
    }else if((f_len=lseek(fd, 0, SEEK_END)) < 0){
        return NULL;
    }else if(close(fd) < 0){
        return NULL;
    }else if((file_mm=(char *)mmap(0, f_len, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED){
        fprintf(stderr, "Not able to memory map your file submitted");
        return NULL;
    }else if(!(json_data = json_parse_ex(0, file_mm, f_len, error))){
        fprintf(stderr, "not able to parse");
        return NULL;
    }
    // now wer have a json_array
    if(json_data->type != json_array){
        fprintf(stderr, "Not the correct format");
        return NULL;
    }
    // close fd and unmmap
    *arr_len = json_data->u.array.length;
    return json_data->u.array.values;
}



//const char *usage = ""
int main(int argc, char **argv){
    int opt, i, j;
    json_value *json_data; //aray
    struct _json_value ** json_ids, *field; 
    char error[100];
    int arr_len, num_fields;
    struct pci_id temp;
    id_map map;


    memset(error, 0, 100);
    memset(&temp, 0, sizeof(temp));

    while((opt=getopt(argc, argv, options)) != -1){

        switch (opt) {
        case 'f':
            // start parsing the json file (requirements )
            if(!(json_ids = parse_json_array_file(optarg, &arr_len, error))){
                fprintf(stderr, error);
                return -1;
            }
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
                        if(!strcmp(field_name, "vendor desc")){
                            temp.vendor_name = field_val_str;
                        }else if(!strcmp(field_name, "device desc")){
                            temp.device_name = field_val_str;
                        }else if(!strcmp(field_name, "subsystem desc")){
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
                
                if(map.insert(&tmp, error) < 0){
                    printf(error);
                }
                memset(&temp, 0, sizeof(temp));


            }// after num of arrays
            // Step 1 - open pci.ids and new pci.ids file
            // Step 2 - read by line and parse and write to new file




            

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