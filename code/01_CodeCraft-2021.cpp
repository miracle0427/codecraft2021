/*
	实现了基本的算法框架，主要是对服务器进行了排序，优先选择成本最低的服务器，当前的总成本为1245007930
*/

#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
using namespace std;
int N;          //服务器类型数量,1-100
int M;          //虚拟机类型数量,1-1000
int T;          //T天的用户请求数据,1-1000
//可以采购的服务器类型，读入之后可能会进行排序，用id来进行服务器的标识
struct Server {
	int id;				//服务器的id号
	char type[25];      //型号
	int cores;          //CPU核数，1-1024
	int memories;       //内存大小，1-1024
	int cost;           //硬件成本，1-500000
	int comsumption;    //每日能耗成本，1-5000

	int a_cores, b_cores;
	int a_memories, b_memories;  //记录a和b剩下的cores和memories
}server[110];
//虚拟机类型，读入之后不会再改变
struct VMachine {
	char type[25];      //虚拟机型号，长度不超过20字符
	int cores;          //CPU核数，1-1024
	int memories;       //内存大小，1-1024
	int isDouble;       //是否双结点，0表示单节点，1表示双节点
} vMachine[1010];
//用户请求，读入之后不会再改变
struct Request {
	int reqType;        //请求类型，0表示添加，1表示删除
	char type[25];      //虚拟机型号
	int ID;             //虚拟机ID
};
vector<Request> requests[1024];
//保存所有购买的服务器
vector<Server> allservers;

//用来保存虚拟机的调度信息，键是虚拟机ID，值分别时虚拟机的虚拟机的类型ID，部署的服务器的类型id，第几个，什么位置
//当进行试探部署的时候add添加，当进行真正部署的时候del进行删除
map<int, tuple<int, int, int, int>>schedule;

//用来保存每台服务器上部署的虚拟机的编号和位置信息,供迁移时使用
map<int, vector<pair<int, int>>> deployInfo;

//保存迁移信息,键是虚拟机ID，值是迁移的服务器ID和位置
map<int, pair<int, int>> migrate;

//加载数据
void load() {
	/*
	FILE * fp = fopen("C:\\Users\\79943\\OneDrive\\codecraft\\training-data\\training-1.txt", "r");
	fscanf(fp, "%d", &N);
	for (int i = 0; i < N; i++) {
		fgetc(fp);
		fscanf(fp, "(%[^,], %d, %d, %d, %d)", server[i].type, &server[i].cores, &server[i].memories, &server[i].cost, &server[i].comsumption);
		server[i].id = i;
		server[i].a_cores = server[i].b_cores = server[i].cores / 2;
		server[i].a_memories = server[i].b_memories = server[i].memories / 2;
	}
	fscanf(fp, "%d", &M);
	for (int i = 0; i < M; i++) {
		fgetc(fp);
		fscanf(fp, "(%[^,], %d, %d, %d)",
			vMachine[i].type, &vMachine[i].cores, &vMachine[i].memories, &vMachine[i].isDouble);
	}
	fscanf(fp, "%d", &T);
	for (int i = 0; i < T; i++) {
		fgetc(fp);
		struct Request req;
		int R = 0;
		fscanf(fp, "%d", &R);
		for (int j = 0; j < R; j++) {
			fgetc(fp);
			char temp[25];
			fscanf(fp, "(%[^,]", temp);
			fgetc(fp);
			if (strcmp("add", temp) == 0) {
				req.reqType = 0;
				fscanf(fp, " %[^,], %d)", req.type, &req.ID);
			}
			else {
				req.reqType = 1;
				fscanf(fp, "%d)", &req.ID);
			}
			requests[i].push_back(req);
		}
	}*/
	
	scanf("%d", &N);
	for (int i = 0; i < N; i++) {
		getchar();
		scanf( "(%[^,], %d, %d, %d, %d)", server[i].type, &server[i].cores, &server[i].memories, &server[i].cost, &server[i].comsumption);
		server[i].id = i;
		server[i].a_cores = server[i].b_cores = server[i].cores / 2;
		server[i].a_memories = server[i].b_memories = server[i].memories / 2;
	}
	scanf( "%d", &M);
	for (int i = 0; i < M; i++) {
		getchar();
		scanf( "(%[^,], %d, %d, %d)",
			vMachine[i].type, &vMachine[i].cores, &vMachine[i].memories, &vMachine[i].isDouble);
	}
	scanf( "%d", &T);
	for (int i = 0; i < T; i++) {
		getchar();
		struct Request req;
		int R = 0;
		scanf( "%d", &R);
		for (int j = 0; j < R; j++) {
			getchar();
			char temp[25];
			scanf( "(%[^,]", temp);
			getchar();
			if (strcmp("add", temp) == 0) {
				req.reqType = 0;
				scanf( " %[^,], %d)", req.type, &req.ID);
			}
			else {
				req.reqType = 1;
				scanf( "%d)", &req.ID);
			}
			requests[i].push_back(req);
		}
	}
}
//对服务器进行排序
bool cmp(Server s1, Server s2) {
	if (s1.comsumption != s2.comsumption) {
		return s1.comsumption < s2.comsumption;
	}else if (s1.cost != s2.cost) {
		return s1.cost < s2.cost;
	}
	else if (s1.cores != s2.cores) {
		return s1.cores > s2.cores;
	}
	else {
		return s1.memories > s2.memories;
	}
	/*
	//假设用户的平均请求CPU为2，用户请求的平均内存为4，服务器运行的平均天数为0.7T
	int cost1 = s1.cost + s1.comsumption * T - (s1.cores / 8) * s1.comsumption * T;
	int cost2 = s2.cost + s2.comsumption * T - (s2.cores / 8) * s2.comsumption * T;
	return cost1 < cost2;*/
}
//获取请求的虚拟机类型数据，返回下标，没找到返回-1
int getVMachine(char * type) {
	for (int i = 0; i < M; i++) {
		if (strcmp(vMachine[i].type, type) == 0) {
			return i;
		}
	}
	return -1;
}
//统计和当前serverid类型相同的服务器的数目，即serverid是第几个该种类型的服务器
int getNumSameType(const vector<Server>& currentServer, int serverid) {
	int sum = 0;
	for (int i = 0; i <= serverid; i++) {
		if (currentServer[i].id == currentServer[serverid].id) {
			sum++;
		}
	}
	return sum;
}
//根据服务器的类型id和数目获得服务器的下标
int getIndex(const vector<Server>& currentServers, int typeID, int num){
	int sum = 0;
	for (int i = 0; i < currentServers.size(); i++) {
		if (currentServers[i].id == typeID) {
			sum++;
			if (sum == num) {
				return i;
			}
		}
	}
	return -1;
}
int getServerIndex(int typeID) {
	for (int i = 0; i < N; i++) {
		if (server[i].id == typeID) {
			return i;
		}
	}
	return -1;
}
//判断现有的服务器是否可以容纳请求
bool canHold(vector<Server>& servers, int cores, int memories, int isDouble, int& serverid, int& location) {
	for (int i = 0; i < servers.size(); i++) {
		if (isDouble == 1) {
			if (servers[i].a_cores >= cores / 2 && servers[i].b_cores >= cores / 2
				&& servers[i].a_memories >= memories / 2 && servers[i].b_memories >= memories / 2) {
				serverid = i;
				location = 2;
				servers[i].a_cores -= cores / 2;
				servers[i].b_cores -= cores / 2;
				servers[i].a_memories -= memories / 2;
				servers[i].b_memories -= memories / 2;
				return true;
			}
		}
		else {
			if (servers[i].a_cores > servers[i].b_cores && servers[i].a_cores >= cores && servers[i].a_memories >= memories) {
				serverid = i;
				location = 0;
				servers[i].a_cores -= cores;
				servers[i].a_memories -= memories;
				return true;
			}
			else if (servers[i].b_cores >= cores && servers[i].b_memories >= memories) {
				serverid = i;
				location = 1;
				servers[i].b_cores -= cores;
				servers[i].b_memories -= memories;
				return true;
			}
		}
	}
	return false;
}
//根据请求购买服务器,返回需要购买的服务器的下标,-1表示购买失败
int buy(vector<Server>& currentServers, int cores, int memories, int isDouble) {
	for (int k = 0; k < N; k++) {
		//如果是双结点,且满足要求
		if (isDouble == 1 && server[k].cores >= cores && server[k].memories >= memories) {
			currentServers.push_back(server[k]);
			//更新容量
			currentServers.back().a_memories -= memories / 2;
			currentServers.back().b_memories -= memories / 2;
			currentServers.back().a_cores -= cores / 2;
			currentServers.back().b_cores -= cores / 2;
			return server[k].id;
		}
		if (isDouble == 0 && server[k].a_cores >= cores && server[k].a_memories / 2 >= memories) {
			currentServers.push_back(server[k]);
			//更新容量
			currentServers.back().a_memories -= memories;
			currentServers.back().a_cores -= cores;
			return server[k].id;
		}
	}
	return -1;
}
void del(vector<Server> & servers, tuple<int,int, int, int> tuples) {
	int vmID = get<0>(tuples);
	int typeID = get<1>(tuples);
	int num = get<2>(tuples);
	int location = get<3>(tuples);

	int cores = vMachine[vmID].cores;
	int memories = vMachine[vmID].memories;
	int serverID = getIndex(servers, typeID, num);

	if (location == 2) {
		servers[serverID].a_cores += cores / 2;
		servers[serverID].b_cores += cores / 2;
		servers[serverID].a_memories += memories / 2;
		servers[serverID].b_memories += memories / 2;
	}
	else if (location == 0) {    //仅部署在a节点
		servers[serverID].a_cores += cores;
		servers[serverID].a_memories += memories;
	}
	else {  //仅部署在B节点
		servers[serverID].b_cores += cores;
		servers[serverID].b_memories += memories;
	}
}
void output(map<int,int> buyServers, int day) {
	//开始输出
	printf("(purchase, %d)\n", buyServers.size());
	map<int, int>::iterator iter = buyServers.begin();
	while (iter != buyServers.end()) {
		printf("(%s, %d)\n", server[getServerIndex(iter->first)].type, iter->second);
		iter++;
	}
	printf("(migration, 0)\n");
}

//根据当前的服务器和已有的虚拟机的分配数据来进行试探应该购买那些服务器,返回购买服务器的map集合
map<int,int> parseRequest(vector<Server> currentServers,int day) {
	map<int, int> buyServers;
	for (int i = 0; i < requests[day].size(); i++) {
		//获取公共的请求信息
		int reqType = requests[day][i].reqType;
		int id = requests[day][i].ID;
		//如果要添加虚拟机
		if (reqType == 0) {
			int index = getVMachine(requests[day][i].type);
			int cores = vMachine[index].cores;
			int memories = vMachine[index].memories;
			int isDouble = vMachine[index].isDouble;

			int serverid = -1, location = -1;
			//如果现有服务器可以容纳
			if (canHold(currentServers, cores, memories, isDouble, serverid, location)) {
				schedule[id] = tuple<int,int, int, int>{ 
					index,
					currentServers[serverid].id,
					getNumSameType(currentServers,serverid),
					location
				};			
			}
			else {//否则购买新的服务器,1个请求最多只会购买一个服务器即可
				int typeID = buy(currentServers, cores, memories, isDouble);
				schedule[id] = tuple<int,int, int, int>{
					index,
					typeID, 
					getNumSameType(currentServers,currentServers.size()-1) , 
					isDouble==1 ? 2 : 0 
				};
				if (buyServers.find(typeID) != buyServers.end()) {
					buyServers[typeID]++;
				}
				else {
					buyServers[typeID] = 1;
				}
			}
		}
		else {  //否则删除虚拟机
			del(currentServers, schedule[id]);
		}
	}
	return buyServers;
}

//此时开始进行真正的分配操作
void postProcess(map<int,int> buyServers, int day) {
	map<int, int>::iterator iter = buyServers.begin();
	while (iter != buyServers.end()) {
		for (int i = 0; i < iter->second; i++) {
			int index = getServerIndex(iter->first);
			allservers.push_back(server[index]);
		}
		iter++;
	}
	for (int i = 0; i < requests[day].size(); i++) {
		//获取所有的请求数据
		int reqType = requests[day][i].reqType;
		int id = requests[day][i].ID;
		//获取调度信息
		tuple<int, int, int, int> result = schedule.at(id);
		int vmID = get<0>(result);
		int type = get<1>(result);
		int num = get<2>(result);
		int location = get<3>(result);

		//根据调度结果进行分配
		int serverid = getIndex(allservers, type, num);
		int cores = vMachine[vmID].cores;
		int memories = vMachine[vmID].memories;

		//添加部署信息
		deployInfo[serverid].push_back(pair<int, int>{id, location});

		//如果要添加虚拟机
		if (reqType == 0) {

			switch (location) {
			case 0: {
				allservers[serverid].a_cores -= cores;
				allservers[serverid].a_memories -= memories;
				break; 
			}
			case 1: {
				allservers[serverid].b_cores -= cores;
				allservers[serverid].b_memories -= memories;
				break; 
			}
			case 2: {
				allservers[serverid].a_cores -= cores/2;
				allservers[serverid].a_memories -= memories/2;
				allservers[serverid].b_cores -= cores/2;
				allservers[serverid].b_memories -= memories/2;
				break; }
			}
			if (allservers[serverid].a_cores < 0 || allservers[serverid].a_memories < 0 ||
				allservers[serverid].b_cores < 0 || allservers[serverid].b_memories < 0) {
				printf("hello world");
			}
			if (location == 2) {
				printf("(%d)\n", serverid);
			}
			else {
				printf("(%d, %c)\n", serverid, location == 0 ? 'A' : 'B');
			}
		}
		else {  //否则删除虚拟机
			//获取虚拟机所在的服务器
			del(allservers,schedule.at(id));
			//从调度信息中删除
			schedule.erase(id);
			//删除部署信息
			vector<pair<int, int>>::iterator iter = deployInfo[serverid].begin();
			while (iter != deployInfo[serverid].end()) {
				if (iter->first == id) {
					deployInfo[serverid].erase(iter);
					break;
				}
				iter++;
			}
		}
	}
}
void compute() {
	sort(server, server + N, cmp);
	int temp = T;
	for (int day = 0; day < temp; day++) {
		//对服务器类型进行排序
		//为了满足当天的请求应该购买的服务器的类型和数量
		//对临时的服务器和临时的虚拟机信息进行预操作
		map<int, int> buyServers = parseRequest(allservers,day);
		output(buyServers, day);
		postProcess(buyServers, day);
	}
}
int main() {
	load();
	compute();
	return 0;
}