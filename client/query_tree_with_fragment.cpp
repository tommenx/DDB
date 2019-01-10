#include <iostream>
#include <string>
#include <stdio.h>
#include <malloc.h>
#include <map>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <cstddef>
#include <algorithm>
#include <thread>
#include "rpc_sql.h"
#include "local_sql.h"
#include "parser.h"
#define MaxTreeNode 100
#define MAX_JOINEDTABLE_NUM 40
// #define MAX_FROM_NUM 10
// #define MAX_COND_NUM 10
// #define MAX_JOIN_NUM 10
#define MAX_SELITEM_NUM 10
// #define MAX_ATTR_COUNT 20
#define MAX_DB_COUNT 5
// #define MAX_FRAG_COUNT 5
#define MAX_TABLE_COUNT 5
#define MAX_CHILD_COUNT 5

using namespace std;

string sites[4] = {"127.0.0.1","127.0.0.1","127.0.0.1","127.0.0.1"};
string dbs[4] = {"db1","db2","db3","db4"};
vector<int> select_ports = {8080,8080,8080,8080};


void initSelect(SiteInfos infos) {
	for(int i = 0;i < infos.sitenum; i++) {
		int id = infos.site[i].siteID -1;
		sites[id] = infos.site[i].ip;
		dbs[id] = infos.site[i].db_name;
		select_ports[id] = std::stoi(infos.site[i].port); 
	}
}

void showInfos() {
	for(int i = 0; i < 4; i++) {
		cout << sites[i] << ":" << select_ports[i] << " " << dbs[i] << endl;
	}
}

// enum OP{
// 	E=1,GE,G,LE,L,NE
// };
// struct SelItem{
// 	string table_name;
	
// 	string col_name;
// };
// struct FromItem{
// 	//Relname
// 	string tb_name;
// //	struct FromList *next;
// };
// struct SelectCondition{
// 	/*
// 	 * cond1:tb1.col_name1 op tb2.col_name2
// 	 * cond2:tb1.col_name1 op value;
// 	 */
	
// 	OP op;
// 	string tb_name;
// 	string col_name;
// 	string value;
// //	TYPE value_type;
// //	struct Condition* next;
// };
// struct Join{
// 	OP op;
// 	string tb_name1;
// 	string tb_name2;
// 	string col_name1;
// 	string col_name2;

// //	struct Join* next;
// };

// struct SelectQuery{
// //	int distinct;
// 	int sel_count;
// 	int cond_count;
// 	int from_count;
// 	int join_count;
// 	SelItem		SelList[MAX_SELITEM_NUM];
// 	FromItem 	FromList[MAX_FROM_NUM];
// 	Join 		JoinList[MAX_JOIN_NUM];
// 	SelectCondition 	CondList[MAX_COND_NUM];
// 	//SelectQuery* next;
// };

struct Opera_Selection{
	struct SelectCondition condition; 
}; 

struct Opera_Project{
	struct SelItem project;
};

struct Opera_SelPro{
	struct SelectCondition condition;
	string attr[MAX_ATTR_COUNT];
};

struct Query_tree_node{
	int type; //0 table 1 selection 2 projection 3 join 4 union 5 fragment  6select+project
	int index; //to build the map;
	int child[MAX_CHILD_COUNT];//left child right child   //after adding fragment information it can not be only two childs
	int parent;
	int childnum;
	int is_needed;
}; 

struct Query_tree{
	Query_tree_node tree_node;
	Query_tree* lchild;
	Query_tree* rchild;	
};

struct Table_change{
	string tablename;    //the change of tablename
	int whether_have_fragment; //0 do not have fragment 1 do have fragment
	int changed_operation_index;   //if this table joined by operation join which index is 4 then the changed_operation_index is 4 
	struct Table_change* next;
	int union_index;
};

struct Select_ConditionH{    //change name
    bool is_needed;
    string attr_name[MAX_ATTR_COUNT];
    OP operation[MAX_ATTR_COUNT];
    string attr_value[MAX_ATTR_COUNT];
};

struct Select_ConditionV{   //change name
    bool is_needed;
    string vKey; 
    int attr_num;
    string attr_list[MAX_ATTR_COUNT];
};

struct Select_ConditionHV{  //change name
    //bool is_needed;
    Select_ConditionH h1;
    int condition_h_column; //有几列有限制 
    //Select_ConditionH h2;
    Select_ConditionV v1;
    int db_id;
};

struct Select_Fragment{
    string mo_tb_name;
    string it_tb_name;
    int db_id;
    int frag_id;  //from 1 
    // id means db nums ->site num
    int type; //0 h 1 v
    Select_ConditionHV condition; 
};

// struct AttrInfo{
// 	string attr_name;
// 	string type;
// 	bool is_key;
// 	int size;
// };

struct Select_Table{
	string tb_name;
	AttrInfo attrs[MAX_ATTR_COUNT];
	int attr_count;
	int is_need_frag;
	int frag_count;
	Select_Fragment fraginfo[MAX_FRAG_COUNT];
	int frag_type; //0 h 1 v 2 mix
	int db_id_needed[MAX_DB_COUNT];
};

struct  Join_Middle_Table{
	Select_Table *newtable;
	string newtablename;
	int db_list[MAX_DB_COUNT];
	string attrs;
	string joinedtable[MAX_JOINEDTABLE_NUM];
	int joinedtable_db[MAX_JOINEDTABLE_NUM];
	//AttrInfo attrs[MAX_ATTR_COUNT];
};

map<int, Join_Middle_Table> join_middle_table;

struct TableList{
	int table_num;
	Select_Table* tbl[MAX_TABLE_COUNT];
};

struct Opera_Union{
	int childnum;
	Select_Table* tbl;
	int fraglist_needed[MAX_FRAG_COUNT];
};

struct Opera_Frag{
	Select_Table* tbl;
	int frag_id; //from 1
};

struct Join_Frag_Info{
	Join join;
	string tbl_to_stay;
	string tbl_to_broadcast;
	int frag_to_stay_id;
	int frag_to_broadcast_id;
	int cast_to_db_id;
	int from_db_id;
}; 

struct Opera_Join{
	struct Join join; 
	int is_done;
	string which_to_broadcast;
	string which_to_stay;
	int joinnum;
	Join_Frag_Info join_frag[MAX_FRAG_COUNT*MAX_FRAG_COUNT]; 
};

struct Query_Plan_Tree{
	Query_tree *tree;
	Query_tree_node query_tree_node[MaxTreeNode];
	int root_query_tree_node;   //the root of the node
	map<int, Table_change*> tree_tablename;
	map<int, Opera_Join> tree_join;
	map<int, Opera_Selection> tree_select;
	map<int, Opera_Project> tree_project;
	map<int, Opera_Union> tree_union;
	map<int, Opera_Frag> tree_frag;
	map<int, Opera_SelPro> tree_selpro;
	int tablenum;
	int joinnum;
	int selectnum;
	int projectnum;
	int fragnum;
	int unionnum;
	int treenodenum;
};

int initList(Table_change* tablelist)
{
	tablelist = (Table_change*)malloc(sizeof(Table_change));
	tablelist->next = NULL;
	return 1;
}
int getlength(Table_change* tablelist)
{
	Table_change* p = tablelist->next;
	int length = 0;
	while(p){
		length++;
		p = p->next;
	}
	return length;
}
void print_list(Table_change* tablelist)
{
	Table_change* p;
	p = tablelist->next;
	int i = 0;
	while(p){
		i++;
		cout<<"the tablename of "<<i<<" tablename is "<<p->tablename<<" which changed by op index "<<p->changed_operation_index<<endl;
		p = p->next;
	}
}
string getfirsttablename(Table_change* tablelist)
{
	return tablelist->next->tablename;
}
Query_tree* generate_tree(Query_tree_node* node,int current_root)
{
	int i;
	/*for(i = 1;i < num;i++) //test whether enter the correct parameter
		cout<<"i    index:"<<node[i].index<<"   type:"<<node[i].type<<"    lchild:"<<node[i].child[0]<<"    rchild:"<<node[i].child[1]<<endl;*/
	Query_tree *tree;
	if(current_root == 0)
		tree = NULL;
	else{
		tree = (Query_tree*)malloc(sizeof(Query_tree)); 
		tree->tree_node = node[current_root];
		//cout<<tree->tree_node.index<<endl;
		tree->lchild = generate_tree(node,node[current_root].child[0]);
		tree->rchild = generate_tree(node,node[current_root].child[1]);
	}
	return tree;
}
void print_tree(Query_tree* tree)
{
	if(tree)
	{
		cout<<"index:"<<tree->tree_node.index;
		if(tree->tree_node.type == 0) cout<<"  a tablename"<<endl;
		else if(tree->tree_node.type == 1) cout<<"  a selection operation"<<endl;
		else if(tree->tree_node.type == 2) cout<<"  a projection operation"<<endl;
		else if(tree->tree_node.type == 3) cout<<"  a join operation"<<endl;
		print_tree(tree->lchild);
		print_tree(tree->rchild);
	}
}
void print_query_plan_tree(Query_Plan_Tree *plantree)
{
	print_tree(plantree->tree);
	cout<<"follow is the description of each index:"<<endl;
	map<int, Table_change*>::iterator itertable;
	map<int, Opera_Join>::iterator iterjoin;
	map<int, Opera_Selection>::iterator iterselect;
	map<int, Opera_Project>::iterator iterproject;
	
	for(int i = 1 ;i <= plantree->treenodenum;i++)
	{
		itertable = plantree->tree_tablename.find(i);
		iterselect = plantree->tree_select.find(i);
		iterproject = plantree->tree_project.find(i);
		iterjoin = plantree->tree_join.find(i);
		if(itertable != plantree->tree_tablename.end())
			cout<<"index "<<i<<" is a table "<<getfirsttablename(itertable->second)<<endl;
		if(iterselect != plantree->tree_select.end())
		{
			cout<<"index "<<i<<" is a select operation and the selected table name is "<<iterselect->second.condition.tb_name<<" and the selected col is ";
			cout<<iterselect->second.condition.col_name<<" and the value is "<<iterselect->second.condition.value<<endl;
		}
		if(iterproject != plantree->tree_project.end())
		{
			cout<<"index "<<i<<" is a project operation and the project table name is "<<iterproject->second.project.table_name<<" and the project col is ";
			cout<<iterproject->second.project.col_name<<endl;
		}
		if(iterjoin != plantree->tree_join.end())
		{
			cout<<"index "<<i<<" is a join operation and the join table name is "<<iterjoin->second.join.tb_name1<<" and the col is ";
			cout<<iterjoin->second.join.col_name1<<" and the other table name is "<<iterjoin->second.join.tb_name2<<" and the col is "<<iterjoin->second.join.col_name2<<endl;
		}
	}

}
void print_query_plan_tree_with_fragment(Query_Plan_Tree *plantree)
{
	//print_tree(plantree->tree);
	cout<<"follow is the description of each index:"<<endl;
	map<int, Table_change*>::iterator itertable;
	map<int, Opera_Join>::iterator iterjoin;
	map<int, Opera_Selection>::iterator iterselect;
	map<int, Opera_Project>::iterator iterproject;
	map<int, Opera_Union>::iterator iterunion;
	map<int, Opera_Frag>::iterator iterfrag;
	map<int, Opera_SelPro>::iterator iterselpro;
	
	for(int i = 1 ;i <= plantree->treenodenum;i++)
	{
		itertable = plantree->tree_tablename.find(i);
		iterselect = plantree->tree_select.find(i);
		iterproject = plantree->tree_project.find(i);
		iterjoin = plantree->tree_join.find(i);
		iterunion = plantree->tree_union.find(i);
		iterfrag = plantree->tree_frag.find(i);
		iterselpro = plantree->tree_selpro.find(i);
		if(itertable != plantree->tree_tablename.end())
			cout<<"index "<<i<<" is a table "<<getfirsttablename(itertable->second)<<endl;
		if(iterselect != plantree->tree_select.end())
		{
			cout<<"index "<<i<<" is a select operation and the selected table name is "<<iterselect->second.condition.tb_name<<" and the selected col is ";
			cout<<iterselect->second.condition.col_name<<" and the value is "<<iterselect->second.condition.value<<endl;
		}
		if(iterproject != plantree->tree_project.end())
		{
			cout<<"index "<<i<<" is a project operation and the project table name is "<<iterproject->second.project.table_name<<" and the project col is ";
			cout<<iterproject->second.project.col_name<<endl;
		}
		if(iterjoin != plantree->tree_join.end())
		{
			cout<<"index "<<i<<" is a join operation and the join table name is "<<iterjoin->second.join.tb_name1<<" and the col is ";
			cout<<iterjoin->second.join.col_name1<<" and the other table name is "<<iterjoin->second.join.tb_name2<<" and the col is "<<iterjoin->second.join.col_name2<<endl;
		}
		if(iterunion != plantree->tree_union.end())
		{
			cout<<"index "<<i<<" is a union operation and it union table "<<iterunion->second.tbl->tb_name<<endl;
		}
		if(iterfrag != plantree->tree_frag.end())
		{
			cout<<"index "<<i<<" is a fragment of table "<<iterfrag->second.tbl->tb_name<<"'s number "<<iterfrag->second.frag_id<<" fragment"<<endl;
		}
		if(iterselpro != plantree->tree_selpro.end())
		{
			cout<<"index "<<i<<" is a select and projection operation and the selected table name is "<<iterselpro->second.condition.tb_name<<" and the selected col is ";
			cout<<iterselpro->second.condition.col_name<<" and the value is "<<iterselpro->second.condition.value;
			cout<<" and project column ";
			for(int k = 0;k<MAX_ATTR_COUNT;k++)
				if(iterselpro->second.attr[k]!="")
					cout<<iterselpro->second.attr[k]<<"   ";
			cout<<endl;
		}
	}

}
void create_ori_tree(SelectQuery &SQ,Query_Plan_Tree &qpt)
{
	Query_tree_node _query_tree_node[MaxTreeNode];  //generate nodes to build a tree 
	Query_Plan_Tree _query_plan_tree;               //the final tree
	Opera_Join _opera_join; 
	Opera_Selection _opera_selection;
	Opera_Project _opera_project;
	//map<int, int> is_table_join;                   //judge whether the table joined
	int i;
	int num = 1;                                   //count of index
	_query_plan_tree.tablenum = SQ.from_count;
//	cout<<_query_plan_tree->tablenum<<endl;
	for(i=1;i<=SQ.from_count;i++)                  //create table node and fill the map
	{
		//从尾部插入链表的方法，初始化这个列表，并且将列表第一个元素设为表的名字 
		Table_change* tablehead = new Table_change;
		//tablehead = (Table_change*)malloc(sizeof(Table_change));
		if(tablehead == NULL)
		{
			cout<<"空间分配失败"<<endl;
			exit(-1); 
		 } 
		tablehead->next = NULL;
		
		Table_change* tabletail = new Table_change;
		tabletail = tablehead;
		
		Table_change* table_temp = new Table_change;
		
		//table_change_list = (Table_change*)malloc(sizeof(Table_change));
		if(table_temp == NULL)
		{
			cout<<"空间分配失败"<<endl;
			exit(-1); 
		 }
		 //tablename是表的初始名
		 //index等于0说明没有被修改过。 
		table_temp->tablename = SQ.FromList[i-1].tb_name;
		table_temp->changed_operation_index = 0; //haven't been changed
		tabletail->next = table_temp;
		tabletail = table_temp;
		table_temp = NULL; 
		
		tabletail->next = NULL;
	//	print_list(tablehead);
	//	Table_change* p;
	//	p = tablehead->next;
	//	cout<<p->changed_operation_index<<endl;
	//	cout<<p->tablename<<endl;
		//构建树的节点集 其中type是表。 
		_query_tree_node[num].type = 0;   //from 1
		_query_tree_node[num].index = num;
		_query_tree_node[num].child[0] = 0;
		_query_tree_node[num].child[1] = 0;
		_query_plan_tree.tree_tablename.insert(pair<int,Table_change*>(num,tablehead));
		num++;
	}
	//判断map 树表  是否正确 
/*	map<int, Table_change*>::iterator iter;
	for(iter=_query_plan_tree.tree_tablename.begin();iter!=_query_plan_tree.tree_tablename.end();iter++)
	{
		cout<<iter->first<<endl;  
		print_list(iter->second);
		//cout<<getlength(iter->second)<<endl;
	}*/
	// done
	_query_plan_tree.joinnum = SQ.join_count;
	for(i=1;i<=SQ.join_count;i++)
	{
		//构建连接的情况的节点集 
		_query_tree_node[num].type = 3;
		_query_tree_node[num].index = num;
		map<int, Table_change*>::iterator iter;
		//在表的map中找这个表是不是被join过 
		for(iter=_query_plan_tree.tree_tablename.begin();iter!=_query_plan_tree.tree_tablename.end();iter++)
		{
			//找到第一个表 
			if(getfirsttablename(iter->second) == SQ.JoinList[i-1].tb_name1)
			{
				//cout<<getfirsttablename(iter->second)<<endl;
				if(getlength(iter->second) == 1) //haven't been joined  如果没有被join过，直接修改child的值，在修改表的链表。 
				{
					_query_tree_node[num].child[0] = iter->first;
					
					Table_change* temp2;
					Table_change* p = iter->second; //在末尾加上这个表的新名字和这个表是被哪个index修改的 
					while(p){
						temp2 = p;
						p = p->next;
					}
					p = new Table_change;
					string jointablename = SQ.JoinList[i-1].tb_name1 + "join" + SQ.JoinList[i-1].tb_name2;
					p->tablename = jointablename;
					p->changed_operation_index = num;
					p->next = NULL;
					temp2->next = p;
				}
				else   //如果被修改了的话，左孩子是上一次被修改的那个index，在修改表的链表。 
				{
					Table_change* temp3 = iter->second;
					int tempindex;
					while(temp3->next)
					{
						temp3 = temp3->next;
					}
					_query_tree_node[num].child[0] = temp3->changed_operation_index;
					
					Table_change* temp4;
					Table_change* p2 = iter->second;
					while(p2){
						temp4 = p2;
						p2 = p2->next;
					}
					p2 = new Table_change;
					string jointablename = "("+temp3->tablename + ")join" + SQ.JoinList[i-1].tb_name2;
					p2->tablename = jointablename;
					p2->changed_operation_index = num;
					p2->next = NULL;
					temp4->next = p2;
				}

			} 
			
			if(getfirsttablename(iter->second) == SQ.JoinList[i-1].tb_name2)
			{
				//cout<<getfirsttablename(iter->second)<<endl;
				if(getlength(iter->second) == 1) //haven't been joined
				{
					_query_tree_node[num].child[1] = iter->first;
					Table_change* temp2;
					Table_change* p = iter->second;
					while(p){
						temp2 = p;
						p = p->next;
					}
					p = new Table_change;
					string jointablename = SQ.JoinList[i-1].tb_name1 + "join" + SQ.JoinList[i-1].tb_name2;
					p->tablename = jointablename;
					p->changed_operation_index = num;
					p->next = NULL;
					temp2->next = p;
				}
				else
				{
					Table_change* temp3 = iter->second;
					int tempindex;
					while(temp3->next)
					{
						temp3 = temp3->next;
					}
					_query_tree_node[num].child[1] = temp3->changed_operation_index;
					
					Table_change* temp4;
					Table_change* p2 = iter->second;
					while(p2){
						temp4 = p2;
						p2 = p2->next;
					}
					p2 = new Table_change;
					string jointablename = "("+temp3->tablename + ")join" + SQ.JoinList[i-1].tb_name2;
					p2->tablename = jointablename;
					p2->changed_operation_index = num;
					p2->next = NULL;
					temp4->next = p2;
				}

			} 
		}
		/*map<int, Table_change*>::iterator iter_test;
		for(iter_test=_query_plan_tree.tree_tablename.begin();iter_test!=_query_plan_tree.tree_tablename.end();iter_test++)
		{
			cout<<iter_test->first<<endl;  
			print_list(iter_test->second);
			//cout<<getlength(iter->second)<<endl;
		}*/
		_opera_join.join = SQ.JoinList[i-1];
		_query_plan_tree.tree_join.insert(pair<int,Opera_Join>(num,_opera_join));
		num++;
		
	}

	_query_plan_tree.selectnum = SQ.cond_count;
	for(i = 1;i <= SQ.cond_count;i++)
	{
		_query_tree_node[num].type = 1;
		_query_tree_node[num].index = num;
		_query_tree_node[num].child[0] = num-1;//左孩子是上一次的join操作的结尾 
		_query_tree_node[num].child[1] = 0; //右孩子为空
		_opera_selection.condition = SQ.CondList[i-1];
		_query_plan_tree.tree_select.insert(pair<int,Opera_Selection>(num,_opera_selection));
		num++;
	}
	
	_query_plan_tree.projectnum = SQ.sel_count;
	for(i=1;i<=SQ.sel_count;i++)
	{
		_query_tree_node[num].type = 2;
		_query_tree_node[num].index = num;
		_query_tree_node[num].child[0] = num-1;//左孩子是上一次的join操作的结尾 
		_query_tree_node[num].child[1] = 0; //右孩子为空
		_opera_project.project = SQ.SelList[i-1];
		_query_plan_tree.tree_project.insert(pair<int,Opera_Project>(num,_opera_project));
		num++;
	}
	
	_query_plan_tree.treenodenum = num-1;
	for(int j = 1;j<=_query_plan_tree.treenodenum;j++)
	{
		_query_plan_tree.query_tree_node[j] = _query_tree_node[j];
	}
	
	Query_tree* _query_tree;
	_query_tree = generate_tree(_query_tree_node,_query_plan_tree.treenodenum);
//	print_tree(_query_tree);
	_query_plan_tree.tree = _query_tree;
//	Query_Plan_Tree *qpt = &_query_plan_tree;
	qpt = _query_plan_tree;
	Query_Plan_Tree *xx = &qpt;
/*	print_query_plan_tree(xx);
	map<int, Table_change*>::iterator itertabletest;
	for(itertabletest = xx->tree_tablename.begin();itertabletest!=xx->tree_tablename.end();itertabletest++)
		cout<<"test tablename map"<<endl<<itertabletest->first<<"    "<<getfirsttablename(itertabletest->second)<<endl;*/
/*	for(i = 1;i < num;i++)
		cout<<"i    index:"<<_query_tree_node[i].index<<"   type:"<<_query_tree_node[i].type<<"    lchild:"<<_query_tree_node[i].child[0]<<"    rchild:"<<_query_tree_node[i].child[1]<<endl;
	map<int, Table_change*>::iterator iter_test;
	for(iter_test=_query_plan_tree.tree_tablename.begin();iter_test!=_query_plan_tree.tree_tablename.end();iter_test++)
	{
		cout<<"index:"<<iter_test->first<<endl;  
		print_list(iter_test->second);
		cout<<"length:"<<getlength(iter_test->second)<<endl;
	}	
	map<int, Opera_Join>::iterator iter_join_tes;
	for(iter_join_tes=_query_plan_tree.tree_join.begin();iter_join_tes!=_query_plan_tree.tree_join.end();iter_join_tes++)
	{
		cout<<"index:"<<iter_join_tes->first<<endl;
		cout<<"value:"<<iter_join_tes->second.join.tb_name1<<"   "<<iter_join_tes->second.join.tb_name2<<"   "<<iter_join_tes->second.join.col_name1<<"   "<<iter_join_tes->second.join.col_name2<<endl;
	}
	map<int,Opera_Selection>::iterator iter_selection_test;
	for(iter_selection_test=_query_plan_tree.tree_select.begin();iter_selection_test!=_query_plan_tree.tree_select.end();iter_selection_test++)
	{
		cout<<"index:"<<iter_selection_test->first<<endl;
		cout<<"value:"<<iter_selection_test->second.condition.tb_name<<"   "<<iter_selection_test->second.condition.col_name<<"   "<<iter_selection_test->second.condition.op<<"   "<<iter_selection_test->second.condition.value<<endl;
	}
	map<int,Opera_Project>::iterator iter_project_test;
	for(iter_project_test=_query_plan_tree.tree_project.begin();iter_project_test!=_query_plan_tree.tree_project.end();iter_project_test++)
	{
		cout<<"index:"<<iter_project_test->first<<endl;
		cout<<"value:"<<iter_project_test->second.project.table_name<<"   "<<iter_project_test->second.project.col_name<<endl;
	}*/
//	return  qpt;
}
void modify_select_tree(Query_Plan_Tree *plantree,int selkey,int tblkey)
{
	int i;
	//selkey's parent point to selkey's child
	//selkey point to tblkey
	//tblkey's parent point to selkey
	int temp = plantree->query_tree_node[selkey].child[0];
	//cout<<temp<<endl;
	plantree->query_tree_node[selkey].child[0]=tblkey;
	//cout<<plantree->query_tree_node[selkey].child[0]<<endl;
	for(i = 1;i<=plantree->treenodenum;i++)
	{
		if(plantree->query_tree_node[i].child[0]==selkey)  {
			//cout<<plantree->query_tree_node[i].index<<endl;
			plantree->query_tree_node[i].child[0] = temp;
		}
		if((plantree->query_tree_node[i].child[0] == tblkey) && (plantree->query_tree_node[i].index != selkey)){
			//cout<<plantree->query_tree_node[i].index;
			plantree->query_tree_node[i].child[0] = selkey;
		} 
		if(plantree->query_tree_node[i].child[1] == tblkey) plantree->query_tree_node[i].child[1] = selkey;
	}
	//for(i = 1;i <= plantree->treenodenum;i++)
	//	cout<<"i    index:"<<plantree->query_tree_node[i].index<<"   type:"<<plantree->query_tree_node[i].type<<"    lchild:"<<plantree->query_tree_node[i].child[0]<<"    rchild:"<<plantree->query_tree_node[i].child[1]<<endl;
}
void put_down_select(Query_Plan_Tree*  originate_tree)
{
	map<int, Opera_Selection>::iterator iter;
	cout<<endl<<"put down select operation"<<endl<<endl;
	map<int, Table_change*>::iterator itertable;
/*	for(itertable = originate_tree->tree_tablename.begin();itertable != originate_tree->tree_tablename.end();itertable++){
		cout<<itertable->first<<endl;
	}*/
	for(iter = originate_tree->tree_select.begin();iter != originate_tree->tree_select.end();iter++)
	{
		string tbname = iter->second.condition.tb_name;//find the table name of each selection
		int tmpindex = 0;
		//cout<<tbname<<endl;
		
		for(itertable = originate_tree->tree_tablename.begin();itertable != originate_tree->tree_tablename.end();itertable++)
		{
			//cout<<itertable->first<<endl;
			if(getfirsttablename(itertable->second) == tbname){
				//cout<<getfirsttablename(itertable->second)<<endl;
				tmpindex = itertable->first;
				//cout<<tmpindex<<endl;
			}
		}
		modify_select_tree(originate_tree,iter->first,tmpindex);
		
	}
	originate_tree->tree = generate_tree(originate_tree->query_tree_node,originate_tree->treenodenum);
	print_query_plan_tree(originate_tree);
}
void find_all_attr_needed(TableList* tblist, SelectQuery &SQ,string tablename,string attr[])
{
	int i,j;
	for(i = 0;i < MAX_ATTR_COUNT ;i++)
		attr[i] = "";
	int flag = 0;
	int have_putin = 0;
	for(i = 0;i < SQ.sel_count;i++)
	{
		if(SQ.SelList[i].table_name == tablename)
		{
			attr[flag] = SQ.SelList[i].col_name;
			flag++;
		}
	}
	for(i = 0;i < SQ.cond_count;i++)
	{
		if(SQ.CondList[i].tb_name == tablename)
		{
			have_putin = 0;
			for(j = 0;j < flag;j++)
			{
				if(SQ.CondList[i].col_name == attr[j])
					have_putin = 1;
			}
			if(have_putin == 0)
			{
				attr[flag] = SQ.CondList[i].col_name;
				flag ++;
			}
		}
	}
	for(i = 0;i < SQ.join_count;i++)
	{
		if(SQ.JoinList[i].tb_name1 == tablename)
		{
			have_putin = 0;
			for(j = 0;j < flag;j++)
			{
				if(SQ.JoinList[i].col_name1 == attr[j])
					have_putin = 1;
			}
			if(have_putin == 0)
			{
				attr[flag] = SQ.JoinList[i].col_name1;
				flag ++;
			}
		}
		if(SQ.JoinList[i].tb_name2 == tablename)
		{
			have_putin = 0;
			for(j = 0;j < flag;j++)
			{
				if(SQ.JoinList[i].col_name2 == attr[j])
					have_putin = 1;
			}
			if(have_putin == 0)
			{
				attr[flag] = SQ.JoinList[i].col_name2;
				flag ++;
			}
		}
	}
	for(i = 0;i < tblist->table_num;i++)
	{
		if(tablename == tblist->tbl[i]->tb_name)
		{
			for(j = 0;j<tblist->tbl[i]->attr_count;j++)
			{
				if(tblist->tbl[i]->attrs[j].is_key)
				{
					have_putin = 0;
					for(int k = 0; k < flag;k++)
					{
						if(tblist->tbl[i]->attrs[j].attr_name == attr[k])
							have_putin = 1;
					}
					if(have_putin == 0)
					{
						attr[flag] = tblist->tbl[i]->attrs[j].attr_name;
						flag++;
					}
				}
			}
		}
	}
}
void tree_to_fragment(SelectQuery &SQ,Query_Plan_Tree*  tree, TableList* tblist)
{
	cout<<endl;
	cout<<"with fragment information:"<<endl;
	cout<<endl;
	//cout<<"into the function"<<endl;
	tree->root_query_tree_node = tree->treenodenum;   //notice the root of the tree
	//cout<<"the root is"<<tree->root_query_tree_node<<endl;
	int i,j;
	for(i = 1;i < MaxTreeNode;i++)
	{
		tree->query_tree_node[i].childnum = 0;
		tree->query_tree_node[i].parent = 0;
		for(j = 2;j < MAX_CHILD_COUNT;j++)
			tree->query_tree_node[i].child[j] = 0;
	}
	for(i = 1;i <= tree->treenodenum;i++)  //find each node's parent
	{
		if(tree->query_tree_node[i].child[0]!=0)
		{
			tree->query_tree_node[i].childnum = 1;
			tree->query_tree_node[tree->query_tree_node[i].child[0]].parent = i;
		}
		if(tree->query_tree_node[i].child[1]!=0)
		{
			tree->query_tree_node[i].childnum = 2;
			tree->query_tree_node[tree->query_tree_node[i].child[1]].parent = i;
		}
	}
/*	for(i = 1;i <= tree->treenodenum;i++)
	{
		if((tree->query_tree_node[i].parent == 0) && (i !=tree->root_query_tree_node))
			cout<<"ERROR:some node in query tree do not have parent!"<<endl;
		cout<<"node "<<i<<"  left child is "<<tree->query_tree_node[i].child[0]<<" and right child is "<<tree->query_tree_node[i].child[1]<<" and parent id "<<tree->query_tree_node[i].parent<<endl;
	}*/
	map<int, Table_change*>::iterator itertable;
	Opera_Union _opera_union;
	tree->unionnum = 0;
	tree->fragnum = 0;
	int num = tree->treenodenum;
	num++;
	for(itertable = tree->tree_tablename.begin();itertable != tree->tree_tablename.end(); itertable++)  // 构造union节点 
	{
		string currtbl_name = getfirsttablename(itertable->second);
		//cout<<"table name:"<<currtbl_name<<endl;
		for(i = 0; i < tblist->table_num; i++)
		{
			if(tblist->tbl[i]->tb_name == currtbl_name)
			{
				if(tblist->tbl[i]->is_need_frag = 1)
				{
					itertable->second->whether_have_fragment = 1; //update whether the table have a fragment
					itertable->second->union_index = num;
					_opera_union.tbl = tblist->tbl[i];
					_opera_union.childnum = tblist->tbl[i]->frag_count;
					for(j = 0; j < tblist->tbl[i]->frag_count;j++)
					{
						_opera_union.fraglist_needed[j] = tblist->tbl[i]->fraginfo[j].frag_id;
					}
					tree->query_tree_node[num].type = 4;
					tree->query_tree_node[num].index = num;
					tree->query_tree_node[num].childnum = tblist->tbl[i]->frag_count;
					tree->tree_union.insert(pair<int,Opera_Union>(num,_opera_union));
					num++;
					tree->unionnum++;
				}
				break;
			}
		}
	}
/*	map<int, Opera_Union>::iterator iterunion;
	for(iterunion = tree->tree_union.begin();iterunion != tree->tree_union.end(); iterunion++)
	{
		cout<<iterunion->first<<"   "<<iterunion->second.tbl->tb_name<<"  "<<iterunion->second.childnum<<"  ";
		for(j = 0;j<iterunion->second.childnum;j++)
		{
			cout<<iterunion->second.fraglist_needed[j]<<" ";
		}
		cout<<endl;
	}*/

	//构造fragment节点
	Opera_Frag _opera_frag;
	map<int, Opera_Union>::iterator iterunion;
	for(iterunion = tree->tree_union.begin();iterunion != tree->tree_union.end(); iterunion++) 
	{
		for(j = 0;j < iterunion->second.tbl->frag_count;j++)
		{
			_opera_frag.tbl = iterunion->second.tbl;
			_opera_frag.frag_id = iterunion->second.tbl->fraginfo[j].frag_id;
			tree->tree_frag.insert(pair<int,Opera_Frag>(num,_opera_frag));
			tree->query_tree_node[num].type = 5;
			tree->query_tree_node[num].index = num;
			tree->query_tree_node[num].childnum = 0;
			tree->query_tree_node[num].parent = iterunion->first;
			tree->query_tree_node[iterunion->first].child[j] = num; 
			num++;
			tree->fragnum++;
		}
	}
	tree->treenodenum = num - 1;
	//update is_needed
	for(i = 1;i <= tree->treenodenum;i++)
		tree->query_tree_node[i].is_needed = 1;
/*	map<int, Opera_Frag>::iterator iterfrag;
	for(iterfrag = tree->tree_frag.begin();iterfrag != tree->tree_frag.end(); iterfrag++)
		cout<<iterfrag->first<<"   "<<iterfrag->second.tbl->tb_name<<"  "<<iterfrag->second.frag_id<<endl;*/
	//cout<<"unionnum "<<tree->unionnum<<"   fragnum "<<tree->fragnum<<endl;
	
/*	for(i = 1;i <= tree->treenodenum;i++)
	{
		cout<<"node "<<i;
		for(j = 0;j<MAX_FRAG_COUNT;j++)
		{
			cout<<" child "<<j<<" is "<<tree->query_tree_node[i].child[j];
		}
		cout<<endl;
	}*/
	
	//print_query_plan_tree_with_fragment(tree);
	//update the query_plan_tree to add fragment infomation in it;
	for(itertable = tree->tree_tablename.begin();itertable != tree->tree_tablename.end(); itertable++)
	{
		if(itertable->second->whether_have_fragment == 1)
		{
			//cout<<itertable->second->union_index<<endl;
			//cout<<"tablename is:"<<getfirsttablename(itertable->second)<<endl;
			int par = tree->query_tree_node[itertable->first].parent;
			int parpar = tree->query_tree_node[par].parent;// parent's parent
			//cout<<"parent: "<<par<<endl;  //3
			int ch,chch; //children and children's children
			for(i = 0;i < MAX_CHILD_COUNT;i++)
			{
				if(tree->query_tree_node[par].child[i] == itertable->first)
					ch = i;
				if(tree->query_tree_node[parpar].child[i] == par)
					chch = i;
			}
			//cout<<"the "<<ch<<" child of par is the table"<<endl;
			if(tree->query_tree_node[par].type != 1)
			{
				//cout<<"the index which parent is not a selection operation is "<<itertable->first<<endl;
				tree->query_tree_node[par].child[ch] = itertable->second->union_index;
				//cout<<"the new child[ch] of par is "<<tree->query_tree_node[par].child[ch]<<endl;
				tree->query_tree_node[itertable->second->union_index].parent = par;
				tree->query_tree_node[itertable->first].parent = -1;
				tree->query_tree_node[itertable->first].is_needed = 0;
			}
			else
			{
				while(tree->query_tree_node[par].type == 1)
				{
					map<int,Opera_SelPro>::iterator iterselpro;
					map<int,Opera_Selection>::iterator iterselect;
					iterselect = tree->tree_select.find(par);
					if(iterselect == tree->tree_select.end())
						cout<<"ERROR: can not find the select operation"<<endl;
					Opera_SelPro _opera_selpro;
					_opera_selpro.condition = iterselect->second.condition;
					find_all_attr_needed(tblist,SQ,getfirsttablename(itertable->second),_opera_selpro.attr);
					iterunion = tree->tree_union.find(itertable->second->union_index);
					int newselect = iterunion->second.childnum;
					//cout<<"the newselect num is "<<newselect<<endl;
					for(j = 1; j  <= newselect; j++)
					{
						tree->query_tree_node[num].type = 6;
						tree->query_tree_node[num].index = num;
						tree->query_tree_node[num].childnum = 1;
						tree->query_tree_node[num].parent = itertable->second->union_index;
						tree->query_tree_node[num].child[0] = tree->query_tree_node[itertable->second->union_index].child[j-1];
						tree->query_tree_node[tree->query_tree_node[itertable->second->union_index].child[j-1]].parent = num;
						tree->query_tree_node[itertable->second->union_index].child[j-1] = num;
						tree->tree_selpro.insert(pair<int,Opera_SelPro>(num,_opera_selpro));
						tree->query_tree_node[itertable->second->union_index].parent = tree->query_tree_node[par].parent;
					
						//cout<<tree->query_tree_node[itertable->second->union_index].child[j-1]<<endl;
						num++;
					} 
				
					//cout<<"parent of index "<<tree->query_tree_node[par].index<<"is "<<tree->query_tree_node[par].parent<<endl;
					tree->query_tree_node[par].is_needed = 0;
					tree->query_tree_node[itertable->first].parent = -1;
					tree->query_tree_node[par].childnum = 0;
					tree->query_tree_node[par].child[0] = 0;
					
					//cout<<"node "<<par<<" parent is "<<tree->query_tree_node[par].parent<<endl;
					parpar = tree->query_tree_node[par].parent;
					for(int kk = 0;kk < MAX_CHILD_COUNT;kk++)
					{
						if(tree->query_tree_node[par].child[kk] == itertable->first)
							ch = kk;
						if(tree->query_tree_node[parpar].child[kk] == par)
							chch = kk;
					}
					tree->query_tree_node[parpar].child[chch] = itertable->second->union_index;
				
				
					for(j = 1;j < MAX_CHILD_COUNT;j++)
						tree->query_tree_node[par].child[j] = 0;
					par = parpar;
				}
				//tree->query_tree_node[par].parent = -1;
			}
		}
	}
	map<int,Opera_Selection>::iterator iterselect;
	for(iterselect = tree->tree_select.begin();iterselect!=tree->tree_select.end();iterselect ++)
		tree->query_tree_node[iterselect->first].parent = -1;
	tree->treenodenum = num-1;
/*	for(i = 1;i <= tree->treenodenum;i++)
	{
		cout<<"node "<<i<<" childnum is "<<tree->query_tree_node[i].childnum<<endl;
	}*/
	for(i = 1;i <= tree->treenodenum;i++)
	{
		cout<<"node "<<i<<" parent is "<<tree->query_tree_node[i].parent<<" and child is ";
		for(j = 0;j<tree->query_tree_node[i].childnum;j++)
		{
			cout<<tree->query_tree_node[i].childnum<<endl;
			cout<<tree->query_tree_node[i].child[j]<<"   ";
		}
		cout<<endl;
	}
	print_query_plan_tree_with_fragment(tree);
}
int judge_contradiction_int(OP op1,OP op2,int value1,int value2)
{
		switch (op1){
		case E:{
			switch (op2)
			{
				case E:{
					if(value1 != value2)  return 1;
					else return 0;
					break;
				}
				case GE:{
					if(value2 >value1) return 1;
					else return 0;
					break;
				}
				case G:{
					if(value2>=value1)  return 1;
					else return 0;
					break;
				}
				case LE:{
					if(value2<value1) return 1;
					else return 0;
					break;
				}
				case L:{
					if(value2<=value1) return 1;
					else return 0;
					break;
				}
				case NE:{
					if(value2==value1) return 1;
					else return 0;
					break;
				}
			}
			break;
		}
		case GE:{
			switch (op2)
			{ 
				case E:{
					if(value2 < value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					return 0;
					break;
				}
				case G:{
					return 0;
					break;
				}
				case LE:{
					if(value2<value1) return 1;
					else return 0;
					break;
				}
				case L:{
					if(value2<=value1) return 1;
					else return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}
			} 
			break;
		}
		case G:{
			switch (op2){ 
				case E:{
					if(value2 < value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					return 0;
					break;
				}
				case G:{
					return 0;
					break;
				}
				case LE:{
					if(value2<value1) return 1;
					else return 0;
					break;
				}
				case L:{
					//cout<<"this situation!"<<endl;
					//if((value2<=value1)||(valueint2<=valueint1))  cout<<"have contradiction!"<<endl;
					if(value2<=value1) return 1;
					else return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}	
			}		
			break;
		}
		case LE:{
			switch (op2){ 
				case E:{
					if(value2 > value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					if(value2>value1)  return 1;
					else return 0;
					break;
				}
				case G:{
					if(value2>=value1) return 1;
					else return 0;
					break;
				}
				case LE:{
					return 0;
					break;
				}
				case L:{
					return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}		
			}				
			break;
		}
		case L:{
			switch (op2){ 
				case E:{
					if(value2 >= value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					if(value2>=value1)  return 1;
					else return 0;
					break;
				}
				case G:{
					if(value2>=value1) return 1;
					else return 0;
					break;
				}
				case LE:{
					return 0;
					break;
				}
				case L:{
					return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}	
			}					
			break;
		}
		case NE:{
			switch (op2)
			{ 
				case E:{
					if(value2 == value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					return 0;
					break;
				}
				case G:{
					return 0;
					break;
				}
				case LE:{
					return 0;
					break;
				}
				case L:{
					return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}
			}						
			break;
		}
	}
}
int judge_contradiction(OP op1,OP op2,string value1,string value2,string datatype) //return 1 have contradiction  return 0 do not have contradiction
{
	//cout<<"op1 "<<op1<<" values1 "<<value1<<" op2 "<<op2<<" value2 "<<value2<<endl;
	if(datatype == "integer")
	{
		int valueint1 = atoi(value1.c_str());
		int valueint2 = atoi(value2.c_str());
		//cout<<"value1 "<<value1<<" to "<<valueint1<<" value2 "<<value2<<" to "<<valueint2<<endl;
		return judge_contradiction_int(op1,op2,valueint1,valueint2);
	}
	switch (op1){
		case E:{
			switch (op2)
			{
				case E:{
					if(value1 != value2)  return 1;
					else return 0;
					break;
				}
				case GE:{
					if(value2 >value1) return 1;
					else return 0;
					break;
				}
				case G:{
					if(value2>=value1)  return 1;
					else return 0;
					break;
				}
				case LE:{
					if(value2<value1) return 1;
					else return 0;
					break;
				}
				case L:{
					if(value2<=value1) return 1;
					else return 0;
					break;
				}
				case NE:{
					if(value2==value1) return 1;
					else return 0;
					break;
				}
			}
			break;
		}
		case GE:{
			switch (op2)
			{ 
				case E:{
					if(value2 < value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					return 0;
					break;
				}
				case G:{
					return 0;
					break;
				}
				case LE:{
					if(value2<value1) return 1;
					else return 0;
					break;
				}
				case L:{
					if(value2<=value1) return 1;
					else return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}
			} 
			break;
		}
		case G:{
			switch (op2){ 
				case E:{
					if(value2 < value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					return 0;
					break;
				}
				case G:{
					return 0;
					break;
				}
				case LE:{
					if(value2<value1) return 1;
					else return 0;
					break;
				}
				case L:{
					//cout<<"this situation!"<<endl;
					//if((value2<=value1)||(valueint2<=valueint1))  cout<<"have contradiction!"<<endl;
					if(value2<=value1) return 1;
					else return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}	
			}		
			break;
		}
		case LE:{
			switch (op2){ 
				case E:{
					if(value2 > value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					if(value2>value1)  return 1;
					else return 0;
					break;
				}
				case G:{
					if(value2>=value1) return 1;
					else return 0;
					break;
				}
				case LE:{
					return 0;
					break;
				}
				case L:{
					return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}		
			}				
			break;
		}
		case L:{
			switch (op2){ 
				case E:{
					if(value2 >= value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					if(value2>=value1)  return 1;
					else return 0;
					break;
				}
				case G:{
					if(value2>=value1) return 1;
					else return 0;
					break;
				}
				case LE:{
					return 0;
					break;
				}
				case L:{
					return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}	
			}					
			break;
		}
		case NE:{
			switch (op2)
			{ 
				case E:{
					if(value2 == value1)  return 1;
					else return 0;
					break;
				}
				case GE:{
					return 0;
					break;
				}
				case G:{
					return 0;
					break;
				}
				case LE:{
					return 0;
					break;
				}
				case L:{
					return 0;
					break;
				}
				case NE:{
					return 0;
					break;
				}
			}						
			break;
		}
	}
}
int whether_joinnode_all_child_areunion(Query_Plan_Tree* tree,int index)
{
	if(tree->query_tree_node[tree->query_tree_node[index].child[0]].type == 4) 
		if(tree->query_tree_node[tree->query_tree_node[index].child[1]].type == 4) 
			return 1;
	return 0;
}
void cut_useless_select_node(Query_Plan_Tree*  tree, TableList* tblis)
{
	cout<<endl;
	cout<<"begin to cut useless select nodes"<<endl;
	cout<<endl;
/*	int i,k,j;
	for(i = 1;i <= tree->treenodenum;i++)
	{
		cout<<"node "<<i<<" parent is "<<tree->query_tree_node[i].parent<<" and child is ";
		for(j = 0;j<tree->query_tree_node[i].childnum;j++)
		{
			cout<<tree->query_tree_node[i].child[j]<<"   ";
		}
		cout<<endl;
	}
	print_query_plan_tree_with_fragment(tree);*/
	//cut select and selectproject
	OP op1,op2;
	string value1,value2;
	string col_name;
	int i,j,k;
	int iscompare = 0;
	int flag;
	int whether_controdiction = 0;
	map<int, Opera_Selection>::iterator itersel;
	map<int, Opera_SelPro>::iterator iterselpro;
	map<int, Table_change*>::iterator itertable;
	map<int, Opera_Frag>::iterator iterfrag;
	map<int, Opera_Union>::iterator iterunion;
/*	for(itersel = tree->tree_select.begin();((tree->query_tree_node[itersel->first].parent != -1)&&(itersel != tree->tree_select.end()));itersel++)
	{
		iscompare= 0;
		whether_controdiction = 0;
		col_name = itersel->second.condition.col_name;
		op1 = itersel->second.condition.op;
		value1 = itersel->second.condition.value;
		cout<<"col_name: "<<col_name<<" op1: "<<op1<<" value1: "<<value1<<endl;
		itertable = tree->tree_tablename.find(tree->query_tree_node[itersel->first].child[0]);
		iterfrag = tree->tree_frag.find(tree->query_tree_node[itersel->first].child[0]);
		if((itertable == tree->tree_tablename.end())&&iterfrag == tree->tree_frag.end())
		{
			cout<<"ERROR: can not find the selection's child'"<<endl;
			break;
		}
		else if ((itertable != tree->tree_tablename.end())&&iterfrag != tree->tree_frag.end())
		{
			cout<<"ERROR: find two of the selection's child'"<<endl;
			break;
		}
		else if((itertable == tree->tree_tablename.end())&&iterfrag != tree->tree_frag.end())
		{
			if(iterfrag->second.tbl->frag_type==0) //h  if v do nothing
			{
				for(i = 0;i<iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id].condition.condition_h_column;i++)
				{
					if(iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id].condition.h1.is_needed&&(iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id].condition.h1.attr_name[i]==col_name))
					{
						iscompare = 1;
						op2 = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id].condition.h1.operation[i];
						value2 = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id].condition.h1.attr_value[i];
					}
					if(iscompare)  flag = judge_contradiction(op1,op2,value1,value2);
					if(flag == 1) whether_controdiction = 1;
				}
			}
		}
		//if it is a table   do nothing
		if(whether_controdiction == 1)
		{
			cout<<itersel->first<<endl;
		}
	}
	
*/
	//for(iterselpro = tree->tree_selpro.begin();(iterselpro != tree->tree_selpro.end());iterselpro++)
	//{
		//cout<<"into this function"<<endl;
	//}
	
	for(iterselpro = tree->tree_selpro.begin();iterselpro != tree->tree_selpro.end();iterselpro++) //for each operation
	{
		if(tree->query_tree_node[iterselpro->first].parent == -1)  continue;
		//cout<<"into this function"<<endl;
		iscompare= 0;
		whether_controdiction = 0;
		col_name = iterselpro->second.condition.col_name;
		op1 = iterselpro->second.condition.op;
		string datatype = "";
		value1 = iterselpro->second.condition.value;
		//cout<<"col_name: "<<col_name<<" op1: "<<op1<<" value1: "<<value1<<endl;
		//cout<<"table or fragment index is "<<tree->query_tree_node[iterselpro->first].child[0]<<endl;
		int child = tree->query_tree_node[iterselpro->first].child[0];
		while(tree->query_tree_node[child].type != 5)
		{
			child = tree->query_tree_node[child].child[0];
		}
		itertable = tree->tree_tablename.find(tree->query_tree_node[iterselpro->first].child[0]);
		iterfrag = tree->tree_frag.find(child);
		if((itertable == tree->tree_tablename.end())&&iterfrag == tree->tree_frag.end())//it is not a  table and a fragment
		{
			cout<<"ERROR: can not find the selection and projection's child"<<endl;
			break;
		}
		else if ((itertable != tree->tree_tablename.end())&&iterfrag != tree->tree_frag.end())  //it is a table and a fragment
		{
			cout<<"ERROR: find two of the selection and projection's child"<<endl;
			break;
		}
		else if((itertable == tree->tree_tablename.end())&&iterfrag != tree->tree_frag.end()) //it is a fragment
		{
			//cout<<"find the fragment: "<<iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].mo_tb_name<<"  ";
			//cout<<iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].frag_id<<endl;
			if(iterfrag->second.tbl->frag_type==0) //h  if v do nothing //if it is a h frag type
			{
				//cout<<"find the fragment: "<<iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].mo_tb_name<<"  ";
				//cout<<iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].frag_id<<endl;
				//cout<<"the condition_h_column is "<<iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].condition.condition_h_column<<endl;
				//cout<<"whether_contradiction"<<whether_controdiction<<endl;
				for(i = 0;i<iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].condition.condition_h_column;i++)  //for each fraginfo column in the frag
				{
					if(iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].condition.h1.is_needed&&(iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].condition.h1.attr_name[i]==col_name))//find the same col_name
					{
						iscompare = 1;
						//cout<<"will execute compare operation"<<endl;
						op2 = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].condition.h1.operation[i];
						value2 = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].condition.h1.attr_value[i];
						for(j = 0;j < iterfrag->second.tbl->attr_count;j++)//find the datatype  
						{
							//cout<<"the attrcount is :"<<iterfrag->second.tbl->tb_name<<"  "<<iterfrag->second.tbl->attr_count<<endl;
							if(iterfrag->second.tbl->attrs[j].attr_name == col_name){
								datatype = iterfrag->second.tbl->attrs[j].type;
								break;
							}		
						}
						//cout<<"datatype is :"<<datatype<<endl;
						//cout<<" op2 : "<<op2<<"value2 : "<<value2<<endl;
					}
					//cout<<"iscompare:"<<iscompare<<endl;
					if(iscompare) {
						flag = judge_contradiction(op1,op2,value1,value2,datatype);
						if(flag == 1) whether_controdiction = 1;
						//cout<<"flag:"<<flag<<endl;
					} 
					
				}
				cout<<"the final whether_contradiction"<<whether_controdiction<<endl;
				if(whether_controdiction == 1)
				{
					cout<<"find controdiction the selectprojection indx is :"<<iterselpro->first<<endl;
					cout<<"the iterselpro condition is"<<iterselpro->second.condition.tb_name<<iterselpro->second.condition.col_name<<iterselpro->second.condition.op<<iterselpro->second.condition.value<<endl;
					//update the tree
					//find which child of iterselpro->first.parent is this iterselpro->first
					int parent = tree->query_tree_node[iterselpro->first].parent;
					int ch;
					for(k = 0;k<tree->query_tree_node[parent].childnum;k++)
					{
						if(tree->query_tree_node[parent].child[k] == iterselpro->first)
						{
							ch = k;
						}
					}
					tree->query_tree_node[parent].child[ch] = 0;
					tree->query_tree_node[iterselpro->first].parent = -1;
					tree->query_tree_node[iterselpro->first].is_needed = 0;
					tree->query_tree_node[tree->query_tree_node[iterselpro->first].child[0]].parent = -1;
					tree->query_tree_node[tree->query_tree_node[iterselpro->first].child[0]].is_needed = 0;
					tree->query_tree_node[iterselpro->first].child[0] = 0;
					tree->query_tree_node[iterselpro->first].childnum = 0;
					//put the child ahead to fill the 0;
					int countflag;
					for(k = 0;k<tree->query_tree_node[parent].childnum;k++)
					{
						tree->query_tree_node[parent].child[ch+k] = tree->query_tree_node[parent].child[ch+k+1];
					}
					tree->query_tree_node[parent].childnum--;
				}
			}

		}

		//if it is a table   do nothing

	}
	for(iterunion = tree->tree_union.begin();iterunion!= tree->tree_union.end();iterunion++)
	{
		if(iterunion->second.childnum == 0)
		{
			tree->query_tree_node[tree->query_tree_node[iterunion->first].parent].childnum -- ;
			tree->query_tree_node[iterunion->first].parent = -1;
		}
	}
	for(i = 1;i <= tree->treenodenum;i++)
	{
		cout<<"node "<<i<<" parent is "<<tree->query_tree_node[i].parent<<" and child is ";
		for(j = 0;j<tree->query_tree_node[i].childnum;j++)
		{
			cout<<tree->query_tree_node[i].child[j]<<"   ";
		}
		cout<<endl;
	}
	//print_query_plan_tree_with_fragment(tree);
}
int judge_whether_have_join_contradiction(SelectQuery &SQ, Query_Plan_Tree*  tree, int frag1_index,int frag2_index,int join_index,int unionindex1,int unionindex2)
{
	map<int, Opera_Join>::iterator iterjoin;
	map<int, Opera_Frag>::iterator iterfrag1, iterfrag2, iterfragtemp;
	map<int, Opera_Union>::iterator iterunion1,iterunion2;
	iterjoin = tree->tree_join.find(join_index);
	iterfrag1 = tree->tree_frag.find(frag1_index);
	iterfrag2 = tree->tree_frag.find(frag2_index);
	iterunion1 = tree->tree_union.find(unionindex1);
	iterunion2 = tree->tree_union.find(unionindex2);
	//找到选择操作  之前挪动了union节点，但是没有更改join操作的对象 
	string tablename1,tablename2;
	tablename1 = iterjoin->second.which_to_broadcast;
	tablename2 = iterjoin->second.which_to_stay;
	//cout<<"tablename1 = "<<tablename1<<" and tablename2 = "<<tablename2<<endl;
	string column_name1="",column_name2="";
	int i,j,k;
	for(i = 0;i < SQ.join_count;i++)
	{
		if((SQ.JoinList[i].tb_name1==tablename1)&&(SQ.JoinList[i].tb_name2==tablename2))
		{
			column_name1 = SQ.JoinList[i].col_name1;
			column_name2 = SQ.JoinList[i].col_name2;
		}
		if((SQ.JoinList[i].tb_name1==tablename2)&&(SQ.JoinList[i].tb_name2==tablename1))
		{
			column_name1 = SQ.JoinList[i].col_name2;
			column_name2 = SQ.JoinList[i].col_name1;
		}
	}
	//cout<<"column_name1 = "<<column_name1<<"  and column_name2 = "<<column_name2<<endl;
	if(iterjoin==tree->tree_join.end()||iterfrag1==tree->tree_frag.end()||iterfrag2==tree->tree_frag.end())
	{
		cout<<"ERROR: when judge whether have join contradiction, can not find the index"<<endl;
		return -1;
	}
//	string tbl_name1 = tree->query_tree_node[iterjoin->first].child[0];
	//string tbl_name2 = iterjoin->second.join.col_name2;
//	cout<<
	if(iterfrag1->second.tbl->tb_name!=tablename1 && iterfrag1->second.tbl->tb_name!=tablename2)
	{
		cout<<"ERROR: when judge whether have join contradiction, the operation join and the fraginformation do not match"<<endl;
		return -1;
	}		
	if(iterfrag2->second.tbl->tb_name!=tablename1 && iterfrag2->second.tbl->tb_name!=tablename2)
	{
		cout<<"ERROR: when judge whether have join contradiction, the operation join and the fraginformation do not match"<<endl;
		return -1;
	}	
	if(iterfrag1->second.tbl->tb_name!=iterjoin->second.join.tb_name1)  //let the first frag point to first table in the join and the second frag point to second table in join
	{
		iterfragtemp = iterfrag1;
		iterfrag1 = iterfrag2;
		iterfrag2 = iterfragtemp;
	}
	OP op1,op2;
	string value1,value2;
	string datatype;
	int flag1 = 0,flag2 = 0;
	int result = 0;
	if(iterfrag1->second.tbl->frag_type != 0 || iterfrag2->second.tbl->frag_type != 0)
		return 0;
	//cout<<"the table name are:"<<tablename1<<tablename2<<"  and the col_name are "<<column_name1<<"  "<<column_name2<<endl;
	for(i = 0;i < iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].condition.condition_h_column;i++)
	{
		flag1 = 0;
		flag2 = 0;
		if(iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].condition.h1.attr_name[i] == column_name1)
		{
			flag1 = 1;
			//cout<<"the iterfrag1's column is "<<iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].condition.h1.attr_name[i]<<endl;
			op1 = iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].condition.h1.operation[i];
			value1 = iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].condition.h1.attr_value[i];
			//cout<<endl<<"op1 is "<<op1<<" and the value1:"<<value1<<endl;
			for(j = 0;j < iterfrag1->second.tbl->attr_count;j++)
			{
				if(iterfrag1->second.tbl->attrs[j].attr_name == column_name1)
					datatype = iterfrag1->second.tbl->attrs[j].type;
			}
			for(j = 0;j < iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].condition.condition_h_column;j++)
			{
				//cout<<"the index of frag2 is "<<iterfrag2->first<<endl;
				//cout<<"the iterfrag2's column is "<<iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].condition.h1.attr_name[j]<<endl;
				flag2 = 0;
				if(iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].condition.h1.attr_name[j] == column_name2)
				{
					flag2 = 1;
					op2 = iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].condition.h1.operation[j];
					value2 = iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].condition.h1.attr_value[j];
					//cout<<endl<<"op2 is "<<op2<<" and the value2:"<<value2<<endl;
				}
				if(flag1==1 && flag2==1)
					result += judge_contradiction(op1,op2,value1,value2,datatype);
			}

		}
	}
	
	if(result>=1)
		return 1;
	return 0;
}
void rebuild_join_condition(SelectQuery &SQ, Query_Plan_Tree*  tree)
{
	int i,j,k;
	map<int, Opera_Join>::iterator iterjoin;
	map<int, Opera_Union>::iterator iterunion1,iterunion2;
	int union1,union2;
	int temp1,temp2;
	string tablename1,tablename2;
	for(iterjoin=tree->tree_join.begin();iterjoin!=tree->tree_join.end();iterjoin++)
	{
		if(whether_joinnode_all_child_areunion(tree,iterjoin->first) == 1)  
		{
			union1 = tree->query_tree_node[iterjoin->first].child[0];
			union2 = tree->query_tree_node[iterjoin->first].child[1];
			//cout<<"union1 is "<<union1<<" and the union2 is "<<union2<<endl;
			iterunion1 = tree->tree_union.find(union1);
			iterunion2 = tree->tree_union.find(union2);
			tablename1 = iterunion1->second.tbl->tb_name;
			tablename2 = iterunion2->second.tbl->tb_name;
			//cout<<"tablename1 is "<<tablename1<<" and the tablename2 is "<<tablename2<<endl;
			for(i = 0;i < SQ.join_count;i++)
			{
				if((SQ.JoinList[i].tb_name1==tablename1)&&(SQ.JoinList[i].tb_name2==tablename2))
				{
					iterjoin->second.join = SQ.JoinList[i];
				}
				if((SQ.JoinList[i].tb_name1==tablename2)&&(SQ.JoinList[i].tb_name2==tablename1))
				{
					iterjoin->second.join = SQ.JoinList[i];
				}
			}
			//cout<<"come to here"<<endl;
		}
		else{
			if(tree->query_tree_node[tree->query_tree_node[iterjoin->first].child[1]].type == 4)
			{
				union2 = tree->query_tree_node[iterjoin->first].child[1];}
			else
				union2 = tree->query_tree_node[iterjoin->first].child[0];
			iterunion2 = tree->tree_union.find(union2);
			tablename2 = iterunion2->second.tbl->tb_name;
			for(i = 0;i < SQ.join_count;i++)
			{
				if(SQ.JoinList[i].tb_name1==tablename2)
				{
					iterjoin->second.join = SQ.JoinList[i];
				}
				if(SQ.JoinList[i].tb_name1==tablename2)
				{
					iterjoin->second.join = SQ.JoinList[i];
				}
			}
		}
	}
	print_query_plan_tree_with_fragment(tree);
}
void how_to_join(SelectQuery &SQ, Query_Plan_Tree*  tree, TableList* tblist)
{
	int i,j,k;
	cout<<endl<<"Begin to handle join operation"<<endl<<endl;
	map<int, Opera_Join>::iterator iterjoin,iterjoin_child,iterjoin_temp;
	map<int, Opera_Union>::iterator iterunion1,iterunion2,iterunion,iterunion_temp;
	map<int, Opera_Frag>::iterator iterfrag1,iterfrag2;
	int unionindex1,unionindex2;
	string column_name1,column_name2,table_name1,table_name2;
	int ch1,ch2;
	int the_other_ch1,the_other_ch2;
	int parent1,parent2;
	//find if there are two tables, the join column in condition are in them fragment condition //let the is_done is 0
	for(iterjoin = tree->tree_join.begin(); iterjoin != tree->tree_join.end();iterjoin++)
	{
		iterjoin->second.is_done = 0;
		column_name1 = iterjoin->second.join.col_name1;
		column_name2 = iterjoin->second.join.col_name2;
		table_name1 = iterjoin->second.join.tb_name1;
		table_name2 = iterjoin->second.join.tb_name2;
		//cout<<"the table_name1 and table_name2 is "<<table_name1<<"  "<<table_name2<<" and the column name1 and the column name2 is "<<column_name1<<"  "<<column_name2<<endl;
		int flag1=0,flag2=0;
		
		for(i = 0;i < tblist->table_num;i++)//find the table
		{
			if(tblist->tbl[i]->tb_name == table_name1)
			{
				//cout<<"find the table_name1"<<tblist->tbl[i]->tb_name<<endl;
				if(tblist->tbl[i]->frag_type==0)  //the table is h fragment
				{
					//cout<<"the table is h fragment"<<tblist->tbl[i]->tb_name<<endl;
					for(j = 0;j < tblist->tbl[i]->fraginfo[0].condition.condition_h_column;j++)  //find the column whether in the fragment attribute
					{
						//cout<<"the frag column name is "<<tblist->tbl[i]->fraginfo[0].condition.h1.attr_name[j]<<endl;
						if(tblist->tbl[i]->fraginfo[0].condition.h1.attr_name[j] == column_name1)
						{
							flag1 = 1;
							break;
						}
						
					}
				}
			}
			if(tblist->tbl[i]->tb_name == table_name2)
			{
				if(tblist->tbl[i]->frag_type==0)  //the table is h fragment
				{
					for(j = 0;j < tblist->tbl[i]->fraginfo[0].condition.condition_h_column;j++)  //find the column whether in the fragment attribute
					{
						if(tblist->tbl[i]->fraginfo[0].condition.h1.attr_name[j] == column_name2)
						{
							flag2 = 1;
							break;
						}
					}
				}
			}			
		}
		if(flag1==1 && flag2==1)
		{
			cout<<"the tablename is "<<table_name1<<" and "<<table_name2;
			for(iterunion = tree->tree_union.begin();iterunion != tree->tree_union.end();iterunion++)
			{
				if(iterunion->second.tbl->tb_name == table_name1)
					unionindex1 = iterunion->first;
				if(iterunion->second.tbl->tb_name == table_name2)
					unionindex2 = iterunion->first;
			}
			parent1 = tree->query_tree_node[unionindex1].parent;
			for(k = 0;k < tree->query_tree_node[parent1].childnum;k++)
				if(tree->query_tree_node[parent1].child[k] == unionindex1)
					ch1 = k;
				else the_other_ch1 = k;
			parent2 = tree->query_tree_node[unionindex2].parent;
			for(k = 0;k < tree->query_tree_node[parent2].childnum;k++)
				if(tree->query_tree_node[parent2].child[k] == unionindex2)
					ch2 = k;	
				else the_other_ch2 = k;
			
			if(whether_joinnode_all_child_areunion(tree,parent1) == 1)  //没有考虑如果两个都在join的1节点怎么办！！ 
			{
				int temp = tree->query_tree_node[parent1].child[the_other_ch1];
				//cout<<endl<<"the temp is "<<temp<<endl;
				tree->query_tree_node[parent1].child[the_other_ch1] = unionindex2;
				//cout<<"parent2 is"<<parent2<<endl<<" and the ch2 is "<<ch2<<endl;
				//cout<<"the tree->query_tree_node[parent2].child[ch2] :"<<tree->query_tree_node[parent2].child[ch2]<<endl;
				tree->query_tree_node[parent2].child[ch2] = temp;
				///cout<<"the tree->query_tree_node[parent2].child[ch2] :"<<tree->query_tree_node[parent2].child[ch2];
				tree->query_tree_node[unionindex2].parent = parent1;
				tree->query_tree_node[temp].parent = parent2;
			}
			if(whether_joinnode_all_child_areunion(tree,parent2) == 1)
			{
				int temp = tree->query_tree_node[parent2].child[the_other_ch2];
				//cout<<endl<<"the temp is "<<temp<<endl;
				tree->query_tree_node[parent2].child[the_other_ch2] = tree->query_tree_node[parent1].child[ch1];
				tree->query_tree_node[parent1].child[ch1] = temp;
				tree->query_tree_node[unionindex1].parent = parent2;
				tree->query_tree_node[temp].parent = parent1;
				//cout<<endl<<"the temp is "<<temp<<endl;
			}
			//update the join node
			//for(k = 0;k<SQ.join_count;k++)
			//{
				//if(SQ.JoinList[k].tb_name1==tree->query_tree_node[tree->query_tree_node[iterjoin->first]].child[])
			//}
		}
		
	}
	
	cout<<endl<<"move the join order"<<endl;
	rebuild_join_condition(SQ, tree);
	for(i = 1;i <= tree->treenodenum;i++)
	{
		cout<<"node "<<i<<" parent is "<<tree->query_tree_node[i].parent<<" and child is ";
		for(j = 0;j<tree->query_tree_node[i].childnum;j++)
		{
			cout<<tree->query_tree_node[i].child[j]<<"   ";
		}
		cout<<endl;
	}
	
	
	//print_query_plan_tree_with_fragment(tree);
	
	//判断是不是h分布， 如果不是两个孩子都是union节点，那么你看child[1],再去找child0 
	//cout<<"come to here"<<endl;
	for(iterjoin = tree->tree_join.begin(); iterjoin != tree->tree_join.end();iterjoin++)
	{
		if((tree->query_tree_node[tree->query_tree_node[iterjoin->first].child[0]].type==3) || (tree->query_tree_node[tree->query_tree_node[iterjoin->first].child[1]].type==3))
		{
			if(tree->query_tree_node[tree->query_tree_node[iterjoin->first].child[1]].type==3)
			{
				int tempp = tree->query_tree_node[iterjoin->first].child[1];
				tree->query_tree_node[iterjoin->first].child[1] = tree->query_tree_node[iterjoin->first].child[0];
				tree->query_tree_node[iterjoin->first].child[0] = tempp;
			}
			//cout<<"into this function"<<endl;
			//unionindex1 = query_tree_node[tree->query_tree_node[iterjoin->first]]
			unionindex2 = tree->query_tree_node[iterjoin->first].child[1];
			//cout<<"unionindex2 = "<<unionindex2<<endl;
			iterjoin_child = tree->tree_join.find(tree->query_tree_node[iterjoin->first].child[0]);

			iterunion2 = tree->tree_union.find(unionindex2);
			if(iterunion2 == tree->tree_union.end())  cout<<"ERROR: can not find the union1 node in the tree"<<endl;
			if(iterjoin->second.join.tb_name1 == iterunion2->second.tbl->tb_name)
			{
				for(iterunion_temp = tree->tree_union.begin();iterunion_temp!=tree->tree_union.end();iterunion_temp++)
				{
					if(iterunion_temp->second.tbl->tb_name == iterjoin->second.join.tb_name2)
					{
						unionindex1 = iterunion_temp->first;
					}
				}
			}
			else
			{
				for(iterunion_temp = tree->tree_union.begin();iterunion_temp!=tree->tree_union.end();iterunion_temp++)
				{
					if(iterunion_temp->second.tbl->tb_name == iterjoin->second.join.tb_name1)
					{
						unionindex1 = iterunion_temp->first;
					}
				}			
			}
			//iterjoin_child = tree->tree_join.find(tree->query_tree_node[iterjoin->first].child[0]);
			//cout<<"the child is a join operation and its index is "<<iterjoin->first<<" and the child is "<<iterjoin_child->first<<endl;
			//unionindex1 = tree->query_tree_node[iterjoin->first].child[0];
			iterunion1 = tree->tree_union.find(unionindex1);
			if(iterunion1 == tree->tree_union.end())  cout<<"ERROR: can not find the union1 node in the tree"<<endl;
			//unionindex2 = tree->query_tree_node[iterjoin->first].child[1];
			
			int fraglist1[MAX_FRAG_COUNT];
			int fraglist2[MAX_FRAG_COUNT];
			int fraglist1_len;
			int fraglist2_len;
			
			cout<<"the first union index is "<<unionindex1<<" and the second is "<<unionindex2<<endl;
			iterjoin->second.which_to_stay= iterunion1->second.tbl->tb_name;
			iterjoin->second.which_to_broadcast = iterunion2->second.tbl->tb_name;
			/*if(tree->query_tree_node[unionindex1].childnum < tree->query_tree_node[unionindex2].childnum)    //may change to the small size of the table!!!
			{
				iterjoin->second.which_to_broadcast = iterunion1->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion2->second.tbl->tb_name;
			}*/
			if(iterunion1->second.tbl->tb_name == "Exam")
			{
				iterjoin->second.which_to_broadcast = iterunion2->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion1->second.tbl->tb_name;
			}
			else if(iterunion2->second.tbl->tb_name == "Exam")
			{
				iterjoin->second.which_to_broadcast = iterunion1->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion2->second.tbl->tb_name;
			}
			else if(iterunion1->second.tbl->tb_name == "Course")
			{
				iterjoin->second.which_to_broadcast = iterunion1->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion2->second.tbl->tb_name;
			}
			else if(iterunion2->second.tbl->tb_name == "Course")
			{
				iterjoin->second.which_to_broadcast = iterunion2->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion1->second.tbl->tb_name;
			}
			else if(tree->query_tree_node[unionindex1].childnum < tree->query_tree_node[unionindex2].childnum)
			{
				iterjoin->second.which_to_broadcast = iterunion1->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion2->second.tbl->tb_name;
			}
			
			cout<<"which to broadcast is "<<iterjoin->second.which_to_broadcast<<endl;
			cout<<"which to stay is "<<iterjoin->second.which_to_stay<<endl;
			int temp;
			cout<<"the fraglist1 is "<<endl;
			fraglist1_len = tree->query_tree_node[unionindex1].childnum;
			for(j = 0;j < tree->query_tree_node[unionindex1].childnum;j++) //the fist union's all fragments
			{
				temp = tree->query_tree_node[unionindex1].child[j];
				//cout<<"the unionindex1"<<unionindex1<<endl;
				//cout<<"the type is "<<tree->query_tree_node[14].type<<endl;
				//cout<<"the type is"<<tree->query_tree_node[tree->query_tree_node[temp].child[j]].type<<endl;
				//temp = tree->query_tree_node[tree->query_tree_node[temp].child[j]].child[0];
				//cout<<"the temp is"<<temp<<endl;		 
				while(tree->query_tree_node[temp].type!=5)
				{
					temp = tree->query_tree_node[temp].child[0];
					//cout<<"the temp is "<<temp<<endl;
				}
					
				fraglist1[j] = temp;
				cout<<fraglist1[j]<<"  ";
			}
			cout<<endl<<"the fraglist2 is "<<endl;
			fraglist2_len = tree->query_tree_node[unionindex2].childnum;
			for(j = 0;j < tree->query_tree_node[unionindex2].childnum;j++)  //the second union's all fragments
			{
				temp = tree->query_tree_node[unionindex2].child[j];
				while(tree->query_tree_node[temp].type!=5)
					temp = tree->query_tree_node[temp].child[0];
				fraglist2[j] = temp;
				cout<<fraglist2[j]<<"  ";
			}
			cout<<endl;
			int tempjoinnum = tree->query_tree_node[unionindex1].childnum * tree->query_tree_node[unionindex2].childnum;
			iterjoin->second.joinnum = tempjoinnum;
			int tempnum = 0;
			for(j = 0;j < fraglist1_len;j++)
			{
				iterfrag1 = tree->tree_frag.find(fraglist1[j]);
				for(k = 0;k < fraglist2_len;k++)
				{
					iterfrag2 = tree->tree_frag.find(fraglist2[k]);
					cout<<"to judge the contradiction "<<fraglist1[j]<<"   "<<fraglist2[k]<<"   "<<iterjoin->first<<endl;
					int tttemp = judge_whether_have_join_contradiction(SQ,tree,fraglist1[j],fraglist2[k],iterjoin->first,unionindex1,unionindex2);
					if(tttemp == -1) return;
					if(tttemp == 1)
					{
						cout<<"find join contradiction and the index are "<<fraglist1[j]<<"   "<<fraglist2[k]<<endl;
					}
					if(tttemp == 0)
					{
						iterjoin->second.join_frag[tempnum].join = iterjoin->second.join;
						iterjoin->second.join_frag[tempnum].tbl_to_stay = iterjoin->second.which_to_stay;
						iterjoin->second.join_frag[tempnum].tbl_to_broadcast = iterjoin->second.which_to_broadcast;
						if(iterfrag1->second.tbl->tb_name==iterjoin->second.which_to_stay)
						{
							iterjoin->second.join_frag[tempnum].frag_to_stay_id = fraglist1[j];
							iterjoin->second.join_frag[tempnum].frag_to_broadcast_id = fraglist2[k];
							iterjoin->second.join_frag[tempnum].cast_to_db_id = iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].db_id;
							iterjoin->second.join_frag[tempnum].from_db_id = iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].db_id;
						}
						if(iterfrag1->second.tbl->tb_name==iterjoin->second.which_to_broadcast)
						{
							iterjoin->second.join_frag[tempnum].frag_to_stay_id = fraglist2[k];
							iterjoin->second.join_frag[tempnum].frag_to_broadcast_id = fraglist1[j];
							iterjoin->second.join_frag[tempnum].cast_to_db_id = iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].db_id;
							iterjoin->second.join_frag[tempnum].from_db_id = iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].db_id;
						}	
						tempnum++;					
					}	
				}
			}
			iterjoin->second.joinnum = tempnum;
			for(int kk = 0;kk < iterjoin->second.joinnum;kk++)
			{
				cout<<"the two fragment are:"<<iterjoin->second.join_frag[kk].frag_to_stay_id<<"  "<<iterjoin->second.join_frag[kk].frag_to_broadcast_id<<endl;
				cout<<"and the condition is:"<<iterjoin->second.join_frag[kk].join.tb_name1<<" "<<iterjoin->second.join_frag[kk].join.tb_name2;
				cout<<"  "<<iterjoin->second.join_frag[kk].join.col_name1<<" "<<iterjoin->second.join_frag[kk].join.col_name2<<endl;
			}
		}
		if((tree->query_tree_node[tree->query_tree_node[iterjoin->first].child[0]].type==4)&&(tree->query_tree_node[tree->query_tree_node[iterjoin->first].child[1]].type==4))
		{
			//cout<<"the two child are union operation "<<iterjoin->first<<endl;
			unionindex1 = tree->query_tree_node[iterjoin->first].child[0];
			iterunion1 = tree->tree_union.find(unionindex1);
			if(iterunion1 == tree->tree_union.end())  cout<<"ERROR: can not find the union1 node in the tree"<<endl;
			unionindex2 = tree->query_tree_node[iterjoin->first].child[1];
			iterunion2 = tree->tree_union.find(unionindex2);
			int fraglist1[MAX_FRAG_COUNT];
			int fraglist2[MAX_FRAG_COUNT];
			int fraglist1_len;
			int fraglist2_len;
			if(iterunion2 == tree->tree_union.end())  cout<<"ERROR: can not find the union1 node in the tree"<<endl;
			//cout<<"the first union index is "<<unionindex1<<" and the second is "<<unionindex2<<endl;
			iterjoin->second.which_to_stay= iterunion1->second.tbl->tb_name;
			iterjoin->second.which_to_broadcast = iterunion2->second.tbl->tb_name;
			if(iterunion1->second.tbl->tb_name == "Exam")
			{
				iterjoin->second.which_to_broadcast = iterunion2->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion1->second.tbl->tb_name;
			}
			else if(iterunion2->second.tbl->tb_name == "Exam")
			{
				iterjoin->second.which_to_broadcast = iterunion1->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion2->second.tbl->tb_name;
			}
			else if(iterunion1->second.tbl->tb_name == "Course")
			{
				iterjoin->second.which_to_broadcast = iterunion1->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion2->second.tbl->tb_name;
			}
			else if(iterunion2->second.tbl->tb_name == "Course")
			{
				iterjoin->second.which_to_broadcast = iterunion2->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion1->second.tbl->tb_name;
			}
			else if(tree->query_tree_node[unionindex1].childnum < tree->query_tree_node[unionindex2].childnum)
			{
				iterjoin->second.which_to_broadcast = iterunion1->second.tbl->tb_name;
				iterjoin->second.which_to_stay = iterunion2->second.tbl->tb_name;
			}
			cout<<"which to broadcast is "<<iterjoin->second.which_to_broadcast<<endl;
			cout<<"which to stay is "<<iterjoin->second.which_to_stay<<endl;
			int temp;
			cout<<"the fraglist1 is "<<endl;
			fraglist1_len = tree->query_tree_node[unionindex1].childnum;
			for(j = 0;j < tree->query_tree_node[unionindex1].childnum;j++) //the fist union's all fragments
			{
				temp = tree->query_tree_node[unionindex1].child[j];
				//cout<<"the unionindex1"<<unionindex1<<endl;
				//cout<<"the type is "<<tree->query_tree_node[14].type<<endl;
				//cout<<"the type is"<<tree->query_tree_node[tree->query_tree_node[temp].child[j]].type<<endl;
				//temp = tree->query_tree_node[tree->query_tree_node[temp].child[j]].child[0];
				//cout<<"the temp is"<<temp<<endl;		 
				while(tree->query_tree_node[temp].type!=5)
				{
					temp = tree->query_tree_node[temp].child[0];
					//cout<<"the temp is "<<temp<<endl;
				}
					
				fraglist1[j] = temp;
				cout<<fraglist1[j]<<"  ";
			}
			cout<<endl<<"the fraglist2 is "<<endl;
			fraglist2_len = tree->query_tree_node[unionindex2].childnum;
			for(j = 0;j < tree->query_tree_node[unionindex2].childnum;j++)  //the second union's all fragments
			{
				temp = tree->query_tree_node[unionindex2].child[j];
				while(tree->query_tree_node[temp].type!=5)
					temp = tree->query_tree_node[temp].child[0];
				fraglist2[j] = temp;
				cout<<fraglist2[j]<<"  ";
			}
			cout<<endl;
			int tempjoinnum = tree->query_tree_node[unionindex1].childnum * tree->query_tree_node[unionindex2].childnum;
			iterjoin->second.joinnum = tempjoinnum;
			int tempnum = 0;
			for(j = 0;j < fraglist1_len;j++)
			{
				iterfrag1 = tree->tree_frag.find(fraglist1[j]);
				for(k = 0;k < fraglist2_len;k++)
				{
					iterfrag2 = tree->tree_frag.find(fraglist2[k]);
					cout<<"to judge the contradiction "<<fraglist1[j]<<"   "<<fraglist2[k]<<"   "<<iterjoin->first<<endl;
					int tttemp = judge_whether_have_join_contradiction(SQ,tree,fraglist1[j],fraglist2[k],iterjoin->first,unionindex1,unionindex2);
					if(tttemp == -1) return;
					if(tttemp == 1)
					{
						cout<<"find join contradiction and the index are "<<fraglist1[j]<<"   "<<fraglist2[k]<<endl;
					}
					if(tttemp == 0)
					{
						iterjoin->second.join_frag[tempnum].join = iterjoin->second.join;
						iterjoin->second.join_frag[tempnum].tbl_to_stay = iterjoin->second.which_to_stay;
						iterjoin->second.join_frag[tempnum].tbl_to_broadcast = iterjoin->second.which_to_broadcast;
						if(iterfrag1->second.tbl->tb_name==iterjoin->second.which_to_stay)
						{
							iterjoin->second.join_frag[tempnum].frag_to_stay_id = fraglist1[j];
							iterjoin->second.join_frag[tempnum].frag_to_broadcast_id = fraglist2[k];
							iterjoin->second.join_frag[tempnum].cast_to_db_id = iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].db_id;
							iterjoin->second.join_frag[tempnum].from_db_id = iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].db_id;
						}
						if(iterfrag1->second.tbl->tb_name==iterjoin->second.which_to_broadcast)
						{
							iterjoin->second.join_frag[tempnum].frag_to_stay_id = fraglist2[k];
							iterjoin->second.join_frag[tempnum].frag_to_broadcast_id = fraglist1[j];
							iterjoin->second.join_frag[tempnum].cast_to_db_id = iterfrag2->second.tbl->fraginfo[iterfrag2->second.frag_id-1].db_id;
							iterjoin->second.join_frag[tempnum].from_db_id = iterfrag1->second.tbl->fraginfo[iterfrag1->second.frag_id-1].db_id;
						}	
						tempnum++;					
					}	
				}
			}
			iterjoin->second.joinnum = tempnum;
			for(int kk = 0;kk < iterjoin->second.joinnum;kk++)
			{
				cout<<"the two fragment are:"<<iterjoin->second.join_frag[kk].frag_to_stay_id<<"  "<<iterjoin->second.join_frag[kk].frag_to_broadcast_id<<endl;
				cout<<"and the condition is:"<<iterjoin->second.join_frag[kk].join.tb_name1<<" "<<iterjoin->second.join_frag[kk].join.tb_name2;
				cout<<"  "<<iterjoin->second.join_frag[kk].join.col_name1<<" "<<iterjoin->second.join_frag[kk].join.col_name2<<endl;
			}
		}
	}
}
void cut_useless_vertical_node(SelectQuery &SQ, Query_Plan_Tree*  tree, TableList* tblist)
{
	int i = 0,j,k,m;
	int tableindex;
	int flag = 1;
	int is_needcut = 0;
	map<int,Opera_Project>::iterator iterproject;
	int fragneeded[MAX_FRAG_COUNT] = {0};
	cout<<endl;
	cout<<"begin to cut useless vertical column"<<endl;
	cout<<endl;
	map<int,Opera_Frag>::iterator iterfrag;
	string attrneeded[MAX_ATTR_COUNT];
	string tablename;
	string attrname;
	int fflag = 0;
	for(i=0;i<tblist->table_num;i++) //find the table vertical fraged
	{
		if(tblist->tbl[i]->frag_type == 1)
		{
			tablename = tblist->tbl[i]->tb_name;
			for(int mm = 0;mm<SQ.from_count;mm++)
			{
				if(tablename == SQ.FromList[mm].tb_name)
				{
					fflag = 1;
				}
			}
			if(fflag == 0) return;
			find_all_attr_needed(tblist,SQ,tablename,attrneeded);
			for(j = 0;j < MAX_FRAG_COUNT;j++) //for each column needed
			{
				if(attrneeded[j]!="")
				{
					attrname = attrneeded[j];
					for(int q = 0;q<tblist->tbl[i]->attr_count;q++)
					{
						if(tblist->tbl[i]->attrs[q].is_key==0 && tblist->tbl[i]->attrs[q].attr_name==attrname)
						{
							for(k = 0;k < tblist->tbl[i]->frag_count;k++)   //for each fragment
							{
								for(m = 0;m < tblist->tbl[i]->fraginfo[k].condition.v1.attr_num;m++)
								{
									if(tblist->tbl[i]->fraginfo[k].condition.v1.attr_list[m] == attrname)
									{
										fragneeded[k] = 1;
									}
								}
							}
						}
					}
				}
			}
			for( j =0;j<tblist->tbl[i]->frag_count;j++)
			{
				if(fragneeded[j] == 0) // the frag don't needed
				{
					for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
					{
						if((iterfrag->second.tbl->tb_name == tablename) && (iterfrag->second.frag_id == j+1))//find the frag
						{
							int curnode = iterfrag->first;
							int childnode;
							int parent = tree->query_tree_node[curnode].parent;
							cout<<"curnode is "<<curnode<<endl;
							while(tree->query_tree_node[curnode].type != 4)
							{
								parent = tree->query_tree_node[curnode].parent;
								if(tree->query_tree_node[parent].type != 4)
									tree->query_tree_node[parent].childnum = 0;
								tree->query_tree_node[curnode].parent = -1;
								childnode = curnode;
								curnode = parent;
								//parent = tree->query_tree_node[curnode].parent;
							}
							//for(k = j;k<tree->query_tree_node[curnode].childnum;k++)
								//tree->query_tree_node[parentindex].child[j+k] = tree->query_tree_node[parentindex].child[j+k+1];
							cout<<"curnode is "<<curnode<<" and the childnode to cut is "<<childnode<<endl;
							for(int kk = 0;kk<tree->query_tree_node[curnode].childnum;kk++)
							{
								if(tree->query_tree_node[curnode].child[kk] == childnode)
								{
									for(int mmm = kk+1;mmm<tree->query_tree_node[curnode].childnum;mmm++)
										tree->query_tree_node[curnode].child[mmm-1] = tree->query_tree_node[curnode].child[mmm];
									tree->query_tree_node[curnode].childnum--;
								}
							}
						}
					}
				}

			}
		}
	}
	cout<<"the needed vertical frag are : ";
	for(i = 0;i<MAX_FRAG_COUNT;i++)
	{
			cout<<fragneeded[i]<<"   ";
	}
	cout<<endl;
	for(i = 1;i <= tree->treenodenum;i++)
	{
		cout<<"node "<<i<<" parent is "<<tree->query_tree_node[i].parent<<" and child is ";
		for(j = 0;j<tree->query_tree_node[i].childnum;j++)
		{
			cout<<tree->query_tree_node[i].child[j]<<"   ";
		}
		cout<<endl;
	}
/*	if(SQ.from_count == 1)
	{
		is_needcut = 1;
		for(i=0;i<MAX_TABLE_COUNT;i++)
		{
			if(tblist->tbl[i]->tb_name == SQ.FromList->tb_name)//find the table
			{
				tableindex = i;
				if(tblist->tbl[i]->frag_type == 1) //the table is vertical divided
				{
					flag = 0;
					for(iterproject = tree->tree_project.begin();iterproject != tree->tree_project.end();iterproject++) //for each project column
					{
						for(j = 0;j<tblist->tbl[i]->frag_count;j++)
						{
							for(k = 0;k < tblist->tbl[i]->fraginfo[j].condition.v1.attr_num;k++)
							{
								if(iterproject->second.project.col_name == tblist->tbl[i]->fraginfo[j].condition.v1.attr_list[k])
								{
									fragneeded[j+1] = 1;
								}
							}
							
						}
					}
				}
				break;
			}
		}
	}
	if(is_needcut == 1){
	
	cout<<"the frag needed is ";
	for(i = 0;i<MAX_FRAG_COUNT;i++)
		cout<<fragneeded[i]<<"    ";
	cout<<endl;
	map<int,Opera_Frag>::iterator iterfrag;
	for( i =0;i<tblist->tbl[tableindex]->frag_count;i++)
	{
		if(fragneeded[i+1] == 0 && flag == 0)
		{
			for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
			{
				if(iterfrag->second.frag_id == i+1)
				{
					tree->query_tree_node[iterfrag->first].is_needed = -1;
					int parentindex = tree->query_tree_node[iterfrag->first].parent;
					for(j = 0;j<tree->query_tree_node[parentindex].childnum;j++)
					{
						if(tree->query_tree_node[parentindex].child[j]==iterfrag->first)
						{
							tree->query_tree_node[iterfrag->first].parent = -1;
							for(k = j;k<tree->query_tree_node[parentindex].childnum;k++)
								tree->query_tree_node[parentindex].child[j+k] = tree->query_tree_node[parentindex].child[j+k+1];
							tree->query_tree_node[parentindex].childnum--;
							
						}
					}
				}
			}
		}
	}
	//print_query_plan_tree_with_fragment(tree);
	for(i = 1;i <= tree->treenodenum;i++)
	{
		cout<<"node "<<i<<" parent is "<<tree->query_tree_node[i].parent<<" and child is ";
		for(j = 0;j<tree->query_tree_node[i].childnum;j++)
		{
			cout<<tree->query_tree_node[i].child[j]<<"   ";
		}
		cout<<endl;
	}
	}*/
	
}
// vector<string> Split(string& src,const string& separator){
//     vector<string> dest;
//     string str = src;
//     string substring;
//     int start = 0, index;
//     dest.clear();
//     index = str.find_first_of(separator,start);
//     do
//     {
//         if (index != string::npos)
//         {    
//             substring = str.substr(start,index-start );
//             dest.push_back(substring);
//             start =index+separator.size();
//             index = str.find(separator,start);
//             if (start == string::npos) break;
//         }
//     }while(index != string::npos);

//     substring = str.substr(start);
//     dest.push_back(substring);
//     return dest;
// }
string print_op(OP op)
{
	string temp="";
	switch (op){
		case E:{
			temp = "=";
			return temp;
			break;
		}
		case GE:{
			temp = ">=";
			return temp;
			break;
		}
		case G:{
			temp = ">";
			return temp;
			break;
		}
		case LE:{
			temp = "<=";
			return temp;
			break;
		}
		case L:{
			temp = "<";
			return temp;
			break;
		}
		case NE:{
			temp = "!=";
			return temp;
			break;
		}
	}
}
string to_String(int n)
{
     int m = n;
     char s[5];
     char ss[5];
     int i=0,j=0;
     if (n < 0)// 处理负数
     {
         m = 0 - m;
         j = 1;
         ss[0] = '-';
     }    
     while (m>0)
     {
         s[i++] = m % 10 + '0';
         m /= 10;
     }
     s[i] = '\0';
     i = i - 1;
     while (i >= 0)
     {
         ss[j++] = s[i--];
     }    
     ss[j] = '\0';    
     return ss;
}
string print_condition(TableList* tblist,SelectCondition condition)
{
	string temp = "";
	temp = temp + condition.tb_name +"."+condition.col_name;
	temp = temp + print_op(condition.op);
	for(int i = 0;i<tblist->table_num;i++)
	{
		if(tblist->tbl[i]->tb_name==condition.tb_name)
		{
			for(int j = 0;j<tblist->tbl[i]->attr_count;j++)
			{
				if(tblist->tbl[i]->attrs[j].attr_name == condition.col_name)
				{
					if(tblist->tbl[i]->attrs[j].type == "char")
					{
						//temp = temp + "\""+condition.value+"\"";
						temp = temp + condition.value;
					}
					else temp = temp + condition.value;
					break;
				}
			}
			break;
		}
	}
	
	return temp;
}
string print_join(Join join,string table_to_broadcast,string table_to_stay,int table_to_broadcast_frag_id,int table_to_stay_frag_id)
{
	string temp = "";
	if(join.tb_name1 == table_to_broadcast)
		temp = temp + join.tb_name1 +"_frag"+ to_String(table_to_broadcast_frag_id) +"." + join.col_name1 + print_op(join.op) + join.tb_name2+"_frag"+ to_String(table_to_stay_frag_id)+"."+join.col_name2;
	else if(join.tb_name1 == table_to_stay)
		temp = temp + join.tb_name1 +"_frag"+ to_String(table_to_stay_frag_id) +"." + join.col_name1 + print_op(join.op) + join.tb_name2+"_frag"+ to_String(table_to_broadcast_frag_id)+"."+join.col_name2;
	return temp;
}
string print_join_condition(Join join,string table_to_broadcast,string table_to_stay,int table_to_broadcast_frag_id,int table_to_stay_frag_id)
{
	string temp = "";
	string tablename1 = join.tb_name1;
	string tablename2 = join.tb_name2; 
	string str = table_to_broadcast;
	int index = str.find_first_of('_',0);
	str = str.substr(0,index);
	if(tablename1 == str)
	{
		temp = table_to_broadcast+"_frag"+to_String(table_to_broadcast_frag_id)+"."+join.col_name1+print_op(join.op)+table_to_stay+"."+tablename2+"_"+join.col_name2;
	}
	else if(tablename2 == str)
	{
		temp = table_to_broadcast+"_frag"+to_String(table_to_broadcast_frag_id)+"."+join.col_name2+print_op(join.op)+table_to_stay+"."+tablename1+"_"+join.col_name1;
	}
	return temp;
}
string get_last_tablename(string tablename,int frag_id)
{
	return tablename +"_frag"+to_String(frag_id) ;
}
string print_create_table(TableList* tblist,string tablename,int frag_id)
{
	string new_tablename = tablename +"_frag"+to_String(frag_id);
	string tempstring = "create table " + new_tablename+" (";
	for(int i = 0;i < tblist->table_num;i++)
	{
		if(tblist->tbl[i]->tb_name == tablename)
		{
			for(int j = 0;j < tblist->tbl[i]->attr_count;j++)
			{
				tempstring += tblist->tbl[i]->attrs[j].attr_name+" "+tblist->tbl[i]->attrs[j].type;
				if(tblist->tbl[i]->attrs[j].type=="char") tempstring += "("+ to_String(tblist->tbl[i]->attrs[j].size)+")";
				if(tblist->tbl[i]->attrs[j].is_key) tempstring += " key";
				tempstring = tempstring+",";
			}
			int length = tempstring.length();
			tempstring = tempstring.substr(0,length-1);
			//tempstring.pop_back();
			tempstring+=");";
		}
	}
	return tempstring;
}
string print_selpro_create_table(SelectQuery &SQ,Query_Plan_Tree*  tree,TableList* tblist,string tablename,int frag_id,string attrneeded[],bool whether_join = false)
{
	string new_tablename;
	string tempstring;
	if(!whether_join)
	{
		new_tablename = tablename +"_frag"+to_String(frag_id) ;
		string droptable = "drop table if exists "+new_tablename+";";
		
		tempstring = "create table " + new_tablename+" (";
	}
	int k = 0;
	int tableindex;
	int i; 
	map<int,Opera_Frag>::iterator iterfrag;
	for(int i = 0;i < tblist->table_num;i++)
	{
		if(tblist->tbl[i]->tb_name == tablename)
		{
			tableindex = i;
			if(tblist->tbl[tableindex]->frag_type != 1){
				for(k = 0;attrneeded[k]!="";k++)
				{
					for(int j = 0;j < tblist->tbl[i]->attr_count;j++)
					{
						if(tblist->tbl[i]->attrs[j].attr_name == attrneeded[k])
						{
							if(whether_join) tempstring += tablename+"_" + tblist->tbl[i]->attrs[j].attr_name+" "+tblist->tbl[i]->attrs[j].type;
							else tempstring += tblist->tbl[i]->attrs[j].attr_name+" "+tblist->tbl[i]->attrs[j].type;
							if(tblist->tbl[i]->attrs[j].type=="char") tempstring+="("+to_String(tblist->tbl[i]->attrs[j].size)+")";
							if(!whether_join&&tblist->tbl[i]->attrs[j].is_key) tempstring += " key";
							tempstring = tempstring+",";			
							break;			
						}
					}
				}
				int length = tempstring.length();
				tempstring = tempstring.substr(0,length-1);
				if(!whether_join) tempstring+=");";
			} 
		}
	}
	if(tblist->tbl[tableindex]->frag_type == 1)
	{
		for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
		{
			if(iterfrag->second.tbl->tb_name == tablename && iterfrag->second.frag_id == frag_id)
			{
				for(i = 0;i<iterfrag->second.tbl->attr_count;i++)
				{
					if(attrneeded[i]!="")
					{
						string attr = attrneeded[i];
					
						for(int j = 0;j < iterfrag->second.tbl->frag_count;j++)
						{
							for(k = 0;k<iterfrag->second.tbl->fraginfo[j].condition.v1.attr_num;k++)
							{
								if(attr == iterfrag->second.tbl->fraginfo[j].condition.v1.attr_list[k])
								{
									int condition_frag_id = j+1;
									if(condition_frag_id == iterfrag->second.frag_id)
									{
										for(int jj = 0;jj < tblist->tbl[tableindex]->attr_count;jj++)
										{
											if(tblist->tbl[tableindex]->attrs[jj].attr_name == attr)
											{
												//tempstring += tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
												if(whether_join) tempstring += tablename+"_" + tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
												else tempstring += tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
												if(tblist->tbl[tableindex]->attrs[jj].type=="char") tempstring+="("+to_String(tblist->tbl[tableindex]->attrs[jj].size)+")";
												if(!whether_join&&tblist->tbl[tableindex]->attrs[jj].is_key) tempstring += " key";
												tempstring = tempstring+",";			
												break;			
											}
										}
									}
								}
							}
						}
					}
					
				}
				int flag1 = 0,flag2 = 0;
				for(int w = 0; w<SQ.from_count;w++)
				{
					if(SQ.FromList[w].tb_name=="Course")  flag1 = 1;
					if(SQ.FromList[w].tb_name=="Teacher") flag2 = 1;
				}
				if(flag1==1 &&flag2==1 && tablename=="Course" && frag_id==1)
				{
					
					if(!whether_join) tempstring = tempstring+"teacher_id integer,";
					else tempstring = tempstring+"Course_teacher_id integer,";
				}
				int length = tempstring.length();
				tempstring = tempstring.substr(0,length-1);
				if(!whether_join) tempstring+=");";
				break;
			}
		}
	}
	return tempstring;	
}
string print_selpro_create_table_vertical(SelectQuery &SQ, Query_Plan_Tree*  tree,TableList* tblist,string tablename,int frag_id,string attrneeded[],bool whether_join = false,bool whether_need_type = false)
{
	//cout<<"whether_need_type"<<whether_need_type<<endl;
	string new_tablename;
	string tempstring;
	if(!whether_join)
	{
		new_tablename = tablename +"_frag"+to_String(frag_id) ;
		tempstring = "create table " + new_tablename+" (";
	}
	int k = 0;
	int tableindex;
	int i; 
	map<int,Opera_Frag>::iterator iterfrag;
	for(int i = 0;i < tblist->table_num;i++)
	{
		if(tblist->tbl[i]->tb_name == tablename)
		{
			tableindex = i;
			if(tblist->tbl[tableindex]->frag_type != 1){
				for(k = 0;attrneeded[k]!="";k++)
				{
					for(int j = 0;j < tblist->tbl[i]->attr_count;j++)
					{
						if(tblist->tbl[i]->attrs[j].attr_name == attrneeded[k])
						{
							if(whether_need_type) tempstring += tblist->tbl[i]->attrs[j].attr_name;
							else if(whether_join) tempstring += tablename+"_" + tblist->tbl[i]->attrs[j].attr_name+" "+tblist->tbl[i]->attrs[j].type;
							else tempstring += tblist->tbl[i]->attrs[j].attr_name+" "+tblist->tbl[i]->attrs[j].type;
							if(!whether_need_type && tblist->tbl[i]->attrs[j].type=="char") tempstring+="("+to_String(tblist->tbl[i]->attrs[j].size)+")";
							if(!whether_join&&tblist->tbl[i]->attrs[j].is_key) tempstring += " key";
							tempstring = tempstring+",";			
							break;			
						}
					}
				}
				int length = tempstring.length();
				tempstring = tempstring.substr(0,length-1);
				if(!whether_join) tempstring+=");";
			} 
		}
	}
	if(tblist->tbl[tableindex]->frag_type == 1)
	{
		for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
		{
			if(iterfrag->second.tbl->tb_name == tablename && iterfrag->second.frag_id == frag_id)
			{
				for(i = 0;i<iterfrag->second.tbl->attr_count;i++)
				{
					if(attrneeded[i]!="")
					{
						string attr = attrneeded[i];
					
						for(int j = 0;j < iterfrag->second.tbl->frag_count;j++)
						{
							for(k = 0;k<iterfrag->second.tbl->fraginfo[j].condition.v1.attr_num;k++)
							{
								if(attr == iterfrag->second.tbl->fraginfo[j].condition.v1.attr_list[k])
								{
									int condition_frag_id = j+1;
									if(condition_frag_id == iterfrag->second.frag_id)
									{
										for(int jj = 0;jj < tblist->tbl[tableindex]->attr_count;jj++)
										{
											if(tblist->tbl[tableindex]->attrs[jj].attr_name == attr)
											{
												//tempstring += tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
													
												if(whether_need_type) tempstring += tblist->tbl[tableindex]->attrs[jj].attr_name;
												else if(whether_join) tempstring += tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
												else tempstring += tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
												if(!whether_need_type && tblist->tbl[tableindex]->attrs[jj].type=="char") tempstring+="("+to_String(tblist->tbl[tableindex]->attrs[jj].size)+")";
												if(!whether_join&&tblist->tbl[tableindex]->attrs[jj].is_key) tempstring += " key";
												tempstring = tempstring+",";			
												break;			
											}
										}
									}
								}
							}
						}
					}
					
				}
				int flag1 = 0,flag2 = 0;
				for(int w = 0; w<SQ.from_count;w++)
				{
					if(SQ.FromList[w].tb_name=="Course")  flag1 = 1;
					if(SQ.FromList[w].tb_name=="Teacher") flag2 = 1;
				}
				if(flag1==1 &&flag2==1 && tablename=="Course" && frag_id==1)
				{
					if(!whether_join) tempstring = tempstring+"teacher_id integer,";
					else tempstring = tempstring+"Course_teacher_id integer,";
				}
				int length = tempstring.length();
				tempstring = tempstring.substr(0,length-1);
				if(!whether_join) tempstring+=");";
				break;
			}
		}
	}
	return tempstring;	
}
string generate(Query_Plan_Tree*  tree,TableList* tblist,string tablename,int frag_id,string attrneeded[],bool whether_join = false)
{
	string new_tablename;
	string tempstring;
	if(!whether_join)
	{
		new_tablename = tablename +"_frag"+to_String(frag_id) ;
		tempstring = "create table " + new_tablename+" (";
	}
	int k = 0;
	int tableindex;
	int i; 
	map<int,Opera_Frag>::iterator iterfrag;
	for(int i = 0;i < tblist->table_num;i++)
	{
		if(tblist->tbl[i]->tb_name == tablename)
		{
			tableindex = i;
			if(tblist->tbl[tableindex]->frag_type != 1){
				for(k = 0;attrneeded[k]!="";k++)
				{
					for(int j = 0;j < tblist->tbl[i]->attr_count;j++)
					{
						if(tblist->tbl[i]->attrs[j].attr_name == attrneeded[k])
						{
							if(whether_join) tempstring += tablename+"_" + tblist->tbl[i]->attrs[j].attr_name+" "+tblist->tbl[i]->attrs[j].type;
							else tempstring += tblist->tbl[i]->attrs[j].attr_name+" "+tblist->tbl[i]->attrs[j].type;
							if(tblist->tbl[i]->attrs[j].type=="char") tempstring+="("+to_String(tblist->tbl[i]->attrs[j].size)+")";
							if(!whether_join&&tblist->tbl[i]->attrs[j].is_key) tempstring += " key";
							tempstring = tempstring+",";			
							break;			
						}
					}
				}
				int length = tempstring.length();
				tempstring = tempstring.substr(0,length-1);
				if(!whether_join) tempstring+=");";
			} 
		}
	}
	if(tblist->tbl[tableindex]->frag_type == 1)
	{
		for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
		{
			if(iterfrag->second.tbl->tb_name == tablename && iterfrag->second.frag_id == frag_id)
			{
				for(i = 0;i<iterfrag->second.tbl->attr_count;i++)
				{
					if(attrneeded[i]!="")
					{
						string attr = attrneeded[i];
					
						for(int j = 0;j < iterfrag->second.tbl->frag_count;j++)
						{
							for(k = 0;k<iterfrag->second.tbl->fraginfo[j].condition.v1.attr_num;k++)
							{
								if(attr == iterfrag->second.tbl->fraginfo[j].condition.v1.attr_list[k])
								{
									int condition_frag_id = j+1;
									if(condition_frag_id == iterfrag->second.frag_id)
									{
										for(int jj = 0;jj < tblist->tbl[tableindex]->attr_count;jj++)
										{
											if(tblist->tbl[tableindex]->attrs[jj].attr_name == attr)
											{
												//tempstring += tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
												if(whether_join) tempstring += tablename+"_" + tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
												else tempstring += tblist->tbl[tableindex]->attrs[jj].attr_name+" "+tblist->tbl[tableindex]->attrs[jj].type;
												if(tblist->tbl[tableindex]->attrs[jj].type=="char") tempstring+="("+to_String(tblist->tbl[tableindex]->attrs[jj].size)+")";
												if(!whether_join&&tblist->tbl[tableindex]->attrs[jj].is_key) tempstring += " key";
												tempstring = tempstring+",";			
												break;			
											}
										}
									}
								}
							}
						}
					}
					
				}
				int length = tempstring.length();
				tempstring = tempstring.substr(0,length-1);
				if(!whether_join) tempstring+=");";
				break;
			}
		}
	}
	return tempstring;	
}
string get_new_join_table_name(string name, int db_id)
{
	string str = name;
    string substring;
    int start = 0, index;
    string name1,name2;
    index = str.find_first_of('_',start);
//    cout<<name<<endl;
    name1 = str.substr(start,index-start );
    int length = str.length();
    str = str.substr(index+1,length-index);
    //cout<<name1<<endl;
    //cout<<str<<endl; 
//    return "aa";
    start = 0;
    index = str.find_first_of('_',start);
    length = str.length();
    str = str.substr(index+1,length-index);
    start = 0;
    index = str.find_first_of('_',start);
    name2 = str.substr(start,index-start );
	return name1+name2+"_frag"+to_String(db_id);
}

//TODO
// void f(string ip,int port,string,string db_name,string stmt,int count){
// 	RPCExecute(string ip,int port,string db_name,string stmt);
// 	count++;
// }

void generate_sql_list(SelectQuery &SQ, Query_Plan_Tree*  tree, TableList* tblist)
{
	//map<int, Table_change*> tree_tablename;
	cout<<endl<<"begin to generate sql for each site"<<endl<<endl;
	map<int, Opera_Join>::iterator iterjoin;
	map<int, Opera_Selection>::iterator iterselect;
	map<int, Opera_Project>::iterator iterproject;
	map<int, Opera_Union>::iterator iterunion;
	map<int, Opera_Frag>::iterator iterfrag,iterfragtemp;
	map<int, Opera_SelPro>::iterator iterselpro;
	string tablename[MAX_TABLE_COUNT] = "";
	string nnewtablename;
	int i,j,k;
	string sql = "";
	int db_id;
	bool result;
	for(i = 0;i<tblist->table_num;i++)
	{
		tablename[i] = tblist->tbl[i]->tb_name;
	}
	//没有考虑table哦，只考虑了frag的情况 
	string createsql = "";
	int temptbl_num = 1;
	string lasttablename = "";
	int fflag1=0,fflag2=0;
	for(int w = 0;w<SQ.from_count;w++)
	{
		if(SQ.FromList[w].tb_name=="Course")   fflag1 = 1;
		if(SQ.FromList[w].tb_name=="Teacher")  fflag2 = 1;
	}
	if(fflag1 == 1 && fflag2 == 1)
	{
		RPCExecute(sites[0],select_ports[0],dbs[0],"drop table if exists Course_temp;");
		sql = "create table Course_temp(id int, name char(80),teacher_id int);";
		//sql = sql.substr(0,sql.length()-1);
		//sql = sql+";";
		cout<<sql<<endl;
		RPCExecute(sites[0],select_ports[0],dbs[0],sql);
		sql = "insert into Course_temp(id,name) select id,name from Course;";
		cout<<sql<<endl;
		RPCExecute(sites[0],select_ports[0],dbs[0],sql);
		RPCExecute(sites[0],select_ports[0],dbs[0],"drop table if exists Course_temp2;");
		sql = "create table Course_temp2(id int ,name char(80),location char(8),credit_hour int,teacher_id int);";
		cout<<sql<<endl;
		RPCExecute(sites[0],select_ports[0],dbs[0],sql);
		cout<<"insert into the table"<<endl;
		RPCInsertFromSelect(sites[1],select_ports[1],sites[0],select_ports[0],"Course","Course_temp2",dbs[1],dbs[0]);
		sql = "update Course_temp,Course_temp2 set Course_temp.teacher_id=Course_temp2.teacher_id where Course_temp.id=Course_temp2.id";
		RPCExecute(sites[0],select_ports[0],dbs[0],sql);
		cout<<sql<<endl;
	}
	if(SQ.from_count==1) lasttablename = SQ.FromList[0].tb_name;
	for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
	{
		string sqlcondition = "";
		string attrneeded[MAX_ATTR_COUNT]={""};
		find_all_attr_needed(tblist,SQ,iterfrag->second.tbl->tb_name,attrneeded);
		int parent = tree->query_tree_node[iterfrag->first].parent;
		if(tree->query_tree_node[parent].type == 6 && iterfrag->second.tbl->frag_type!=1)  //the frag's parent is a select and projection operation and the frag type is not vertical
		{
			db_id = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].db_id;
			//find_all_attr_needed(SQ,iterfrag->second.tbl->tb_name,attrneeded);
			//createsql = print_selpro_create_table(tblist,iterfrag->second.tbl->tb_name,iterfrag->second.frag_id,attrneeded);
			//cout<<createsql<<endl;
			sql = "create table ";
			lasttablename = get_last_tablename(iterfrag->second.tbl->tb_name,iterfrag->second.frag_id);
			sql += lasttablename+" as select ";
			string droptable = "drop table if exists "+lasttablename+";";
		    RPCExecute(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],droptable);
			//cout<<"into the function"<<endl;
			while(tree->query_tree_node[parent].type == 6)
			{
				iterselpro = tree->tree_selpro.find(parent);
				if(iterselpro == tree->tree_selpro.end())
					cout<<"ERROR: can not find the select and projection operation in sql generation";
				sqlcondition += print_condition(tblist,iterselpro->second.condition) + " and ";
				//cout<<sqlcondition
				parent = tree->query_tree_node[parent].parent;
			}
			//sql = "insert into table "+ lasttablename+" select ";
			for( i = 0;i<MAX_ATTR_COUNT;i++)
			{
				if(attrneeded[i]!="")
				{
					sql = sql+iterfrag->second.tbl->tb_name+"." + attrneeded[i]+",";
				}
			}
			int length = sql.length();
			sql.replace(length-1,1,"");
			sql = sql + " from " + iterfrag->second.tbl->tb_name +" where " + sqlcondition;
			length = sql.length();
			sql.replace(length-4,4,";");
			//sql = sql + ";";
			cout<<sql<<endl;
			cout<<"send it to DB "<<db_id<<endl;
			//db_id = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].db_id;
			RPCExecute(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],sql);
			//if(!result) {cout<<"ERROR: here 1 failed"<<endl;}
			//cout<<"result is "<<result<<endl;
			
			temptbl_num++;
		}
		else if(tree->query_tree_node[parent].type == 6 && iterfrag->second.tbl->frag_type==1)
		{
			db_id = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].db_id;
			sql = "create table ";
			lasttablename = get_last_tablename(iterfrag->second.tbl->tb_name,iterfrag->second.frag_id);
			string droptable = "drop table if exists "+lasttablename+";";
			RPCExecute(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],droptable);
			sql += lasttablename+" as select ";
			//string sqlconditionlist[MAX_FRAG_COUNT] ={""};
			//string sqlproject[MAX_FRAG_COUNT] = {""};
			string sqlproject,sqlcondtition;
			while(tree->query_tree_node[parent].type == 6)
			{
				iterselpro = tree->tree_selpro.find(parent);
				if(iterselpro == tree->tree_selpro.end())
					cout<<"ERROR: can not find the select and projection operation in sql generation";
				int condition_frag_id;
				string attr = iterselpro->second.condition.col_name;
				for(j = 0;j < iterfrag->second.tbl->frag_count;j++)
				{
					for(k = 0;k<iterfrag->second.tbl->fraginfo[j].condition.v1.attr_num;k++)
					{
						if(attr == iterfrag->second.tbl->fraginfo[j].condition.v1.attr_list[k])
						{
							condition_frag_id = j+1;
							if(condition_frag_id == iterfrag->second.frag_id)
							{
								sqlcondition += print_condition(tblist,iterselpro->second.condition) + " and ";
							}
							//sqlconditionlist[condition_frag_id-1] += print_condition(tblist,iterselpro->second.condition) + " and ";
						}
					}
				}
				parent = tree->query_tree_node[parent].parent;
			}
			for( i = 0;i<MAX_ATTR_COUNT;i++)
			{
				if(attrneeded[i]!="")
				{
					//sql = sql+iterfrag->second.tbl->tb_name+"." + iterselpro->second.attr[i]+",";
					string attr = attrneeded[i];
					
					for(j = 0;j < iterfrag->second.tbl->frag_count;j++)
					{
						for(k = 0;k<iterfrag->second.tbl->fraginfo[j].condition.v1.attr_num;k++)
						{
							if(attr == iterfrag->second.tbl->fraginfo[j].condition.v1.attr_list[k])
							{
								int condition_frag_id = j+1;
								if(condition_frag_id == iterfrag->second.frag_id)
								{
									if(fflag1 == 1 && fflag2==1 && lasttablename=="Course_frag1")
									{
										sqlproject+= iterfrag->second.tbl->tb_name+"_temp." + attrneeded[i]+",";
									}
									else{
										sqlproject+= iterfrag->second.tbl->tb_name+"." + attrneeded[i]+",";
									}
								}
								//sqlproject+= iterfrag->second.tbl->tb_name+"." + iterselpro->second.attr[i]+",";
							}
						}
					}
				}
			}
			if(fflag1 == 1 && fflag2==1 && lasttablename=="Course_frag1")
				sqlproject+="teacher_id,";
			sql += sqlproject;
			int length = sql.length();
			sql.replace(length-1,1,"");
			sql = sql + " from " + iterfrag->second.tbl->tb_name;
			if(fflag1 == 1 && fflag2==1 && lasttablename=="Course_frag1") sql = sql+"_temp";
			if(sqlcondition != "")
			{
				sql = sql +" where " + sqlcondition;
				length = sql.length();
				sql.replace(length-4,4,";");
			}
			else sql = sql +";";
			cout<<sql<<endl;
			db_id = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].db_id;
			cout<<"send it to DB "<<db_id<<endl;
			RPCExecute(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],sql);
		}
		else if(tree->query_tree_node[iterfrag->first].parent != -1){
			lasttablename = get_last_tablename(iterfrag->second.tbl->tb_name,iterfrag->second.frag_id);
			db_id = iterfrag->second.tbl->fraginfo[iterfrag->second.frag_id-1].db_id;
			sql = "create table "+ lasttablename + " as select ";
			string attrneeded[MAX_ATTR_COUNT]={""};
			find_all_attr_needed(tblist,SQ,iterfrag->second.tbl->tb_name,attrneeded);
			for(int q = 0;q < MAX_ATTR_COUNT;q++)
			{
				if(attrneeded[q]!="")
				{
					sql = sql + attrneeded[q]+",";
				}
			}
			//sql.pop_back();
			sql = sql.substr(0,sql.length()-1);
			sql = sql + " from "+iterfrag->second.tbl->tb_name+";";
			string droptable = "drop table if exists "+lasttablename+";";
			RPCExecute(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],droptable);
			cout<<sql<<endl;
			
			cout<<"send it to DB "<<db_id<<endl;		
			result = RPCExecute(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],sql);
			//if(!result) {cout<<"ERROR: here 1 failed"<<endl;}
			//cout<<"result is "<<result<<endl;	
		}
	}
	
	string tablename1,tablename2;
	for(iterjoin = tree->tree_join.begin();iterjoin != tree->tree_join.end();iterjoin++)
	{
		sql = "";
		int staytableindex,broadcasttableindex;
		//cout<<"all join num is "<<iterjoin->second.joinnum<<endl;
		string newattrs="";
		string newattrs1 = "";
		string tempattrs = "";
		Join_Middle_Table join_table;
		for(k = 0;k < MAX_JOINEDTABLE_NUM;k++)
			join_table.joinedtable[k] = "";
		for(k = 0;k < MAX_DB_COUNT;k++)
			join_table.db_list[k] = 0;
		//没有写broadcast呢 
		int lchildindex = tree->query_tree_node[iterjoin->first].child[0];
		int rchildindex = tree->query_tree_node[iterjoin->first].child[1];
		if(tree->query_tree_node[lchildindex].type == 4 && tree->query_tree_node[rchildindex].type == 4){
			cout<<"all join num is "<<iterjoin->second.joinnum<<endl;
			for(j = 0;j < iterjoin->second.joinnum;j++)
			{
			
				
			cout<<"broadcast "<<iterjoin->second.join_frag[j].tbl_to_broadcast<<"'s frag_id "<<iterjoin->second.join_frag[j].frag_to_broadcast_id<<" from db_id "<<iterjoin->second.join_frag[j].from_db_id;
			cout<<" to db_id "<<iterjoin->second.join_frag[j].cast_to_db_id<<" to join with "<<iterjoin->second.join_frag[j].tbl_to_stay<<"'s frag_id "<<iterjoin->second.join_frag[j].frag_to_stay_id<<endl;
			int frag_to_broadcast_table_id,frag_to_stay_table_id;
			map<int, Opera_Frag>::iterator iterfragtobroadcast,iterfragtostay;
			//the iterjoin->second.join_frag[j].frag_to_broadcast_id is the index of the frag_id  the frag_to_broadcast_table_id is the frag_id of the table;
			if(tree->query_tree_node[lchildindex].type == 4 && tree->query_tree_node[rchildindex].type == 4) //the join node 's children are all tables
			{
				iterfragtobroadcast = tree->tree_frag.find(iterjoin->second.join_frag[j].frag_to_broadcast_id);
				if(iterfragtobroadcast == tree->tree_frag.end())  cout<<"ERROR: can not find the fragment in generate sql";
				frag_to_broadcast_table_id = iterfragtobroadcast->second.frag_id;
			
				iterfragtostay = tree->tree_frag.find(iterjoin->second.join_frag[j].frag_to_stay_id);
				if(iterfragtostay == tree->tree_frag.end())  cout<<"ERROR: can not find the fragment in generate sql";
				frag_to_stay_table_id = iterfragtostay->second.frag_id;
				if(iterjoin->second.join_frag[j].from_db_id != iterjoin->second.join_frag[j].cast_to_db_id)
				{
					string attrneeded[MAX_ATTR_COUNT]={""};
					find_all_attr_needed(tblist,SQ,iterjoin->second.join_frag[j].tbl_to_broadcast,attrneeded);
					sql = print_selpro_create_table(SQ,tree,tblist,iterjoin->second.join_frag[j].tbl_to_broadcast,frag_to_broadcast_table_id,attrneeded);
					cout<<sql<<endl;
					cout<<"send it to db "<<iterjoin->second.join_frag[j].cast_to_db_id<<endl;
					cout<<"insert into table"<<endl;
					cout<<"to"<<endl;
					string new_tablename = iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(frag_to_broadcast_table_id) ;
					string droptable = "drop table if exists "+new_tablename+";";
					RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],droptable);
					RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],sql);
					cout<<"come to here"<<endl;
					string newtablename = iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(frag_to_broadcast_table_id);
					cout<<"from "<<frag_to_broadcast_table_id<<" to "<<frag_to_stay_table_id;
					RPCInsertFromSelect(sites[frag_to_broadcast_table_id-1],select_ports[frag_to_broadcast_table_id-1],sites[frag_to_stay_table_id-1],select_ports[frag_to_stay_table_id-1],newtablename,newtablename,dbs[frag_to_broadcast_table_id-1],dbs[frag_to_stay_table_id-1]);
				}
				string newtablename = iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(frag_to_broadcast_table_id)+"_"+ iterjoin->second.join_frag[j].tbl_to_stay+"_frag"+to_String(frag_to_stay_table_id);
				string droptable = "drop table if exists "+newtablename+";";
				RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],droptable);
				sql = "create table " + newtablename;
				string table_broadcast_attrneeded[MAX_ATTR_COUNT]={""};
				find_all_attr_needed(tblist,SQ,iterjoin->second.join_frag[j].tbl_to_broadcast,table_broadcast_attrneeded);
				string table_stay_attrneeded[MAX_ATTR_COUNT]={""};
				find_all_attr_needed(tblist,SQ,iterjoin->second.join_frag[j].tbl_to_stay,table_stay_attrneeded);
				
				sql += "("+print_selpro_create_table(SQ,tree,tblist,iterjoin->second.join_frag[j].tbl_to_broadcast,frag_to_broadcast_table_id,table_broadcast_attrneeded,true);
				sql += "," +print_selpro_create_table(SQ,tree,tblist,iterjoin->second.join_frag[j].tbl_to_stay,frag_to_stay_table_id,table_stay_attrneeded,true)+");";
				cout<<endl<<sql<<endl;
				RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],sql);
				tempattrs = print_selpro_create_table(SQ,tree,tblist,iterjoin->second.join_frag[j].tbl_to_broadcast,frag_to_broadcast_table_id,table_broadcast_attrneeded,true) +  
					"," +print_selpro_create_table(SQ,tree,tblist,iterjoin->second.join_frag[j].tbl_to_stay,frag_to_stay_table_id,table_stay_attrneeded,true);
				if(j==0)  newattrs = tempattrs;
				if( j!=0 && newattrs != tempattrs)  newattrs1 = tempattrs;
				//cout<<"the new attrs is "<<newattrs<<endl;
				cout<<"send it to db "<<iterjoin->second.join_frag[j].cast_to_db_id<<endl;
				string insertsql;
				insertsql = "insert into "+newtablename+" select * from ";
				tablename1 = iterjoin->second.join_frag[j].tbl_to_broadcast + "_frag"+to_String(frag_to_broadcast_table_id);
				tablename2 = iterjoin->second.join_frag[j].tbl_to_stay + "_frag"+to_String(frag_to_stay_table_id);
				insertsql = insertsql+ tablename1 + ", "+tablename2 +" where ";
				insertsql = insertsql + print_join(iterjoin->second.join,iterjoin->second.join_frag[j].tbl_to_broadcast,iterjoin->second.join_frag[j].tbl_to_stay,frag_to_broadcast_table_id,frag_to_stay_table_id) +";";
				cout<<insertsql<<endl;
				cout<<"send it to db "<<iterjoin->second.join_frag[j].cast_to_db_id<<endl;
				//TODO concurrency

				RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],insertsql);
				// thread t (f,sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],insertsql,count);
				// t.detach();
				join_table.joinedtable[j] = newtablename;
				join_table.joinedtable_db[j] = iterjoin->second.join_frag[j].cast_to_db_id;
				db_id =  iterjoin->second.join_frag[j].cast_to_db_id;
				join_table.db_list[db_id] = 1;
			}
			}
		}
		
		else if(tree->query_tree_node[lchildindex].type != 4 || tree->query_tree_node[rchildindex].type != 4)
		{
			int joinindex, unionindex;
			if(tree->query_tree_node[lchildindex].type == 4 && tree->query_tree_node[rchildindex].type == 3)
			{
				joinindex = rchildindex;
				unionindex = lchildindex;
			}
			else if(tree->query_tree_node[rchildindex].type == 4 && tree->query_tree_node[lchildindex].type == 3)
			{
				joinindex = lchildindex;
				unionindex = rchildindex;					
			}
			else
				cout<<"ERROR:The join node error in generate sqllist"<<endl;
			map<int,Join_Middle_Table>::iterator iterjointable;
			cout<<endl;
			iterjointable = join_middle_table.find(joinindex);
			//if(iterjointable != join_middle_table.end())
			//	cout<<"find the join node and the table name is "<<iterjointable->second.newtablename;
			map<int, Opera_Union>::iterator tempiterunion;
			tempiterunion = tree->tree_union.find(unionindex);
			if(tempiterunion == tree->tree_union.end())  cout<<"ERROR: can not find the union in generate sql";
			//cout<<"the union tablename is "<<tempiterunion->second.tbl->tb_name<<endl;
			//cout<<"the stay table is"<<iterjoin->second.which_to_stay<<endl;	
			map<int, Opera_Join>::iterator childiterjoin;
			childiterjoin = tree->tree_join.find(joinindex);	
			string childstaytable = childiterjoin->second.which_to_stay;
			//cout<<"child stay table "<<childstaytable;
			int joinnum1,joinnum2;
			int uniontableindex;
			for(int m = 0;m<tblist->table_num;m++)
			{
				if(tblist->tbl[m]->tb_name == childstaytable)
				{
					joinnum1 = tblist->tbl[m]->frag_count;
				}
				else if(tblist->tbl[m]->tb_name == tempiterunion->second.tbl->tb_name)
				{
					//joinnum2 = tblist->tbl[m]->frag_count;
					uniontableindex = m;
				}
			}
			joinnum2 = tree->query_tree_node[tempiterunion->first].childnum;
			int joinnum = joinnum1*joinnum2;
			cout<<"all join num is "<<joinnum<<endl;
			iterjoin->second.which_to_broadcast = tempiterunion->second.tbl->tb_name;
			iterjoin->second.which_to_stay = iterjointable->second.newtablename;
			//cout<<"which to broadcast is "<<iterjoin->second.which_to_broadcast<<" and which to stay is "<<iterjoin->second.which_to_stay<<endl;
			iterjoin->second.joinnum = joinnum;
			int numtemp = 0;
			//cout<<"the jointable's db_list is "<<endl;
			for(j = 0;j < MAX_DB_COUNT; j++)
			{
				//cout<<iterjointable->second.db_list[j]<<" ";
				if(iterjointable->second.db_list[j]==1)
				{

					for(int q = 0;q < joinnum2 ;q++)
					{
						int childindex = tree->query_tree_node[tempiterunion->first].child[q];
						while(tree->query_tree_node[childindex].type != 5)
							childindex = tree->query_tree_node[childindex].child[0];
						map<int, Opera_Frag>::iterator tempiterfrag;
						tempiterfrag = tree->tree_frag.find(childindex);
						iterjoin->second.join_frag[numtemp].tbl_to_stay = iterjoin->second.which_to_stay+"_frag"+to_String(j);
						iterjoin->second.join_frag[numtemp].tbl_to_broadcast = iterjoin->second.which_to_broadcast;
						iterjoin->second.join_frag[numtemp].from_db_id = tempiterfrag->second.frag_id;
						iterjoin->second.join_frag[numtemp].cast_to_db_id = j;
						numtemp++;
					}
				}

			}
			for(j = 0;j < joinnum;j++)
			{
				cout<<"broadcast "<<iterjoin->second.join_frag[j].tbl_to_broadcast<<" from db_id "<<iterjoin->second.join_frag[j].from_db_id;
				cout<<" to db_id "<<iterjoin->second.join_frag[j].cast_to_db_id<<" to join with "<<iterjoin->second.join_frag[j].tbl_to_stay<<endl;
				/*if(iterjoin->second.which_to_stay == tempiterunion->second.tbl->tb_name)
				{
					string temp;
					//cout<<"want to change"<<endl;
					temp = iterjoin->second.which_to_broadcast;
					iterjoin->second.which_to_broadcast = iterjoin->second.which_to_stay;
					iterjoin->second.which_to_stay = temp;
					temp = iterjoin->second.join_frag[j].tbl_to_stay;
					iterjoin->second.join_frag[j].tbl_to_stay = iterjoin->second.join_frag[j].tbl_to_broadcast;
					iterjoin->second.join_frag[j].tbl_to_broadcast = temp;
					int tempint;
					tempint = iterjoin->second.join_frag[j].frag_to_stay_id;
					iterjoin->second.join_frag[j].frag_to_stay_id = iterjoin->second.join_frag[j].frag_to_broadcast_id;
					iterjoin->second.join_frag[j].frag_to_broadcast_id = tempint;
					tempint = iterjoin->second.join_frag[j].cast_to_db_id;
					iterjoin->second.join_frag[j].cast_to_db_id = iterjoin->second.join_frag[j].from_db_id;
					iterjoin->second.join_frag[j].from_db_id = tempint;
				}*/
				//cout<<"table to broadcast is "<<iterjoin->second.which_to_broadcast<<" and the table to stay is "<<iterjoin->second.which_to_stay<<endl;
				//iterfragtobroadcast = tree->tree_frag.find(iterjoin->second.join_frag[j].frag_to_broadcast_id);
				//if(iterfragtobroadcast == tree->tree_frag.end())  cout<<"ERROR: can not find the fragment in generate sql";
				//frag_to_broadcast_table_id = iterfragtobroadcast->second.frag_id;
			//
				//iterfragtostay = tree->tree_frag.find(iterjoin->second.join_frag[j].frag_to_stay_id);
				//if(iterfragtostay == tree->tree_frag.end())  cout<<"ERROR: can not find the fragment in generate sql";
				//frag_to_stay_table_id = iterfragtostay->second.frag_id;
				if(iterjoin->second.join_frag[j].from_db_id != iterjoin->second.join_frag[j].cast_to_db_id)
				{
					string attrneeded[MAX_ATTR_COUNT]={""};
					find_all_attr_needed(tblist,SQ,iterjoin->second.join_frag[j].tbl_to_broadcast,attrneeded);
					sql = print_selpro_create_table(SQ,tree,tblist,iterjoin->second.join_frag[j].tbl_to_broadcast,iterjoin->second.join_frag[j].from_db_id,attrneeded);
				    string new_tablename = iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(iterjoin->second.join_frag[j].from_db_id) ;
					string droptable = "drop table if exists "+new_tablename+";";
					RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],droptable);
					cout<<sql<<endl;
					cout<<"send it to db "<<iterjoin->second.join_frag[j].cast_to_db_id<<endl;
					cout<<"insert into table"<<endl;
					RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],sql);
					string newtablename = iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(iterjoin->second.join_frag[j].from_db_id);
					RPCInsertFromSelect(sites[iterjoin->second.join_frag[j].from_db_id-1],select_ports[iterjoin->second.join_frag[j].from_db_id-1],sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],newtablename,newtablename,dbs[iterjoin->second.join_frag[j].from_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1]);
				}
				string newtablename = iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(iterjoin->second.join_frag[j].from_db_id)+"_"
							+ iterjointable->second.newtablename+"_frag"+to_String(iterjoin->second.join_frag[j].cast_to_db_id);
				sql = "create table " + newtablename + "(";
				string droptable = "drop table if exists "+newtablename+";";
				RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],droptable);
				string table_broadcast_attrneeded[MAX_ATTR_COUNT]={""};
				find_all_attr_needed(tblist,SQ,iterjoin->second.join_frag[j].tbl_to_broadcast,table_broadcast_attrneeded);
				string uniondesc = print_selpro_create_table(SQ,tree,tblist,iterjoin->second.join_frag[j].tbl_to_broadcast,iterjoin->second.join_frag[j].from_db_id,table_broadcast_attrneeded,true);
				sql = sql + uniondesc;
				sql = sql + "," + iterjointable->second.attrs +");";
				tempattrs = uniondesc + "," + iterjointable->second.attrs;
				if(j==0)  newattrs = tempattrs;
				if( j!=0 && newattrs != tempattrs)  newattrs1 = tempattrs;
				//cout<<uniondesc<<endl;
				cout<<sql<<endl;
				cout<<"sent it to db "<<iterjoin->second.join_frag[j].cast_to_db_id<<endl;
				RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],sql);
				string insertsql = "";
				insertsql = "insert into "+newtablename+" select * from "
						+iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(iterjoin->second.join_frag[j].from_db_id)+","
						+iterjointable->second.newtablename+"_frag"+to_String(iterjoin->second.join_frag[j].cast_to_db_id);
				insertsql += " where ";
				
				//if(iterjoin->second.join.tb_name1+"_frag"+to_String(iterjoin->second.join_frag[j].cast_to_db_id) == iterjoin->second.join_frag[j].tbl_to_broadcast)		
				//	iterjoin->second.join.tb_name2 = iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(iterjoin->second.join_frag[j].from_db_id);
				//else if(iterjoin->second.join.tb_name2+"_frag"+to_String(iterjoin->second.join_frag[j].cast_to_db_id) == iterjoin->second.join_frag[j].tbl_to_broadcast)
				//	iterjoin->second.join.tb_name2 = iterjoin->second.join_frag[j].tbl_to_broadcast +"_frag"+to_String(iterjoin->second.join_frag[j].from_db_id);
				insertsql += print_join_condition(iterjoin->second.join,iterjoin->second.join_frag[j].tbl_to_broadcast,iterjoin->second.join_frag[j].tbl_to_stay,iterjoin->second.join_frag[j].from_db_id,iterjoin->second.join_frag[j].cast_to_db_id);
				insertsql += ";";
				cout<<insertsql<<endl;
				cout<<"sent it to db "<<iterjoin->second.join_frag[j].cast_to_db_id<<endl;
				RPCExecute(sites[iterjoin->second.join_frag[j].cast_to_db_id-1],select_ports[iterjoin->second.join_frag[j].cast_to_db_id-1],dbs[iterjoin->second.join_frag[j].cast_to_db_id-1],insertsql);
				join_table.joinedtable[j] = newtablename;
				join_table.joinedtable_db[j] = iterjoin->second.join_frag[j].cast_to_db_id;
				db_id =  iterjoin->second.join_frag[j].cast_to_db_id;
				join_table.db_list[db_id] = 1;
			}
		}
		int flag = 0;
		string basetable =  "";
		int frag_type1,frag_type2;
		map<int,Join_Middle_Table>::iterator iterjointable;
		for(int q = 0; q < tblist->table_num;q++)
		{
			if(tblist->tbl[q]->tb_name  == iterjoin->second.which_to_stay)
				staytableindex = q;
			else if(tblist->tbl[q]->tb_name  == iterjoin->second.which_to_broadcast)
				broadcasttableindex = q;
		}
		if(tree->query_tree_node[lchildindex].type != 4 || tree->query_tree_node[rchildindex].type != 4)
		{
			int childjoinindex;
			if(tree->query_tree_node[lchildindex].type != 4) childjoinindex = lchildindex;
			else childjoinindex = rchildindex;
			iterjointable = join_middle_table.find(childjoinindex);
			if(iterjointable == join_middle_table.end())  cout<<"ERROR: can not find the join node in join_middle_table";
			
		}
		frag_type1 = tblist->tbl[broadcasttableindex]->frag_type;
		if(tree->query_tree_node[lchildindex].type != 4 || tree->query_tree_node[rchildindex].type != 4)
			frag_type2 = iterjointable->second.newtable->frag_type;
		else frag_type2 = tblist->tbl[staytableindex]->frag_type;
		//cout<<"frag_type1 is "<<frag_type1<<"frag_type2 is "<<frag_type2<<endl;
		for(k = 0;k < MAX_DB_COUNT;k++)
		{
			flag = 0;
			basetable = "";
			for(int m = 0;m<MAX_JOINEDTABLE_NUM;m++)
			{
				if(flag == 0 && join_table.joinedtable_db[m] == k && join_table.joinedtable[m]!="") 
				{
					basetable = join_table.joinedtable[m];
					flag = 1;
				}
				else if(frag_type1==0 && frag_type2==0 && flag == 1 && join_table.joinedtable_db[m] == k && join_table.joinedtable[m]!="")
				{
					sql = "insert into "+ basetable + " select * from "+ join_table.joinedtable[m]+";";
					cout<<sql<<endl;
					cout<<"send it to db "<<k<<endl;
					RPCExecute(sites[k-1],select_ports[k-1],dbs[k-1],sql);
				}
				else if((frag_type1!=0 || frag_type2!=0) && flag == 1 && join_table.joinedtable_db[m] == k && join_table.joinedtable[m]!=""){
					sql = " ";
					//cout<<"the newattrs is :"<<newattrs<<endl;
					//cout<<"the other newattrs is :"<<newattrs1<<endl;
					vector<string> temp_attr,temp_attr1;
					temp_attr = Split(newattrs,",");
					temp_attr1 = Split(newattrs1,",");
					vector<string>::iterator iter;
					//for (int ii=0;ii<temp_attr.size();ii++)
        			//	cout<<temp_attr.at(ii)<<endl;
        			//for (int ii=0;ii<temp_attr1.size();ii++)
        			//	cout<<temp_attr1.at(ii)<<endl;
					for( int p = 0;p<temp_attr.size();p++)
					{
						iter = find(temp_attr1.begin(),temp_attr1.end(),temp_attr.at(p));
						if(iter == temp_attr1.end())
							temp_attr1.push_back(temp_attr.at(p));
					}
					//for (int ii=0;ii<temp_attr1.size();ii++)
        			//	cout<<temp_attr1.at(ii)<<endl;
        			string newtablename = get_new_join_table_name(basetable,k);
        			string droptable = "drop table if exists "+newtablename + ";";
        			RPCExecute(sites[k-1],select_ports[k-1],dbs[k-1],droptable);
        			sql = "create table " + newtablename;
        			string sqlattrs = "";
        			for(int p = 0;p < temp_attr1.size();p++)
        			{
        				sqlattrs = sqlattrs + temp_attr1.at(p) +",";
					}
					sqlattrs = sqlattrs.substr(0,sqlattrs.length()-1);
					newattrs = sqlattrs;
					sql = sql + "(" + sqlattrs + ");";
					cout<<sql<<endl;
					cout<<"send it to db "<<k<<endl;
					RPCExecute(sites[k-1],select_ports[k-1],dbs[k-1],sql);
					sql = "insert into " +newtablename + " select ";
					for(int p = 0;p < temp_attr1.size();p++)
					{
						string tempst = temp_attr1.at(p);
						int index = tempst.find_first_of(" ",0);
						tempst = tempst.substr(0,index);
						std::size_t found = tempst.find("Course");
						if(found==std::string::npos)  {
							tempst = basetable+"."+tempst;
						}
						if(tempst == "Course_id")  tempst = basetable+".Course_id";
						if(tempst == "Course_teacher_id") tempst = basetable+".Course_id";
						//if(tempst == "Course_id")  tempst = basetable+".Course_id";
						sql = sql + tempst +",";
					}
					sql = sql.substr(0,sql.length()-1);
					sql = sql + " from " + basetable + "," + join_table.joinedtable[m] + " where "+ basetable + ".Course_id = " + join_table.joinedtable[m] + ".Course_id;";
					if(SQ.join_count>=2)
					{
						int fflag=0;
						for(int w=0;w<SQ.from_count;w++)
						{
							if(SQ.FromList[w].tb_name == "Student")
							{
								fflag = 1;
							}
						}
						if(fflag == 1) {
							sql = sql.substr(0,sql.length()-1);
							sql = sql+" and "+basetable + ".Student_id = " + join_table.joinedtable[m] + ".Student_id;";
						}
					}
					cout<<sql<<endl;
					//nnewtablename = basetable;
					RPCExecute(sites[k-1],select_ports[k-1],dbs[k-1],sql);
				}
			}
			if(frag_type1==0 && frag_type2==0 && flag == 1 && basetable != "")
			{
				
				string newtablename = get_new_join_table_name(basetable,k);
				sql = "alter table "+basetable+" rename to "+newtablename+";";
				cout<<sql<<endl;
				cout<<"send it to db "<<k<<endl;
				string droptable = "drop table if exists "+newtablename+";";
				RPCExecute(sites[k-1],select_ports[k-1],dbs[k-1],droptable);
				RPCExecute(sites[k-1],select_ports[k-1],dbs[k-1],sql);
				//写死了  ，就让4个分片的留下，剩下的去boradcast 
				int index = newtablename.find_first_of('_',0);
				//cout<<newtablename.substr(0,index)<<endl;
				join_table.newtable = new Select_Table;
				join_table.newtablename= newtablename.substr(0,index);

				
				//join_table.newtable->fraginfo = iterjoin->second.table_to_stay;
				//cout<<join_table.newtable->tb_name<<endl;
				
				//for(iterjointable = join_middle_table.begin();iterjointable != join_middle_table.end();iterjointable++)
				//	cout<<"aa"<<endl;
			}
			else if((frag_type1!=0 || frag_type2!=0) && flag == 1 && basetable != "")
			{
				string newtablename = get_new_join_table_name(basetable,k);
				//sql = "alter table "+basetable+" rename to "+newtablename+";";
				//cout<<sql<<endl;
				//cout<<"send it to db "<<k<<endl;
				//RPCExecute(sites[k-1],dbs[k-1],sql);
				//写死了  ，就让4个分片的留下，剩下的去boradcast 
				int index = newtablename.find_first_of('_',0);
				//cout<<newtablename.substr(0,index)<<endl;
				join_table.newtable = new Select_Table;
				join_table.newtablename= newtablename.substr(0,index);
			}
				
		}
		//for(int r = 0; r <tblist->tbl[staytableindex]->frag_count;r++)
		//{
		//	join_table.newtable->fraginfo[r] = tblist->tbl[staytableindex]->fraginfo[r]; //正常应该这样弄，就是更新这个join_table;但是我就全部join了。 
	//	}
		
		join_table.newtable = tblist->tbl[staytableindex];
		join_table.attrs = newattrs;
		join_middle_table.insert(pair<int,Join_Middle_Table>(iterjoin->first,join_table));
		
	}
	vector<string> count_size;
	vector<string> tmps;
	int count = 0;
	sql = "";
	//sql = sql + "select ";
	int size = 0;
	int tableindex;
	if(SQ.join_count == 0)
	//if(SQ.join_count == 0 && SQ.cond_count == 0)
	{
		string tablename = SQ.FromList[0].tb_name;
		for(int q = 0; q < tblist->table_num; q++)
		{
			if(SQ.FromList[0].tb_name == tblist->tbl[q]->tb_name)
				tableindex = q;
		}
		if(SQ.from_count==1  && tblist->tbl[tableindex]->frag_type==0)
		//if(SQ.from_count==1 && SQ.cond_count==0 && tblist->tbl[tableindex]->frag_type==0)
		{
			
			for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
			{
				if(iterfrag->second.tbl->tb_name == tablename && tree->query_tree_node[iterfrag->first].parent != -1)
				{
					sql = "select ";
					for(iterproject = tree->tree_project.begin();iterproject!=tree->tree_project.end();iterproject++)
					{
						sql = sql + iterproject->second.project.col_name+",";
					}
					int length = sql.length();
					sql.replace(length-1,1," ");
					sql = sql + "from " + SQ.FromList[0].tb_name;
					
					string tablenametale = "_frag"+to_String(iterfrag->second.frag_id);
					sql = sql + tablenametale+";";
					cout<<sql<<endl;
					int db_id = iterfrag->second.frag_id;
					cout<<"send it to db "<<iterfrag->second.frag_id<<endl;
					
					//cout<<"sent it to db "<<iterfrag->second.frag_id<<endl;
					string str = RPCExecuteQuery(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],sql);
					//str.pop_back();
					vector<string> res = Split(str,"\n");
					count += res.size();
					int tempcount[4];
					cout << str;
					
				}
				
			}
			
			cout << "COUNT : " << count << endl;	
		}
		else if(SQ.from_count==1 && tblist->tbl[tableindex]->frag_type==1)
		//else if(SQ.from_count==1 && SQ.cond_count==0 && tblist->tbl[tableindex]->frag_type==1)
		{
			count = 0;
			int fragcount_needed = 0;
			for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
			{
				if(iterfrag->second.tbl->tb_name == tablename && tree->query_tree_node[iterfrag->first].parent != -1)
				{
					fragcount_needed ++;
				}
			}
			cout<<"the fragcount_needed "<<fragcount_needed<<endl;
			if(fragcount_needed == 1 )
			{
				for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
				{
					if(iterfrag->second.tbl->tb_name == tablename && tree->query_tree_node[iterfrag->first].parent != -1)
					{
						sql = "select ";
						for(iterproject = tree->tree_project.begin();iterproject!=tree->tree_project.end();iterproject++)
						{
							sql = sql + iterproject->second.project.col_name+",";
						}
						int length = sql.length();
						sql.replace(length-1,1," ");
						sql = sql + "from " + SQ.FromList[0].tb_name;
					
						string tablenametale = "_frag"+to_String(iterfrag->second.frag_id);
						sql = sql + tablenametale+";";
						cout<<sql<<endl;
						int db_id = iterfrag->second.frag_id;
						cout<<"send it to db "<<iterfrag->second.frag_id<<endl;
						
						string str = RPCExecuteQuery(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],sql);
						//str.pop_back();
						vector<string> res = Split(str,"\n");
						count += res.size();
						cout << str;
						
					}
				
				}
				cout << "COUNT : " << count << endl;
			}
			else if(fragcount_needed >1 && SQ.cond_count == 0)
			{
				for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
				{
					if(iterfrag->second.tbl->tb_name == tablename && tree->query_tree_node[iterfrag->first].parent != -1)
					{
						db_id = iterfrag->second.frag_id;
						sql = "drop table if exists "+tablename+"_frag"+to_String(iterfrag->second.frag_id)+";";
						cout<<sql<<endl;
						cout<<"send it to db "<<iterfrag->second.frag_id<<endl;
						RPCExecute(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],sql);
						sql = "create table "+tablename+"_frag"+to_String(iterfrag->second.frag_id)+" as select ";
						string attrneeded[MAX_ATTR_COUNT]={""};
						find_all_attr_needed(tblist,SQ,tablename,attrneeded);
						string str = print_selpro_create_table_vertical(SQ,tree,tblist,tablename,iterfrag->second.frag_id,attrneeded,true,true);
						//cout<<"str is"<<str<<endl;
						sql = sql + str;
						sql = sql + " from "+tablename+";";
						cout<<sql<<endl;
						cout<<"send it to db "<<iterfrag->second.frag_id<<endl;
						RPCExecute(sites[db_id-1],select_ports[db_id-1],dbs[db_id-1],sql);
					}
				
				}
				string droptable = "drop table if exists Course_frag1;";
				RPCExecute(sites[1],select_ports[1],dbs[1],droptable);
				sql = "create table Course_frag1 (";
				string attrneeded[MAX_ATTR_COUNT]={""};
				find_all_attr_needed(tblist,SQ,tablename,attrneeded);
				sql = sql + print_selpro_create_table_vertical(SQ,tree,tblist,tablename,1,attrneeded,true);
				sql = sql +");";
				cout<<sql<<endl;
				RPCExecute(sites[1],select_ports[1],dbs[1],sql);
				cout<<"insert into the table"<<endl;
				
				RPCInsertFromSelect(sites[0],select_ports[0],sites[1],select_ports[1],"Course_frag1","Course_frag1",dbs[0],dbs[1]);
				sql = "select ";
				for(iterproject = tree->tree_project.begin();iterproject!=tree->tree_project.end();iterproject++)
				{
					
					if(iterproject->second.project.col_name == "id")
						sql = sql +"Course_frag1.id,";
					else sql = sql + iterproject->second.project.col_name+",";
				}
				int length = sql.length();
				sql.replace(length-1,1," ");
				sql = sql+"from Course_frag1, Course_frag2 where Course_frag1.id=Course_frag2.id";
				cout<<sql<<endl;
				cout<<"send it to db1"<<endl;
				
				string str = RPCExecuteQuery(sites[1],select_ports[1],dbs[1],sql);
				vector<string> res = Split(str,"\n");
				count += res.size();
				cout << str;
				cout << "COUNT : " << count << endl;
			}
			else if(SQ.cond_count != 0)
			{
				//cout<<"into this part"<<endl;
				RPCExecute(sites[1],select_ports[1],dbs[1],"drop table if exists Course_frag1;");
				sql = "create table Course_frag1 (";
				string attrneeded[MAX_ATTR_COUNT]={""};
				find_all_attr_needed(tblist,SQ,tablename,attrneeded);
				sql = sql + print_selpro_create_table_vertical(SQ,tree,tblist,tablename,1,attrneeded,true);
				sql = sql +");";
				cout<<sql<<endl;
				RPCExecute(sites[1],select_ports[1],dbs[1],sql);
				cout<<"insert into the table"<<endl;
				
				RPCInsertFromSelect(sites[0],select_ports[0],sites[1],select_ports[1],"Course_frag1","Course_frag1",dbs[0],dbs[1]);
				sql = "select ";
				for(iterproject = tree->tree_project.begin();iterproject!=tree->tree_project.end();iterproject++)
				{
					
					if(iterproject->second.project.col_name == "id")
						sql = sql +"Course_frag1.id,";
					else sql = sql + iterproject->second.project.col_name+",";
				}
				int length = sql.length();
				sql.replace(length-1,1," ");
				sql = sql+"from Course_frag1, Course_frag2 where Course_frag1.id=Course_frag2.id";
				cout<<sql<<endl;
				cout<<"send it to db1"<<endl;
				
				string str = RPCExecuteQuery(sites[1],select_ports[1],dbs[1],sql);
			//	str.pop_back();
				vector<string> res = Split(str,"\n");
				count += res.size();
				cout << str;
				cout << "COUNT : " << count << endl;
			}
		}

	}
	//else if(SQ.join_count==0 && SQ.cond_count!=0)
	//{
	//	string tablename = SQ.FromList[0].tb_name;
		//for(int p = 0;p < )

	//}
	//map<int, Opera_Join>::iterator iterjoin;
	map<int, Join_Middle_Table>::iterator iterjoinmiddle;
	count = 0;
	if(SQ.join_count != 0)
	{
		for(iterjoin = tree->tree_join.begin();iterjoin!=tree->tree_join.end();iterjoin++)
		{
			int parent = tree->query_tree_node[iterjoin->first].parent;
			if(tree->query_tree_node[parent].type == 2)
			{
				iterjoinmiddle = join_middle_table.find(iterjoin->first);
				if(iterjoinmiddle == join_middle_table.end())  cout<<"ERROR:can not find the middle join table in sql generate"<<endl;
				for(int q = 0;q < MAX_DB_COUNT;q++)
				{
					if(iterjoinmiddle->second.db_list[q] == 1)
					{
						sql = "select distinct ";
						for(iterproject = tree->tree_project.begin();iterproject!=tree->tree_project.end();iterproject++)
						{
							sql = sql + iterproject->second.project.table_name + "_" + iterproject->second.project.col_name+",";
						}
						sql = sql.substr(0,sql.length()-1);
						sql = sql + " from "+iterjoinmiddle->second.newtablename+"_frag"+to_String(q)+";";
						cout<<sql<<endl;
						cout<<"send it to db"<<q<<endl;
						string str = RPCExecuteQuery(sites[q-1],select_ports[q-1],dbs[q-1],sql);
						//str.pop_back();
						vector<string> res = Split(str,"\n");
						count += res.size();
						cout << str;
						
					}
				}
				cout << "COUNT : " << count << endl;
			}
		}
	}
	
/*	for(iterproject = tree->tree_project.begin();iterproject!=tree->tree_project.end();iterproject++)
	{
		int childindex = tree->query_tree_node[iterproject->first].child[0];
		if(tree->query_tree_node[childindex].type == 3)
		{
			//string 
		}
		//sql = sql + iterproject->second.project.table_name +"."+ iterproject->second.project.col_name+",";
	} 
	int length = sql.length();
	sql.replace(length-1,1," ");
	sql = sql + "from " ;
	for(i = 0;i<SQ.from_count;i++)
	{
		sql = sql + SQ.FromList[i].tb_name +",";
	}
	length = sql.length();
	sql.replace(length-1,1,"");
	sql = sql + ";";

	if(SQ.from_count == 1)
	{
		string tablename = SQ.FromList[0].tb_name;
		for(i = 0;i < MAX_TABLE_COUNT;i++)
		{
			if(SQ.FromList->tb_name == tablename)
			{
				for(iterfrag = tree->tree_frag.begin();iterfrag!=tree->tree_frag.end();iterfrag++)
				{
					if(tree->query_tree_node[iterfrag->first].parent != -1)
					{
						cout<<sql<<endl;
						int sqlsize = 0;
						int db_id = iterfrag->second.frag_id;
						cout<<"sent it to db "<<iterfrag->second.frag_id<<endl;
						string createstring = "";
						//createstring = print_create_table(tblist,tablename);
						//string str = RPCExecuteQuery(sites[db_id-1],dbs[db_id-1],sql);
						//str.pop_back();
						//vector<string> res = Split(str,"\n");
						//count += res.size();
						//cout << str;
					}
				}
				break;
			}
		}
		cout << "COUNT : " << count << endl;
	}*/
}
//代码要求三表连接时，两边都有的表在中间，然后连接顺序他也要在中间 
void do_select(SelectQuery &SQ)
{

	Select_Table *Student = new Select_Table;
	Student->tb_name = "Student";
	Student->attr_count = 5;
	Student->attrs[0].attr_name = "id";
	Student->attrs[0].type = "integer";
	Student->attrs[0].is_key = 1;
	Student->attrs[0].size = 0;
	Student->attrs[1].attr_name = "name";
	Student->attrs[1].type = "char";
	Student->attrs[1].is_key = 0;
	Student->attrs[1].size = 25;
	Student->attrs[2].attr_name = "sex";
	Student->attrs[2].type = "char";
	Student->attrs[2].is_key = 0;
	Student->attrs[2].size = 1;
	Student->attrs[3].attr_name = "age";
	Student->attrs[3].type = "integer";
	Student->attrs[3].is_key = 0;
	Student->attrs[3].size = 0;
	Student->attrs[4].attr_name = "degree";
	Student->attrs[4].type = "integer";
	Student->attrs[4].is_key = 0;
	Student->attrs[4].size = 0;
	Student->is_need_frag = 1;
	Student->frag_count = 3;
	Student->frag_type = 0;
	Student->db_id_needed[0] = 1;
	Student->db_id_needed[1] = 1;
	Student->db_id_needed[2] = 1;
	Student->db_id_needed[3] = 0;
	Student->db_id_needed[4] = 0;
	Student->fraginfo[0].mo_tb_name = "Student";
	Student->fraginfo[0].it_tb_name = "Student_1";
	Student->fraginfo[0].type = 0;
	Student->fraginfo[0].db_id = 1;
	Student->fraginfo[0].frag_id = 1;
	Student->fraginfo[0].condition.condition_h_column = 2;
	Student->fraginfo[0].condition.h1.is_needed=1;
	Student->fraginfo[0].condition.v1.is_needed = 0;
	Student->fraginfo[0].condition.h1.attr_name[0] = "id";
	Student->fraginfo[0].condition.h1.operation[0] = L;
	Student->fraginfo[0].condition.h1.attr_value[0] = "1050000";
	Student->fraginfo[0].condition.h1.attr_name[1] = "id";
	Student->fraginfo[0].condition.h1.operation[1] = L;
	Student->fraginfo[0].condition.h1.attr_value[1] = "1050000";

	Student->fraginfo[1].mo_tb_name = "Student";
	Student->fraginfo[1].it_tb_name = "Student_2";
	Student->fraginfo[1].type = 0;
	Student->fraginfo[1].db_id = 2;
	Student->fraginfo[1].frag_id = 2;
	Student->fraginfo[1].condition.condition_h_column = 2;
	Student->fraginfo[1].condition.h1.is_needed=1;
	Student->fraginfo[1].condition.v1.is_needed = 0;
	Student->fraginfo[1].condition.h1.attr_name[0] = "id";
	Student->fraginfo[1].condition.h1.operation[0] = GE;
	Student->fraginfo[1].condition.h1.attr_value[0] = "1050000";
	Student->fraginfo[1].condition.h1.attr_name[1] = "id";
	Student->fraginfo[1].condition.h1.operation[1] = L;
	Student->fraginfo[1].condition.h1.attr_value[1] = "1100000";
	
	Student->fraginfo[2].mo_tb_name = "Student";
	Student->fraginfo[2].it_tb_name = "Student_3";
	Student->fraginfo[2].type = 0;
	Student->fraginfo[2].db_id = 3;
	Student->fraginfo[2].frag_id = 3;
	Student->fraginfo[2].condition.condition_h_column = 2;
	Student->fraginfo[2].condition.h1.is_needed=1;
	Student->fraginfo[2].condition.v1.is_needed = 0;
	Student->fraginfo[2].condition.h1.attr_name[0] = "id";
	Student->fraginfo[2].condition.h1.operation[0] = GE;
	Student->fraginfo[2].condition.h1.attr_value[0] = "1100000";
	Student->fraginfo[2].condition.h1.attr_name[1] = "id";
	Student->fraginfo[2].condition.h1.operation[1] = GE;
	Student->fraginfo[2].condition.h1.attr_value[1] = "1100000";


	Select_Table *Teacher = new Select_Table;
	Teacher->tb_name = "Teacher";
	Teacher->attr_count = 3;
	Teacher->attrs[0].attr_name = "id";
	Teacher->attrs[0].type = "integer";
	Teacher->attrs[0].is_key = 1;
	Teacher->attrs[0].size = 0;
	Teacher->attrs[1].attr_name = "name";
	Teacher->attrs[1].type = "char";
	Teacher->attrs[1].is_key = 0;
	Teacher->attrs[1].size = 25;
	Teacher->attrs[2].attr_name = "title";
	Teacher->attrs[2].type = "integer";
	Teacher->attrs[2].is_key = 0;
	Teacher->attrs[2].size = 0;
	Teacher->is_need_frag = 1;
	Teacher->frag_count = 4;
	Teacher->frag_type = 0;
	Teacher->db_id_needed[0] = 1;
	Teacher->db_id_needed[1] = 1;
	Teacher->db_id_needed[2] = 1;
	Teacher->db_id_needed[3] = 1;
	Teacher->db_id_needed[4] = 0;
	Teacher->fraginfo[0].mo_tb_name = "Teacher";
	Teacher->fraginfo[0].it_tb_name = "Teacher_1";
	Teacher->fraginfo[0].db_id = 1;
	Teacher->fraginfo[0].frag_id = 1;
	Teacher->fraginfo[0].type = 0;
	Teacher->fraginfo[0].condition.condition_h_column = 2;
	Teacher->fraginfo[0].condition.h1.is_needed=1;
	Teacher->fraginfo[0].condition.v1.is_needed = 0;
	Teacher->fraginfo[0].condition.h1.attr_name[0] = "id";
	Teacher->fraginfo[0].condition.h1.operation[0] = L;
	Teacher->fraginfo[0].condition.h1.attr_value[0] = "2010000";
	Teacher->fraginfo[0].condition.h1.attr_name[1] = "title";
	Teacher->fraginfo[0].condition.h1.operation[1] = NE;
	Teacher->fraginfo[0].condition.h1.attr_value[1] = "3";

	Teacher->fraginfo[1].mo_tb_name = "Teacher";
	Teacher->fraginfo[1].it_tb_name = "Teacher_2";
	Teacher->fraginfo[1].db_id = 2;
	Teacher->fraginfo[1].frag_id = 2;
	Teacher->fraginfo[1].type = 0;
	Teacher->fraginfo[1].condition.condition_h_column = 2;
	Teacher->fraginfo[1].condition.h1.is_needed=1;
	Teacher->fraginfo[1].condition.v1.is_needed = 0;
	Teacher->fraginfo[1].condition.h1.attr_name[0] = "id";
	Teacher->fraginfo[1].condition.h1.operation[0] = L;
	Teacher->fraginfo[1].condition.h1.attr_value[0] = "2010000";
	Teacher->fraginfo[1].condition.h1.attr_name[1] = "title";
	Teacher->fraginfo[1].condition.h1.operation[1] = E;
	Teacher->fraginfo[1].condition.h1.attr_value[1] = "3";

	Teacher->fraginfo[2].mo_tb_name = "Teacher";
	Teacher->fraginfo[2].it_tb_name = "Teacher_3";
	Teacher->fraginfo[2].db_id = 3;
	Teacher->fraginfo[2].frag_id = 3;
	Teacher->fraginfo[2].type = 0;
	Teacher->fraginfo[2].condition.condition_h_column = 2;
	Teacher->fraginfo[2].condition.h1.is_needed=1;
	Teacher->fraginfo[2].condition.v1.is_needed = 0;
	Teacher->fraginfo[2].condition.h1.attr_name[0] = "id";
	Teacher->fraginfo[2].condition.h1.operation[0] = GE;
	Teacher->fraginfo[2].condition.h1.attr_value[0] = "2010000";
	Teacher->fraginfo[2].condition.h1.attr_name[1] = "title";
	Teacher->fraginfo[2].condition.h1.operation[1] = NE;
	Teacher->fraginfo[2].condition.h1.attr_value[1] = "3";

	Teacher->fraginfo[3].mo_tb_name = "Teacher";
	Teacher->fraginfo[3].it_tb_name = "Teacher_3";
	Teacher->fraginfo[3].db_id = 4;
	Teacher->fraginfo[3].frag_id = 4;
	Teacher->fraginfo[3].type = 0;
	Teacher->fraginfo[3].condition.condition_h_column = 2;
	Teacher->fraginfo[3].condition.h1.is_needed=1;
	Teacher->fraginfo[3].condition.v1.is_needed = 0;
	Teacher->fraginfo[3].condition.h1.attr_name[0] = "id";
	Teacher->fraginfo[3].condition.h1.operation[0] = GE;
	Teacher->fraginfo[3].condition.h1.attr_value[0] = "2010000";
	Teacher->fraginfo[3].condition.h1.attr_name[1] = "title";
	Teacher->fraginfo[3].condition.h1.operation[1] = E;
	Teacher->fraginfo[3].condition.h1.attr_value[1] = "3";

	Select_Table *Course = new Select_Table;
	Course->tb_name = "Course";
	Course->attr_count = 5;
	Course->attrs[0].attr_name = "id";
	Course->attrs[0].type = "integer";
	Course->attrs[0].is_key = 1;
	Course->attrs[0].size = 0;
	Course->attrs[1].attr_name = "name";
	Course->attrs[1].type = "char";
	Course->attrs[1].is_key = 0;
	Course->attrs[1].size = 80;
	Course->attrs[2].attr_name = "location";
	Course->attrs[2].type = "char";
	Course->attrs[2].is_key = 0;
	Course->attrs[2].size = 6;
	Course->attrs[3].attr_name = "credit_hour";
	Course->attrs[3].type = "integer";
	Course->attrs[3].is_key = 0;
	Course->attrs[3].size = 0;
	Course->attrs[4].attr_name = "teacher_id";
	Course->attrs[4].type = "integer";
	Course->attrs[4].is_key = 0;
	Course->attrs[4].size = 0;
	
	Course->is_need_frag = 1;
	Course->frag_count = 2;
	Course->frag_type = 1;
	Course->db_id_needed[0] = 1;
	Course->db_id_needed[1] = 1;
	Course->db_id_needed[2] = 0;
	Course->db_id_needed[3] = 0;
	Course->db_id_needed[4] = 0;
	Course->fraginfo[0].mo_tb_name = "Course";
	Course->fraginfo[0].it_tb_name = "Course_1";
	Course->fraginfo[0].type = 1;
	Course->fraginfo[0].db_id = 1;
	Course->fraginfo[0].frag_id = 1;
	Course->fraginfo[0].condition.h1.is_needed=0;
	Course->fraginfo[0].condition.v1.is_needed = 1;
	Course->fraginfo[0].condition.v1.attr_num = 2;
	Course->fraginfo[0].condition.v1.attr_list[0] = "id";
	Course->fraginfo[0].condition.v1.attr_list[1] = "name";
	Course->fraginfo[0].condition.v1.vKey = "id";
	
	Course->fraginfo[1].mo_tb_name = "Course";
	Course->fraginfo[1].it_tb_name = "Course_2";
	Course->fraginfo[1].type = 1;
	Course->fraginfo[1].db_id = 2;
	Course->fraginfo[1].frag_id = 2;
	Course->fraginfo[1].condition.h1.is_needed=0;
	Course->fraginfo[1].condition.v1.is_needed = 1;
	Course->fraginfo[1].condition.v1.attr_num = 4;
	Course->fraginfo[1].condition.v1.attr_list[0] = "id";
	Course->fraginfo[1].condition.v1.attr_list[1] = "location";
	Course->fraginfo[1].condition.v1.attr_list[2] = "credit_hour";
	Course->fraginfo[1].condition.v1.attr_list[3] = "teacher_id";
	Course->fraginfo[1].condition.v1.vKey = "id";


	Select_Table *Exam = new Select_Table;
	Exam->tb_name = "Exam";
	Exam->attr_count = 3;
	Exam->attrs[0].attr_name = "student_id";
	Exam->attrs[0].type = "integer";
	Exam->attrs[0].is_key = 1;
	Exam->attrs[0].size = 0;
	Exam->attrs[1].attr_name = "course_id";
	Exam->attrs[1].type = "integer";
	Exam->attrs[1].is_key = 1;
	Exam->attrs[1].size = 0;
	Exam->attrs[2].attr_name = "mark";
	Exam->attrs[2].type = "integer";
	Exam->attrs[2].is_key = 0;
	Exam->attrs[2].size = 0;
	Exam->is_need_frag = 1;
	Exam->frag_count = 4;
	Exam->frag_type = 0;
	Exam->db_id_needed[0] = 1;
	Exam->db_id_needed[1] = 1;
	Exam->db_id_needed[2] = 1;
	Exam->db_id_needed[3] = 1;
	Exam->db_id_needed[4] = 0;
	
	Exam->fraginfo[0].mo_tb_name = "Exam";
	Exam->fraginfo[0].it_tb_name = "Exam_1";
	Exam->fraginfo[0].db_id = 1;
	Exam->fraginfo[0].frag_id = 1;
	Exam->fraginfo[0].type = 0;
	Exam->fraginfo[0].condition.condition_h_column = 2;
	Exam->fraginfo[0].condition.h1.is_needed=1;
	Exam->fraginfo[0].condition.v1.is_needed = 0;
	Exam->fraginfo[0].condition.h1.attr_name[0] = "student_id";
	Exam->fraginfo[0].condition.h1.operation[0] = L;
	Exam->fraginfo[0].condition.h1.attr_value[0] = "1070000";
	Exam->fraginfo[0].condition.h1.attr_name[1] = "course_id";
	Exam->fraginfo[0].condition.h1.operation[1] = L;
	Exam->fraginfo[0].condition.h1.attr_value[1] = "301200";

	Exam->fraginfo[1].mo_tb_name = "Exam";
	Exam->fraginfo[1].it_tb_name = "Exam_2";
	Exam->fraginfo[1].db_id = 2;
	Exam->fraginfo[1].frag_id = 2;
	Exam->fraginfo[1].type = 0;
	Exam->fraginfo[1].condition.condition_h_column = 2;
	Exam->fraginfo[1].condition.h1.is_needed=1;
	Exam->fraginfo[1].condition.v1.is_needed = 0;
	Exam->fraginfo[1].condition.h1.attr_name[0] = "student_id";
	Exam->fraginfo[1].condition.h1.operation[0] = L;
	Exam->fraginfo[1].condition.h1.attr_value[0] = "1070000";
	Exam->fraginfo[1].condition.h1.attr_name[1] = "course_id";
	Exam->fraginfo[1].condition.h1.operation[1] = GE;
	Exam->fraginfo[1].condition.h1.attr_value[1] = "301200";

	Exam->fraginfo[2].mo_tb_name = "Exam";
	Exam->fraginfo[2].it_tb_name = "Exam_3";
	Exam->fraginfo[2].db_id = 3;
	Exam->fraginfo[2].frag_id = 3;
	Exam->fraginfo[2].type = 0;
	Exam->fraginfo[2].condition.condition_h_column = 2;
	Exam->fraginfo[2].condition.h1.is_needed=1;
	Exam->fraginfo[2].condition.v1.is_needed = 0;
	Exam->fraginfo[2].condition.h1.attr_name[0] = "student_id";
	Exam->fraginfo[2].condition.h1.operation[0] = GE;
	Exam->fraginfo[2].condition.h1.attr_value[0] = "1070000";
	Exam->fraginfo[2].condition.h1.attr_name[1] = "course_id";
	Exam->fraginfo[2].condition.h1.operation[1] = L;
	Exam->fraginfo[2].condition.h1.attr_value[1] = "301200";
	
	Exam->fraginfo[3].mo_tb_name = "Exam";
	Exam->fraginfo[3].it_tb_name = "Exam_4";
	Exam->fraginfo[3].db_id = 4;
	Exam->fraginfo[3].frag_id = 4;
	Exam->fraginfo[3].type = 0;
	Exam->fraginfo[3].condition.condition_h_column = 2;
	Exam->fraginfo[3].condition.h1.is_needed=1;
	Exam->fraginfo[3].condition.v1.is_needed = 0;
	Exam->fraginfo[3].condition.h1.attr_name[0] = "student_id";
	Exam->fraginfo[3].condition.h1.operation[0] = GE;
	Exam->fraginfo[3].condition.h1.attr_value[0] = "1070000";
	Exam->fraginfo[3].condition.h1.attr_name[1] = "course_id";
	Exam->fraginfo[3].condition.h1.operation[1] = GE;
	Exam->fraginfo[3].condition.h1.attr_value[1] = "301200";
	
	TableList * tblist = new TableList;
	tblist->table_num = 4;
	tblist->tbl[0] = Student;
	tblist->tbl[1] = Teacher;
	tblist->tbl[2] = Course;
	tblist->tbl[3] = Exam;
	
	//also should change SQ's column !!!
	
/*	tblist->tbl[0]->attrs[0].attr_name = "student_id";
	tblist->tbl[0]->fraginfo[0].condition.h1.attr_name[0] = "student_id";
	tblist->tbl[0]->fraginfo[0].condition.h1.attr_name[1] = "student_id";
	tblist->tbl[0]->fraginfo[1].condition.h1.attr_name[0] = "student_id";
	tblist->tbl[0]->fraginfo[1].condition.h1.attr_name[1] = "student_id";
	tblist->tbl[0]->fraginfo[2].condition.h1.attr_name[0] = "student_id";
	tblist->tbl[0]->fraginfo[2].condition.h1.attr_name[1] = "student_id";
	
	tblist->tbl[1]->attrs[0].attr_name = "teacher_id";
	tblist->tbl[1]->fraginfo[0].condition.h1.attr_name[0] = "teacher_id";
	tblist->tbl[1]->fraginfo[1].condition.h1.attr_name[0] = "teacher_id";
	tblist->tbl[1]->fraginfo[2].condition.h1.attr_name[0] = "teacher_id";
	tblist->tbl[1]->fraginfo[3].condition.h1.attr_name[0] = "teacher_id";
	
	tblist->tbl[2]->attrs[0].attr_name = "course_id";
	tblist->tbl[2]->fraginfo[0].condition.v1.attr_list[0] = "course_id";
	tblist->tbl[2]->fraginfo[1].condition.v1.attr_list[0] = "course_id";*/
	
/* //2.1 select * from Student	
	SelectQuery SQ;
	SQ.sel_count = 5;
	SQ.cond_count = 0;
	SQ.from_count = 1;
	SQ.join_count= 0;


	SQ.FromList[0].tb_name = "Student";

	SQ.SelList[0].table_name = "Student";
	SQ.SelList[0].col_name = "id";
	SQ.SelList[1].table_name = "Student";
	SQ.SelList[1].col_name = "name";
	SQ.SelList[2].table_name = "Student";
	SQ.SelList[2].col_name = "sex";
	SQ.SelList[3].table_name = "Student";
	SQ.SelList[3].col_name = "age";
	SQ.SelList[4].table_name = "Student";
	SQ.SelList[4].col_name = "degree";*/
	
/*	//2.2 select Course.name from Course
	SelectQuery SQ;
	SQ.sel_count = 1;
	SQ.cond_count = 0;
	SQ.from_count = 1;
	SQ.join_count= 0;
	SQ.FromList[0].tb_name = "Course";
	SQ.SelList[0].table_name = "Course";
	SQ.SelList[0].col_name = "name";
	
	//SQ.CondList[0].op = G;
	//SQ.CondList[0].tb_name = "Course";
	//SQ.CondList[0].col_name = "location";
	//SQ.CondList[0].value = "2";*/
	
/*	//2.3 select * from Course where credit_hour>2 and location = 'CB_3';
	SelectQuery SQ;
	SQ.sel_count = 5;
	SQ.cond_count = 2;
	SQ.from_count = 1;
	SQ.join_count= 0; 
	SQ.SelList[0].table_name = "Course";
	SQ.SelList[0].col_name = "id";
	SQ.SelList[1].table_name = "Course";
	SQ.SelList[1].col_name = "name";
	SQ.SelList[2].table_name = "Course";
	SQ.SelList[2].col_name = "location";
	SQ.SelList[3].table_name = "Course";
	SQ.SelList[3].col_name = "credit_hour";
	SQ.SelList[4].table_name = "Course";
	SQ.SelList[4].col_name = "teacher_id";
	
	SQ.CondList[0].op = G;
	SQ.CondList[0].tb_name = "Course";
	SQ.CondList[0].col_name = "credit_hour";
	SQ.CondList[0].value = "2";
	
	SQ.CondList[1].op = E;
	SQ.CondList[1].tb_name = "Course";
	SQ.CondList[1].col_name = "location";
	SQ.CondList[1].value = "CB-3";
	
	SQ.FromList[0].tb_name = "Course";*/
	
/*	//2.4 select course_id,mark from Exam
	SelectQuery SQ;
	SQ.sel_count = 2;
	SQ.cond_count = 0;
	SQ.from_count = 1;
	SQ.join_count= 0; 	
	SQ.SelList[0].table_name = "Exam";
	SQ.SelList[0].col_name = "course_id";
	SQ.SelList[1].table_name = "Exam";
	SQ.SelList[1].col_name = "mark";
	SQ.FromList[0].tb_name = "Exam";*/
	
/*	//2.5 select Course.name,Course.credit_hour,Teacher.name from Course, Teacher where
	//Course.teacher_id = Teacher.id and Course.credit_hour > 2 and Teacher.title = 3
	SelectQuery SQ;
	SQ.sel_count = 3;
	SQ.cond_count = 2;
	SQ.from_count = 2;
	SQ.join_count= 1; 
	SQ.SelList[0].table_name = "Course";
	SQ.SelList[0].col_name = "name";
	SQ.SelList[1].table_name = "Course";
	SQ.SelList[1].col_name = "credit_hour";
	SQ.SelList[2].table_name = "Teacher";
	SQ.SelList[2].col_name = "name";
	SQ.FromList[0].tb_name = "Course";
	SQ.FromList[1].tb_name = "Teacher";
	SQ.CondList[0].op = G;
	SQ.CondList[0].tb_name = "Course";
	SQ.CondList[0].col_name = "credit_hour";
	SQ.CondList[0].value = "2";
	
	SQ.CondList[1].op = E;
	SQ.CondList[1].tb_name = "Teacher";
	SQ.CondList[1].col_name = "title";
	SQ.CondList[1].value = "3";
	
	SQ.JoinList[0].tb_name1 = "Course";
	SQ.JoinList[0].tb_name2 = "Teacher";
	SQ.JoinList[0].col_name1 = "teacher_id";
	SQ.JoinList[0].col_name2 = "id";
	SQ.JoinList[0].op = E;*/
	
/*	//2.6 select Student.name,Exam.mark from Studet,Exam where Student.id = Exam.student_id;
	SelectQuery SQ;
	SQ.sel_count = 2;
	SQ.cond_count = 1;
	SQ.from_count = 2;
	SQ.join_count= 1;
	SQ.SelList[0].table_name = "Student";
	SQ.SelList[0].col_name = "name";
	SQ.SelList[1].table_name = "Exam";
	SQ.SelList[1].col_name = "mark"; 
	SQ.FromList[0].tb_name = "Student";
	SQ.FromList[1].tb_name = "Exam";
	SQ.JoinList[0].tb_name1 = "Student";
	SQ.JoinList[0].tb_name2 = "Exam";
	SQ.JoinList[0].col_name1 = "id";
	SQ.JoinList[0].col_name2 = "student_id";
	SQ.JoinList[0].op = E;
	
	SQ.CondList[0].op = L;
	SQ.CondList[0].tb_name = "Exam";
	SQ.CondList[0].col_name = "mark";
	SQ.CondList[0].value = "60";*/
	
/*	//2.7select Student.id,Student.name,Exam.mark,Course.name from Student,Exam,Course
	//where Student.id = Exam.student_id and Exam.course_id = Course.id and Student.age>26 and Course.location<>'CB-3'
	
	SelectQuery SQ;
	SQ.sel_count = 4;
	SQ.cond_count = 2;
	SQ.from_count = 3;
	SQ.join_count= 2;
	SQ.SelList[0].table_name = "Student";
	SQ.SelList[0].col_name = "id";
	SQ.SelList[1].table_name = "Student";
	SQ.SelList[1].col_name = "name"; 
	SQ.SelList[2].table_name = "Exam";
	SQ.SelList[2].col_name = "mark";
	SQ.SelList[3].table_name = "Course";
	SQ.SelList[3].col_name = "name"; 		
	SQ.FromList[0].tb_name = "Student";
	SQ.FromList[1].tb_name = "Exam";
	SQ.FromList[2].tb_name = "Course";
	SQ.CondList[0].op = G;
	SQ.CondList[0].tb_name = "Student";
	SQ.CondList[0].col_name = "age";
	SQ.CondList[0].value = "26";
	
	SQ.CondList[1].op = NE;
	SQ.CondList[1].tb_name = "Course";
	SQ.CondList[1].col_name = "location";
	SQ.CondList[1].value = "CB-3";	
	SQ.JoinList[0].tb_name1 = "Student";
	SQ.JoinList[0].tb_name2 = "Exam";
	SQ.JoinList[0].col_name1 = "id";
	SQ.JoinList[0].col_name2 = "student_id";
	SQ.JoinList[0].op = E;
	SQ.JoinList[1].tb_name1 = "Exam";
	SQ.JoinList[1].tb_name2 = "Course";
	SQ.JoinList[1].col_name1 = "course_id";
	SQ.JoinList[1].col_name2 = "id";
	SQ.JoinList[1].op = E;*/
	
 /*  //2.8 tesr join order
	SelectQuery SQ;
	SQ.sel_count = 4;
	SQ.cond_count = 2;
	SQ.from_count = 3;
	SQ.join_count= 2;
	SQ.SelList[0].table_name = "Student";
	SQ.SelList[0].col_name = "id";
	SQ.SelList[1].table_name = "Student";
	SQ.SelList[1].col_name = "name"; 
	SQ.SelList[2].table_name = "Exam";
	SQ.SelList[2].col_name = "mark";
	SQ.SelList[3].table_name = "Course";
	SQ.SelList[3].col_name = "name"; 		
	SQ.FromList[0].tb_name = "Course";
	SQ.FromList[1].tb_name = "Exam";
	SQ.FromList[2].tb_name = "Student";
	SQ.CondList[0].op = G;
	SQ.CondList[0].tb_name = "Student";
	SQ.CondList[0].col_name = "age";
	SQ.CondList[0].value = "26";
	
	SQ.CondList[1].op = NE;
	SQ.CondList[1].tb_name = "Course";
	SQ.CondList[1].col_name = "location";
	SQ.CondList[1].value = "CB-3";	
	SQ.JoinList[0].tb_name1 = "Course";
	SQ.JoinList[0].tb_name2 = "Exam";
	SQ.JoinList[0].col_name1 = "id";
	SQ.JoinList[0].col_name2 = "course_id";
	SQ.JoinList[0].op = E;
	SQ.JoinList[1].tb_name1 = "Student";
	SQ.JoinList[1].tb_name2 = "Exam";
	SQ.JoinList[1].col_name1 = "id";
	SQ.JoinList[1].col_name2 = "student_id";
	SQ.JoinList[1].op = E;*/

		
	/*//2.9select Student.id,Student.name,Exam.mark,Course.name from Student,Exam,Course,Teacher
	//where Student.id = Exam.student_id and Exam.course_id = Course.id and Teacher.id = Course.teacher_id and Student.age>26 and Course.location<>'CB-3' and Teacher.title = 3;
	SelectQuery SQ;
	SQ.sel_count = 4;
	SQ.cond_count = 2;
	SQ.from_count = 4;
	SQ.join_count= 3;
	SQ.SelList[0].table_name = "Student";
	SQ.SelList[0].col_name = "id";
	SQ.SelList[1].table_name = "Student";
	SQ.SelList[1].col_name = "name"; 
	SQ.SelList[2].table_name = "Exam";
	SQ.SelList[2].col_name = "mark";
	SQ.SelList[3].table_name = "Course";
	SQ.SelList[3].col_name = "name"; 		
	SQ.FromList[0].tb_name = "Teacher";
	SQ.FromList[1].tb_name = "Course";
	SQ.FromList[2].tb_name = "Exam";
	SQ.FromList[3].tb_name = "Student";
	SQ.CondList[0].op = E;
	SQ.CondList[0].tb_name = "Teacher";
	SQ.CondList[0].col_name = "title";
	SQ.CondList[0].value = "3";
	
	SQ.CondList[1].op = G;
	SQ.CondList[1].tb_name = "Course";
	SQ.CondList[1].col_name = "credit_hour";
	SQ.CondList[1].value = "2";	

	//SQ.CondList[2].op = E;
//	SQ.CondList[2].tb_name = "Teacher";
//	SQ.CondList[2].col_name = "title";
//	SQ.CondList[2].value = "3";	
	
	SQ.JoinList[0].tb_name1 = "Student";
	SQ.JoinList[0].tb_name2 = "Exam";
	SQ.JoinList[0].col_name1 = "id";
	SQ.JoinList[0].col_name2 = "student_id";
	SQ.JoinList[0].op = E;
	SQ.JoinList[1].tb_name1 = "Exam";
	SQ.JoinList[1].tb_name2 = "Course";
	SQ.JoinList[1].col_name1 = "course_id";
	SQ.JoinList[1].col_name2 = "id";
	SQ.JoinList[1].op = E;
	
	SQ.JoinList[2].tb_name1 = "Teacher";
	SQ.JoinList[2].tb_name2 = "Course";
	SQ.JoinList[2].col_name1 = "id";
	SQ.JoinList[2].col_name2 = "teacher_id";
	SQ.JoinList[2].op = E;*/

/*	//2.10	
	SelectQuery SQ;
	SQ.sel_count = 5;
	SQ.cond_count = 5;
	SQ.from_count = 1;
	SQ.join_count= 0;
	SQ.SelList[0].table_name = "Course";
	SQ.SelList[0].col_name = "id";
	SQ.SelList[1].table_name = "Course";
	SQ.SelList[1].col_name = "name"; 
	SQ.SelList[2].table_name = "Course";
	SQ.SelList[2].col_name = "location";
	SQ.SelList[3].table_name = "Course";
	SQ.SelList[3].col_name = "credit_hour"; 
	SQ.SelList[4].table_name = "Course";
	SQ.SelList[4].col_name = "teacher_id";
	
	SQ.CondList[0].op = G;
	SQ.CondList[0].tb_name = "Course";
	SQ.CondList[0].col_name = "id";
	SQ.CondList[0].value = "3012000";
	
	SQ.CondList[1].op = NE;
	SQ.CondList[1].tb_name = "Course";
	SQ.CondList[1].col_name = "location";
	SQ.CondList[1].value = "CB-3";	

	SQ.CondList[2].op = E;
	SQ.CondList[2].tb_name = "Course";
	SQ.CondList[2].col_name = "name";
	SQ.CondList[2].value = "mark";	
	
	SQ.CondList[3].op = G;
	SQ.CondList[3].tb_name = "Course";
	SQ.CondList[3].col_name = "credit_hour";
	SQ.CondList[3].value = "3";
	
	SQ.CondList[4].op = GE;
	SQ.CondList[4].tb_name = "Course";
	SQ.CondList[4].col_name = "teacher_id";
	SQ.CondList[4].value = "2010000";	

	SQ.FromList[0].tb_name = "Course";*/
	
/*	//2.11 select * from Course;
	SelectQuery SQ;
	SQ.sel_count = 4;
	SQ.cond_count = 0;
	SQ.from_count = 1;
	SQ.join_count= 0;


	SQ.FromList[0].tb_name = "Course";

	SQ.SelList[0].table_name = "Course";
	SQ.SelList[0].col_name = "id";
	SQ.SelList[1].table_name = "Course";
	SQ.SelList[1].col_name = "name";
	SQ.SelList[2].table_name = "Course";
	SQ.SelList[2].col_name = "location";
	SQ.SelList[3].table_name = "Course";
	SQ.SelList[3].col_name = "credit_hour";*/
	
	/*Query_Plan_Tree* originate_tree ;
	originate_tree = create_ori_tree(SQ);*/
	
/*	//2.12
	SelectQuery SQ;
	SQ.sel_count = 4;
	SQ.cond_count = 2;
	SQ.from_count = 1;
	SQ.join_count= 0; 
	SQ.SelList[0].table_name = "Student";
	SQ.SelList[0].col_name = "id";
	SQ.SelList[1].table_name = "Student";
	SQ.SelList[1].col_name = "name";
	SQ.SelList[2].table_name = "Student";
	SQ.SelList[2].col_name = "sex";
	SQ.SelList[3].table_name = "Student";
	SQ.SelList[3].col_name = "age";
	
	SQ.CondList[0].op = G;
	SQ.CondList[0].tb_name = "Student";
	SQ.CondList[0].col_name = "age";
	SQ.CondList[0].value = "26";
	
	SQ.CondList[1].op = E;
	SQ.CondList[1].tb_name = "Student";
	SQ.CondList[1].col_name = "sex";
	SQ.CondList[1].value = "F";
	
	SQ.FromList[0].tb_name = "Student";*/
	
	
	Query_Plan_Tree originate_tree ;
//	originate_tree = create_ori_tree(SQ,originate_tree);
	create_ori_tree(SQ,originate_tree);
	Query_Plan_Tree* xx = &originate_tree;
	cout<<"the original tree"<<endl;
	cout<<endl;
	print_query_plan_tree(xx);
/*	map<int, Table_change*>::iterator itertabletest;
	for(itertabletest = xx->tree_tablename.begin();itertabletest!=xx->tree_tablename.end();itertabletest++)
		cout<<"test tablename map"<<endl<<itertabletest->first<<"    "<<getfirsttablename(itertabletest->second)<<endl;*/
	put_down_select(xx);
	
/*	string attr[MAX_ATTR_COUNT];
	find_all_attr_needed(SQ,"customer",attr);
	cout<<"find all the attr used"<<endl;
	for(int k = 0;k<MAX_ATTR_COUNT;k++)
		cout<<attr[k]<<"   ";
	cout<<endl;*/
	

	
	tree_to_fragment(SQ,xx,tblist);
	cut_useless_vertical_node(SQ,xx,tblist);
	cut_useless_select_node(xx,tblist);
	// 下面还要做一个join的过程 
	how_to_join(SQ,xx,tblist);
	
	generate_sql_list(SQ,xx,tblist);
	
	// return 0;
}
