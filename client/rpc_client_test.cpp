#include "rpc_sql.h"
#include "local_sql.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

#define MAX_ATTR_COUNT 10
#define MAX_COND_COUNT 10
#define MAX_FRAG_COUNT 10

using namespace std;


// create 语句解析出来的东西
struct AttrInfo {
    string tb_name;
    string attr_name;
    string type;
    bool is_key;
    int size;
};


// 从metadataManager中获取的表的信息
struct Attribution{
    string attr_name;
    string type;
    bool is_key;
    int size;
    
};

// 从metadataManager中获取的表的信息
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
    string table;
    string column;
    string value;
    TYPE  type;
};

//DELETE FROM TABLE table where table.cloumn op value AND ...
struct DeleteQuery{
    string tb_name;
    int cond_count;
    Condition *conditions;

};

int attr_count = 3;

AttrInfo attrs[MAX_ATTR_COUNT];

string get_op_stmt(OP op);

void initCreate() {
    AttrInfo a = {"ccc","p_id","integer",false,-1};
    AttrInfo b = {"ccc","name","char",false,100};
    AttrInfo c = {"ccc","nation","char",false,3};

    attrs[0] = a;
    attrs[1] = b;
    attrs[2] = c;
}
/*
*   string arr_name;
*   string type;
*   int size;
*   bool is_key;
*/

TableInfo info;
void init_tmp_create() {
    Attribution a = {"p_id","integer",true,-1};
    Attribution b = {"name","char",false,100};
    Attribution c = {"nation","char",false,3};
    info = {{a,b,c},"publisher",3};
}


Condition conditions[MAX_COND_COUNT];
int cond_count = 3;
DeleteQuery *deleteQuery = (DeleteQuery*)malloc(sizeof(DeleteQuery));

void initCondition(){
    Condition a = {E,"","name","Xiaohong",VARCHAR};
    Condition b = {G,"","rank","1",INTEGER};
    Condition c = {E,"","family","test",CHAR};
    conditions[0] = a;
    conditions[1] = b;
    conditions[2] = c;

    deleteQuery->tb_name = "customer";
    deleteQuery->cond_count = cond_count;
    deleteQuery->conditions = conditions;
}


// create table publisher(p_id integer key,name char(100),nation char(3));
string gen_create_stmt(){
    string stmt = "CREATE TABLE ";
    stmt.append(attrs[0].tb_name);
    stmt.append("(");
    for(int i = 0;i < attr_count;i++){
        stmt.append(attrs[i].attr_name);
        stmt.append(" ");
        stmt.append(attrs[i].type);
        stmt.append(" ");
        if(attrs[i].is_key){
            stmt.append("key");
        }
        if(attrs[i].size != -1){
            stmt.append("(");
            stmt.append(std::to_string(attrs[i].size));
            stmt.append(")");
        }
        stmt.append(",");
    }
    stmt.pop_back();
    stmt.append(")");
    stmt.append(";");
    return stmt;
}

string gen_tmp_create_stmt(TableInfo info){
    string stmt = "CREATE TABLE tmp_" + info.tb_name + "(";
    for(int i = 0; i < info.attr_count; i++){
        stmt.append(info.attrs[i].attr_name);
        stmt.append(" ");
        stmt.append(info.attrs[i].type);
        stmt.append(" ");
        if(info.attrs[i].is_key){
            stmt.append("key");
        }
        if(info.attrs[i].size != -1){
            stmt.append("(");
            stmt.append(std::to_string(info.attrs[i].size));
            stmt.append(")");
        }
        stmt.append(",");
    }
    stmt.pop_back();
    stmt.append(")");
    stmt.append(";");
    return stmt; 
}

string gen_condition_stmt(Condition *cond_list,int cond_count) {
    if (cond_count == 0 || cond_list == NULL)
        return "";
    string tmp_stmt;
    for(int i = 0;i < cond_count;i++){
        if(cond_list[i].table != ""){
            tmp_stmt.append(cond_list[i].table);
            tmp_stmt.append(".");
        }
        tmp_stmt.append(cond_list[i].column);
        tmp_stmt.append(" ");
        tmp_stmt.append(get_op_stmt(cond_list[i].op));
        tmp_stmt.append(" ");
        if (cond_list[i].type == VARCHAR || cond_list[i].type == CHAR){
            tmp_stmt.append("'");
            tmp_stmt.append(cond_list[i].value);
            tmp_stmt.append("'");
        }
        else
            tmp_stmt.append(cond_list[i].value);
        if(i != cond_count-1){
            tmp_stmt.append(" AND ");
        }
//        std::cout << tmp_stmt << std::endl;
    }
    return tmp_stmt;
}

string get_op_stmt(OP op) {
    string operation;
    switch(op) {
        case E:operation = "=";break;
        case NE:operation = "!=";break;
        case G:operation = ">";break;
        case GE:operation = ">=";break;
        case LE:operation = "<=";break;
        case L:operation = "<";break;
    }
    return operation;
}

// delete from publisher where id = 104001;
string gen_delete_stmt(){
    string stmt = "DELETE FROM ";
    stmt.append(deleteQuery->tb_name);
    stmt.append(" WHERE ");
    stmt.append(gen_condition_stmt(deleteQuery->conditions,deleteQuery->cond_count));
    stmt.append(";");
    return stmt;
}


struct ConditionH{
    bool is_needed;
    string attr_name;
    string operation;
    string attr_value;
};

struct ConditionV{
    bool is_needed;
    int attr_num;
    string attr_list[MAX_ATTR_COUNT];
};

struct ConditionHV{
    bool is_needed;
    ConditionH h1;
    ConditionH h2;
    ConditionV v1;
};


struct Fragment{
    string tb_name;
    // id means db nums ->site num
    int frag_count;
    ConditionHV conditions[MAX_FRAG_COUNT];
    // 似乎写错了。到时候加上
    string db_name;

};

Fragment fragment;
Fragment book;
Fragment orders;
Fragment customer;

void init_frag_info(){
    ConditionHV a = {
        true,
        ConditionH {true,"p_id","<","104000"},
        ConditionH {true,"nation","=","'PRC'"},
        ConditionV {false}
    };
    ConditionHV b = {
        true,
        ConditionH {true,"p_id","<","104000"},
        ConditionH {true,"nation","=","'USA'"},
        ConditionV {false}
    };
    ConditionHV c = {
        true,
        ConditionH {true,"p_id",">=","104000"},
        ConditionH {true,"nation","=","'PRC'"},
        ConditionV {false}
    };
    ConditionHV d = {
        true,
        ConditionH {true,"p_id",">=","104000"},
        ConditionH {true,"nation","=","'USA'"},
        ConditionV {false,}
    };
    fragment = {"publisher",4,{a,b,c,d}};

    //book
    ConditionHV b_a = {
        true,
        ConditionH {true,"b_id","<","205000"},
        ConditionH {false},
        ConditionV {false}
    };
    ConditionHV b_b = {
        true,
        ConditionH {true,"b_id",">=","205000"},
        ConditionH {true,"b_id","<","210000"},
        ConditionV {false}
    };
    ConditionHV b_c = {
        true,
        ConditionH {true,"b_id",">","210000"},
        ConditionH {false},
        ConditionV {false}
    };
    ConditionHV b_d = {
        false,
        ConditionH {false},
        ConditionH {false},
        ConditionV {false}
    };
    book = {"book",3,{b_a,b_b,b_c,b_d}};

    //orders
    ConditionHV o_a = {
        true,
        ConditionH {true,"c_id","<","307000"},
        ConditionH {true,"b_id","<","215000"},
        ConditionV {false}
    };
    ConditionHV o_b = {
        true,
        ConditionH {true,"c_id","<","307000"},
        ConditionH {true,"b_id",">=","215000"},
        ConditionV {false}
    };
    ConditionHV o_c = {
        true,
        ConditionH {true,"c_id",">=","307000"},
        ConditionH {true,"b_id","<","215000"},
        ConditionV {false}
    };
    ConditionHV o_d = {
        true,
        ConditionH {true,"c_id",">=","307000"},
        ConditionH {true,"b_id",">=","215000"},
        ConditionV {false}
    };
    orders = {"orders",4,{o_a,o_b,o_c,o_d}};


    // customer
    ConditionHV c_a = {
        true,
        ConditionH {false},
        ConditionH {false},
        ConditionV {true,2,{"c_id","name"}}
    };
    ConditionHV c_b = {
        true,
        ConditionH {false},
        ConditionH {false},
        ConditionV {true,2,{"c_id","rank"}}
    };
    ConditionHV c_c = {
        false,
        ConditionH {false},
        ConditionH {false},
        ConditionV {false}
    };
    ConditionHV c_d = {
        false,
        ConditionH {false},
        ConditionH {false},
        ConditionV {false}
    };
    customer = {"customer",4,{c_a,c_b,c_c,c_d}};
}








//init insert
string insert_records[MAX_ATTR_COUNT] = {"123","'High Education Press'","'USA'"};
int insert_record_count = 3;
string insert_table_name = "publisher";

string insert_records_book[MAX_ATTR_COUNT] = {"205001","'lalalala'","'xixixixixi'","104001","100"};
int book_count = 5;
string book_table = "book";

string insert_records_orders[MAX_ATTR_COUNT] = {"3004", "295001","5"};
int orders_count = 3;
string orders_table = "orders";

string insert_records_customer[MAX_ATTR_COUNT] = {"300001","'Xiaoming'","1"};
int customer_count = 3;
string customer_table = "customer";



// return which site you shoud insert the record
// fit horizontal
int compare(Fragment f,string insert_records[],int count){
    int k = -1;
    // already tested
    if(f.tb_name == "publisher"){
        // cout << 111 << endl;
        // cout << f.frag_count << endl;
        for(int i = 0;i < f.frag_count;i++){
            // 处理p_id
            int t_id = std::stoi(insert_records[0]);
            int f_id = std::stoi(f.conditions[i].h1.attr_value);
            // cout << "t_id:" << t_id << "f_id:" << f_id << endl;
            if(f.conditions[i].h1.operation == "<"){
                if(t_id >= f_id)
                    continue;
            }
            else if(f.conditions[i].h1.operation == ">="){
                if(t_id < f_id)
                    continue;
            }
            // 处理nation
            // cout << "k=" << k << endl;
            string t_nation = insert_records[2];
            string f_nation = f.conditions[i].h2.attr_value;
            // cout << t_nation << " : " << f_nation << endl;
            if(t_nation == f_nation){
                k = i;
                break;
            }else{
                continue;
            }
        }
    }
    // already tested
    if(f.tb_name == "book"){
        for(int i = 0; i < f.frag_count; i++){
            if(f.conditions[i].is_needed){
                if(f.conditions[i].h1.is_needed){
                    cout << insert_records[0] << endl;
                    cout << f.conditions[i].h1.attr_value << endl;
                    int t_b_id = std::stoi(insert_records[0]);
                    int f_b_id = std::stoi(f.conditions[i].h1.attr_value);
                    if(f.conditions[i].h1.operation == "<"){
                        if(t_b_id >= f_b_id)
                            continue;
                    }
                    else if(f.conditions[i].h1.operation == ">="){
                        if(t_b_id < f_b_id)
                            continue;
                    }
                }
                if(f.conditions[i].h2.is_needed){
                    int t_b_id = std::stoi(insert_records[0]);
                    int f_b_id = std::stoi(f.conditions[i].h2.attr_value);
                    if(f.conditions[i].h2.operation == ">="){
                        if(t_b_id < f_b_id)
                            continue;
                    }
                    else if(f.conditions[i].h2.operation == "<"){
                        if(t_b_id >= f_b_id)
                            continue;
                    }
                }
            }
            k = i;
            break;
        }
    }

    // already tested
    if(f.tb_name == "orders"){
        for(int i = 0; i < f.frag_count; i++){
            // h1 c_id
            int h1_t_id = std::stoi(insert_records[0]);
            int h1_f_id = std::stoi(f.conditions[i].h1.attr_value);
            cout << h1_t_id << " : " << h1_f_id << endl;

            if(f.conditions[i].h1.operation == "<"){
                if(h1_t_id >= h1_f_id){
                    continue;
                }
            }
            else if(f.conditions[i].h2.operation == ">="){
                if(h1_t_id < h1_f_id){
                    continue;
                }
            }
            // h2 b_id
            int h2_t_id = std::stoi(insert_records[1]);
            int h2_f_id = std::stoi(f.conditions[i].h2.attr_value);
             // cout << h2_t_id << " : " << h2_f_id << endl;


            if(f.conditions[i].h2.operation == "<"){
                if(h2_t_id >= h2_f_id){
                    // cout << "1111" << endl;
                    continue;
                }
            }
            else if(f.conditions[i].h2.operation == ">="){
                if(h2_t_id < h2_f_id){
                    // cout << "2222" << endl;
                    continue;
                }
            }
            k = i;
            break;
        }
    }
    return k;
}

string gen_insert_stmt(string insert_records[],int insert_record_count,string tmp){
    string stmt = "INSERT INTO "+ tmp + insert_table_name + " VALUES(";
    for(int i = 0; i < insert_record_count; i++){
        stmt += insert_records[i];
        stmt += ",";
    }
    stmt.pop_back();
    stmt += ");";
    return stmt;
}

// 失误了，应该用列的编号的
// struct VInsert{
//     string stmt;
//     int siteID;
// }

// vector<string> v_insert(Fragment fragment,stirng insert_records[],int insert_record_count){

// }

vector<string> v_insert(Fragment fragment, string insert_records[], int insert_record_count){
    vector<string> stmts;
    if(insert_record_count < 3){
        return stmts;
    }

    string stmt1 = "INSERT INTO " 
    + fragment.tb_name
    +"( c_id,name) VALUES ("
    +insert_records[0]
    +", "
    +insert_records[1]
    +");";

    string stmt2 = "INSERT INTO " 
    + fragment.tb_name
    +"( c_id,rank) VALUES ("
    +insert_records[0]
    +", "
    +insert_records[2]
    +");";

    stmts.push_back(stmt1);
    stmts.push_back(stmt2);
    return stmts;
}





string sites[] = {"192.168.1.101","192.168.1.102","192.168.1.103","192.168.1.104"};

// 首先根据表的名称进行选择，判断选择垂直还是水平分片
void insert(){
    // 水平分片
    {
        string insert_stmt = gen_insert_stmt(insert_records, insert_record_count, "");
        int k = compare(fragment,insert_records,3);
        string site = sites[k];
        cout << "insert_stmt" << endl;
        cout << insert_stmt << endl;
        cout << "site" << endl;
        cout << site << endl;
    }
    {
        string stmt = gen_insert_stmt(insert_records_book, book_count, "");
        int k = compare(book,insert_records_book,book_count);
        cout << "stmt: " <<  stmt << endl;
        cout << "site: " << k << endl;

    }
    {
        string stmt = gen_insert_stmt(insert_records_orders, orders_count, "");
        int k = compare(orders,insert_records_orders,orders_count);
        cout << "stmt: " <<  stmt << endl;
        cout << "site: " << k << endl;
    }

    // 垂直分片
    {
        vector<string>stmts = v_insert(customer,insert_records_customer,customer_count);
        for(string s:stmts){
            cout << s << endl;
        }
    }


}



// load 语法
// load tablename filepath
struct Load{
    string tb_name;
    string filepath;
};

// where h1.attr_name h1.attr_condition h1.attr_value AND ... 
// only fit to hor
string gen_con_by_con(ConditionHV hv){
    string str = "";
    if(hv.is_needed){
        if(hv.h1.is_needed){
            str = str 
            + hv.h1.attr_name 
            + " " 
            + hv.h1.operation
            + " "
            + hv.h1.attr_value;
        }
        if(hv.h2.is_needed){
            str = str 
            + " AND "
            + hv.h2.attr_name
            + " "
            + hv.h2.operation
            + " "
            + hv.h2.attr_value;
        }
    }
    return str;
}

// 水平分片的表
vector<string> gen_select_by_frag(Fragment fragment,string tmp_tb_name){
    vector<string>stmts;
    for(int i = 0; i < fragment.frag_count; i++){
        string str = "SELECT * FROM " 
        + tmp_tb_name
        + " WHERE " 
        + gen_con_by_con(fragment.conditions[i]);
        +";";
        stmts.push_back(str); 
    }
    return stmts;
}

vector<string> gen_v_select_by_frag(Fragment fragment,string tmp_tb_name){
    vector<string>stmts;
    for(int i = 0;i < fragment.frag_count;i++){
        string stmt = "SELECT ";
        if(fragment.conditions[i].is_needed && fragment.conditions[i].v1.is_needed){
            for(int j = 0; j < fragment.conditions[i].v1.attr_num; j++){
                // cout << fragment.conditions[i].v1.attr_list[i] << endl;
                stmt += fragment.conditions[i].v1.attr_list[j];
                stmt += ",";
            }
            stmt.pop_back();
            stmt += " FROM ";
            stmt += tmp_tb_name;
            stmt += ";";
            stmts.push_back(stmt);
        }
        
    }
    return stmts;
}



vector<string> Split(string& src,const string& separator){
    vector<string> dest;
    string str = src;
    string substring;
    int start = 0, index;
    dest.clear();
    index = str.find_first_of(separator,start);
    do
    {
        if (index != string::npos)
        {    
            substring = str.substr(start,index-start );
            dest.push_back(substring);
            start =index+separator.size();
            index = str.find(separator,start);
            if (start == string::npos) break;
        }
    }while(index != string::npos);

    substring = str.substr(start);
    dest.push_back(substring);
    return dest;
}

string gen_batch_insert(TableInfo info,string src){
    string stmt = "INSERT INTO " + info.tb_name + " VALUES";
    string str = src;
    vector<string>dest = Split(str,"\n");
    for(auto it = dest.begin(); it != dest.end(); it++){
        stmt += "( " + *it + " ),";
    }
    stmt.pop_back();
    stmt += ";";
    return stmt;
}

// 写死了，利用id
string gen_batch_insert_v(string src,int c){
    string str = src;
    string stmt = "INSERT INTO ";
    if(0 == c){
        stmt += "customer(c_id,name)VALUES";
        vector<string>dest = Split(str,"\n");
        for(auto it = dest.begin(); it != dest.end(); it++){
            stmt += "( " + *it + " ),";
        }
        stmt.pop_back();
        stmt += ";";
    }
    else if(1 == c){
        stmt += "customer(c_id,rank)VALUES";
        vector<string>dest = Split(str,"\n");
        for(auto it = dest.begin(); it != dest.end(); it++){
            stmt += "( " + *it + " ),";
        }
        stmt.pop_back();
        stmt += ";";
    }
    return stmt; 
}

string gen_drop_tmp_table(string tmp_tb_name){
    return "DROP TABLE " + tmp_tb_name + ";";
}

// 1、首先创建临时表——根据表名和表的信息
// 2、把数据导入到本地数据库的临时表中
// 3、首先根据分片的信息生成与之相对应的select语句
// 4、根据得到的结果和分片的信息生成批量的插入语句
// 5、并行的执行RPC的插入语句
// 6、删除本地的tmp表

void load_data(){
    // 初始化表的信息
    // info
    init_tmp_create();
    Load load = {"publisher","/var/lib/mysql-files/data/publisher.tsv"};
    string create_tmp = gen_tmp_create_stmt(info);
    cout << create_tmp << endl;
    // localExecute("checkin",create_tmp);
    string tmp_tb_name = "tmp_" + load.tb_name;
    cout << "table_name: " << load.tb_name << endl;
    cout << "create_stmt: " << create_tmp << endl;
    // 导入临时数据
    // localInsertTable("checkin",load.filepath,tmp_tb_name);
    //  根据分片信息生成查询的语句
    vector<string> aaa = gen_v_select_by_frag(customer,"tmp_customer");
    for(auto i = aaa.begin(); i != aaa.end(); i++){
        cout << *i << endl;
    }
    // vector<string> stmts = gen_select_by_frag(fragment,tmp_tb_name);
    // for(string str : stmts){
    //     cout << str << endl;
    // }
    // test start
    // string str = stmts[0];
    // string str = "SELECT * FROM tmp_publisher WHERE p_id < 100020 AND nation = 'PRC';";
    // string res = localExecuteQuery("checkin",str);
    // cout << str << endl;
    // cout << res << endl;
    // test end
    // 根据查询的结果生成批量插入的语句
    // string rpc_insert_stmt = gen_batch_insert(info,res);
    // cout << rpc_insert_stmt << endl;
    // 插入到远端的DB中
    // RPCExecute("127.0.0.1","checkin",rpc_insert_stmt);
    // 删除临时表
    // string drop_tmp_table = gen_drop_tmp_table(tmp_tb_name);
    // cout << drop_tmp_table << endl;
    // localExecute("checkin",drop_tmp_table);

}









int main(){
    // initCondition();
       // string con = gen_condition_stmt(deleteQuery->conditions,deleteQuery->cond_count);
    // string del_stmt = "delete from publisher where nation = 'PRC';";
    // initCreate();
    // string del_stmt = gen_create_stmt();
    // RPCExecute("127.0.0.1","checkin",del_stmt);
    // localExecute("checkin",del_stmt);
    // std::cout << del_stmt << std::endl;
    // bool localInsertTable(string db_name,string sql_flle,string table_name) 
    // bool res = localInsertTable("checkin","/var/lib/mysql-files/data/customer.tsv","customer");
    // if(res)
    //     std::cout << "ok" << std::endl;
    // else
    //     std::cout << "no" << std::endl;
    // string s = localExecuteQuery("checkin","select * from customer");
    // cout << s << endl;
    // init_tmp_create();
    // cout << gen_tmp_create_stmt() << endl;
    init_frag_info();
    // insert();
    // cout << gen_tmp_insert_stmt() << endl;
    // cout << compare(fragment,insert_records,3) << endl;

    load_data();
    return 0;
}













































