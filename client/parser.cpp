#include "parser.h"

// string gen_create_stmt(AttrInfo attrs[],int attr_count){
//     string stmt = "CREATE TABLE ";
//     stmt.append(attrs[0].tb_name);
//     stmt.append("(");
//     for(int i = 0;i < attr_count;i++){
//         stmt.append(attrs[i].attr_name);
//         stmt.append(" ");
//         stmt.append(attrs[i].type);
//         stmt.append(" ");
//         if(attrs[i].is_key){
//             stmt.append("key");
//         }
//         if(attrs[i].size != -1){
//             stmt.append("(");
//             stmt.append(std::to_string(attrs[i].size));
//             stmt.append(")");
//         }
//         stmt.append(",");
//     }
//     stmt.pop_back();
//     stmt.append(")");
//     stmt.append(";");
//     return stmt;
// }

string gen_create_stmt(Table table){
    string stmt = "CREATE TABLE ";
    stmt.append(table.tb_name);
    stmt.append("(");
    for(int i = 0;i < table.attr_count;i++){
        stmt.append(table.attrs[i].attr_name);
        stmt.append(" ");
        stmt.append(table.attrs[i].type);
        stmt.append(" ");
        if(table.attrs[i].is_key){
            stmt.append("key");
        }
        if(table.attrs[i].size != -1){
            stmt.append("(");
            stmt.append(std::to_string(table.attrs[i].size));
            stmt.append(")");
        }
        stmt.append(",");
    }
    stmt.pop_back();
    stmt.append(")");
    stmt.append(";");
    return stmt;
}

string gen_tmp_create_stmt(Table info){
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

string gen_condition_stmt(Condition cond_list[],int cond_count) {
    if (cond_count == 0 || cond_list == NULL)
        return "";
    string tmp_stmt;
    for(int i = 0;i < cond_count;i++){
        // if(cond_list[i].table != ""){
        //     tmp_stmt.append(cond_list[i].table);
        //     tmp_stmt.append(".");
        // }
        tmp_stmt.append(cond_list[i].column);
        tmp_stmt.append(" ");
        tmp_stmt.append(get_op_stmt(cond_list[i].op));
        tmp_stmt.append(" ");
        // if (cond_list[i].type == VARCHAR || cond_list[i].type == CHAR){
        //     tmp_stmt.append("'");
        //     tmp_stmt.append(cond_list[i].value);
        //     tmp_stmt.append("'");
        // }
        // else
            tmp_stmt.append(cond_list[i].value);
        if(i != cond_count-1){
            tmp_stmt.append(" AND ");
        }
    }
    return tmp_stmt;
}

string gen_delete_stmt(DeleteQuery deleteQuery){
    string stmt = "DELETE FROM ";
    stmt.append(deleteQuery.tb_name);
    string cond_stmt = gen_condition_stmt(deleteQuery.conditions,deleteQuery.cond_count);
    if(cond_stmt != ""){
        stmt.append(" WHERE ");
        stmt.append(cond_stmt);
    }
    stmt.append(";");
    return stmt;
}

string gen_insert_stmt(string insert_records[],int insert_record_count,string tmp,string tb_name){
    string stmt = "INSERT INTO "+ tmp + tb_name + " VALUES(";
    for(int i = 0; i < insert_record_count; i++){
        stmt += insert_records[i];
        stmt += ",";
    }
    stmt.pop_back();
    stmt += ");";
    return stmt;
}

vector<string> v_insert(Fragment1 fragment, string insert_records[], int insert_record_count){
    vector<string> stmts;
    // if(insert_record_count < 3){
    //     return stmts;
    // }

    string stmt1 = "INSERT INTO " 
    + fragment.tb_name
    +"( id,name) VALUES ("
    +insert_records[0]
    +", "
    +insert_records[1]
    +");";

    string stmt2 = "INSERT INTO " 
    + fragment.tb_name
    +"( id,location,credit_hour,teacher_id) VALUES ("
    +insert_records[0]
    +", "
    +insert_records[2]
    +", "
    +insert_records[3]
    +", "
    +insert_records[4]
    +");";

    stmts.push_back(stmt1);
    stmts.push_back(stmt2);
    return stmts;
}

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

vector<string> gen_select_by_frag(Fragment1 fragment,string tmp_tb_name){
    vector<string>stmts;
    for(int i = 0; i < fragment.frag_count; i++){
        if(fragment.conditions[i].is_needed){
            string str = "SELECT * FROM " 
            + tmp_tb_name;
            string con = gen_con_by_con(fragment.conditions[i]);
            if(str != ""){
                str += " WHERE ";
                str += con;
            }
            str += ";";
            stmts.push_back(str);
        } 
    }
    return stmts;
}

vector<string> gen_v_select_by_frag(Fragment1 fragment,string tmp_tb_name){
    vector<string>stmts;
    for(int i = 0;i < fragment.frag_count;i++){
        string stmt = "SELECT ";
        if(fragment.conditions[i].is_needed && fragment.conditions[i].v1.is_needed){
            for(int j = 0; j < fragment.conditions[i].v1.attr_num; j++){
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
    if(src == "")
        return dest;
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

string gen_batch_insert(Table info,string src){
    string stmt = "INSERT INTO " + info.tb_name + " VALUES";
    string str = src;
    if(str == "NULL")
        return str;
    vector<string>dest = Split(str,"\n");
    for(auto it = dest.begin(); it != dest.end(); it++){
        stmt += "( " + *it + " ),";
    }
    stmt.pop_back();
    stmt += ";";
    return stmt;
}

string gen_batch_insert_v(string src,int c){
    string str = src;
    string stmt = "INSERT INTO ";
    if(0 == c){
        stmt += "Course(id,name)VALUES";
        vector<string>dest = Split(str,"\n");
        for(auto it = dest.begin(); it != dest.end(); it++){
            stmt += "( " + *it + " ),";
        }
        stmt.pop_back();
        stmt += ";";
    }
    else if(1 == c){
        stmt += "Course(id,location,credit_hour,teacher_id)VALUES";
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

// 根据OP生成对应的字符串
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


































