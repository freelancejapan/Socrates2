// 模拟苏格拉底麦田主动方和被动方
//

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <sstream>
#include <random>
#include <mutex>
#include <fstream>

//非UI程序，需要将每个线程join一下，否则会运行异常
std::vector<std::thread*> gloablThreadList;
std::mutex globalMutex;

struct active_participator;
struct passive_participator;

//主动方
struct active_participator {
	bool isAvailable = true;
	double val = 0;
	std::vector<passive_participator*> meetList;
};

//被动方
struct passive_participator {
	bool isAvailable = true;
	double val = 0;
	std::mutex mtx;
	std::vector<active_participator*> receiveList;
};

//模拟被动方行为策略
void receiver_func(passive_participator* pp, active_participator* ap)
{
	pp->mtx.lock();
	pp->receiveList.push_back(ap);
	pp->mtx.unlock();
}

//假设每个主动方都要会见所有被动方
void starter_func(int location, active_participator* p, std::vector<passive_participator*>* passiveList)
{
	//std::cout << "active " << p->val << " start location is " << location << std::endl;
	int sz = passiveList->size();
	int topass = sz / 10;
	int passed = 0;
	double threshold = 0;
	for (int i = 0; i < sz; i++) {
		int tmpidx = (location + i) % sz;
		//设定阈值
		if (passed < topass) {
			if (passiveList->at(tmpidx)->val > threshold) {
				threshold = passiveList->at(tmpidx)->val;
			}
			passed++;
		}
		else {
			if (passiveList->at(tmpidx)->isAvailable && passiveList->at(tmpidx)->val > threshold) {
				//发送邀约
				//以新建一个线程的方式来发送邀约

				passiveList->at(tmpidx)->mtx.lock();
				passiveList->at(tmpidx)->receiveList.push_back(p);
				passiveList->at(tmpidx)->mtx.unlock();

				//std::thread* tmp = new std::thread(receiver_func, passiveList->at(tmpidx), p);
				//globalMutex.lock();
				//gloablThreadList.push_back(tmp);
				//globalMutex.unlock();

				break;
			}
		}
	}
}

//int tempint = 0;
//void thread_plus(int p1)
//{
//	std::cout << "Parameter is " << p1 << std::endl;
//	int max = INT_MAX - 100;
//	while (tempint < max)
//	{
//		tempint = tempint + 1;
//	}
//}
//void thread_minus(double p1)
//{
//	std::cout << "Parameter is " << p1 << std::endl;
//	int min = INT_MIN + 100;
//	while (tempint > min)
//	{
//		tempint = tempint - 1;
//	}
//}

//经模拟后发现，两种方式设置被接受方的邀约列表，结果一致
//且100人，主动方放弃10%的策略会造成被接受方平均收到不到一个邀约，最高收到7个邀约
void collectInfos()
{
	std::random_device rd;
	std::mt19937 mt(rd());

	//被动主动参与方估值随机设定器
	//假设每个参与方的差距在5倍以内
	std::uniform_real_distribution<double> dist(0.1, 0.5);

	const int participatorCount = 100;
	//主动方起始位置随机设定器
	std::uniform_int_distribution<> rndActiveStarter(0, participatorCount - 1);

	std::vector<active_participator*>* activeList = new std::vector<active_participator*>;
	std::vector<passive_participator*>* passiveList = new std::vector<passive_participator*>;

	//生成两个长度相同的list
	//并给每个参与方一个随机数值
	for (int i = 0; i < participatorCount; i++)
	{
		active_participator* pactive = new active_participator;
		pactive->val = dist(mt);
		passive_participator* ppassive = new passive_participator;
		ppassive->val = dist(mt);
		activeList->push_back(pactive);
		passiveList->push_back(ppassive);
	}

	//针对每个主动方，让其从被动列方list的某个随机位置开始遍历，按照自己的策略发出邀约
	//针对每个主动方，启动一个线程
	for (int i = 0; i < participatorCount; i++)
	{
		int loc = rndActiveStarter(mt);
		std::thread* tmp = new std::thread(starter_func, loc, activeList->at(i), passiveList);
		//globalMutex.lock();
		gloablThreadList.push_back(tmp);
		//globalMutex.unlock();
	}

	//针对每个被动方，当其收到邀约时，根据自己的策略选择是否接受，当接受时此二者顺利退出

	//std::this_thread::sleep_for(std::chrono::milliseconds(400));
	for (int i = 0; i < gloablThreadList.size(); i++)
	{
		gloablThreadList[i]->join();
	}

	std::ofstream myfile;
	myfile.open("C:/Users/Admin/Desktop/tmp.csv", std::ios::app);
	//打印每个被动方接收到的邀约
	for (int i = 0; i < participatorCount; i++)
	{
		myfile << passiveList->at(i)->val << "," << 1.0 * passiveList->at(i)->receiveList.size() / participatorCount << std::endl;
	}
	myfile.close();

	//清理内存
	for (int i = 0; i < gloablThreadList.size(); i++)
	{
		delete gloablThreadList[i];
	}
	gloablThreadList.clear();
	for (int i = 0; i < activeList->size(); i++)
	{
		activeList->at(i)->meetList.clear();
		delete activeList->at(i);
	}
	activeList->clear();
	for (int i = 0; i < passiveList->size(); i++)
	{
		passiveList->at(i)->receiveList.clear();
		delete passiveList->at(i);
	}
	passiveList->clear();
	delete activeList;
	delete passiveList;
}

//假设麦穗和麦穗间最大有三倍差距
void calcDistribution(int topass,
	int repeatimes,
	double wheat_min_val,
	double wheat_max_val,
	int total_wheat_count, std::vector<std::string>* rescontainer) {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist(wheat_min_val, wheat_max_val);

	std::vector<double> income_list;

	for (int tmprepeat = 0; tmprepeat < repeatimes; tmprepeat++) {
		std::vector<double> wheat_list;
		double total = 0;
		for (int i = 0; i < total_wheat_count; i++)
		{
			double tmp = dist(mt);
			total += tmp;
			wheat_list.push_back(tmp);
		}
		double avg = total / total_wheat_count;

		double threshold = 0;
		int passed_count = 0;
		double selected = 0;
		for (int i = 0; i < total_wheat_count; i++)
		{
			//PASS掉前K个麦穗，设定阈值
			if (passed_count < topass) {
				if (wheat_list[i] > threshold) threshold = wheat_list[i];
				passed_count++;
			}
			else {
				//从K + 1开始，发现一株比阈值大的立刻终止
				if (wheat_list[i] > threshold) {
					selected = wheat_list[i];
					break;
				}
			}
		}
		income_list.push_back(selected / avg);
	}

	//write to file to check distribution

	if (rescontainer)
	{

		//打印每个被动方接收到的邀约
		for (int i = 0; i < income_list.size(); i++)
		{
			std::stringstream ss;
			ss << topass << "," << total_wheat_count << "," << repeatimes << "," << income_list[i];
			rescontainer->push_back(ss.str());
		}
	}
	else
	{
		std::ofstream myfile;
		myfile.open("C:/Users/Admin/Desktop/income.csv", std::ios::app);
		//打印每个被动方接收到的邀约
		for (int i = 0; i < income_list.size(); i++)
		{
			myfile << topass << "," << total_wheat_count << "," << repeatimes << "," << income_list[i] << std::endl;
		}
		myfile.close();
	}
}

int total_wheat = 100;
int repeat_times_each = 10000;
double wheat_min_val = 10;
double wheat_max_val = 30;

void simulateActiveParticipatorOnly()
{
	for (int i = 1; i < total_wheat; i++) {
		calcDistribution(i, repeat_times_each, wheat_min_val, wheat_max_val, total_wheat, nullptr);
	}
}

void simulateActiveParticipatorOnly_Multithread()
{
	int max_thread = std::thread::hardware_concurrency();
	std::vector<std::thread*> vec;
	std::vector<std::vector<std::string>*> rescontainer;

	for (int i = 1; i < total_wheat; i++) {
		if (vec.size() >= max_thread) {
			//wait thread execution
			for (int j = 0; j < vec.size(); j++)
			{
				vec[j]->join();
			}
			//free memory
			for (int j = 0; j < vec.size(); j++)
			{
				delete vec[j];
			}
			vec.clear();
		}

		std::vector<std::string>* tmpcontainer = new std::vector<std::string>();
		rescontainer.push_back(tmpcontainer);
		std::thread* tmp = new std::thread(calcDistribution,
			i,
			repeat_times_each,
			wheat_min_val,
			wheat_max_val,
			total_wheat,
			tmpcontainer);
		vec.push_back(tmp);
	}

	for (int i = 0; i < vec.size(); i++)
	{
		vec[i]->join();
	}
	//free memory
	for (int i = 0; i < vec.size(); i++)
	{
		delete vec[i];
	}
	vec.clear();

	//store to file
	std::ofstream myfile;
	myfile.open("C:/Users/Admin/Desktop/income.csv", std::ios::app);
	//打印每个被动方接收到的邀约
	for (int i = 0; i < rescontainer.size(); i++)
	{
		for (int j = 0; j < rescontainer[i]->size(); j++) {
			myfile << rescontainer[i]->at(j) << std::endl;
		}
	}
	myfile.close();
}

int main()
{
	//测试循环打印vector
	//std::vector<int> vec = { 1,2,3,4,5,6 };
	//int sz = vec.size();
	//int topass = sz / 10;
	//int passed = 0;
	//double threshold = 0;
	//int location = sz;
	//for (int i = 0; i < sz; i++) {
	//	int tmpidx = (location + i) % sz;
	//	std::cout << vec[tmpidx] << " ";
	//}
	//if (location > -1) return 0;

	auto start = std::chrono::high_resolution_clock::now();

	simulateActiveParticipatorOnly_Multithread();

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Execution Time is " << duration.count() << std::endl;

	return 2;

	for (int i = 0; i < 2000; i++) {
		collectInfos();
	}

	return 1;

	std::cout << "Hardware supported number of threads = "
		<< std::thread::hardware_concurrency() << std::endl;

	std::random_device rd;
	std::mt19937 mt(rd());

	//被动主动参与方估值随机设定器
	//假设每个参与方的差距在5倍以内
	std::uniform_real_distribution<double> dist(0.1, 0.5);


	const int participatorCount = 100;
	//主动方起始位置随机设定器
	std::uniform_int_distribution<> rndActiveStarter(0, participatorCount - 1);

	std::vector<active_participator*>* activeList = new std::vector<active_participator*>;
	std::vector<passive_participator*>* passiveList = new std::vector<passive_participator*>;

	//生成两个长度相同的list
	//并给每个参与方一个随机数值
	for (int i = 0; i < participatorCount; i++)
	{
		active_participator* pactive = new active_participator;
		pactive->val = dist(mt);
		passive_participator* ppassive = new passive_participator;
		ppassive->val = dist(mt);
		activeList->push_back(pactive);
		passiveList->push_back(ppassive);
	}

	//针对每个主动方，让其从被动列方list的某个随机位置开始遍历，按照自己的策略发出邀约
	//针对每个主动方，启动一个线程
	for (int i = 0; i < participatorCount; i++)
	{
		int loc = rndActiveStarter(mt);
		std::thread* tmp = new std::thread(starter_func, loc, activeList->at(i), passiveList);
		gloablThreadList.push_back(tmp);
	}

	//针对每个被动方，当其收到邀约时，根据自己的策略选择是否接受，当接受时此二者顺利退出

	//启动两个线程，一个负责+1，一个负责-1，直到共有变量到达临界值
	//int,long,double都会达到临界值
	//std::thread t1(thread_plus,3);
	//std::thread t2(thread_minus,4.1);
	//t1.join();
	//t2.join();

	//std::this_thread::sleep_for(std::chrono::seconds(1));

	for (int i = 0; i < gloablThreadList.size(); i++)
	{
		gloablThreadList[i]->join();
	}

	std::ofstream myfile;
	myfile.open("C:/Users/Admin/Desktop/tmp.csv", std::ios::app);
	//打印每个被动方接收到的邀约
	for (int i = 0; i < participatorCount; i++)
	{
		myfile << passiveList->at(i)->val << "," << 1.0 * passiveList->at(i)->receiveList.size() / participatorCount << std::endl;
		//std::cout << 1.0 * passiveList->at(i)->receiveList.size() / participatorCount << std::endl;
	}
	myfile.close();


	std::cout << "Exit of Main function" << std::endl;
	return 0;
}