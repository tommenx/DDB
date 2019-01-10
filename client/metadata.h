#ifndef _DDB_METADATA_H_
#define _DDB_METADATA_H_
#include <curl/curl.h>  
#include <json/json.h>  
#include <iostream>  
#include <string>
#include <map> 
#include <regex>
#include "parser.h"

#define MAX_FRAG_COUNT1 100
#define MAX_FRAG_V 10
#define MAX_FRAG_H 10
// #define MAX_ATTR_COUNT1 100

using namespace std;
using namespace Json;

// struct AttrInfo
// {
// string attr_name;
// string type;
// bool is_key;
// int size;
// };
// struct ConditionH1{
//     bool is_needed;
//     string attr_name;
//     string operation;
//     string attr_value;
// };


// struct ConditionV{
//     bool is_needed=false;
//     int attr_num;
//     string attr_list[MAX_ATTR_COUNT];
// };

// struct ConditionHV{
//     bool is_needed;
//     ConditionH1 h1;
//     ConditionH1 h2;
//     ConditionV v1;
//     int db_id;
// };


// struct Fragment1{
//     string tb_name;
//     // id means db nums ->site num
//     int frag_count;
//     ConditionHV conditions[MAX_FRAG_COUNT];
//     // 似乎写错了。到时候加上
    
// };
// struct Table{
// string tb_name;
// AttrInfo attrs[100];
// int attr_count;
// };
struct ConditionH{
    string attr_name="";
    string operation="";
    string attr_value="";
};

struct frag{
    int DBnum;
    int condition_h_count = 0;
    ConditionH condition_h[MAX_FRAG_H];
    int condition_v_count = 0;
    string condition_v[MAX_FRAG_V];
};

struct Fragment{
    string tb_name;
    int frag_count=0;
    frag fragment[MAX_FRAG_COUNT1];
};
int etcd_set(char *key, char *value, char *token);
int etcd_set2(char *key, char *value, char *token);
string  Create_Dir(string &etcd_url,string &etcd_dir);  
bool Insert_Attrvalue(string &dir,string &value);
size_t write_data(void *buffer, size_t size, size_t nmemb, void *stream);
bool Save_Table(Table TableInfo);
bool Save_Fragment(Fragment table);
string Search_Value(string &dir);
Fragment To_json(string &info,string &tablename);
Fragment Get_Fragment(string &tablename);
Table To_Table(string &info,string &tablename);
Table Get_Table(string &tablename);
Fragment1 Change_Frag(Fragment f1);
Fragment1 Get_Fragment1(string &tablename);
void Get_TableAttrs(string &tablename,string str[]);
SiteInfos Get_SiteInfo();
bool Save_SiteInfo(SiteInfo site);
#endif /*_DDB_METADATA_H_*/