//试图遍历当天的请求和现有的所有的服务器资源，寻找使得某一台服务器剩余的core最少的请求对，时间复杂度高，超时严重
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
using namespace std;
long money = 0;
int N;          //服务器类型数量,1-100
int M;          //虚拟机类型数量,1-1000
int T;          //T天的用户请求数据,1-1000
//可以采购的服务器类型，读入之后可能会进行排序
//增加了type、num、sequence字段
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
}server[110];
//虚拟机类型，读入之后不会再改变
struct VMachine {
	char type[25];      //虚拟机型号，长度不超过20字符
	int cores;          //CPU核数，1-1024
	int memories;       //内存大小，1-1024
	int isDouble;       //是否双结点，0表示单节点，1表示双节点
} vMachine[1010];
//用户请求，读入之后可能会对用户的请求进行排序
//增加了sequence字段，按用户的请求顺序进行赋值
struct Request {
	int sequence;		//表示用户的请求顺序
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
//键是服务器的sequence字段，值是虚拟机的id号和location信息
map<int, vector<pair<int, int>>> deployInfo;

//加载数据
void load() {
	/*
	FILE * fp = fopen("C:\\Users\\79943\\OneDrive\\codecraft\\training-data\\training-1.txt", "r");
	fscanf(fp, "%d", &N);
	for (int i = 0; i < N; i++) {
		fgetc(fp);
		fscanf(fp, "(%[^,], %d, %d, %d, %d)", server[i].name, &server[i].cores, &server[i].memories, &server[i].cost, &server[i].comsumption);
		server[i].type = i;
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
			req.sequence = j;
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
		scanf("(%[^,], %d, %d, %d, %d)", server[i].name, &server[i].cores, &server[i].memories, &server[i].cost, &server[i].comsumption);
		server[i].type = i;
		server[i].a_cores = server[i].b_cores = server[i].cores / 2;
		server[i].a_memories = server[i].b_memories = server[i].memories / 2;
	}
	scanf("%d", &M);
	for (int i = 0; i < M; i++) {
		getchar();
		scanf("(%[^,], %d, %d, %d)",
			vMachine[i].type, &vMachine[i].cores, &vMachine[i].memories, &vMachine[i].isDouble);
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
			}
			else {
				req.reqType = 1;
				scanf("%d)", &req.ID);
			}
			requests[i].push_back(req);
		}
	}
}
//获取请求的虚拟机类型数据，返回下标，没找到返回-1
int getVMachine(const char * type) {
	for (int i = 0; i < M; i++) {
		if (strcmp(vMachine[i].type, type) == 0) {
			return i;
		}
	}
	return -1;
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
		if (server[i].type == type) {
			return i;
		}
	}
	return -1;
}
//对服务器进行排序
bool cmpServer(Server s1, Server s2) {
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
}
//对已购得的服务器进行排序
bool cmpAllServer(Server s1, Server s2) {
	int surplus1 = s1.a_cores + s1.b_cores;
	int surplus2 = s2.a_cores + s2.b_cores;
	return surplus1 < surplus2;
}
//对一天内用户的请求进行排序，add请求在前，请求core比较少在前
bool cmpRequest(Request r1, Request r2) {
	
	int index1 = getVMachine(r1.type);
	int index2 = getVMachine(r2.type);
	int isDouble1 = vMachine[index1].isDouble;
	int isDouble2 = vMachine[index2].isDouble;
	int core1 = vMachine[index1].cores;
	int core2 = vMachine[index2].cores;
	int memory1 = vMachine[index1].memories;
	int memory2 = vMachine[index2].memories;

	if (r1.reqType != r2.reqType) {
		return r1.reqType < r2.reqType;	//add在前处理，否则会出现一致性问题
	}else if (isDouble1 != isDouble2) {
		return isDouble1 > isDouble2;
	}else if (core1 != core2) {
		return core1 > core2;			//先处理请求core比较少的请求
	}else{
		return memory1 > memory2;
	}
}
//恢复初始的用户请求顺序
bool cmpRecoverRequestSequence(Request r1, Request r2) {
	return r1.sequence < r2.sequence;
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
			return server[k].type;
		}
		if (isDouble == 0 && server[k].a_cores >= cores && server[k].a_memories / 2 >= memories) {
			currentServers.push_back(server[k]);
			//更新容量
			currentServers.back().a_memories -= memories;
			currentServers.back().a_cores -= cores;
			return server[k].type;
		}
	}
	return -1;
}
void del(vector<Server> & servers, tuple<int, int, int, int> tuples) {
	int vmID = get<0>(tuples);
	int type = get<1>(tuples);
	int num = get<2>(tuples);
	int location = get<3>(tuples);

	int cores = vMachine[vmID].cores;
	int memories = vMachine[vmID].memories;
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
		printf("(%s, %d)\n", server[index].name, num);
		for (int i = 0; i < num; i++) {
			money += server[index].cost + server[index].comsumption * (T - day);
			allservers.push_back(server[index]);
			allservers.back().sequence = allservers.size() - 1;
			allservers.back().num = typeNum + i + 1;
		}
		iter++;
	}
	printf("(migration, 0)\n");
}

//最佳分配,遍历当前所有服务器和所有请求，寻找使得服务器的剩下的cores最小的virtualID,要求输入的requests全都是add请求
//serverid,reqID,location
//没找到返回 -1,-1,-1
tuple<int,int,int> bestFit(const vector<Server>& currentServers,const vector<Request>& requests) {
	int mincost = 0x3fffffff,serverID = -1, reqID = -1, location = -1;
	for (int i = 0; i < currentServers.size(); i++) {
		for (int j = 0; j < requests.size(); j++) {
			//获取公共的请求信息
			int id = requests[j].ID;
			//如果要添加虚拟机
			int index = getVMachine(requests[j].type);
			int cores = vMachine[index].cores;
			int memories = vMachine[index].memories;
			int isDouble = vMachine[index].isDouble;

			if (isDouble) {
				if (currentServers[i].a_cores >= cores / 2 && currentServers[i].b_cores >= cores / 2
					&& currentServers[i].a_memories >= memories / 2 && currentServers[i].b_memories >= memories / 2) {
					if (currentServers[i].a_cores + currentServers[i].b_cores - cores < mincost) {
						serverID = i;
						reqID = j;
						location = 2;
						mincost = currentServers[i].a_cores + currentServers[i].b_cores - cores;
					
					}
				}
			}
			else {
				if (currentServers[i].a_cores > currentServers[i].b_cores){
					if (currentServers[i].a_cores >= cores && currentServers[i].a_memories >= memories) {
						if (currentServers[i].a_cores + currentServers[i].b_cores - cores < mincost) {
							serverID = i;
							reqID = j;
							location = 0;
							mincost = currentServers[i].a_cores + currentServers[i].b_cores - cores;

						}
					}
				}
				else if (currentServers[i].b_cores >= cores && currentServers[i].b_memories >= memories) {
					if (currentServers[i].a_cores + currentServers[i].b_cores - cores < mincost) {
						serverID = i;
						reqID = j;
						location = 1;
						mincost = currentServers[i].a_cores + currentServers[i].b_cores - cores;

					}
				}
			}
		}
	}
	//比例cost，得出最小的cost的serverid和虚拟机id
	return tuple<int, int, int>{serverID,reqID,location};
}

//根据当前的服务器和已有的虚拟机的分配数据来进行试探应该购买那些服务器,返回购买服务器的map集合
map<int, int> parseRequest(vector<Server> currentServers, int day) {
	map<int, int> buyServers;
	vector<Request> addReq;
	for (int i = 0; i < requests[day].size(); i++) {
		if (requests[day][i].reqType == 0) {
			addReq.push_back(requests[day][i]);
		}
	}			
	sort(addReq.begin(), addReq.end(), cmpRequest);
	do{
		tuple<int,int,int> res = bestFit(currentServers, addReq);
		int serverID = get<0>(res);
		int reqID = get<1>(res);
		int location = get<2>(res);
		if (get<0>(res) == -1) {
			int id = addReq[0].ID;
			int index = getVMachine(addReq[0].type);
			int cores = vMachine[index].cores;
			int memories = vMachine[index].memories;
			int isDouble = vMachine[index].isDouble;
			//买服务器
			int type = buy(currentServers, cores, memories, isDouble);
			int num = getNumSameType(currentServers, type);
			currentServers.back().num = num;
			schedule[id] = tuple<int, int, int, int>{
				index,
				type,
				num,
				isDouble == 1 ? 2 : 0
			};
			if (buyServers.find(type) != buyServers.end()) {
				buyServers[type]++;
			}
			else {
				buyServers[type] = 1;
			}
			addReq.erase(addReq.begin());
		}
		else {
			int index = getVMachine(addReq[reqID].type);
			int cores = vMachine[index].cores;
			int memories = vMachine[index].memories;

			if (location == 2) {
				currentServers[serverID].a_cores -= cores / 2;
				currentServers[serverID].b_cores -= cores / 2;
				currentServers[serverID].a_memories -= memories / 2;
				currentServers[serverID].b_memories -= memories / 2;
			}
			else if (location == 0) {
				currentServers[serverID].a_cores -= cores;
				currentServers[serverID].a_memories -= memories;
			}
			else {
				currentServers[serverID].b_cores -= cores;
				currentServers[serverID].b_memories -= memories;
			}
			schedule[addReq[reqID].ID] = tuple<int, int, int, int>{
				getVMachine(addReq[reqID].type),
				currentServers[serverID].type,
				currentServers[serverID].num,
				location
			};
			addReq.erase(addReq.begin() + reqID);
		}
	} while (addReq.size() > 0);
	return buyServers;
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
		int cores = vMachine[vmID].cores;
		int memories = vMachine[vmID].memories;
		
		//添加部署信息
		int sequence = allservers[serverid].sequence;
		
		//如果要添加虚拟机
		if (reqType == 0) {
			
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
//若当天请求的前半部分中的 del请求 要大于后半部分的 add请求时 不进行排序，否则可以进行排序
//因为del请求比较多的时候释放出来的空间可能刚好够后续的add请求的使用，进行之后会增加购买开销
bool isSortBetter(const vector<Request>& requests) {
	int half = requests.size() / 2;
	int numAdd = 0, numDel = 0;
	for (int i = 0; i < half; i++) {
		if (requests[i].reqType == 1) {
			numDel++;
		}
	}
	for (int i = half; i < requests.size(); i++) {
		if (requests[i].reqType == 0) {
			numAdd++;
		}
	}
	if (numDel > numAdd) {
		return false;
	}
	return true;
}
void compute() {
	sort(server, server + N, cmpServer);
	for (int day = 0; day < T; day++) {
		//对服务器类型进行排序
		//sort(allservers.begin(), allservers.end(), cmpAllServer);
		
		//对请求进行排序
		//if (isSortBetter(requests[day])) {
		//	sort(requests[day].begin(), requests[day].end(), cmpRequest);
		//}

		//为了满足当天的请求应该购买的服务器的类型和数量
		//对临时的服务器和临时的虚拟机信息进行预操作
		map<int, int> buyServers = parseRequest(allservers, day);
		output(buyServers, day);
		//恢复初始请求顺序
		//sort(requests[day].begin(), requests[day].end(), cmpRecoverRequestSequence);

		postProcess(day);
	}
}
int main() {
	load();
	compute();
	printf("wss%d\n", money);
	return 0;
}