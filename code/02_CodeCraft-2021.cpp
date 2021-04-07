/*
通过对服务器添加sequence字段，试图实现最佳适应算法，但是没有成功，因为用户的请求是core和memory两个维度，不适用于最佳适应算法，必须通过试探分配
来确保真正分配操作的可行性。

*/#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <map>
using namespace std;
int N;          //服务器类型数量,1-100
int M;          //虚拟机类型数量,1-1000
int T;          //T天的用户请求数据,1-1000
//可以采购的服务器类型，读入之后可能会进行排序，用id来进行服务器的标识
struct Server {
	int id;				//服务器的id号，用来标识服务器的类型
	int sequence;		//用来标识服务器的序列
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

//用来保存虚拟机的调度信息，键是虚拟机ID，值分别时虚拟机的类型ID，部署的服务器的sequence和location
map<int, tuple<int, int, int>>schedule;

//用来保存每台服务器sequence上部署的虚拟机的编号和位置信息,供迁移时使用
map<int, vector<pair<int, int>>> deployInfo;

//保存迁移信息,键是虚拟机ID，值是迁移的服务器sequence和位置
map<int, pair<int, int>> migrate;

//加载数据
void load() {
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
	}
	/*
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
	}*/
}
//对服务器进行排序
bool cmpServers(Server s1, Server s2) {
	if (s1.comsumption != s2.comsumption) {
		return s1.comsumption < s2.comsumption;
	}else if (s1.cost != s2.cost) {
		return s1.cost < s2.cost;
	}else if (s1.cores != s2.cores) {
		return s1.cores > s2.cores;
	}
	else {
		return s1.memories > s2.memories;
	}
}
//剩下空间小的排在前面，最佳适应算法
bool cmpAllServers(Server s1, Server s2) {
	int surplus1 = s1.a_cores + s1.b_cores;
	int surplus2 = s2.a_cores + s2.b_cores;
	return surplus1 < surplus2;
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
//根据服务器的sequence获得服务器的下标
int getIndex(const vector<Server>& currentServers, int sequence){
	for (int i = 0; i < currentServers.size(); i++) {
		if (currentServers[i].sequence == sequence) {
			return i;
		}
	}
	return -1;
}
//根据服务器的类型id获得服务器的下标
int getServerIndex(int typeID) {
	for (int i = 0; i < N; i++) {
		if (server[i].id == typeID) {
			return i;
		}
	}
	return -1;
}
//判断现有的服务器是否可以容纳请求
bool canHold(vector<Server>& servers, int cores, int memories, int isDouble, int& sequence, int& location) {
	for (int i = 0; i < servers.size(); i++) {
		if (isDouble == 1) {
			if (servers[i].a_cores >= cores / 2 && servers[i].b_cores >= cores / 2
				&& servers[i].a_memories >= memories / 2 && servers[i].b_memories >= memories / 2) {
				sequence = servers[i].sequence;
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
				sequence = servers[i].sequence;
				location = 0;
				servers[i].a_cores -= cores;
				servers[i].a_memories -= memories;
				return true;
			}
			else if (servers[i].b_cores >= cores && servers[i].b_memories >= memories) {
				sequence = servers[i].sequence;
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
			currentServers.back().sequence = currentServers.size() - 1;
			return server[k].id;
		}
		if (isDouble == 0 && server[k].a_cores >= cores && server[k].a_memories / 2 >= memories) {
			currentServers.push_back(server[k]);
			//更新容量
			currentServers.back().a_memories -= memories;
			currentServers.back().a_cores -= cores;
			currentServers.back().sequence = currentServers.size() - 1;
			return server[k].id;
		}
	}
	return -1;
}
void del(vector<Server> & servers, tuple<int, int, int> tuples) {
	int vmID = get<0>(tuples);
	int sequence = get<1>(tuples);
	int location = get<2>(tuples);

	int cores = vMachine[vmID].cores;
	int memories = vMachine[vmID].memories;
	int index = getIndex(servers, sequence);

	if (location == 2) {
		servers[index].a_cores += cores / 2;
		servers[index].b_cores += cores / 2;
		servers[index].a_memories += memories / 2;
		servers[index].b_memories += memories / 2;
	}
	else if (location == 0) {    //仅部署在a节点
		servers[index].a_cores += cores;
		servers[index].a_memories += memories;
	}
	else {  //仅部署在B节点
		servers[index].b_cores += cores;
		servers[index].b_memories += memories;
	}
}
void output(map<int,int> buyServers, int day) {
	//开始输出
	printf("(purchase, %d)\n", buyServers.size());
	map<int, int>::iterator iter = buyServers.begin();
	while (iter != buyServers.end()) {
		int index = getServerIndex(iter->first);
		int num = iter->second;
		printf("(%s, %d)\n", server[index].type, num);
		for (int i = 0; i < num; i++) {
			allservers.push_back(server[index]);
			allservers.back().sequence = allservers.size() - 1;
		}
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

			int sequence = -1, location = -1;
			//如果现有服务器可以容纳
			if (canHold(currentServers, cores, memories, isDouble, sequence, location)) {
				schedule[id] = tuple<int, int, int>{ 
					index,
					sequence,
					location
				};			
			}
			else {//否则购买新的服务器,1个请求最多只会购买一个服务器即可
				int typeID = buy(currentServers, cores, memories, isDouble);
				schedule[id] = tuple<int, int, int>{
					index,
					currentServers.back().sequence,
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
	sort(allservers.begin(), allservers.end(), cmpAllServers);
	for (int i = 0; i < requests[day].size(); i++) {
		if (i == 134) {
			printf("hello");
		}
		//获取所有的请求数据
		int reqType = requests[day][i].reqType;
		int id = requests[day][i].ID;
		//如果要添加虚拟机
		if (reqType == 0) {
			int index = getVMachine(requests[day][i].type);
			int cores = vMachine[index].cores;
			int memories = vMachine[index].memories;
			int isDouble = vMachine[index].isDouble;

			int sequence = -1, location = -1;
			//购买服务器之后可以容纳
			canHold(allservers, cores, memories, isDouble, sequence, location);
			
			schedule[id] = tuple<int, int, int>{ index, sequence,	location};

			deployInfo[sequence].push_back(pair<int, int>{id, location});

			if (location == 2) {
				printf("(%d)\n", sequence);
			}
			else {
				printf("(%d, %c)\n", sequence ,location == 0 ? 'A' : 'B');
			}
		}else {  //否则删除虚拟机
			del(allservers, schedule[id]);
			//删除部署信息
			int serverID = get<1>(schedule[id]);
			vector<pair<int, int>>::iterator iter = deployInfo[serverID].begin();
			while (iter != deployInfo[serverID].end()) {
				if (iter->first == id) {
					deployInfo[serverID].erase(iter);
					break;
				}
				iter++;
			}
			schedule.erase(id);
		}
	}
}
void compute() {
	//对服务器类型进行排序
	sort(server, server + N, cmpServers);
	for (int day = 0; day < T; day++) {
		sort(allservers.begin(), allservers.end(), cmpAllServers);
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