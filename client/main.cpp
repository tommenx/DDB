#include "sql_exec.h"
#include "metadata.h"

// select ok
void testCreate(){
    // AttrInfo attrs_publisher[MAX_ATTR_COUNT];
    // AttrInfo attrs_book[MAX_ATTR_COUNT];
    // AttrInfo attrs_customer[MAX_ATTR_COUNT];
    // AttrInfo attrs_orders[MAX_ATTR_COUNT];
    // AttrInfo a = {"p_id","integer",false,-1};
    // AttrInfo b = {"name","char",false,100};
    // AttrInfo c = {"nation","char",false,3};
    // AttrInfo d = {"b_id","integer",false,-1};
    // AttrInfo e = {"title","char",false,100};
    // AttrInfo f = {"authors","char",false,20};
    // AttrInfo g = {"p_id","integer",false,-1,};
    // AttrInfo h = {"copies","integer",false,-1};
    // AttrInfo i = {"c_id","integer",false,-1};
    // AttrInfo j = {"b_id","integer",false,-1};
    // AttrInfo k = {"quantity","integer",false,-1};
    // AttrInfo m = {"c_id","integer",true,-1};
    // AttrInfo n = {"name","char",false,25};
    // AttrInfo o = {"rank","integer",false,-1};
    // attrs_publisher[0] = a;
    // attrs_publisher[1] = b;
    // attrs_publisher[2] = c;
    // attrs_book[0] = d;
    // attrs_book[1] = e;
    // attrs_book[2] = f;
    // attrs_book[3] = g;
    // attrs_book[4] = h;
    // attrs_orders[0] = i;
    // attrs_orders[1] = j;
    // attrs_orders[2] = k;
    // attrs_customer[0] = m;
    // attrs_customer[1] = n;
    // attrs_customer[2] = o;

    // Table pub = {"publisher",{a,b,c},3};
    // Table book = {"book",{d,e,f,g,h},5};
    // Table cus = {"customer",{i,j,k},3};
    // Table ord = {"orders",{m,n,o},3};


    // int publisher_count = 3;
    // int book_count = 5;
    // int orders_count = 3;
    // int customer_count = 3;
    std::string createSQL1 = "create table book(b_id integer key,title char(100),authors char(20),p_id integer,copies integer);";
    std::string createSQL2 = "create table orders(c_id integer,b_id integer,quantity integer);";
    Table table;
    CreateParser(createSQL2,table);
    printCreate(table);
    create_table(table);
    // create_table(book);
    // create_table(cus);
    // create_table(ord);
}

// Fragment publisher;
// Fragment book;
// Fragment orders;
// Fragment customer;

TableInfo info_pub;
TableInfo info_book;
TableInfo info_orders;
TableInfo info_customer;

void initTableInfo(){
    info_pub = {{{"p_id","integer",true,-1},{"name","char",false,100},{"nation","char",false,3}},"publisher",3};
    info_book = {{{"b_id","integer",true,-1},{"title","char",false,100},{"authors","char",false,20},{"p_id","integer",false,-1},{"copies","integer",false,-1}},"book",5};
    info_orders = {{{"c_id","integer",false,-1},{"b_id","integer",false,-1},{"quantity","integer",false,-1}},"orders",3};
    info_customer = {{{"c_id","integer",true,-1},{"name","char",false,25},{"rank","int",false,-1}},"customer",3};
}


// void initFragInfo(){
//     ConditionHV a = {
//         true,
//         ConditionH {true,"p_id","<","104000"},
//         ConditionH {true,"nation","=","'PRC'"},
//         ConditionV {false}
//     };
//     ConditionHV b = {
//         true,
//         ConditionH {true,"p_id","<","104000"},
//         ConditionH {true,"nation","=","'USA'"},
//         ConditionV {false}
//     };
//     ConditionHV c = {
//         true,
//         ConditionH {true,"p_id",">=","104000"},
//         ConditionH {true,"nation","=","'PRC'"},
//         ConditionV {false}
//     };
//     ConditionHV d = {
//         true,
//         ConditionH {true,"p_id",">=","104000"},
//         ConditionH {true,"nation","=","'USA'"},
//         ConditionV {false,}
//     };
//     publisher = {"publisher",4,{a,b,c,d}};

//     //book
//     ConditionHV b_a = {
//         true,
//         ConditionH {true,"b_id","<","205000"},
//         ConditionH {false},
//         ConditionV {false}
//     };
//     ConditionHV b_b = {
//         true,
//         ConditionH {true,"b_id",">=","205000"},
//         ConditionH {true,"b_id","<","210000"},
//         ConditionV {false}
//     };
//     ConditionHV b_c = {
//         true,
//         ConditionH {true,"b_id",">","210000"},
//         ConditionH {false},
//         ConditionV {false}
//     };
//     ConditionHV b_d = {
//         false,
//         ConditionH {false},
//         ConditionH {false},
//         ConditionV {false}
//     };
//     book = {"book",4,{b_a,b_b,b_c,b_d}};

//     //orders
//     ConditionHV o_a = {
//         true,
//         ConditionH {true,"c_id","<","307000"},
//         ConditionH {true,"b_id","<","215000"},
//         ConditionV {false}
//     };
//     ConditionHV o_b = {
//         true,
//         ConditionH {true,"c_id","<","307000"},
//         ConditionH {true,"b_id",">=","215000"},
//         ConditionV {false}
//     };
//     ConditionHV o_c = {
//         true,
//         ConditionH {true,"c_id",">=","307000"},
//         ConditionH {true,"b_id","<","215000"},
//         ConditionV {false}
//     };
//     ConditionHV o_d = {
//         true,
//         ConditionH {true,"c_id",">=","307000"},
//         ConditionH {true,"b_id",">=","215000"},
//         ConditionV {false}
//     };
//     orders = {"orders",4,{o_a,o_b,o_c,o_d}};


//     // customer
//     ConditionHV c_a = {
//         true,
//         ConditionH {false},
//         ConditionH {false},
//         ConditionV {true,2,{"c_id","name"}}
//     };
//     ConditionHV c_b = {
//         true,
//         ConditionH {false},
//         ConditionH {false},
//         ConditionV {true,2,{"c_id","rank"}}
//     };
//     ConditionHV c_c = {
//         false,
//         ConditionH {false},
//         ConditionH {false},
//         ConditionV {false}
//     };
//     ConditionHV c_d = {
//         false,
//         ConditionH {false},
//         ConditionH {false},
//         ConditionV {false}
//     };
//     customer = {"customer",4,{c_a,c_b,c_c,c_d}};
// }

void testDelete(){
    // DeleteQuery pub1 = {"publisher",1,{{E,"","p_id","232",INTEGER}}};
    // DeleteQuery pub2 = {"publisher",1,{{E,"","nation","PRC",CHAR}}};
    // DeleteQuery book = {"book",1,{{E,"","copies","100",INTEGER}}};
    // DeleteQuery order = {"orders",0};

    // delete_table(pub1);
    // delete_table(pub2);
    // delete_table(book);
    // delete_table(order);

    // std::string deleteSQL2 = "delete from book where copies = 100;";
    // DeleteQuery ddddd;
    // deleteParser(deleteSQL2,ddddd);
    // printDeleteParser(ddddd);
    // delete_table(ddddd);
}


void testInsert(){
   
    // Insert a = {"publisher",{"123","'dsadghaksgdakd'","'USA'"},3};
    // Insert b = {"publisher",{"232","'dsadghaksgdakd'","'PRC'"},3};
    // Insert c = {"publisher",{"104001","'dsadghaksgdakd'","'PRC'"},3};
    // Insert d = {"publisher",{"104002","'dsadghaksgdakd'","'USA'"},3};

    // Insert aa = {"book",{"204999","'lalalala'","'xixixixixi'","104001","100"},5};
    // Insert bb = {"book",{"205000","'lalalala'","'xixixixixi'","104001","100"},5};
    // Insert cc = {"book",{"210000","'lalalala'","'xixixixixi'","104001","100"},5};

    // Insert aaa = {"orders",{"306999", "214999","5"},3};
    // Insert bbb = {"orders",{"306888", "215000","5"},3};
    // Insert ccc = {"orders",{"307000", "214888","5"},3};
    // Insert ddd = {"orders",{"307001", "215001","5"},3};

    // Insert aaaa = {"customer",{"300001","'Xiaoming'","1"},3};
    // Insert bbbb = {"customer",{"300002","'Xiaoming'","2"},3};
    // Insert cccc = {"customer",{"300003","'Xiaoming'","3"},3};


    // insert_table(a,publisher,a.values_count);
    // insert_table(b,publisher,b.values_count);
    // insert_table(c,publisher,c.values_count);
    // insert_table(d,publisher,d.values_count);
    // insert_table(aa,book,aa.values_count);
    // insert_table(bb,book,aa.values_count);
    // insert_table(cc,book,aa.values_count);
    // insert_table(aaa,orders,aaa.values_count);
    // insert_table(bbb,orders,bbb.values_count);
    // insert_table(ccc,orders,ccc.values_count);
    // insert_table(ddd,orders,ddd.values_count);
    // insert_table(aaaa,customer,aaaa.values_count);
    // insert_table(bbbb,customer,bbbb.values_count);
    // insert_table(cccc,customer,cccc.values_count);


    // std::string insertSQL1 = "insert into book values(295001,'DDB','Oszu',104001,100);";
    // Insert cccc;
    // InsertParser(insertSQL1,cccc);
    // printInsert(cccc);
    // insert_table(cccc,book,cccc.values_count);

    


}

// void testLoad(){
//     Load load_publisher = {"publisher","/var/lib/mysql-files/data/publisher.tsv"};
//     Load load_book = {"book","/var/lib/mysql-files/data/book.tsv"};
//     Load load_orders = {"orders","/var/lib/mysql-files/data/order.tsv"};
//     Load load_customer = {"customer","/var/lib/mysql-files/data/customer.tsv"};
//     load_data(info_pub,publisher,load_publisher);
//     load_data(info_book,book,load_book);
//     load_data(info_orders,orders,load_orders);
//     load_data(info_customer,customer,load_customer);
// }

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

void test3()
{
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



int main(){
    // 初始化分片信息
    // initFragInfo();
    // 初始化表的信息
    // initTableInfo();
    // testCreate();
    // testInsert();
    // testDelete();
    // testLoad();
    testFrag();


}
