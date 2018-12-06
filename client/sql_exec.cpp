#include "sql_exec.h"



int compare(Fragment1 f,string insert_records[]);


vector<string> ip_vec = {"127.0.0.1","127.0.0.1","127.0.0.1","127.0.0.1"};
vector<string> db_names = {"db1","db2","db3","db4"};
string me_ip = "127.0.0.1";
string me_db = "db1";

void load_data(TableInfo info, Fragment1 fragment, Load load){
    cout << "LOAD DATA" << endl;
    // 新建临时表
    string create_tmp = gen_tmp_create_stmt(info);
    localExecute(me_db,create_tmp);
    cout << "CREATE TEMP TBALE" << endl;
    // 获取临时表名
    string tmp_tb_name = "tmp_" + load.tb_name;
    // 导入临时数据
    localInsertTable(me_db,load.filepath,tmp_tb_name);
    cout << "LOAD DATA INTO TEMP TABLE" << endl;
    //  根据分片信息生成查询的语句
    // 垂直分片
    if(load.tb_name == "customer"){
        vector<string> stmts = gen_v_select_by_frag(fragment,tmp_tb_name);
        for(int i = 0;i < stmts.size();i++){
            string str = stmts[i];
            // string str = "SELECT c_id,name FROM tmp_customer WHERE c_id < 300003;";
            string res = localExecuteQuery(me_db,str);
            string rpc_insert_stmt = gen_batch_insert_v(res,i);
            RPCExecute(ip_vec[i],db_names[i],rpc_insert_stmt);
        }
    }
    // 水平分片
    else{
        vector<string> stmts = gen_select_by_frag(fragment,tmp_tb_name);
        cout << "SIZE = " << stmts.size() << endl;
        for(int i = 0; i < stmts.size(); i++){
            string res = localExecuteQuery(me_db,stmts[i]);
            string rpc_insert_stmt = gen_batch_insert(info,res);
            RPCExecute(ip_vec[i],db_names[i],rpc_insert_stmt);

        }
    }

    string drop_tmp_table = gen_drop_tmp_table(tmp_tb_name);
    localExecute(me_db,drop_tmp_table);
    cout << "DROP TEMP TABLE" << endl;
    cout << "DONE" << endl;
}


void create_table(Table &table){
    cout << "CREATE TABLE" << endl;
    string stmt = gen_create_stmt(table);
    for(int i = 0;i < ip_vec.size(); i++){
        RPCExecute(ip_vec[i],db_names[i],stmt);
        cout << ip_vec[i] << " : " << db_names[i] << " : "<< stmt << endl;
    }
    cout << "DONE" << endl;
}


void delete_table(DeleteQuery deleteQuery){
    cout << "DELETE FROM TABLE" << endl;
    string stmt = gen_delete_stmt(deleteQuery);
    for(int i = 0;i < ip_vec.size(); i++){
        RPCExecute(ip_vec[i],db_names[i],stmt);
        cout << ip_vec[i] << " : " << db_names[i] << " : "<< stmt << endl;
    }
    cout << "DONE" << endl;
}



void insert_table(Insert &insert,Fragment1 fragment,int values_count){
    cout << "INSERT INTO TABLE" << endl;
    if(insert.tb_name == "customer"){
        vector<string>stmts = v_insert(fragment,insert.valuesList,values_count);
        for(int i = 0;i < stmts.size();i++){
            RPCExecute(ip_vec[i],db_names[i],stmts[i]);
            cout << ip_vec[i] << " : " << db_names[i] << " : "<< stmts[i] << endl;
        }
    }
    else{
        string stmt = gen_insert_stmt(insert.valuesList, values_count, "",insert.tb_name);
        cout << stmt << endl;
        int k = compare(fragment, insert.valuesList);
        // RPCExecute(ip_vec[k],db_names[k],stmt);
        cout << ip_vec[k] << " : " << db_names[k] << " : "<< stmt << endl;
    }
    cout << "DONE" << endl;
}




// return which site you shoud insert the record
// fit horizontal
int compare(Fragment1 f,string insert_records[]){
    int k = -1;
    // already tested
    if(f.tb_name == "publisher"){
        // cout << 111 << endl;
        cout << f.frag_count << endl;
        for(int i = 0;i < f.frag_count;i++){
            // 处理p_id
            cout << f.conditions[i].h1.attr_value << endl;
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
            cout << f.conditions[i].h2.attr_value << endl;
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


bool CreateParser(std::string createSql,Table& sql){
    std::regex createPattern("create\\s+table\\s+(\\w+)\\((.*)\\);");
    std::smatch createResults;
    std::string splitAtt;
    if (regex_match(createSql, createResults, createPattern)){
        sql.tb_name = createResults[1];
        splitAtt = createResults[2];
    }else{
        //std::cout<<"Parsing error"<<std::endl;
        return false;
    }


    std::regex table_att_pattern("(\\w+)\\s+([\\w\\(\\)]+)(\\s+key)?");
    std::string::const_iterator att_Start = splitAtt.begin();
    std::string::const_iterator att_End = splitAtt.end();
    std::smatch attTemp_result;
    std::string attTemp[5];
    while (regex_search(att_Start, att_End, attTemp_result, table_att_pattern))
    {
        attTemp[sql.attr_count++] = attTemp_result[0];
        att_Start = attTemp_result[0].second;
    }
    //cout<<sql.attr_count<<endl;

    std::regex attTyPattern("(\\w+)\\s+([\\w\\(\\)]+)(\\s+key)?");
    std::smatch attTyResults;
    for( int i = 0 ; i < sql.attr_count ; i++ ){
        if (regex_match(attTemp[i], attTyResults, attTyPattern)){
            sql.attrs[i].attr_name = attTyResults[1];
            sql.attrs[i].type = attTyResults[2];
            if(attTyResults[3] == " key")
                sql.attrs[i].is_key = 1;
        }
    }

    for( int i = 0 ; i < sql.attr_count ; i++ ){
        if(sql.attrs[i].type == "integer"){
            sql.attrs[i].size = -1;
        }else{
            std::regex sizePattern("char\\((\\d+)\\)");
            std::smatch sizeResults;
            if(regex_match(sql.attrs[i].type, sizeResults, sizePattern)){
                std::string sizetemp = sizeResults[1];
                std::stringstream stream(sizetemp);
                stream>>sql.attrs[i].size;
                sql.attrs[i].type = "char";
            }else{std::cout<<"error type size"<<std::endl;}
        }
    }
    // std::cout<<sql.tb_name<<" "<<sql.attr_count<<std::endl;
    // for( int i = 0 ; i < sql.attr_count ; i++ ) {
    //     std::cout << sql.attrs[i].attr_name<<" ";
    //     std::cout << sql.attrs[i].type<<" ";
    //     std::cout << sql.attrs[i].size<<" ";
    //     std::cout << sql.attrs[i].is_key<<std::endl;
    // }
    return true;

}


void printCreate(Table& sql){
    std::cout<<sql.tb_name<<" "<<sql.attr_count<<std::endl;
    for( int i = 0 ; i < sql.attr_count ; i++ ) {
        std::cout << sql.attrs[i].attr_name<<" ";
        std::cout << sql.attrs[i].type<<" ";
        std::cout << sql.attrs[i].size<<" ";
        std::cout << sql.attrs[i].is_key<<std::endl;
    }
}

bool InsertParser(std::string insertStr , Insert& sql){
    std::regex insertPattern("insert\\s+into\\s+(\\w+)\\s+values\\((.*)\\);");
    std::smatch insertResults;
    std::string value_list;
    if (regex_match(insertStr, insertResults, insertPattern)){
        sql.tb_name = insertResults[1];
        value_list = insertResults[2];
        std::smatch val_result;
        std::regex val_pattern("'?\\w+'?");
        std::string::const_iterator val_Start = value_list.begin();
        std::string::const_iterator val_End = value_list.end();
        while (regex_search(val_Start, val_End, val_result, val_pattern))
        {
            sql.valuesList[sql.values_count++] = val_result[0];
            val_Start = val_result[0].second;
        }
    }else{
        //std::cout<<"Parsing error"<<std::endl;
        return false;
    }
    return true;
}


void printInsert(Insert &sql){
    std::cout<<sql.tb_name<<" "<<sql.values_count<<std::endl;
    for(int i = 0 ; i < sql.values_count ; i++ ){
        std::cout<<sql.valuesList[i]<<std::endl;
    }
}


bool deleteParser(std::string str , DeleteQuery& sql){
    std::regex delPattern("delete\\s+from\\s+(\\w+)\\s+where\\s+(.*);");
    std::regex del_no_where_Pattern("delete\\s+from\\s+(\\w+);");
    std::smatch delResults;
    std::smatch del_no_where_Result;
    std::string del_con;
    if(regex_match(str, del_no_where_Result, del_no_where_Pattern)){
        sql.tb_name = del_no_where_Result[1];
        sql.cond_count = 0;
    }else if(regex_match(str, delResults, delPattern)){
        sql.tb_name = delResults[1];
        del_con = delResults[2];
        std::string del_temp[15];
        parserCon( del_con , del_temp);
        int m = 0;
        for( int i = 0 ; i < lenn(del_temp) ; m++ ){
            sql.conditions[m].column = del_temp[i++];
            if(del_temp[i] == "="){
                sql.conditions[m].op = E;
            }
            else if(del_temp[i] == ">"){
                sql.conditions[m].op = G;
            }
            else if(del_temp[i] == ">="){
                sql.conditions[m].op = GE;
            }
            else if(del_temp[i] == "<="){
                sql.conditions[m].op = LE;
            }
            else if(del_temp[i] == "<"){
                sql.conditions[m].op = L;
            }
            i++;
            //sql.conditions[m].op = del_temp[i++];
            sql.conditions[m].value = del_temp[i++];
            sql.cond_count++;
        }
    }else{
        //std::cout<<"Parsing error"<<std::endl;
        return false;
    }
    return true;
}

void printDeleteParser(DeleteQuery& sql){
    std::cout<<sql.tb_name<<" "<<sql.cond_count<<std::endl;
    for( int i = 0 ; i < sql.cond_count ; i++ ){
        std::cout<<sql.conditions[i].column<<" ";
        std::cout<<sql.conditions[i].op<<" ";
        std::cout<<sql.conditions[i].value<<std::endl;
    }
}

void parserCon( std::string instr, std::string outstr[]){
    instr = instr+" and";
    std::smatch outTemp_result;
    std::regex instr_pattern("\\w+\\s+[\\>=\\=\\<\\>\\<=]+\\s+'?\\w+'?\\s+and");
    std::string::const_iterator instr_Start = instr.begin();
    std::string::const_iterator instr_End = instr.end();
    std::string outTemp[5];
    int Att_index = 0;
    while (regex_search(instr_Start, instr_End, outTemp_result, instr_pattern))
    {
        outTemp[Att_index] = outTemp_result[0];
        outTemp[Att_index] = outTemp[Att_index].substr(0, outTemp[Att_index].length() - 4);
        Att_index++;
        instr_Start = outTemp_result[0].second;
    }
    std::smatch outstr_result;
    std::regex out_pattern("(\\w+)\\s+([\\>=\\=\\<\\>\\<=]+)\\s+('?\\w+'?)");
    int outstr_index=0;
    for(int outstr_indexi = 0 ; outstr_indexi < lenn(outTemp) ; outstr_indexi++ ){
        regex_match(outTemp[outstr_indexi], outstr_result, out_pattern);
        outstr[outstr_index++] = outstr_result[1];
        outstr[outstr_index++] = outstr_result[2];
        outstr[outstr_index++] = outstr_result[3];
    }
}

std::string trimSpace(std::string s) {
    if (s.empty())
        return s;
    std::string::size_type i = 0, j = 0;
    while (i < s.size()) {
        j = s.find_first_of(" ", i);
        if (j > s.size())
            break;
        s.erase(j, 1);
        i++;
    }
    return s;
}

int lenn(std::string a[]){
    int c=0;
    while(a[c] != "\0"){c++;}
    return c;
}

































