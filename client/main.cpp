#include "sql_exec.h"
#include "rpc_sql.h"
#include "local_sql.h"
#include "metadata.h"
#include "global.h"
#include "rpc_server.h"
#include "query_tree_with_fragment.h"
#include <thread>
using namespace std;

// select ok
void testCreate(){
    // AttrInfo a = {"id","integer",true,-1};
    // AttrInfo b = {"name","char",false,25};
    // AttrInfo c = {"sex","char",false,1};
    // AttrInfo d = {"age","integer",false,-1};
    // AttrInfo e = {"degree","integer",false,-1};

    // AttrInfo f = {"id","integer",true,-1};
    // AttrInfo g = {"name","char",false,25};
    // AttrInfo h = {"title","integer",false,-1};

    // AttrInfo i = {"id","integer",true,-1};
    // AttrInfo j = {"name","char",false,80};
    // AttrInfo q = {"location","char",false,8};
    // AttrInfo k = {"credit_hour","integer",false,-1};
    // AttrInfo m = {"teacher_id","integer",false,-1};

    // AttrInfo n = {"student_id","integer",false,-1};
    // AttrInfo o = {"course_id","integer",false,-1};
    // AttrInfo p = {"mark","integer",false,-1};

    // Table stu = {"Student",{a,b,c,d,e},5};
    // Table tea = {"Teacher",{f,g,h},3};
    // Table cou = {"Course",{i,j,q,k,m},5};
    // Table exa = {"Exam",{n,o,p},3};

    string lists[4];
    lists[0] = "create table Student(id int key,name char(25),sex char(1),age int,degree int);";
    lists[1] = "create table Teacher(id int key,name char(25),title int);";
    lists[2] = "create table Course(id int key,name char(80),location char(8),credit_hour int,teacher_id int);";
    lists[3] = "create table Exam(student_id int,course_id int,mark int);";
    // for(int i = 0; i < 4; i++){
    //     Table table = CreateParser(lists[i]);
    //     create_table(table);
    // }
    // printCreate(table);
    // printCreate(table2);

    // table2 = CreateParser(lists[1]);
    // printCreate(table2);
    // create_table(table);
    // create_table(table);
    // printCreate(table);
    // Table teacher;
    // Table course;
    // Table exam;
    // Table student;
    // CreateParser(createSql1,student);
    // CreateParser(createSql2,teacher);
    // CreateParser(createSql3,course);
    // CreateParser(createSql4,exam);
    // printCreate(teacher);
    // printCreate(course);
    // printCreate(exam);
    // create_table(table);
    // create_table(student);
    // create_table(teacher);
    // create_table(course);
    // create_table(exam);

}

Fragment1 teacher;
Fragment1 student;
Fragment1 course;
Fragment1 exam;

// TableInfo info_tea;
// TableInfo info_stu;
// TableInfo info_cor;
// TableInfo info_exa;

// void initTableInfo(){
//     info_stu = {{{"id","integer",true,-1},{"name","char",false,25},{"sex","char",false,1},{"age","integer",false,-1},{"degree","integer",false,-1}},"Student",5};
//     info_tea = {{{"id","integer",true,-1},{"name","char",false,25},{"title","integer",false,-1}},"Teacher",3};
//     info_cor = {{{"id","integer",true,-1},{"name","char",false,80},{"location","char",false,8},{"credit_hour","integer",false,-1},{"teacher_id","integer",false,-1}},"Course",5};
//     info_exa = {{{"student_id","integer",false,-1},{"course_id","integer",false,-1},{"mark","integer",false,-1}},"Exam",3};
// }


void initFragInfo(){
    ConditionHV a = {
        true,
        ConditionH1 {true,"id","<","1050000"},
        ConditionH1 {false},
        ConditionV {false},
        1
    };
    ConditionHV b = {
        true,
        ConditionH1 {true,"id",">=","1050000"},
        ConditionH1 {true,"id","<","1100000"},
        ConditionV {false},
        2
    };
    ConditionHV c = {
        true,
        ConditionH1 {true,"id",">=","1100000"},
        ConditionH1 {false},
        ConditionV {false},
        3
    };
    ConditionHV d = {
        false,
        ConditionH1 {false},
        ConditionH1 {false},
        ConditionV {false},
        4
    };
    student = {"Student",4,{a,b,c,d}};

    //book
    ConditionHV b_a = {
        true,
        ConditionH1 {true,"id","<","2010000"},
        ConditionH1 {true,"title","<>","3"},
        ConditionV {false},
        1
    };
    ConditionHV b_b = {
        true,
        ConditionH1 {true,"id","<","2010000"},
        ConditionH1 {true,"title","=","3"},
        ConditionV {false},
        2
    };
    ConditionHV b_c = {
        true,
        ConditionH1 {true,"id",">=","2010000"},
        ConditionH1 {true,"title","<>","3"},
        ConditionV {false},
        3
    };
    ConditionHV b_d = {
        true,
        ConditionH1 {true,"id",">=","2010000"},
        ConditionH1 {true,"title","=","3"},
        ConditionV {false},
        4
    };
    teacher = {"Teacher",4,{b_a,b_b,b_c,b_d}};

    //orders
    ConditionHV o_a = {
        true,
        ConditionH1 {true,"student_id","<","1070000"},
        ConditionH1 {true,"course_id","<","301200"},
        ConditionV {false},
        1
    };
    ConditionHV o_b = {
        true,
        ConditionH1 {true,"student_id","<","1070000"},
        ConditionH1 {true,"course_id",">=","301200"},
        ConditionV {false},
        2
    };
    ConditionHV o_c = {
        true,
        ConditionH1 {true,"student_id",">=","1070000"},
        ConditionH1 {true,"course_id","<","301200"},
        ConditionV {false},
        3
    };
    ConditionHV o_d = {
        true,
        ConditionH1 {true,"student_id",">=","1070000"},
        ConditionH1 {true,"course_id",">=","301200"},
        ConditionV {false},
        4
    };
    exam = {"Exam",4,{o_a,o_b,o_c,o_d}};


    // customer
    ConditionHV c_a = {
        true,
        ConditionH1 {false},
        ConditionH1 {false},
        ConditionV {true,2,{"id","name"}},
        1
    };
    ConditionHV c_b = {
        true,
        ConditionH1 {false},
        ConditionH1 {false},
        ConditionV {true,4,{"id","location","credit_hour","teacher_id"}},
        2
    };
    ConditionHV c_c = {
        false,
        ConditionH1 {false},
        ConditionH1 {false},
        ConditionV {false},
        3
    };
    ConditionHV c_d = {
        false,
        ConditionH1 {false},
        ConditionH1 {false},
        ConditionV {false},
        4
    };
    course = {"Course",4,{c_a,c_b,c_c,c_d}};
}

void testDelete(){
    // DeleteQuery t1 = {"Teacher",1,{{E,"title","1"}}};
    // DeleteQuery t2 = {"Teacher",2,{{GE,"id","2900010"},{E,"title","2"}}};
    // DeleteQuery s1 = {"Student",1,{{E,"id","100001"}}};
    // DeleteQuery c1 = {"Course",1,{{E,"location","'CB-3'"}}};
    // DeleteQuery e1 = {"Exam",0};
    // DeleteQuery pub1 = {"publisher",1,{{E,"","p_id","232",INTEGER}}};
    // DeleteQuery pub2 = {"publisher",1,{{E,"","nation","PRC",CHAR}}};
    // DeleteQuery book = {"book",1,{{E,"","copies","100",INTEGER}}};
    // DeleteQuery order = {"orders",0};

    // delete_table(pub1);
    // delete_table(pub2);
    // delete_table(book);
    // delete_table(order);

    // delete_table(t1);
    // delete_table(t2);
    // delete_table(s1);
    // delete_table(c1);
    // delete_table(e1);


    // std::string deleteSQL2 = "delete from book where copies = 100;";
    // DeleteQuery ddddd;
    // deleteParser(deleteSQL2,ddddd);
    // printDeleteParser(ddddd);
    // delete_table(ddddd);

    string lists[5];
    lists[0] = "delete from Teacher where title = 1;";
    lists[1] = "delete from Teacher where id >= 2900010 and title = 2;";
    lists[2] = "delete from Student where id = 1000001;";
    lists[3] = "delete from Course where location = 'CB-3';";
    lists[4] = "delete from Exam;";
    for(int i = 0; i < 5; i++){
        DeleteQuery q;
        deleteParser(lists[i], q);
        printDeleteParser(q);
        delete_table(q);
    }
}


void testInsert(){
   
    // Insert a = {"Student",{"1000001","'xiao ming'","'M'","20","1"},5};
    // Insert b = {"Teacher",{"2900010","'Santa Claus'","2"},3};
    // Insert c = {"Teacher",{"2900011","'Santa Claus'","3"},3};
    // Insert d = {"Course",{"3900001","'Defence Against Dark Arts'","'CB-3'","4","20001"},5};
    // Insert e = {"Exam",{"190001","3900001","96"},3};


    // insert_table(a,student);
    // insert_table(b,teacher);
    // insert_table(c,teacher);
    // insert_table(d,course);
    // insert_table(e,exam);
    string list[5];
    list[0] = "insert into Student values(1000001,'xiao ming','M',20,1);";
    list[1] = "insert into Teacher values(2900010,'Santa Claus',2);";
    list[2] = "insert into Teacher values(2000001,'St. Nicholas',1);";
    list[3] = "insert into Course values(3900001,'Defence Against Dark Arts','CB-3',4,200001);";
    list[4] = "insert into Exam values(1900001,3900001,96);";
    Insert a;
    Insert b;
    Insert c;
    Insert d;
    Insert e;
    InsertParser(list[0],a);
    InsertParser(list[1],b);
    InsertParser(list[2],c);
    InsertParser(list[3],d);
    InsertParser(list[4],e);
    insert_table(a,student);
    insert_table(b,teacher);
    insert_table(c,teacher);
    insert_table(d,course);
    insert_table(e,exam);
    // InsertParser(sql,mm);
    // printInsert(mm);
    // insert_table(mm,student);

    // std::string insertSQL1 = "insert into book values(295001,'DDB','Oszu',104001,100);";
    // Insert cccc;
    // InsertParser(insertSQL1,cccc);
    // printInsert(cccc);
    // insert_table(cccc,book,cccc.values_count);
}

void testLoad(){
    Load load_teacher = {"Teacher","/var/lib/mysql-files/data/teacher.tsv"};
    Load load_student = {"Student","/var/lib/mysql-files/data/student.tsv"};
    Load load_course = {"Course","/var/lib/mysql-files/data/course.tsv"};
    Load load_exam = {"Exam","/var/lib/mysql-files/data/exam.tsv"};
    // Load load_teacher;
    // Load load_student;
    // Load load_course;
    // Load load_exam;
    // string str1 = "load Student /var/lib/mysql-files/data/student.tsv";
    // string str2 = "load Teacher /var/lib/mysql-files/data/teacher.tsv";
    // string str3 = "load Course /var/lib/mysql-files/data/course.tsv";
    // string str4 = "load Exam /var/lib/mysql-files/data/exam.tsv";
    // ParserLoad(str1,load_student);
    // ParserLoad(str2,load_teacher);
    // ParserLoad(str3,load_course);
    // ParserLoad(str4,load_exam);
    // printLoad(loadSQL);

    // load_data(info_tea,teacher,load_teacher);
    // load_data(info_stu,student,load_student);
    // load_data(info_cor,course,load_course);
    // load_data(info_exa,exam,load_exam);
}

 Fragment read_fragment(){
    Fragment f1;
    f1.tb_name="publisher";
    f1.frag_count=4;
    f1.fragment[0].DBnum=0;
    f1.fragment[0].condition_h_count=2;
    f1.fragment[0].condition_h[0].attr_name="p_id";
    f1.fragment[0].condition_h[0].attr_value="104000";
    f1.fragment[0].condition_h[0].operation="<";
    f1.fragment[0].condition_h[1].attr_name="nation";
    f1.fragment[0].condition_h[1].attr_value="'PRC'";
    f1.fragment[0].condition_h[1].operation="=";
    // f1.fragment[0].condition_v_count=0;
    // // f1.fragment[0].condition_v[0]="c_id";
    // // f1.fragment[0].condition_v[1]="name";
    f1.fragment[1].condition_h_count=2;
    f1.fragment[1].DBnum=1;
    f1.fragment[1].condition_h[0].attr_name="p_id";
    f1.fragment[1].condition_h[0].operation="<";
    f1.fragment[1].condition_h[0].attr_value="104000";
    f1.fragment[1].condition_h[1].attr_name="nation";
    f1.fragment[1].condition_h[1].operation="=";
    f1.fragment[1].condition_h[1].attr_value="'USA'";
    // f1.fragment[1].condition_v_count=0;
    // f1.fragment[1].condition_v[0]="c_id";
    // f1.fragment[1].condition_v[1]="rank";
    f1.fragment[2].DBnum=2;
    f1.fragment[2].condition_h_count=2;
    f1.fragment[2].condition_h[0].attr_name="p_id";
    f1.fragment[2].condition_h[0].operation=">=";
    f1.fragment[2].condition_h[0].attr_value="104000";
    f1.fragment[2].condition_h[1].attr_name="nation";
    f1.fragment[2].condition_h[1].operation="=";
    f1.fragment[2].condition_h[1].attr_value="'PRC'";
    f1.fragment[3].DBnum=3;
    f1.fragment[3].condition_h_count=2;
    f1.fragment[3].condition_h[0].attr_name="p_id";
    f1.fragment[3].condition_h[0].operation=">=";
    f1.fragment[3].condition_h[0].attr_value="104000";
    f1.fragment[3].condition_h[1].attr_name="nation";
    f1.fragment[3].condition_h[1].operation="=";
    f1.fragment[3].condition_h[1].attr_value="'USA'";

    return f1;
}

void test3(){
    string tablename="publisher";

    Fragment1 f_c = Get_Fragment1(tablename);
    cout<<f_c.tb_name<<endl;
    cout<<f_c.frag_count<<endl;

    for(int i=0;i<f_c.frag_count;i++)
    {
        cout<<"此时i的值："<<i<<endl;
        cout<<f_c.conditions[i].is_needed<<endl;
        cout<<f_c.conditions[i].db_id<<endl;
        cout<<f_c.conditions[i].h1.is_needed<<endl;
        cout<<f_c.conditions[i].h1.attr_name<<endl;
        cout<<f_c.conditions[i].h1.attr_value<<endl;
        cout<<f_c.conditions[i].h1.operation<<endl;
        cout<<f_c.conditions[i].h2.is_needed<<endl;
        cout<<f_c.conditions[i].h2.attr_name<<endl;
        cout<<f_c.conditions[i].h2.attr_value<<endl;
        cout<<f_c.conditions[i].h2.operation<<endl;
        if(f_c.conditions[i].v1.is_needed)
        {

            cout<<f_c.conditions[i].v1.attr_num<<endl;
            cout<<f_c.conditions[i].v1.is_needed<<endl;
            for(int j = 0;j<f_c.conditions[i].v1.attr_num;j++)
            {
                cout<<"我要输出了！！"<<endl;
                cout<<f_c.conditions[i].v1.attr_list[j]<<endl;
            }
        }
       
    }
}


void testFrag(){
    Fragment1 f2;
    Fragment f1 = read_fragment();
    // if(Save_Fragment(f1)){
    //     string tb_name = "publisher";
    //     f2 = Get_Fragment1(tb_name);
    // }
    // std::string sql = "insert into book values(295001,'DDB','Oszu',104001,100);";
    std::string sql = "insert into publisher  values(194001,'High Education Press', 'PRC');";
    Insert a;
    InsertParser(sql,a);
    for(int i = 0;i < a.values_count;i++){
        cout << a.valuesList[i] << endl;
    }
    // string tb_name = "publisher";
    // f2 = Get_Fragment1(tb_name);
    // insert_table(a,f2,a.values_count);
    // test3();
    // printInsert(a); 
}

// void testInstertFromSelect(){
//     string ip1 = "127.0.0.1";
//     string ip2 = "127.0.0.1";
//     string tb1 = "db";
//     string tb2 = "db";
//     string db1 = "db1";
//     string db2 = "db2";
//     RPCInsertFromSelect(ip1,ip2,tb1,tb2,db1,db2);
// }

void localExecuteStmt(){
    // string sql = "create table tb1 (a int,b int ,c int)";
    // cout << "#res is " << localExecute("db1",sql) << endl;
    // cout << "#res is " << localExecute("db1",sql) << endl;
    int count = 0;
    string list[4];
    list[0] = "select Student_name,Exam_mark from StudentExam_frag1;";
    list[1] = "select Student_name,Exam_mark from StudentExam_frag2;";
    list[2] = "select Student_name,Exam_mark from StudentExam_frag3;";
    list[3] = "select Student_name,Exam_mark from StudentExam_frag4;";
    // for(int i = 0; i < 4; i++){
    //     string db = "db" + (i+1);
    //     string res = RPCExecuteQuery("127.0.0.1",8080,db,list[i]);
    //     vector<string> nums = Split(res,"\n");
    //     cout << "COUNT:" << nums.size() << endl;
    //     count += nums.size();
    // }
    string sql = "select distinct name from Student;";
    // string res = localExecuteQuery("db1",sql);
    string res = RPCExecuteQuery("127.0.0.1",8080,"db1",sql);
    vector<string> nums = Split(res,"\n");
    cout << "count" << nums.size() << endl;
    cout << "res" << res << "sddsd" << endl;
}



// int main(){
    // 初始化分片信息
    // initFragInfo();
    // 初始化表的信息
    // initTableInfo();
    // testCreate();
    // testInsert();
    // testDelete();
    // testLoad();
    // testFrag();
    // testInstertFromSelect();
    // localExecuteStmt();
    // cout << "can go on ????" << endl;
    // cout << "yes" << endl;
// }

void printFragment1(Fragment1 frag){
    cout << frag.tb_name << endl;
    for(int i = 0;i < frag.frag_count; i++){
        if(frag.conditions[i].is_needed){
            if(frag.conditions[i].h1.is_needed){
                cout << frag.conditions[i].h1.attr_name << " " << frag.conditions[i].h1.operation << frag.conditions[i].h1.attr_value << endl;
            }
            if(frag.conditions[i].h2.is_needed){
                cout <<" 222" << frag.conditions[i].h2.attr_name << " " << frag.conditions[i].h2.operation << frag.conditions[i].h2.attr_value << endl;
            }
        }
    }
}

int main(int argc,char *argv[]){
    int port = std::stoi(argv[1]);
    thread th1(startServer,port);
    th1.detach();
    bool isSetSite = false;
    while(true){
        string sql = "";
        Table table;
        Insert insert;
        DeleteQuery deleteQuery;
        SelectQuery selectQuery;
        Load load;
        SiteInfo setinfo;
        Fragment fragment;
        getline(cin,sql);
        if("show siteinfo;" == sql){
            SiteInfos infos =  Get_SiteInfo();
            cout << "111" << endl;
            setSiteInfos(infos);
            initSelect(infos);
            isSetSite = true;
            printMessage();
            cout << "222" << endl;
            showInfos();
        }
        else if(CreateParser(sql,table)){
            if (!isSetSite){
                SiteInfos infos =  Get_SiteInfo();
                setSiteInfos(infos);
                isSetSite = true;
            }
            create_table(table);
            Save_Table(table);
        }
        else if(InsertParser(sql,insert)){
            Fragment1 frag = Get_Fragment1(insert.tb_name);
            insert_table(insert,frag);
        }
        else if(deleteParser(sql,deleteQuery)){
            delete_table(deleteQuery);
        }
        else if(ParserLoad(sql,load)){
            Fragment1 frag = Get_Fragment1(load.tb_name);
            Table info = Get_Table(load.tb_name);
            printFragment1(frag);
            load_data(info,frag,load);

        }
        else if(ParserDefineSite(sql,setinfo)){
            Save_SiteInfo(setinfo);
            // printDefineSite(setinfo);
        }
        else if(FragmentParser(sql,fragment)){
            printFragment(fragment);
            Save_Fragment(fragment);
        }else if(SelectParser(sql,selectQuery)){
            printSelectQuery(selectQuery);
            do_select(selectQuery);
        }
    }
}





















