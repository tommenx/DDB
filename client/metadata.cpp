#include "metadata.h"
// #include "parser.h"
#define URL_MAX_LEN 50  
#define VALUE_LEN 1024

Fragment Get_Fragment(string &tablename)
//获取fragment数据，并且返回F1.参数是表名
{
    string dir = "fragmentinfo/"+tablename;
    string info  = Search_Value(dir);
    Fragment f1 = To_json(info,tablename);
    return f1;
}

int etcd_set(char *key, char *value, char *token)  
{  //向etcd创建dir数据
  
   
    CURLcode return_code;  
    char etcd_url[URL_MAX_LEN];  
    char etcd_value[VALUE_LEN];   
      
        return_code = curl_global_init(CURL_GLOBAL_SSL);  
    if (CURLE_OK != return_code)  
    {  
        //cerr << "init libcurl failed." << endl;  
        printf("init libcurl failed\n");  
        return -1;  
    }  
  
    sprintf(etcd_url, "http://127.0.0.1:2379/v2/keys%s", key);  
    sprintf(etcd_value, "dir=%s", value);  
    // 获取easy handle  
    CURL *easy_handle = curl_easy_init();  
    if (NULL == easy_handle)  
    {  
        //cerr << "get a easy handle failed." << endl;  
        printf("get a easy handle failed.\n");  
        curl_global_cleanup();   
        return -1;  
    }  
    // 设置easy handle属性  
    curl_easy_setopt(easy_handle, CURLOPT_URL, etcd_url);   
    curl_easy_setopt(easy_handle, CURLOPT_POST, 1);  
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, etcd_value);  
    curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "PUT");  
   
    // 执行数据请求  
    curl_easy_perform(easy_handle);   
  
    // 释放资源  
    curl_easy_cleanup(easy_handle);  
    curl_global_cleanup();  
    return 0;  
} 
int etcd_set2(char *key, char *value, char *token)  
//向etcd存储value数据
{    
   
    CURLcode return_code;   
    char etcd_value[VALUE_LEN];   
      
    return_code = curl_global_init(CURL_GLOBAL_SSL);  
    if (CURLE_OK != return_code)  
    {  
        //cerr << "init libcurl failed." << endl;  
        printf("init libcurl failed\n");  
        return -1;  
    }  
    string a = key;
    string str = "http://127.0.0.1:2379/v2/keys"+a;
    char *ss=(char*)str.c_str();
    sprintf(etcd_value, "value=%s", value);  
    // 获取easy handle  
    CURL *easy_handle = curl_easy_init();  
    if (NULL == easy_handle)  
    {  
        //cerr << "get a easy handle failed." << endl;  
        printf("get a easy handle failed.\n");  
        curl_global_cleanup();   
        return -1;  
    }  
    // 设置easy handle属性  
    curl_easy_setopt(easy_handle, CURLOPT_URL, ss);   
    curl_easy_setopt(easy_handle, CURLOPT_POST, 1);  
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, etcd_value);  
    curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "PUT");  
   
    // 执行数据请求  
    curl_easy_perform(easy_handle);   
  
    // 释放资源  
    curl_easy_cleanup(easy_handle);  
    curl_global_cleanup();  
    return 0;  
}  
string  Create_Dir(string &etcd_url,string &etcd_dir)
{ //在etcd里面创建dir
    string str=etcd_url+"/"+etcd_dir;
    cout<<str<<endl;
    char *p=(char*)str.c_str();
    etcd_set(p, "true", NULL);  
    return str; 
}
bool Insert_Attrvalue(string &dir,string &value)
{  //调用存储数据的etcd接口  

    string str=dir;
    char *p=(char*)str.c_str();
    char *data=(char*)value.c_str();
    etcd_set2(p, data, NULL);  
    return true; 
      
}
size_t write_data(void *buffer, size_t size, size_t nmemb, void *stream) 
//从etcd里面取得部分数据
{ 
    strncat((char*)stream,(char*)buffer,size*nmemb);
    return nmemb*size; 
} 
bool Save_Table(Table TableInfo)
{//存储table数据
    map<int,string> mapAttrinfo;
    mapAttrinfo.insert(pair<int,string>(0,"attr_name"));
    mapAttrinfo.insert(pair<int,string>(1,"is_key"));
    mapAttrinfo.insert(pair<int,string>(2,"size"));
    mapAttrinfo.insert(pair<int,string>(3,"type"));
    mapAttrinfo.insert(pair<int,string>(4,"index"));
    Table t1 = TableInfo;
    string tablename = t1.tb_name;
    string etcd_url = "/tableinfo";
    string dir = Create_Dir(etcd_url,tablename);
    if(dir!="fail")
    {
        for(int i =0;i<(t1.attr_count+1);i++)
        {
            string etcd_dir = dir;
            string attrname = t1.attrs[i].attr_name;
            string new_dir = Create_Dir(etcd_dir,attrname);
            string attr_info = mapAttrinfo[0];
            string key = new_dir+"/"+attr_info; 
            string value = t1.attrs[i].attr_name;
            if(!Insert_Attrvalue(key,value)==true)
            {return false;}
            string attr_info1 = mapAttrinfo[1];
            string key1 = new_dir+"/"+attr_info1;
            string value1;
            if(t1.attrs[i].is_key)
            {
                value1 ="true";
            }
            else{
                value1="false";
            }
            if(!Insert_Attrvalue(key1,value1)==true)
            {return false;}
            string attr_info2 = mapAttrinfo[2];
            string key2 = new_dir+"/"+attr_info2;
            int data1 =t1.attrs[i].size;
            string value2 = to_string(data1);
            if(!Insert_Attrvalue(key2,value2)==true)
            {return false;}
            string attr_info3 = mapAttrinfo[3];
            string key3 = new_dir+"/"+attr_info3; 
            string value3 = t1.attrs[i].type;
            if(!Insert_Attrvalue(key3,value3)==true)
            {
                return false;
            }
            string attr_info4 = mapAttrinfo[4];
            string key4 = new_dir+"/"+attr_info4; 
            string value4 = to_string(i);
            if(!Insert_Attrvalue(key4,value4)==true)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}
bool Save_Fragment(Fragment table)
{//存储fragment
    Fragment f1 = table;
    map<int,string> mapAttrinfo;
    mapAttrinfo.insert(pair<int,string>(0,"attr_name"));
    mapAttrinfo.insert(pair<int,string>(1,"operation"));
    mapAttrinfo.insert(pair<int,string>(2,"attr_value"));
    string etcd_frag = f1.tb_name;
    string etcd_url = "/fragmentinfo";
    string dir = Create_Dir(etcd_url,etcd_frag);
    for(int i = 0;i<f1.frag_count;i++)
    {
        int site  = f1.fragment[i].DBnum;
        string sitename = to_string(site);
        string etcd_dir = dir;
        string new_dir = Create_Dir(etcd_dir,sitename);
        string howtofrag = "H";
        string h_dir =Create_Dir(new_dir,howtofrag);
        for(int j = 0;j<f1.fragment[i].condition_h_count;j++)
        {
                string hname = to_string(j);
                string h_info_dir = Create_Dir(h_dir,hname);
                string attr_info1 = mapAttrinfo[0];
                string key1 = h_info_dir+"/"+attr_info1; 
                string value1 = f1.fragment[i].condition_h[j].attr_name;
                if(!Insert_Attrvalue(key1,value1)==true)
                {
                return false;
                }
                
                string attr_info2 = mapAttrinfo[1];
                string key2 = h_info_dir+"/"+attr_info2; 
                string value2 = f1.fragment[i].condition_h[j].operation;
                if(!Insert_Attrvalue(key2,value2)==true)
                {
                return false;
                }
                 string attr_info3 = mapAttrinfo[2];
                string key3 = h_info_dir+"/"+attr_info3; 
                string value3 = f1.fragment[i].condition_h[j].attr_value;
                if(!Insert_Attrvalue(key3,value3)==true)
                {
                return false;
                }
        }
        if(f1.fragment[i].condition_v_count!=0)
        {
        string howtofrag1 = "V";
        string v_dir =Create_Dir(new_dir,howtofrag1);
        for(int t = 0;t<f1.fragment[i].condition_v_count;t++)
        {
        
            string vname = to_string(t);
            string v_info_dir = v_dir+"/"+vname;
            string value = f1.fragment[i].condition_v[t];
            if(!Insert_Attrvalue(v_info_dir,value)==true)
            {
                
                return false;
            }
        }
        }
    }
}
string Search_Value(string &dir)
//将全部数据合并
{
    string etcd_url = "http://127.0.0.1/v2/keys/"+dir+"?recursive=true";
        char *ss=(char*)etcd_url.c_str();
     CURLcode return_code;  
    return_code = curl_global_init(CURL_GLOBAL_SSL);  
    if (CURLE_OK != return_code)  
    {  
        cerr << "init libcurl failed." << endl;  
        return "";  
    }  
  
    // 获取easy handle  
    CURL *easy_handle = curl_easy_init();  
    if (NULL == easy_handle)  
    {  
        cerr << "get a easy handle failed." << endl;  
        curl_global_cleanup();   
  
        return "";  
    }  
  
    char * buff_p = NULL;  
    char result[5000] = "";
    // 设置easy handle属性  
    curl_easy_setopt(easy_handle, CURLOPT_URL,ss);  
    curl_easy_setopt(easy_handle, CURLOPT_PORT, 2379);  
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &write_data);  
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &result);  
  
    // 执行数据请求  
  
    curl_easy_perform(easy_handle);   
  
    // 释放资源  
  
    curl_easy_cleanup(easy_handle);  
    curl_global_cleanup();  
    string info = result;
    return info;
}
Fragment To_json(string &info,string &tablename)
//用json格式解析fragment的数据
{
    Value root;  
    Value node;
    Value node1;
    Value node2;  
    Reader reader;  
    FastWriter writer; 
    string json = info;
    
     if(!reader.parse(json, root))  
    {  
        cout << "parse json error" << endl;   

    }  
    string nodeString = writer.write(root["node"]);  
    if(!reader.parse(nodeString, node))  
    {  
        cout << "parse json error" << endl;   
 
    }  

    Fragment f1;
    f1.tb_name=tablename;
    Value nodei;
        string table_name = writer.write(node["nodes"]);
        string nodestringi = writer.write(node["nodes"]);
        if(!reader.parse(nodestringi, nodei))  
        {  
        cout << "parse json error" << endl;   
  
        }  
        f1.frag_count=nodei.size();
        for(int j = 0;j<f1.frag_count;j++)
        {
            Value nodej;
            string nodestringj = writer.write(nodei[j]["nodes"]);
            if(!reader.parse(nodestringj, nodej))  
            {  
            cout << "parse json error" << endl;   
  
            } 
            
            string text = writer.write(nodei[j]["key"]);
            regex pattern("\"/(.*)/(.*)/(.*)\"\n");
            smatch results;
            int indexID;
            if (regex_match(text, results, pattern)) {
                    string Dnum = results[3];
                    int DDum = atoi(Dnum.c_str());
                    indexID=DDum-1;
                    cout<<"fragment:"<<DDum<<endl;
                    cout<<"j:"<<j<<endl;
                    f1.fragment[indexID].DBnum=DDum;
                    cout<<"f1:"<<f1.fragment[indexID].DBnum<<endl;
            }
            else {
            cout << "match failed: " << text << endl;
            }

            for(int k =0;k<nodej.size();k++)
            {
                Value nodek;
                string nodestringk = writer.write(nodej[k]["nodes"]);
                if(!reader.parse(nodestringk, nodek))  
                {  
                cout << "parse json error" << endl;   
 
                } 
            
                for(int w = 0;w<nodek.size();w++)
                {
                     Value nodew;
                    string nodestringw = writer.write(nodek[w]["nodes"]);
                    if(!reader.parse(nodestringw, nodew))  
                    {  
                    cout << "parse json error" << endl;   
  
                    } 
                    if(nodestringw.size()==5)
                    {
                        f1.fragment[indexID].condition_v_count=nodek.size();
                        string sss = writer.write(nodek[w]["value"]);
                        string kkk = writer.write(nodek[w]["key"]);
                        cout<<kkk<<endl;
                        regex pattern1("\"/(.*)/(.*)/(.*)/(.*)/(.*)\"\n");
                        smatch results1;
                        string indexs;
                        if (regex_match(kkk, results1, pattern1)) {
                             indexs = results1[5];
                            }
                        else {
                                cout << "match failed: " << text << endl;
                            }
                        cout<<"分片："<<indexs<<endl;
                        int v_index= stoi(indexs);
                        regex pattern("\"(.*)\"\n");
                        smatch results;
                        string Dnum;
                        if (regex_match(sss, results, pattern)) {
                             Dnum = results[1];

                            }
                        else {
                                cout << "match failed: " << text << endl;
                            }
                           
                        f1.fragment[indexID].condition_v[v_index]=Dnum;
                    }
                    else{
                        f1.fragment[indexID].condition_h_count=nodek.size();
                        for(int t=0;t<nodew.size();t++)
                        {
                           string attr_info1= "attr_name";
                           string attr_info2= "operation";
                           string attr_info3= "attr_value";
                           string s1 = writer.write(nodew[t]["key"]);
                           if(s1.find(attr_info1)<s1.length())
                           {    
            
                               string sss = writer.write(nodew[t]["value"]);
                               regex pattern("\"(.*)\"\n");
                                smatch results;
                                string Dnum;
                                if (regex_match(sss, results, pattern)) {
                                        Dnum = results[1];

                                }
                                else {
                                cout << "match failed: " << text << endl;
                                }
                               f1.fragment[indexID].condition_h[w].attr_name=Dnum; 
                               
                           }
                            if(s1.find(attr_info2)<s1.length())
                           {
                                string sss = writer.write(nodew[t]["value"]);
                               regex pattern("\"(.*)\"\n");
                                smatch results;
                                string Dnum;
                                if (regex_match(sss, results, pattern)) {
                                        Dnum = results[1];

                                }
                                else {
                                cout << "match failed: " << text << endl;
                                }
                               f1.fragment[indexID].condition_h[w].operation=Dnum;
                              
                           }
                            if(s1.find(attr_info3)<s1.length())
                           {
                                string sss = writer.write(nodew[t]["value"]);
                               regex pattern("\"(.*)\"\n");
                                smatch results;
                                string Dnum;
                                if (regex_match(sss, results, pattern)) {
                                      Dnum = results[1];
                                }
                                else {
                                cout << "match failed: " << text << endl;
                                }
                               f1.fragment[indexID].condition_h[w].attr_value=Dnum;
                               
                           }

                        }
                    }                 
                }
               
            }
        }

    
    return f1;   
}
Table To_Table(string &info,string &tablename)
//用json格式解析table的数据
{
    Table t1;
    t1.tb_name=tablename;
    Value root;
    Value node;
    Reader reader;  
    FastWriter writer; 
    string json = info;
     if(!reader.parse(json, root))  
    {  
        cout << "parse json error" << endl;   

    }  
    string nodeString = writer.write(root["node"]);  
    if(!reader.parse(nodeString, node))  
    {  
        cout << "parse json error" << endl;   
 
    }  
    Value nodei;
    string nodestringi = writer.write(node["nodes"]);
    if(!reader.parse(nodestringi, nodei))  
    {  
        cout << "parse json error" << endl;   
    }   
    t1.attr_count=nodei.size();
    for(int k = 0;k<nodei.size();k++)
    {
        Value nodek;
        string nodestringk = writer.write(nodei[k]["nodes"]);
         cout<<nodestringk<<endl;
        if(!reader.parse(nodestringk, nodek))  
        {  
            cout << "parse json error" << endl;   
        } 
        int index;
        string attr_info5= "index";
          
        for(int t =0;t<nodek.size();t++)
            {
                string nodestringt = writer.write(nodek[t]["key"]);
                if(nodestringt.find(attr_info5)<nodestringt.length())
                {
                    string sss = writer.write(nodek[t]["value"]);
                    regex pattern("\"(.*)\"\n");
                    smatch results;
                    string Dnum;
                    if (regex_match(sss, results, pattern)) {
                    Dnum = results[1];

                    }
                    else {
                            cout << "match failed: " << sss << endl;
                    }
                    index = stoi(Dnum);
                    cout<<"index:"<<index<<endl;
                }
            }
        for(int j =0;j<nodek.size();j++)
        {
            Value nodej;
            string nodestringj = writer.write(nodek[j]["key"]);
           
            string attr_info1= "attr_name";
            string attr_info2= "is_key";
            string attr_info3= "size";
            string attr_info4= "type";
          if(nodestringj.find(attr_info1)<nodestringj.length())
            {
                string sss = writer.write(nodek[j]["value"]);
                regex pattern("\"(.*)\"\n");
                smatch results;
                string Dnum;
                if (regex_match(sss, results, pattern)) {
                Dnum = results[1];

                }
                else {
                        cout << "match failed: " << sss << endl;
                }
                t1.attrs[index].attr_name=Dnum; 
                        
            }
            else if(nodestringj.find(attr_info2)<nodestringj.length())
            {
               
                string sss = writer.write(nodek[j]["value"]);
                regex pattern("\"(.*)\"\n");
                smatch results;
                string Dnum;
                if (regex_match(sss, results, pattern)) {
                Dnum = results[1];

                }
                else {
                        cout << "match failed: " << sss << endl;
                }
                if(Dnum=="true")
                {
                     t1.attrs[index].is_key=true;
                }
                else{
                    t1.attrs[index].is_key=false;
                }
                

                 
            }
            else if(nodestringj.find(attr_info3)<nodestringj.length())
            {
                string sss = writer.write(nodek[j]["value"]);
                regex pattern("\"(.*)\"\n");
                smatch results;
                string Dnum;
                if (regex_match(sss, results, pattern)) {
                Dnum = results[1];
                
                }
                else {
                        cout << "match failed: " << sss << endl;
                }
                t1.attrs[index].size=stoi(Dnum);
  
            
            }
            else if(nodestringj.find(attr_info4)<nodestringj.length())
            {
                               string sss = writer.write(nodek[j]["value"]);
                regex pattern("\"(.*)\"\n");
                smatch results;
                string Dnum;
                if (regex_match(sss, results, pattern)) {
                Dnum = results[1];

                }
                else {
                        cout << "match failed: " << sss << endl;
                }
                t1.attrs[index].type=Dnum;    


            }

        }
    }
    return t1;
}
Table Get_Table(string &tablename)
//获取table数据，并且返回T1.参数是表名
{
    string dir = "tableinfo/"+tablename;
    string info = Search_Value(dir);
    Table t1 = To_Table(info,tablename);
    // return t1;
    
    return t1;

}
Fragment1 Change_Frag(Fragment f1)
{
    Fragment1 f_c;
    f_c.tb_name=f1.tb_name;
    f_c.frag_count=f1.frag_count;

    for(int i=0;i<f1.frag_count;i++)
    {
        f_c.conditions[i].db_id=f1.fragment[i].DBnum;

        if(f1.fragment[i].condition_h_count==0)
        {
            if(f1.fragment[i].condition_v_count==0)
            {
                f_c.conditions[i].is_needed=false;
            }
            else{
                f_c.conditions[i].is_needed=true;
            }
            
            
        }
        else{
            f_c.conditions[i].is_needed=true;
        }
        f_c.conditions[i].h1.is_needed=false;
        f_c.conditions[i].h2.is_needed=false;
        for(int j =0;j<f1.fragment[i].condition_h_count;j++)
        {
            if(j==0)
            {
                f_c.conditions[i].h1.is_needed=true;
                f_c.conditions[i].h1.attr_name=f1.fragment[i].condition_h[j].attr_name;
                f_c.conditions[i].h1.attr_value=f1.fragment[i].condition_h[j].attr_value;
                f_c.conditions[i].h1.operation=f1.fragment[i].condition_h[j].operation;
            }
            else if(j==1)
            {
                f_c.conditions[i].h2.is_needed=true;
                f_c.conditions[i].h2.attr_name=f1.fragment[i].condition_h[j].attr_name;
                f_c.conditions[i].h2.attr_value=f1.fragment[i].condition_h[j].attr_value;
                f_c.conditions[i].h2.operation=f1.fragment[i].condition_h[j].operation;
            }
        }
        if(f1.fragment[i].condition_v_count==0)
        {
            f_c.conditions[i].v1.is_needed=false;
        }
        else{
            f_c.conditions[i].v1.is_needed=true;
            f_c.conditions[i].v1.attr_num=f1.fragment[i].condition_v_count;
            for(int k=0;k<f1.fragment[i].condition_v_count;k++)
            {
                
                f_c.conditions[i].v1.attr_list[k]=f1.fragment[i].condition_v[k];
            }
        }
        
    }
    return f_c;
}
Fragment1 Get_Fragment1(string &tablename)
{
    Fragment f1 = Get_Fragment(tablename);

    Fragment1 f_c = Change_Frag(f1);
    return f_c;
}
void Get_TableAttrs(string &tablename,string str[])
{//获取属性列名
    
    Table t1 = Get_Table(tablename);
    for(int i=0;i<t1.attr_count;i++)
    {
        str[i]=t1.attrs[i].attr_name;
    }
    
}

SiteInfos Get_SiteInfo()
{
    SiteInfos s;
    string dir = "siteinfo/";
    string info  = Search_Value(dir);
    Value root;  
    Value node;
    Value node1;
    Value node2;  
    Reader reader;  
    FastWriter writer; 
    string json = info;
     if(!reader.parse(json, root))  
    {  
        cout << "parse json error" << endl;   

    }  
    string nodeString = writer.write(root["node"]);  
    if(!reader.parse(nodeString, node))  
    {  
        cout << "parse json error" << endl;   
 
    }  
    Value nodei;
    string nodestringi = writer.write(node["nodes"]);
    if(!reader.parse(nodestringi, nodei))  
    {  
    cout << "parse json error" << endl;   
    }  
    s.sitenum = nodei.size();
    for(int j = 0;j<s.sitenum;j++)
        {
            string jj = writer.write(nodei[j]["key"]);
            regex pattern("\"/siteinfo/(.*)\"\n");
            smatch results;
            string Dnum1;
            if (regex_match(jj, results, pattern))
            {
                Dnum1 = results[1];
            }
            else 
            {
                cout << "match failed: " << jj << endl;
            }

            int index2 = stoi(Dnum1);
            Value nodej;
            string nodestringj = writer.write(nodei[j]["nodes"]);
            if(!reader.parse(nodestringj, nodej))  
            {  
            cout << "parse json error" << endl;   
            }          
            for(int k =0;k<nodej.size();k++)
            {
                string nodestringk = writer.write(nodej[k]["key"]);
                string attr_info1= "ip";
                string attr_info2= "port";
                string attr_info3= "db_name";
                
                if(nodestringk.find(attr_info1)<nodestringk.length())
                {
                    string sss = writer.write(nodej[k]["value"]);
                    regex pattern("\"(.*)\"\n");
                    smatch results;
                    string Dnum;
                    if (regex_match(sss, results, pattern)) {
                    Dnum = results[1];
                    }
                    else {
                            cout << "match failed: " << sss << endl;
                    }
                    s.site[j].ip=Dnum;
                    s.site[j].siteID=index2;                       
                }
                else if(nodestringk.find(attr_info2)<nodestringk.length())
                {
                    string sss = writer.write(nodej[k]["value"]);
                    regex pattern("\"(.*)\"\n");
                    smatch results;
                    string Dnum;
                    if (regex_match(sss, results, pattern)) {
                    Dnum = results[1];
                    }
                    else {
                            cout << "match failed: " << sss << endl;
                    }
                    s.site[j].port=Dnum;                  
                }
                else if(nodestringk.find(attr_info3)<nodestringk.length())
                {
                    string sss = writer.write(nodej[k]["value"]);
                    regex pattern("\"(.*)\"\n");
                    smatch results;
                    string Dnum;
                    if (regex_match(sss, results, pattern)) {
                    Dnum = results[1];
                    }
                    else {
                            cout << "match failed: " << sss << endl;
                    }
                    s.site[j].db_name=Dnum;                
                }
            }
        }
    return s;
}

bool Save_SiteInfo(SiteInfo site)
{//存储site数据
    map<int,string> mapsiteinfo;
    mapsiteinfo.insert(pair<int,string>(0,"ip"));
    mapsiteinfo.insert(pair<int,string>(1,"db_name"));
    mapsiteinfo.insert(pair<int,string>(2,"port"));
    
    SiteInfo s1 = site;
    
    string tablename = to_string(s1.siteID);
    string etcd_url = "/siteinfo";
    string dir = Create_Dir(etcd_url,tablename);
    if(dir!="fail")
    {
        string attr_info0 = mapsiteinfo[0];
        string key0 = dir+"/"+attr_info0;
        string value0 = s1.ip;
        if(!Insert_Attrvalue(key0,value0)==true)
        {return false;}
        string attr_info1 = mapsiteinfo[1];
        string key1 = dir+"/"+attr_info1;
        string value1 = s1.db_name;
        if(!Insert_Attrvalue(key1,value1)==true)
        {return false;}
        string attr_info2 = mapsiteinfo[2];
        string key2 = dir+"/"+attr_info2;
        string value2 = s1.port;
        if(!Insert_Attrvalue(key2,value2)==true)
        {return false;} 
        return true;
        }
        return false;
}