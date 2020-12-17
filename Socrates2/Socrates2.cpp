// 模拟苏格拉底麦田主动方和被动方
//

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <sstream>
#include <random>
#include <mutex>

//非UI程序，需要将每个线程join一下，否则会运行异常
std::vector<std::thread*> gloablThreadList;

struct active_participator;
struct passive_participator;

//主动方
struct active_participator {
	bool isAvailable = false;
	double val = 0;
	std::vector<passive_participator*> meetList;
};

//被动方
struct passive_participator {
	bool isAvailable;
	double val = 0;
	std::mutex mtx;
	std::vector<active_participator*> receiveList;
};

std::vector<std::string> outlist;
int looptimes = 1000000;
int tempint = 0;

std::string globalString = "";

void thread_function()
{
	for (int i = 0; i < looptimes; i++)
	{
		std::stringstream ss;
		ss << "thread function Executing for id = " << std::this_thread::get_id();
		//outlist.push_back(ss.str());
		//globalString = globalString + ss.str() + "\n";
	}
}

void thread_plus(int p1)
{
	std::cout << "Parameter is " << p1 << std::endl;
	int max = INT_MAX - 100;
	while (tempint < max)
	{
		tempint = tempint + 1;
	}
}
void thread_minus(double p1)
{
	std::cout << "Parameter is " << p1 << std::endl;
	int min = INT_MIN + 100;
	while (tempint > min)
	{
		tempint = tempint - 1;
	}
}

void receiver_func(passive_participator* pp, active_participator* ap)
{
	pp->mtx.lock();
	//pp->receiveList.push_back(ap);
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

				//passiveList->at(tmpidx)->mtx.lock();
				//passiveList->at(tmpidx)->receiveList.push_back(p);
				//passiveList->at(tmpidx)->mtx.unlock();

				std::thread* tmp = new std::thread(receiver_func, passiveList->at(tmpidx), p);
				//gloablThreadList.push_back(tmp);
			}
		}
	}
}

int main()
{
	std::cout << "Hardware supported number of threads = "
		<< std::thread::hardware_concurrency() << std::endl;

	std::random_device rd;
	std::mt19937 mt(rd());

	//假设每个参与方的差距在5倍以内
	std::uniform_real_distribution<double> dist(0.1, 0.5);


	/*for (int i = 0; i < 16; ++i)
		std::cout << dist(mt) << "\n";*/

	const int participatorCount = 100;
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




	//std::thread t1(thread_plus,3);
	//std::thread t2(thread_minus,4.1);
	//std::thread t3(thread_function);
	//std::thread t4(thread_function);
	//std::thread t5(thread_function);
	//std::thread t6(thread_function);

	//for (int i = 0; i < 10000; i++);
	//std::cout << "Display From MainThread" << std::endl;
	//t1.join();
	//t2.join();
	//t3.join();
	//t4.join();
	//t5.join();
	//t6.join();

	/*for (int i = 0; i < outlist.size(); i++)
	{
		std::cout << outlist[i] << std::endl;
	}*/

	std::this_thread::sleep_for(std::chrono::seconds(1));

	//for (int i = 0; i < gloablThreadList.size(); i++)
	//{
	//	gloablThreadList[i]->join();
	//}

	std::cout << "Exit of Main function" << std::endl;
	return 0;
}