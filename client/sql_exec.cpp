#include "sql_exec.h"



int compare(Fragment1 f,string insert_records[]);
vector<string> ip_vec = {"127.0.0.1","127.0.0.1","127.0.0.1","127.0.0.1"};
vector<string> db_names = {"db1","db2","db3","db4"};
vector<int> ports = {8080,8080,8080,8080};
string me_ip = "127.0.0.1";
string me_db = "db1";
int me_port = 8080;


void setSiteInfos(SiteInfos infos) {
    for(int i = 0;i < infos.sitenum; i++) {
        int id = infos.site[i].siteID - 1;
        ip_vec[id] = infos.site[i].ip;
        db_names[id] = infos.site[i].db_name;
        ports[id] = std::stoi(infos.site[i].port);
    }
}

void setMe(string ip,int port,string db_name){
    me_port = port;
    me_ip = ip;
    me_db = db_name;
}

void printMessage(){
    for(int i = 0; i < 4; i++) {
        cout << ip_vec[i] << ":" << ports[i] << " : " << db_names[i] << endl;
    }
    // cout << "me" << endl;
    // cout << me_ip << ":" << me_port << " : " << me_db << endl;
}

void load_data(Table info, Fragment1 fragment, Load load){
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
    if(load.tb_name == "Course"){
        vector<string> stmts = gen_v_select_by_frag(fragment,tmp_tb_name);
        for(int i = 0;i < stmts.size();i++){
            string str = stmts[i];
            string res = localExecuteQuery(me_db,str);
            string rpc_insert_stmt = gen_batch_insert_v(res,i);
            RPCExecute(ip_vec[i],ports[i],db_names[i],rpc_insert_stmt);
        }
    }
    // 水平分片
    else{
        vector<string> stmts = gen_select_by_frag(fragment,tmp_tb_name);
        // cout << "SIZE = " << stmts.size() << endl;
        for(int i = 0; i < stmts.size(); i++){
            // string res = "   1111";
            string res = localExecuteQuery(me_db,stmts[i]);
            cout << "num[" << i << "]: " <<  stmts[i] << endl;
            if(res == ""){
                continue;
            }
            // cout <<"dadad" << res <<"dasdasad"<< endl;
            string rpc_insert_stmt = gen_batch_insert(info,res);
            // int id = fragment.conditions[i].db_id - 1; 
            // cout << "num[" << i << "]: " <<  rpc_insert_stmt << endl;
            RPCExecute(ip_vec[i],ports[i],db_names[i],rpc_insert_stmt);
        }
    }

    string drop_tmp_table = gen_drop_tmp_table(tmp_tb_name);
    localExecute(me_db,drop_tmp_table);
    cout << "DROP TEMP TABLE" << endl;
    cout << "DONE" << endl;
}


void create_table(Table table){
    cout << "CREATE TABLE" << endl;
    string stmt = gen_create_stmt(table);
    for(int i = 0;i < ip_vec.size(); i++){
        RPCExecute(ip_vec[i],ports[i],db_names[i],stmt);
        cout << ip_vec[i] << " : " << db_names[i] << " : "<< stmt << endl;
    }
    cout << "DONE" << endl;
}


void delete_table(DeleteQuery deleteQuery){
    cout << "DELETE FROM TABLE" << endl;
    if(deleteQuery.tb_name == "Course"){
        string t  = "SELECT id from Course WHERE " + gen_condition_stmt(deleteQuery.conditions,deleteQuery.cond_count);
        // cout << t << endl;
        string res = RPCExecuteQuery(ip_vec[1],ports[1],db_names[1],t);
        vector<string> a = Split(res,"\n"); 
        string s = "DELETE FROM Course WHERE id = ";
        for(int i = 0; i < a.size(); i++) {
            s += a[i];
        }
        s += ";";
        cout << s << endl;
        RPCExecute(ip_vec[0],ports[0],db_names[0],s);
        RPCExecute(ip_vec[1],ports[1],db_names[1],s);
        return;
    }
    string stmt = gen_delete_stmt(deleteQuery);
    for(int i = 0;i < ip_vec.size(); i++){
        RPCExecute(ip_vec[i],ports[i],db_names[i],stmt);
        cout << ip_vec[i] << " : " << db_names[i] << " : "<< stmt << endl;
    }
    cout << "DONE" << endl;
}



void insert_table(Insert &insert,Fragment1 fragment){
    cout << "INSERT INTO TABLE" << endl;
    if(insert.tb_name == "Course"){
        vector<string>stmts = v_insert(fragment,insert.valuesList,insert.values_count);
        for(int i = 0;i < stmts.size();i++){
            RPCExecute(ip_vec[i],ports[i],db_names[i],stmts[i]);
            cout << ip_vec[i] << " : " << db_names[i] << " : "<< stmts[i] << endl;
        }
    }
    else{
        string stmt = gen_insert_stmt(insert.valuesList, insert.values_count, "",insert.tb_name);
        cout << stmt << endl;
        int k = compare(fragment, insert.valuesList) - 1;
        RPCExecute(ip_vec[k],ports[k],db_names[k],stmt);
        cout << ip_vec[k] << " : " << db_names[k] << " : "<< stmt << endl;
    }
    cout << "DONE" << endl;
}




// return which site you shoud insert the record
// fit horizontal
int compare(Fragment1 f,string insert_records[]){
    int k = -1;
    // already tested
    if(f.tb_name == "Teacher"){
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
            // 处理title
            cout << f.conditions[i].h2.attr_value << endl;
            int t_title = std::stoi(insert_records[2]);
            int f_title = std::stoi(f.conditions[i].h2.attr_value);
            if(f.conditions[i].h2.operation == "<>" && t_title != f_title){
                k = f.conditions[i].db_id;
                break;
            } else if(f.conditions[i].h2.operation == "=" && t_title == f_title) {
                k = f.conditions[i].db_id;
                break;
            }
            // cout << t_nation << " : " << f_nation << endl;
            // if(t_nation == f_nation){
            //     k = i;
            //     break;
            // }else{
            //     continue;
            // }
        }
    }
    // already tested
    if(f.tb_name == "Student"){
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
            // cout << "i:" << i << endl;
            k = f.conditions[i].db_id;
            break;
        }
    }

    // already tested
    if(f.tb_name == "Exam"){
        for(int i = 0; i < f.frag_count; i++){
            // h1 c_id
            int h1_t_id = std::stoi(insert_records[0]);
            int h1_f_id = std::stoi(f.conditions[i].h1.attr_value);
            cout <<"111 "<< h1_t_id << " : " << h1_f_id << endl;

            if(f.conditions[i].h1.operation == "<"){
                if(h1_t_id >= h1_f_id){
                    continue;
                }
            }
            else if(f.conditions[i].h1.operation == ">="){
                if(h1_t_id < h1_f_id){
                    continue;
                }
            }
            // cout << "iiii: " << i << endl;
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
            k = f.conditions[i].db_id;
            break;
        }
    }
    return k;
}


bool CreateParser(std::string createSql,Table &sql){
    std::regex createPattern("create\\s+table\\s+(\\w+)\\((.*)\\);");
    std::smatch createResults;
    std::string splitAtt;
    if (regex_match(createSql, createResults, createPattern)){
        sql.tb_name = createResults[1];
        splitAtt = createResults[2];
    }else{
        //std::cout<<"Parsing error"<<std::endl;
        // return sql;
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
        if(sql.attrs[i].type == "int"){
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
        std::regex val_pattern("'?[\\w\\s\\.\\-]+'?");
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
            else if(del_temp[i] == "<>"){
                sql.conditions[m].op = NE;
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
    std::regex instr_pattern("\\w+\\s+[\\>=\\=\\<\\>\\<=]+\\s+'?[\\w\\s+\\-\\.]+'?\\s+and");
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
    std::regex out_pattern("(\\w+)\\s+([\\>=\\=\\<\\>\\<=]+)\\s+('?[\\w\\s+\\-\\.]+'?)");
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



void sel_parser(std::string sel_item ,SelectQuery& sql){
    std::regex sel_item_Pattern("[\\w]+\\.[\\w]+");
    std::smatch sel_item_Results;
    std::string::const_iterator start = sel_item.begin();
    std::string::const_iterator end = sel_item.end();
    std::string selectTempo[7];
    int i = 0 ;
    while(regex_search(start, end, sel_item_Results, sel_item_Pattern))
    {
        std::string msg(sel_item_Results[0].first, sel_item_Results[0].second);
        selectTempo[i] = msg;
        i++;
        start = sel_item_Results[0].second;
    }
    int c=0;
    while(selectTempo[c] != "\0"){c++;}
    sql.sel_count = c;
    for(int i = 0 ; i < c ; i++ ){
        int index = selectTempo[i].find_first_of(".");
        sql.SelList[i].table_name = selectTempo[i].substr(0,index);
        sql.SelList[i].col_name = selectTempo[i].substr(index+1);
    }

}
void fro_parser(std::string fro_item,SelectQuery& sql){
    std::regex fro_item_Pattern("[\\w]+,");
    std::smatch fro_item_Results;
    std::string::const_iterator start = fro_item.begin();
    std::string::const_iterator end = fro_item.end();
    std::string fromTempo[7];
    int i = 0 ;
    while(regex_search(start, end, fro_item_Results, fro_item_Pattern))
    {
        std::string msg(fro_item_Results[0].first, fro_item_Results[0].second);
        fromTempo[i] = msg;
        i++;
        start = fro_item_Results[0].second;
    }
    int c=0;
    while(fromTempo[c] != "\0"){c++;}
    sql.from_count = c;
    for(int i = 0 ; i < c ; i++ ){
        fromTempo[i].pop_back();
        sql.FromList[i].tb_name = fromTempo[i];
    }
}
void whe_parser(std::string whe_item,SelectQuery& sql){
    std::regex whe_item_Pattern("(and\\s+[\\w]+\\.[\\w]+[\\>=\\=\\<\\>\\<=]+[\\w]+\\.[\\w]+)|"
                                "(and\\s+[\\w]+\\.[\\w]+[\\>=\\=\\<\\>\\<=]+[\\d]+)|"
                                "(and\\s+[\\w]+\\.[\\w]+[\\>=\\=\\<\\>\\<=]+'[\\w\\-\\.\\s+]+')");
    std::smatch whe_item_Results;
    std::string::const_iterator start = whe_item.begin();
    std::string::const_iterator end = whe_item.end();
    std::string whereTempo[20];
    int i = 0 ;
    while(regex_search(start, end, whe_item_Results, whe_item_Pattern))
    {
        std::string msg(whe_item_Results[0].first, whe_item_Results[0].second);
        whereTempo[i] = msg;
        i++;
        start = whe_item_Results[0].second;
    }

    int c=0;
    while(whereTempo[c] != "\0"){c++;}

    for(int i = 0 ; i < c ; i++ ){
        whereTempo[i] = whereTempo[i].substr(4);
    }

    std::regex joinPattern("(\\w+)\\.(\\w+)=(\\w+)\\.(\\w+)");
    std::regex comIntPattern("(\\w+)\\.(\\w+)([\\>=\\=\\<\\>\\<=]+)([\\w]+)");
    std::regex comStrPattern("(\\w+)\\.(\\w+)([\\>=\\=\\<\\>\\<=]+)('[\\w\\-\\.\\s+]+')");
    std::smatch joinResults;
    std::smatch comIntResults;
    std::smatch comStrResults;
    int joinC = 0 ;
    int condC = 0 ;

    for(int i = 0 ; i < c ; i++ ){
        if (regex_match(whereTempo[i], joinResults, joinPattern)){
            sql.JoinList[joinC].op = E;
            sql.JoinList[joinC].tb_name1 = joinResults[1];
            sql.JoinList[joinC].col_name1 = joinResults[2];
            sql.JoinList[joinC].tb_name2 = joinResults[3];
            sql.JoinList[joinC].col_name2 = joinResults[4];
            sql.join_count = ++joinC;
            continue;
        }
        if (regex_match(whereTempo[i], comIntResults, comIntPattern)){
            sql.CondList[condC].tb_name = comIntResults[1];
            sql.CondList[condC].col_name = comIntResults[2];
            if(comIntResults[3] == "=") sql.CondList[condC].op = E;
            if(comIntResults[3] == ">") sql.CondList[condC].op = G;
            if(comIntResults[3] == ">=") sql.CondList[condC].op = GE;
            if(comIntResults[3] == "<") sql.CondList[condC].op = L;
            if(comIntResults[3] == "<=") sql.CondList[condC].op = LE;
            if(comIntResults[3] == "<>") sql.CondList[condC].op = NE;
            sql.CondList[condC].value = comIntResults[4];
            sql.cond_count = ++condC;
            continue;
        }
        if (regex_match(whereTempo[i], comStrResults, comStrPattern)){
            sql.CondList[condC].tb_name = comStrResults[1];
            sql.CondList[condC].col_name = comStrResults[2];
            if(comStrResults[3] == "=") sql.CondList[condC].op = E;
            if(comStrResults[3] == ">") sql.CondList[condC].op = G;
            if(comStrResults[3] == ">=") sql.CondList[condC].op = GE;
            if(comStrResults[3] == "<") sql.CondList[condC].op = L;
            if(comStrResults[3] == "<=") sql.CondList[condC].op = LE;
            if(comStrResults[3] == "<>") sql.CondList[condC].op = NE;
            sql.CondList[condC].value = comStrResults[4];
            sql.cond_count = ++condC;
            continue;
        }
    }


}

bool SelectParser(std::string selectSql,SelectQuery &sql){

    std::string sel_item,fro_item,whe_item;

    std::regex selectPattern("select(.*)from(.*)where(.*)");
    std::regex select_no_whe_Pattern("select(.*)from\\s+(\\w+);");
    std::smatch selectResults;
    std::smatch select_no_whe_Results;

    if(regex_match(selectSql, select_no_whe_Results, select_no_whe_Pattern)){
        sql.FromList[0].tb_name = select_no_whe_Results[2];
        sql.from_count =1;
        sel_item = select_no_whe_Results[1];
        if(sel_item == " * "){
            std::string atttemp[10];
            Get_TableAttrs(sql.FromList[0].tb_name,atttemp);

            for( int attindex = 0 ;attindex<lenn(atttemp);attindex++){
                sql.sel_count++;
                sql.SelList[attindex].table_name = sql.FromList[0].tb_name;
                sql.SelList[attindex].col_name = atttemp[attindex];
            }
            return true;
        }
        sel_item = trimSpace(sel_item+',');
        sel_parser(sel_item ,sql);
    }else if(regex_match(selectSql, selectResults, selectPattern)){
        sel_item = selectResults[1];
        fro_item = selectResults[2];
        whe_item = selectResults[3];
        sel_item = trimSpace(sel_item+',');
        fro_item = trimSpace(fro_item+',');
        whe_item.pop_back();
        whe_item = "and"+ whe_item;
        fro_parser(fro_item,sql);
        if(sel_item == "*,"){
            std::string atttemp[10];
            Get_TableAttrs(sql.FromList[0].tb_name,atttemp);
            for( int attindex = 0 ;attindex<lenn(atttemp);attindex++){
                sql.sel_count++;
                sql.SelList[attindex].table_name = sql.FromList[0].tb_name;
                sql.SelList[attindex].col_name = atttemp[attindex];
            }
            whe_parser(whe_item,sql);
        }else{
            sel_parser(sel_item ,sql);
            whe_parser(whe_item,sql);
        }
    }else{
        return false;
        //std::cout<<"Parsing error"<<std::endl;
    }
    return true;

//    sel_item = trimSpace(sel_item+',');
//    fro_item = trimSpace(fro_item+',');
//    whe_item.pop_back();
//    whe_item = "and"+ whe_item;
//
//    sel_parser(sel_item ,sql);
//    fro_parser(fro_item,sql);
//    whe_parser(whe_item,sql);

}

void printSelectQuery(SelectQuery &sql1){
    for(int i = 0 ; i < sql1.sel_count ; i ++ ){
        std::cout<<sql1.SelList[i].table_name<<" "<<sql1.SelList[i].col_name<<std::endl;
    }
    //std::cout<<std::endl;
    for(int i = 0 ; i < sql1.from_count ; i ++ ){
        std::cout<<sql1.FromList[i].tb_name<<std::endl;
    }
    //std::cout<<std::endl;
    for(int i = 0 ; i < sql1.join_count ; i++ ){
        std::cout<<sql1.JoinList[i].tb_name1<<" "<<sql1.JoinList[i].col_name1<<" "<<sql1.JoinList[i].op<<" "<<sql1.JoinList[i].tb_name2<<" "<<sql1.JoinList[i].col_name2<<std::endl;
    }
    //std::cout<<std::endl;
    for( int i= 0 ; i < sql1.cond_count ; i++ ){
        std::cout<<sql1.CondList[i].tb_name<<" "<<sql1.CondList[i].col_name<<" ";
        if(sql1.CondList[i].op == E) std::cout<<"= ";
        if(sql1.CondList[i].op == GE) std::cout<<">= ";
        if(sql1.CondList[i].op == G) std::cout<<"> ";
        if(sql1.CondList[i].op == LE) std::cout<<"<= ";
        if(sql1.CondList[i].op == L) std::cout<<"< ";
        if(sql1.CondList[i].op == NE) std::cout<<"<> ";
        std::cout<<sql1.CondList[i].value<<std::endl;
    }
}


bool ParserLoad(std::string str , Load& sql){
    std::regex loadPattern("load\\s+(\\w+)\\s+(.*)");
    std::smatch loadResults;
    if (regex_match(str, loadResults, loadPattern)){
        sql.tb_name = loadResults[1];
        sql.filepath = loadResults[2];
    }else{
        return false;
    }
    return true;
}

void printLoad(Load& sql){
    std::cout<<sql.tb_name<<std::endl;
    std::cout<<sql.filepath<<std::endl;
}

bool ParserDefineSite(std::string str , SiteInfo& sql){
    std::regex dsPattern("define\\s+site\\s+(\\d+)\\s+(\\w+)\\s+([\\w\\.]+):(\\d+)");
    std::smatch dsResults;
    if (regex_match(str, dsResults, dsPattern)){
        std::string siteIDtemp = dsResults[1];
        std::stringstream stream(siteIDtemp);
        stream>>sql.siteID;
        sql.db_name = dsResults[2];
        sql.ip = dsResults[3];
        sql.port = dsResults[4];
    }else{
        return false;
    }
    return true;
}

void printDefineSite(SiteInfo& sql){
    std::cout<<sql.siteID<<std::endl;
    std::cout<<sql.db_name<<std::endl;
    std::cout<<sql.ip<<std::endl;
    std::cout<<sql.port<<std::endl;
}


bool FragmentParser(std::string fragmentSql,Fragment& sql){

    std::regex fragPattern("frag\\s+(\\w+)(.*\\s+db1)?(\\s+.*\\s+db2)?(\\s+.*\\s+db3)?(\\s+.*\\s+db4)?;");
    std::smatch fragResults;
    std::string fragTemp[4];
    int loc_fag = 0;
    if (regex_match(fragmentSql, fragResults, fragPattern)){
        sql.tb_name = fragResults[1];
        if(fragResults[2]!="\0"){
            sql.fragment[loc_fag].DBnum=1;
            sql.frag_count++;
            fragTemp[loc_fag] = fragResults[2];
            fragTemp[loc_fag] = fragTemp[loc_fag].substr(0, fragTemp[loc_fag].length() - 4);
            loc_fag++;
        }
        if(fragResults[3]!="\0"){
            sql.fragment[loc_fag].DBnum=2;
            sql.frag_count++;
            fragTemp[loc_fag] = fragResults[3];
            fragTemp[loc_fag] = fragTemp[loc_fag].substr(0, fragTemp[loc_fag].length() - 4);
            loc_fag++;
        }
        if(fragResults[4]!="\0"){
            sql.fragment[loc_fag].DBnum=3;
            sql.frag_count++;
            fragTemp[loc_fag] = fragResults[4];
            fragTemp[loc_fag] = fragTemp[loc_fag].substr(0, fragTemp[loc_fag].length() - 4);
            loc_fag++;
        }
        if(fragResults[5]!="\0"){
            sql.fragment[loc_fag].DBnum=4;
            sql.frag_count++;
            fragTemp[loc_fag] = fragResults[5];
            fragTemp[loc_fag] = fragTemp[loc_fag].substr(0, fragTemp[loc_fag].length() - 4);
        }
    }else{
        //std::cout<<"Parsing error"<<std::endl;
        return false;
    }


    std::regex vPattern("^\\((.*)\\)");
    std::smatch vResults;
    std::regex hvPattern("(.*)\\((.*)\\)");
    std::smatch hvResults;

    for( int i = 0 ; i < sql.frag_count ;  i++ ){
        fragTemp[i] = fragTemp[i].substr(1);
        //std::cout<<fragTemp[i]<<std::endl;
        if(regex_match(fragTemp[i], vResults, vPattern)){
            parserAtt(vResults[1],sql.fragment[i].condition_v);
            sql.fragment[i].condition_v_count = lenn(sql.fragment[i].condition_v);
            continue;
        }
        if(regex_match(fragTemp[i], hvResults, hvPattern)){
            parserAtt(hvResults[2],sql.fragment[i].condition_v);
            sql.fragment[i].condition_v_count = lenn(sql.fragment[i].condition_v);
            std::string split1[15];
            parserCon(hvResults[1],split1);
            for(int j = 0 ,jj = 0 ; j < lenn(split1) ; jj++ ){
                sql.fragment[i].condition_h[jj].attr_name = split1[j++];
                sql.fragment[i].condition_h[jj].operation = split1[j++];
                sql.fragment[i].condition_h[jj].attr_value = split1[j++];
                sql.fragment[i].condition_h_count++;
            }
            continue;
        }
        std::string split2[15];
        parserCon(fragTemp[i],split2);
        for(int j = 0 ,jj = 0 ; j < lenn(split2) ; jj++ ){
            sql.fragment[i].condition_h[jj].attr_name = split2[j++];
            sql.fragment[i].condition_h[jj].operation = split2[j++];
            sql.fragment[i].condition_h[jj].attr_value = split2[j++];
            sql.fragment[i].condition_h_count++;
        }
    }
    return true;
}

void printFragment(Fragment &sql){
    std::cout<<sql.tb_name<<" "<<sql.frag_count<<std::endl;
    for( int i = 0 ; i < sql.frag_count ; i++ ) {
        std::cout << sql.fragment[i].DBnum << std::endl;
        for (int j = 0; j < sql.fragment[i].condition_h_count; j++) {
            std::cout << sql.fragment[i].condition_h[j].attr_name;
            std::cout << sql.fragment[i].condition_h[j].operation;
            std::cout << sql.fragment[i].condition_h[j].attr_value << std::endl;
        }
        for (int m = 0; m < sql.fragment[i].condition_v_count; m++) {
            std::cout << sql.fragment[i].condition_v[m] << std::endl;
        }
    }
}

void parserAtt( std::string instr, std::string outstr[]){
    instr = instr+",";
    std::smatch outstr_result;
    std::regex instr_pattern("\\w+,");
    std::string::const_iterator instr_Start = instr.begin();
    std::string::const_iterator instr_End = instr.end();
    int Att_index = 0;
    while (regex_search(instr_Start, instr_End, outstr_result, instr_pattern))
    {
        outstr[Att_index] = outstr_result[0];
        outstr[Att_index].pop_back();
        Att_index++;
        instr_Start = outstr_result[0].second;
    }
}


























