#ifndef _DDB_PARSER_H_
#define _DDB_PARSER_H_
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

#define MAX_ATTR_COUNT 10
#define MAX_COND_COUNT 10
#define MAX_FRAG_COUNT 10
using namespace std;

// get from sql parser
// used to generate sql to create table
struct AttrInfo {
    // string tb_name;
    string attr_name;
    string type;
    bool is_key=0;
    int size;
};

struct Table{
    string tb_name;
    AttrInfo attrs[MAX_ATTR_COUNT];
    int attr_count=0;   
};


// get from tableMetadata
struct Attribution{
    string attr_name;
    string type;
    bool is_key;
    int size; 
};

// get from tableMetadata
struct TableInfo{
    Attribution attrs[MAX_ATTR_COUNT];
    string tb_name;
    int attr_count;
};

enum OP {
    E ,
    NE,
    G,
    GE,
    L,
    LE
};

enum TYPE {
    INTEGER,
    CHAR,
    VARCHAR
};


//where table.col op value
struct Condition  {
    OP op;
    // string table;
    string column;
    string value;
    // TYPE  type;
};

//DELETE FROM TABLE table where table.cloumn op value AND ...
struct DeleteQuery{
    string tb_name;
    int cond_count = 0;
    Condition conditions[MAX_COND_COUNT];

};

struct Insert{
    string tb_name;
    string valuesList[5];
    int values_count=0;
};

struct ConditionH1{
    bool is_needed;
    string attr_name;
    string operation;
    string attr_value;
};

struct ConditionV{
    bool is_needed = false;
    int attr_num;
    string attr_list[MAX_ATTR_COUNT];
};

struct ConditionHV{
    bool is_needed;
    ConditionH1 h1;
    ConditionH1 h2;
    ConditionV v1;
    int db_id;
};


struct Fragment1{
    string tb_name;
    // id means db nums ->site num
    int frag_count;
    ConditionHV conditions[MAX_FRAG_COUNT];
    // 似乎写错了。到时候加上
    // string db_name;

};

// load 语法
// load tablename filepath
struct Load{
    string tb_name;
    string filepath;
};


// 根据OP生成对应的字符串
string get_op_stmt(OP op);


// 根据解析出来的create 向每一个站点发送创建的语句
string gen_create_stmt(Table table);


// 根据元数据管理创建临时表
string gen_tmp_create_stmt(TableInfo info);


// 根据condition_list 以及 cond_count
// (DELETE FROM publisher where) table.column op value
string gen_condition_stmt(Condition cond_list[],int cond_count);

// 根据deleteQuery生成对应的delete语句 
// delete from publisher where id = 104001;
string gen_delete_stmt(DeleteQuery deleteQuery);

// 根据insert_records 生成对应的insert语句
// 仅适用于水平分片
string gen_insert_stmt(string insert_records[],int insert_record_count,string tmp,string tb_name);

// 根据insert_records生成insert语句
// 仅适用于垂直分片，返回一个vector
vector<string> v_insert(Fragment1 fragment, string insert_records[], int insert_record_count);

// where h1.attr_name h1.attr_condition h1.attr_value AND ... 
// only fit to hor
string gen_con_by_con(ConditionHV hv);

// 根据水平分片的信息生成select语句
// 输入当前表的分片信息，以及期待的临时表的名字
vector<string> gen_select_by_frag(Fragment1 fragment,string tmp_tb_name);

// 根据垂直分片的信息生成select语句
// 输入当前表的分片信息，期待的临时表的名字
vector<string> gen_v_select_by_frag(Fragment1 fragment,string tmp_tb_name);

// 字符串的分割
vector<string> Split(string& src,const string& separator);

// 根据表的结构信息生成和查询结果的字符串生成批量的插入语句
string gen_batch_insert(TableInfo info,string src);

// 写死了，用于服务于垂直分片
// 生成批量的插入语句
// c表示分片的编号：0、1
string gen_batch_insert_v(string src,int c);

// 根据表名生成删除表的语句
string gen_drop_tmp_table(string tmp_tb_name);

bool deleteParser(std::string str , DeleteQuery& sql);
void printDeleteParser(DeleteQuery &sql);

#endif /*_DDB_PARSER_H_*/







































