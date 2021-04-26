#ifndef PCI_ID_UPDATER_H_
#define PCI_ID_UPDATER_H_
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


#define PCI_ID_NAME_MAX 100
#define PCI_ID_MAX 0xffff

#define PCI_ID_VENDOR_KEY "vendor"
#define PCI_ID_DEVICE_KEY "device"
#define PCI_ID_SVENDOR_KEY "sub_vendor"
#define PCI_ID_SDEVICE_KEY "sub_device"

#define PCI_ID_DEVICE_NAME_KEY "device_name"
#define PCI_ID_VENDOR_NAME_KEY "vendor_name"
#define PCI_ID_SUBSYSTEM_NAME_KEY "subsys_name"



template<class T, class C = std::vector<T>, class P = std::less<typename C::value_type> >
struct pQueue :
   std::priority_queue<T,C,P> {
   typename C::iterator begin() { return std::priority_queue<T, C, P>::c.begin(); }
   typename C::iterator end() { return std::priority_queue<T, C, P>::c.end(); }
};





struct sub_id{
    uint16_t vid, did;
    std::string name;

    sub_id(uint16_t vid, uint16_t did, std::string name)
    : vid(vid),did(did),name(name){}
    friend bool operator<(const sub_id &a, const sub_id &b){
        return a.vid < b.vid && a.did < b.did;
    }
   

};

class device_id{
    private:
        pQueue <sub_id>subsys; 
        std::string name;
        int id;
    public:
        device_id(int id, std::string &&name): id(id),name(name){}
        inline int getNumOfSubSys(){return this->subsys.size();}
        inline int getID()const{return this->id;}
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
        friend bool operator<(const device_id &a, const device_id &b){
            return a.getID() < b.getID();
        }




};
class vendor_id{
    private:
        pQueue <device_id> devs; 
        std::string name;
        int id;
    public:
        vendor_id(int id, std::string &&name):
            name(name),id(id){}

        inline int getNumOfDevs() const{return this->devs.size();}
        inline int getID()const {return this->id;}
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
        inline pQueue<device_id> &getDevices(){return devs;}


        bool inDevs(int did){
            for(device_id d:devs)
                if(d.getID() == did)
                    return true;
            return false;
        }
        friend bool operator<(const vendor_id &a, const vendor_id &b)
        {
            return a.getID() < b.getID();
        }




};
// kinda like builder
class pci_ids{
    private:
        pQueue <vendor_id> vens;
    public:
        pci_ids(){}
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
        /*
        std::ostream& operater<<(std::ostream &o, const pci_ids &ids){
        for(vendor_id &v: ids.) 
        }
        */


        

};



class VendorIDBuilder{
    private:
        int vendor, device, svendor, sdevice;
        char *vendor_name, *device_name, *subsystem_name;
    
    public:
        VendorIDBuilder(): vendor(-1),device(-1),svendor(-1),
                           sdevice(-1),vendor_name(NULL),
                           device_name(NULL), subsystem_name(NULL){}

        VendorIDBuilder &append(char * key, char *val);
        VendorIDBuilder &append(char * key, int val);
        vendor_id &build();
        void clear();


};
class JSONFileParser{
    private:
        static VendorIDBuilder builder;
        JSONFileParser();
        static bool isValidID(int id);

        static json_value *get_json_array_file(const char *json_file, char *error);
    public:
        static int parse_json_array_file(const char *json_file, pci_ids &ids, char *error);

};





#endif