#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <map>
using namespace std;
int N;          //服务器类型数量,1-100
int M;          //虚拟机类型数量,1-1000
int T;          //T天的用户请求数据,1-1000
//可以采购的服务器类型，读入之后可能会进行排序
struct Server {
	int type;			//服务器的类型
	int num;			//这是第几个该种类型的服务器
	int sequence;		//正式进行购买服务器时所对服务器进行的编号，与购买顺序密切相关
	char name[25];      //型号
	int cores;          //CPU核数，1-1024
	int memories;       //内存大小，1-1024
	int cost;           //硬件成本，1-500000
	int comsumption;    //每日能耗成本，1-5000

	int a_cores, b_cores;
	int a_memories, b_memories;  //记录a和b剩下的cores和memories
	double costPerformance;		//性价比
	double coreMemRation;		//核内比
};
//虚拟机类型，读入之后不会再改变
struct VMachine {
	char type[25];      //虚拟机型号，长度不超过20字符
	int cores;          //CPU核数，1-1024
	int memories;       //内存大小，1-1024
	int isDouble;       //是否双结点，0表示单节点，1表示双节点
};
//用户请求，读入之后可能会对用户的请求进行排序
struct Request {
	int sequence;		//表示用户的请求顺序
	int reqType;        //请求类型，0表示添加，1表示删除
	char type[25];		//虚拟机型号
	int ID;             //虚拟机ID
	int cores;			//请求的cores
	int memories;		//请求的memories
	int isDouble;		//是否双节点部署，0表示单节点，1表示双结点
};
vector<Server> servers;
vector<Server> server1, server2;	//其中server1的核内比大于1，server2的核内比小于1
vector<VMachine> vMachines;
vector<Request> requests[1024];
//保存所有购买的服务器
vector<Server> allservers;

//用来保存虚拟机的调度信息，键是虚拟机ID，值分别时虚拟机类型下标、部署的服务器的类型id，第几个，什么位置
map<int, tuple<int,int, int, int>>schedule;

//用来保存每台服务器上部署的虚拟机的编号和位置信息,供迁移时使用
//键是服务器的sequence字段，值是虚拟机的id号和location信息
map<int, vector<pair<int, int>>> deployInfo;
//获取请求的虚拟机类型数据，返回下标，没找到返回-1
int getVMachine(const char * type) {
	for (int i = 0; i < M; i++) {
		if (strcmp(vMachines[i].type, type) == 0) {
			return i;
		}
	}
	return -1;
}

//加载数据
void loadFile() {
	FILE * fp = fopen("C:\\Users\\79943\\OneDrive\\codecraft\\training-data\\training-1.txt", "r");
	fscanf(fp, "%d", &N);
	for (int i = 0; i < N; i++) {
		fgetc(fp);
		Server server;
		fscanf(fp, "(%[^,], %d, %d, %d, %d)", server.name, &server.cores, &server.memories, &server.cost, &server.comsumption);
		server.type = i;
		server.costPerformance = 1.0 * server.cost / (server.cores + server.memories);
		server.coreMemRation = 1.0 * server.cores / server.memories;
		server.a_cores = server.b_cores = server.cores / 2;
		server.a_memories = server.b_memories = server.memories / 2;
		servers.push_back(server);
	}
	fscanf(fp, "%d", &M);
	for (int i = 0; i < M; i++) {
		fgetc(fp);
		VMachine vMachine;
		fscanf(fp, "(%[^,], %d, %d, %d)", vMachine.type, &vMachine.cores, &vMachine.memories, &vMachine.isDouble);
		vMachines.push_back(vMachine);
	}
	fscanf(fp, "%d", &T);
	for (int i = 0; i < T; i++) {
		fgetc(fp);
		struct Request req;
		int R = 0;
		fscanf(fp, "%d", &R);
		for (int j = 0; j < R; j++) {
			req.sequence = j;
			fgetc(fp);
			char temp[25];
			fscanf(fp, "(%[^,]", temp);
			fgetc(fp);
			if (strcmp("add", temp) == 0) {
				req.reqType = 0;
				fscanf(fp, " %[^,], %d)", req.type, &req.ID);
				int index = getVMachine(req.type);
				req.cores = vMachines[index].cores;
				req.memories = vMachines[index].memories;
				req.isDouble = vMachines[index].isDouble;
			}
			else {
				req.reqType = 1;
				fscanf(fp, "%d)", &req.ID);
			}
			requests[i].push_back(req);
		}
	}
	fclose(fp);
}
void load() {
	scanf("%d", &N);
	for (int i = 0; i < N; i++) {
		getchar();
		Server server;
		scanf("(%[^,], %d, %d, %d, %d)", server.name, &server.cores, &server.memories, &server.cost, &server.comsumption);
		server.type = i;
		server.costPerformance = 1.0 * server.cost / (server.cores + server.memories);
		server.coreMemRation = 1.0 * server.cores / server.memories;
		server.a_cores = server.b_cores = server.cores / 2;
		server.a_memories = server.b_memories = server.memories / 2;
		servers.push_back(server);
	}
	scanf("%d", &M);
	for (int i = 0; i < M; i++) {
		getchar();
		VMachine vMachine;
		scanf("(%[^,], %d, %d, %d)", vMachine.type, &vMachine.cores, &vMachine.memories, &vMachine.isDouble);
		vMachines.push_back(vMachine);
	}
	scanf("%d", &T);
	for (int i = 0; i < T; i++) {
		getchar();
		struct Request req;
		int R = 0;
		scanf("%d", &R);
		for (int j = 0; j < R; j++) {
			req.sequence = j;
			getchar();
			char temp[25];
			scanf("(%[^,]", temp);
			getchar();
			if (strcmp("add", temp) == 0) {
				req.reqType = 0;
				scanf(" %[^,], %d)", req.type, &req.ID);
				int index = getVMachine(req.type);
				req.cores = vMachines[index].cores;
				req.memories = vMachines[index].memories;
				req.isDouble = vMachines[index].isDouble;
			}
			else {
				req.reqType = 1;
				scanf("%d)", &req.ID);
			}
			requests[i].push_back(req);
		}
	}
}

//统计currentServer中type类型的数量
int getNumSameType(const vector<Server>& currentServer, int type) {
	int sum = 0;
	for (int i = 0; i < currentServer.size(); i++) {
		if (currentServer[i].type == type) {
			sum++;
		}
	}
	return sum;
}
//根据服务器的类型id和数目获得服务器的下标
int getIndex(const vector<Server>& currentServers, int typeID, int num) {
	for (int i = 0; i < currentServers.size(); i++) {
		if (currentServers[i].type == typeID && currentServers[i].num == num) {
			return i;
		}
	}
	return -1;
}
//根据服务器的type获得在server中的下标
int getServerIndex(int type) {
	for (int i = 0; i < N; i++) {
		if (servers[i].type == type) {
			return i;
		}
	}
	return -1;
}

//判断此服务器是否可以容纳请求
bool canHold(vector<Server>& servers, const Request& req, int& serverid, int& location) {
	int cores = req.cores;
	int memories = req.memories;
	int isDouble = req.isDouble;
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
			if (servers[i].a_cores > servers[i].b_cores) {
				if (servers[i].a_cores >= cores && servers[i].a_memories >= memories) {
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
			else {
				if (servers[i].b_cores >= cores && servers[i].b_memories >= memories) {
					serverid = i;
					location = 1;
					servers[i].b_cores -= cores;
					servers[i].b_memories -= memories;
					return true;
				}
				else if (servers[i].a_cores >= cores && servers[i].a_memories >= memories) {
					serverid = i;
					location = 0;
					servers[i].a_cores -= cores;
					servers[i].a_memories -= memories;
					return true;
				}
			}
		}
	}
	return false;
}
//双结点部署的canHold
void canHold(vector<Request> req, vector<Server>&server,int& index,vector<int>& res) {
	int minDiff = -1;			//记录差值
	for (int i = 0; i < server.size(); i++) {
		vector<int>temp;
		int cores = 0;
		int memories = 0;
		int j = 0;
		for (; j < req.size(); j++) {
			cores += req[j].cores;
			memories += req[j].memories;
			if (cores <= min(server[i].a_cores,server[i].b_cores) 
				&& memories <= min(server[i].a_memories,server[i].b_memories)) {
				temp.push_back(j);
			}
			else {
				cores -= req[j].cores;
				memories -= req[j].memories;
				break;
			}
		}
		if (cores + memories) {
			int diff =  cores + memories;
			if (diff > minDiff) {
				minDiff = diff;
				index = i;
				res = temp;
			}
		}
	}
	for (int i = 0; i < res.size(); i++) {
		server[index].a_cores -= req[res[i]].cores / 2;
		server[index].b_cores -= req[res[i]].cores / 2;
		server[index].a_memories -= req[res[i]].memories / 2;
		server[index].b_memories -= req[res[i]].memories / 2;
	}
}

//单节点部署的canHold
void canHoldSingle(vector<Request> req, vector<Server>&server,int& index,vector<pair<int,int> >&res) {
	int minDiff = -1;		//记录差值

	for (int i = 0; i < server.size(); i++) {
		vector<pair<int, int>> temp;
		int a_cores = 0, b_cores = 0;
		int a_memories = 0, b_memories = 0;
		for (int j = 0; j < req.size(); j++) {
			if (a_cores < b_cores) {
				if (a_cores + req[j].cores <= server[i].a_cores && a_memories + req[j].memories <= server[i].a_memories) {
					a_cores += req[j].cores; a_memories += req[j].memories;
					temp.push_back(pair<int, int>{j, 0});
				}
				else if (b_cores + req[j].cores <= server[i].b_cores && b_memories + req[j].memories <= server[i].b_memories) {
					b_cores += req[j].cores; b_memories += req[j].memories;
					temp.push_back(pair<int, int>{j, 1});
				}
				else {
					break;
				}
			}
			else {
				if (b_cores + req[j].cores <= server[i].b_cores && b_memories + req[j].memories <= server[i].b_memories) {
					b_cores += req[j].cores; b_memories += req[j].memories;
					temp.push_back(pair<int, int>{j, 1});
				}
				else if (a_cores + req[j].cores <= server[i].a_cores && a_memories + req[j].memories <= server[i].a_memories) {
					a_cores += req[j].cores; a_memories += req[j].memories;
					temp.push_back(pair<int, int>{j, 0});
				}
				else {
					break;
				}
			}
		}
		if (a_cores + b_cores + a_memories + b_memories) {
			int diff =  a_cores + b_cores + a_memories + b_memories;
			if (diff > minDiff) {
				minDiff = diff;
				index = i;
				res = temp;
			}
		}
	}
	for (int i = 0; i < res.size(); i++) {
		if (res[i].second == 0) {
			server[index].a_cores -= req[res[i].first].cores ;
			server[index].a_memories -= req[res[i].first].memories ;

		}
		else {
			server[index].b_cores -= req[res[i].first].cores;
			server[index].b_memories -= req[res[i].first].memories;
		}
	}
}
//根据请求购买服务器,返回需要购买的服务器的下标,-1表示购买失败
vector<int> buy(vector<Server>& currentServers,vector<Request> req,vector<Server>server) {
	int index = -1;					//记录服务器的id
	vector<int> res;				//记录服务器中虚拟机的id
	int minDiff = 0x3fffffff;			//记录差值
	for (int i = 0; i < server.size(); i++) {
		vector<int>temp;
		int cores = 0;
		int memories = 0;
		int j = 0;
		for (; j <req.size(); j++) {
			cores += req[j].cores;
			memories += req[j].memories;
			if (cores <= server[i].cores && memories <= server[i].memories) {
				temp.push_back(j);
			}
			else {
				cores -= req[j].cores;
				memories -= req[j].memories;
				break;
			}
		}
		if (cores + memories) {
			int diff = server[i].cores + server[i].memories - cores - memories;
			if (diff < minDiff) {
				minDiff = diff;
				index = i;
				res = temp;
			}
		}
	}
	currentServers.push_back(server[index]);
	currentServers.back().num = getNumSameType(currentServers, server[index].type);
	return res;
}
vector<pair<int, int>> buySingle(vector<Server>& currentServers, vector<Request> req, vector<Server>server) {
	int index = -1;					//记录服务器的id
	vector<pair<int, int>> res;		//记录服务器中虚拟机的id和位置
	int minDiff = 0x3fffffff;		//记录差值

	for (int i = 0; i < server.size(); i++) {
		vector<pair<int, int>> temp;
		int a_cores = 0, b_cores = 0;
		int a_memories = 0, b_memories = 0;
		for (int j = 0; j < req.size();j++) {
			if (a_cores < b_cores) {
				if (a_cores + req[j].cores <= server[i].a_cores && a_memories + req[j].memories <= server[i].a_memories) {
					a_cores += req[j].cores; a_memories += req[j].memories;
					temp.push_back(pair<int, int>{j, 0});
				}
				else if (b_cores + req[j].cores <= server[i].b_cores && b_memories + req[j].memories <= server[i].b_memories) {
					b_cores += req[j].cores; b_memories += req[j].memories;
					temp.push_back(pair<int, int>{j, 1});
				}
				else {
					break;
				}
			}
			else {
				if (b_cores + req[j].cores <= server[i].b_cores && b_memories + req[j].memories <= server[i].b_memories) {
					b_cores += req[j].cores; b_memories += req[j].memories;
					temp.push_back(pair<int, int>{j, 1});
				}else if (a_cores + req[j].cores <= server[i].a_cores && a_memories + req[j].memories <= server[i].a_memories) {
					a_cores += req[j].cores; a_memories += req[j].memories;
					temp.push_back(pair<int, int>{j, 0});
				}
				else {
					break;
				}
			} 
		}
		
		if (a_cores + b_cores + a_memories + b_memories) {
			int diff = server[i].cores + server[i].memories - a_cores - b_cores - a_memories - b_memories;
			if (diff < minDiff) {
				minDiff = diff;
				index = i;
				res = temp;
			}
		}
	}
	currentServers.push_back(server[index]);
	currentServers.back().num = getNumSameType(currentServers, server[index].type);
	return res;
}

bool cmpCoreReq(Request r1, Request r2) {
	return r1.cores < r2.cores;
}
bool cmpMemReq(Request r1, Request r2) {
	return r1.memories < r2.memories;
}
bool cmpCoreServer(Server s1, Server s2) {
	return s1.a_cores + s1.b_cores < s2.a_cores + s2.b_cores;
}
bool cmpMemServer(Server s1, Server s2) {
	return s1.a_memories + s1.b_memories < s2.a_memories + s2.b_memories;
}
//将请求分为4类，双结点核请求，双结点内存请求，单节点核请求，单节点内存请求；
void classifyRequest(vector<Request> &doubleCoreReq,vector<Request>&doubleMemoryReq,
	vector<Request>&singleCoreReq,vector<Request>&singleMemoryReq,int day) {
	for (int i = 0; i < requests[day].size(); i++) {
		if (requests[day][i].reqType == 0) {
			if (requests[day][i].isDouble) {
				requests[day][i].cores > requests[day][i].memories ? doubleCoreReq.push_back(requests[day][i]):	doubleMemoryReq.push_back(requests[day][i]);
			}
			else {
				requests[day][i].cores > requests[day][i].memories ? singleCoreReq.push_back(requests[day][i]) : singleMemoryReq.push_back(requests[day][i]);
			}
		}
	}
	sort(doubleCoreReq.begin(), doubleCoreReq.end(), cmpCoreReq);
	sort(doubleMemoryReq.begin(), doubleMemoryReq.end(), cmpMemReq);
	sort(singleCoreReq.begin(), singleCoreReq.end(), cmpCoreReq);
	sort(singleMemoryReq.begin(), singleMemoryReq.end(), cmpMemReq);
}
//将核服务器从currentServers中删除，添加到result中并按照core进行排序
void getCoreServer(vector<Server>& currentServers, vector<Server>&result) {
	for (int i = currentServers.size() - 1; i >= 0; i--) {
		if ((currentServers[i].a_cores + currentServers[i].b_cores) > (currentServers[i].a_memories + currentServers[i].b_memories)) {
			result.push_back(currentServers[i]);
			currentServers.erase(currentServers.begin() + i);
		}
	}
	sort(result.begin(), result.end(), cmpCoreServer);
}
//将内存服务器从currentServers中删除，添加到result中并按照memory进行排序
void getMemoryServer(vector<Server>& currentServers, vector<Server>&result) {
	for (int i = currentServers.size() - 1; i >= 0; i--) {
		if ((currentServers[i].a_cores + currentServers[i].b_cores) < (currentServers[i].a_memories + currentServers[i].b_memories)) {
			result.push_back(currentServers[i]);
			currentServers.erase(currentServers.begin() + i);
		}
	}
	sort(result.begin(), result.end(), cmpMemServer);
}
void parseDoubleCoreReq(vector<Server>& currentServers, vector<Request>& doubleCoreReq,map<int,int>&buyServers) {
	//vector<Server> coreServer;
	//getCoreServer(currentServers, coreServer);

	while (true) {
		int serverid = -1, location = -1;
		if (doubleCoreReq.size() && canHold(currentServers, doubleCoreReq[0], serverid, location)) {
			schedule[doubleCoreReq[0].ID] = tuple<int, int, int, int>{
				getVMachine(doubleCoreReq[0].type),currentServers[serverid].type,currentServers[serverid].num,location
			};
			doubleCoreReq.erase(doubleCoreReq.begin());
		}
		else {
			break;
		}
	}



	//将当前请求可以放在已有的核服务器中的请求放进去
	/*
	while (true) {
		int serverid = -1;
		vector<int> res;
		canHold(doubleCoreReq, coreServer, serverid, res);
		if (serverid != -1 && doubleCoreReq.size()){
			for (int i = res.size() -1; i >= 0; i--) {
				schedule[doubleCoreReq[ res[i]].ID] = tuple<int, int, int, int>{
					getVMachine(doubleCoreReq[res[i]].type),
					coreServer[serverid].type,
					coreServer[serverid].num,
					2
				};
				doubleCoreReq.erase(doubleCoreReq.begin() + res[i]);
			}
		}
		else {
			break;
		}
	}
	//将coreServers添加到原来的vector中
	for (int i = 0; i < coreServer.size(); i++) {
		currentServers.push_back(coreServer[i]);
	}*/
	//若有请求尚未放进去，可进行服务器的购买
	while (doubleCoreReq.size()) {
		//返回购买的虚拟机的id的集合
		vector<int> res = buy(currentServers, doubleCoreReq, server1);
		if (buyServers.find(currentServers.back().type) != buyServers.end()) {
			buyServers[currentServers.back().type]++;
		}
		else {
			buyServers[currentServers.back().type] = 1;
		}
		for (int i = 0; i < res.size(); i++) {
			currentServers.back().a_cores -= doubleCoreReq[res[i]].cores / 2;
			currentServers.back().b_cores -= doubleCoreReq[res[i]].cores / 2;
			currentServers.back().a_memories -= doubleCoreReq[res[i]].memories / 2;
			currentServers.back().b_memories -= doubleCoreReq[res[i]].memories / 2;
			schedule[doubleCoreReq[res[i]].ID] = tuple<int,int, int, int>{
				getVMachine(doubleCoreReq[res[i]].type),currentServers.back().type,currentServers.back().num,2
			};
		}
		for (int i = res.size() - 1; i >= 0; i--) {
			doubleCoreReq.erase(doubleCoreReq.begin() + res[i]);
		}
	}
}
void parseDoubleMemoryReq(vector<Server>& currentServers, vector<Request>& doubleMemoryReq, map<int, int>&buyServers) {
	//vector<Server> memoryServer;
	//getMemoryServer(currentServers, memoryServer);
	//将当前请求可以放在已有的核服务器中的请求放进去
	
	while (true) {
		int serverid = -1, location = -1;
		if (doubleMemoryReq.size() && canHold(currentServers, doubleMemoryReq[0], serverid, location)) {
			schedule[doubleMemoryReq[0].ID] = tuple<int, int, int, int>{
				getVMachine(doubleMemoryReq[0].type),currentServers[serverid].type,currentServers[serverid].num,location
			};
			doubleMemoryReq.erase(doubleMemoryReq.begin());
		}
		else {
			break;
		}
	}/*
	while (true) {
		int serverid = -1;
		vector<int> res;
		canHold(doubleMemoryReq, memoryServer, serverid, res);
		if (serverid != -1 && doubleMemoryReq.size()) {
			for (int i = res.size() - 1; i >= 0; i--) {
				schedule[doubleMemoryReq[res[i]].ID] = tuple<int, int, int, int>{
					getVMachine(doubleMemoryReq[res[i]].type),
					memoryServer[serverid].type,
					memoryServer[serverid].num,
					2
				};
				doubleMemoryReq.erase(doubleMemoryReq.begin() + res[i]);
			}
		}
		else {
			break;
		}
	}*/
	//将coreServers添加到原来的vector中
	/*for (int i = 0; i < memoryServer.size(); i++) {
		currentServers.push_back(memoryServer[i]);
	}*/
	//若有请求尚未放进去，可进行服务器的购买
	while (doubleMemoryReq.size()) {
		//返回购买的虚拟机的id的集合
		vector<int> res = buy(currentServers, doubleMemoryReq, server2);
		if (buyServers.find(currentServers.back().type) != buyServers.end()) {
			buyServers[currentServers.back().type]++;
		}
		else {
			buyServers[currentServers.back().type] = 1;
		}
		for (int i = 0; i < res.size(); i++) {
			currentServers.back().a_cores -= doubleMemoryReq[res[i]].cores / 2;
			currentServers.back().b_cores -= doubleMemoryReq[res[i]].cores / 2;
			currentServers.back().a_memories -= doubleMemoryReq[res[i]].memories / 2;
			currentServers.back().b_memories -= doubleMemoryReq[res[i]].memories / 2;
			schedule[doubleMemoryReq[res[i]].ID] = tuple<int, int, int, int>{
				getVMachine(doubleMemoryReq[res[i]].type),currentServers.back().type,currentServers.back().num,2
			};
		}
		for (int i = res.size() - 1; i >= 0; i--) {
			doubleMemoryReq.erase(doubleMemoryReq.begin() + res[i]);
		}
	}
}
void parseSingleCoreReq(vector<Server>& currentServers, vector<Request>& singleCoreReq, map<int, int>&buyServers) {
	vector<Server> coreServer;
	getCoreServer(currentServers, coreServer);
	//将当前请求可以放在已有的核服务器中的请求放进去
	
	while (true) {
		int serverid = -1, location = -1;
		if (singleCoreReq.size() && canHold(coreServer, singleCoreReq[0], serverid, location)) {
			schedule[singleCoreReq[0].ID] = tuple<int, int, int, int>{
				getVMachine(singleCoreReq[0].type),coreServer[serverid].type,coreServer[serverid].num,location
			};
			singleCoreReq.erase(singleCoreReq.begin());
		}
		else {
			break;
		}
	}/*
	while (true) {
		int serverid = -1;
		vector<pair<int,int>> res;
		canHoldSingle(singleCoreReq, coreServer, serverid, res);
		if (serverid != -1 && singleCoreReq.size()) {
			for (int i = res.size() - 1; i >= 0; i--) {
				schedule[singleCoreReq[res[i].first].ID] = tuple<int, int, int, int>{
					getVMachine(singleCoreReq[res[i].first].type),
					coreServer[serverid].type,
					coreServer[serverid].num,
					res[i].second
				};
				singleCoreReq.erase(singleCoreReq.begin() + res[i].first);
			}
		}
		else {
			break;
		}
	}*/
	//将coreServers添加到原来的vector中
	for (int i = 0; i < coreServer.size(); i++) {
		currentServers.push_back(coreServer[i]);
	}
	//若有请求尚未放进去，可进行服务器的购买
	while (singleCoreReq.size()) {
		//返回购买的虚拟机的id的集合
		vector<pair<int,int>> res = buySingle(currentServers, singleCoreReq, server1);
		if (buyServers.find(currentServers.back().type) != buyServers.end()) {
			buyServers[currentServers.back().type]++;
		}
		else {
			buyServers[currentServers.back().type] = 1;
		}
		for (int i = 0; i < res.size(); i++) {
			if (res[i].second == 0) {//放在A节点
				currentServers.back().a_cores -= singleCoreReq[res[i].first].cores;
				currentServers.back().a_memories -= singleCoreReq[res[i].first].memories;
				schedule[singleCoreReq[res[i].first].ID] = tuple<int, int, int, int>{
					getVMachine(singleCoreReq[res[i].first].type),currentServers.back().type,currentServers.back().num,0
				};
			}
			else {
				currentServers.back().b_cores -= singleCoreReq[res[i].first].cores;
				currentServers.back().b_memories -= singleCoreReq[res[i].first].memories;
				schedule[singleCoreReq[res[i].first].ID] = tuple<int, int, int, int>{
					getVMachine(singleCoreReq[res[i].first].type),currentServers.back().type,currentServers.back().num,1
				};
			}
		}
		for (int i = res.size() - 1; i >= 0; i--) {
			singleCoreReq.erase(singleCoreReq.begin() + res[i].first);
		}
	}
}
void parseSingleMemoryReq(vector<Server>& currentServers, vector<Request>& singleMemoryReq, map<int, int>&buyServers) {
	vector<Server> memoryServer;
	getMemoryServer(currentServers, memoryServer);
	//将当前请求可以放在已有的核服务器中的请求放进去
	
	while (true) {
		int serverid = -1, location = -1;
		if (singleMemoryReq.size() && canHold(memoryServer, singleMemoryReq[0], serverid, location)) {
			schedule[singleMemoryReq[0].ID] = tuple<int, int, int, int>{
				getVMachine(singleMemoryReq[0].type),memoryServer[serverid].type,memoryServer[serverid].num,location
			};
			singleMemoryReq.erase(singleMemoryReq.begin());
		}
		else {
			break;
		}
	}/*
	while (true) {
		int serverid = -1;
		vector<pair<int, int>> res;
		canHoldSingle(singleMemoryReq, memoryServer, serverid, res);
		if (serverid != -1 && singleMemoryReq.size()) {
			for (int i = res.size() - 1; i >= 0; i--) {
				schedule[singleMemoryReq[res[i].first].ID] = tuple<int, int, int, int>{
					getVMachine(singleMemoryReq[res[i].first].type),
					memoryServer[serverid].type,
					memoryServer[serverid].num,
					res[i].second
				};
				singleMemoryReq.erase(singleMemoryReq.begin() + res[i].first);
			}
		}
		else {
			break;
		}
	}*/
	//将coreServers添加到原来的vector中
	for (int i = 0; i < memoryServer.size(); i++) {
		currentServers.push_back(memoryServer[i]);
	}
	//若有请求尚未放进去，可进行服务器的购买
	while (singleMemoryReq.size()) {
		//返回购买的虚拟机的id的集合
		vector<pair<int, int>> res = buySingle(currentServers, singleMemoryReq, server2);
		if (buyServers.find(currentServers.back().type) != buyServers.end()) {
			buyServers[currentServers.back().type]++;
		}
		else {
			buyServers[currentServers.back().type] = 1;
		}
		for (int i = 0; i < res.size(); i++) {
			if (res[i].second == 0) {//放在A节点
				currentServers.back().a_cores -= singleMemoryReq[res[i].first].cores;
				currentServers.back().a_memories -= singleMemoryReq[res[i].first].memories;
				schedule[singleMemoryReq[res[i].first].ID] = tuple<int, int, int, int>{
					getVMachine(singleMemoryReq[res[i].first].type),currentServers.back().type,currentServers.back().num,0
				};
			}
			else {
				currentServers.back().b_cores -= singleMemoryReq[res[i].first].cores;
				currentServers.back().b_memories -= singleMemoryReq[res[i].first].memories;
				schedule[singleMemoryReq[res[i].first].ID] = tuple<int, int, int, int>{
					getVMachine(singleMemoryReq[res[i].first].type),currentServers.back().type,currentServers.back().num,1
				};
			}
		}
		for (int i = res.size() - 1; i >= 0; i--) {
			singleMemoryReq.erase(singleMemoryReq.begin() + res[i].first);
		}
	}
}
//根据当前的服务器和已有的虚拟机的分配数据来进行试探应该购买那些服务器,返回购买服务器的map集合
map<int, int> parseRequest(vector<Server> currentServers, int day) {
	map<int, int> buyServers;
	vector<Request> doubleCoreReq,doubleMemoryReq,singleCoreReq,singleMemoryReq;
	classifyRequest(doubleCoreReq, doubleMemoryReq, singleCoreReq, singleMemoryReq, day);
	/*
	vector<Request> temp;
	for (int i = 0; i < doubleCoreReq.size()/2; i++) {
		temp.push_back(doubleCoreReq[i]);
		temp.push_back(doubleCoreReq[doubleCoreReq.size() - i - 1]);
	}
	if (doubleCoreReq.size() % 2 == 1) {
		temp.push_back(doubleCoreReq[doubleCoreReq.size() / 2 ]);
	}
	doubleCoreReq = temp;
	temp.clear();
	for (int i = 0; i < doubleMemoryReq.size() / 2; i++) {
		temp.push_back(doubleMemoryReq[i]);
		temp.push_back(doubleMemoryReq[doubleMemoryReq.size() - i - 1]);
	}
	if (doubleMemoryReq.size() % 2 == 1) {
		temp.push_back(doubleMemoryReq[doubleMemoryReq.size() / 2 ]);
	}
	doubleMemoryReq = temp;
	temp.clear();
	for (int i = 0; i < singleCoreReq.size() / 2; i++) {
		temp.push_back(singleCoreReq[i]);
		temp.push_back(singleCoreReq[singleCoreReq.size() - i - 1]);
	}
	if (singleCoreReq.size() % 2 == 1) {
		temp.push_back(singleCoreReq[singleCoreReq.size() / 2 ]);
	}
	singleCoreReq = temp;
	temp.clear();
	for (int i = 0; i < singleMemoryReq.size() / 2; i++) {
		temp.push_back(singleMemoryReq[i]);
		temp.push_back(singleMemoryReq[singleMemoryReq.size() - i - 1]);
	}
	if (singleMemoryReq.size() % 2 == 1) {
		temp.push_back(singleMemoryReq[singleMemoryReq.size() / 2 ]);
	}
	singleMemoryReq = temp;
	*/

	parseDoubleCoreReq(currentServers,doubleCoreReq,buyServers);
	parseDoubleMemoryReq(currentServers,doubleMemoryReq,buyServers);
	parseSingleCoreReq(currentServers,singleCoreReq,buyServers);
	parseSingleMemoryReq(currentServers,singleMemoryReq,buyServers);
	
	return buyServers;
}
void del(vector<Server> & servers, tuple<int, int, int, int> tuples) {
	int vmID = get<0>(tuples);
	int type = get<1>(tuples);
	int num = get<2>(tuples);
	int location = get<3>(tuples);

	int cores = vMachines[vmID].cores;
	int memories = vMachines[vmID].memories;
	int serverID = getIndex(servers, type, num);

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
void output(map<int, int> buyServers, int day) {
	//开始输出
	printf("(purchase, %d)\n", buyServers.size());
	map<int, int>::iterator iter = buyServers.begin();
	while (iter != buyServers.end()) {
		int index = getServerIndex(iter->first);
		int num = iter->second;
		int typeNum = getNumSameType(allservers, iter->first);
		printf("(%s, %d)\n", servers[index].name, num);
		for (int i = 0; i < num; i++) {
			allservers.push_back(servers[index]);
			allservers.back().sequence = allservers.size() - 1;
			allservers.back().num = typeNum + i + 1;
		}
		iter++;
	}
	printf("(migration, 0)\n");
}
//此时开始进行真正的分配操作
void postProcess(int day) {
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
		int cores = requests[day][i].cores;
		int memories = requests[day][i].memories;

		int sequence = allservers[serverid].sequence;

		//如果要添加虚拟机
		if (reqType == 0) {
			
			//添加部署信息
			deployInfo[sequence].push_back(pair<int, int>{id, location});

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
				allservers[serverid].a_cores -= cores / 2;
				allservers[serverid].a_memories -= memories / 2;
				allservers[serverid].b_cores -= cores / 2;
				allservers[serverid].b_memories -= memories / 2;
				break; }
			}
			if (location == 2) {
				printf("(%d)\n", sequence);
			}
			else {
				printf("(%d, %c)\n", sequence, location == 0 ? 'A' : 'B');
			}
		}
		else {  //否则删除虚拟机
			//获取虚拟机所在的服务器
			del(allservers, schedule.at(id));
			//从调度信息中删除
			schedule.erase(id);
			//删除部署信息
			vector<pair<int, int>>::iterator iter = deployInfo[sequence].begin();
			while (iter != deployInfo[sequence].end()) {
				if (iter->first == id) {
					deployInfo[sequence].erase(iter);
					break;
				}
				iter++;
			}
		}
	}
}
//将服务器按照性价比从低到高进行排序
bool cmpServer(Server s1, Server s2) {
	//return s1.costPerformance < s2.costPerformance;
	return s1.cost < s2.cost;
}

void compute() {
	//首先将服务器按照性价比进行升序排序
	for (int i = 0; i < servers.size(); i++) {
		if (servers[i].coreMemRation > 1) {
			server1.push_back(servers[i]);
		}
		else {
			server2.push_back(servers[i]);
		}
	}
	sort(server1.begin(), server1.end(), cmpServer);
	sort(server2.begin(), server2.end(), cmpServer);
	for (int day = 0; day < T; day++) {
		map<int, int> buyServers = parseRequest(allservers, day);
		output(buyServers, day);
		postProcess(day);
	}
}

int main() {
//	loadFile();
	load();
	compute();
	return 0;
}
