#include <bits/stdint-uintn.h>
#include <map>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "json.h"
using namespace std;

static const char options[] = "j:";
static const char help_msg[] = 
"Usage: pci-id-upator <input-pci.ids-file> <updated-pci.ids-file> <switches>\n"
"-f <json-format-pci-ids>\n"
"-i <vendor>:[<device>:<device-name>:[<subvendor>:<subdevice>:<subsystem_name]]\n"
;

#define PCI_ID_NAME_MAX 100
struct pci_id{
    uint16_t vendor, device, svendor, sdevice;
    char vendor_name[PCI_ID_NAME_MAX], device_name[PCI_ID_NAME_MAX];
    char svendor_name[PCI_ID_NAME_MAX], sdevice_name[PCI_ID_NAME_MAX];
};
struct pci_id_vendor{
    uint16_t vendor;
    char vendor_name[PCI_ID_NAME_MAX];
};

struct pci_id_device{
    uint16_t vendor;
    char vendor_name[PCI_ID_NAME_MAX];
};


class pci_id{
    private:
        uint16_t device;

        
        
};

map<uint16_t, {}> 



int get_len_file(const char *file){
    int len, fd;
    if((fd=open(file, O_RDONLY)) < 0)
        return -1;
    else if((len=lseek(fd, 0, SEEK_END)) < 0)
        return -1;
    else if(close(fd) < 0)
        return -1;
    return len;  
}

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
    *arr_len = json_data->u.array.length;
    return json_data->u.array.values;
}

void insert(struct pci_id **list, struct pci_id *new)
{
    if(!*list){
        // sort maybe
        *list=calloc(1, ) 
        // insert first item
    }else{

    }


}
//const char *usage = ""
int main(int argc, char **argv){
    int opt, f_len, ifd, ofd, num_ids, i, j;
    char *file_mm;
    struct pci_id_list *head = NULL;
    json_value *json_data; //aray
    struct _json_value ** json_ids, *field; 
    char error[100];
    int arr_len;
    struct pci_id temp;
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
                int num_fields = json_ids[i]->u.object.length;
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
                            temp.vendor = field_val;
                        }else if(!strcmp(device_name, "vendor desc")){

                        }
 

                    }else{
                        // change to regex (all hex or digits)
                        if(!strcmp(field_name, "vendor")){
                            temp.vendor = field_val;
                        }else if (!strcmp(field_name, "device")){
                            temp.device = field_val;
                        }else if (!strcmp(field_name, "svendor")){
                            temp.svendor = field_val;
                        }else if (!strcmp(feild_name, "sdevice")){
                            temp.sdevice = field_val;
                        }else{
                            // not correect format; please check how the input  
                        }


                    }

                    

                }


            }
            // sort head and open the pci.ids
            // read line by line till vendor is <= vendor_in_file




            

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