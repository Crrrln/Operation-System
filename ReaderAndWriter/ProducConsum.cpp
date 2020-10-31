#include "stdafx.h"
#include "windows.h"
#include <conio.h>
#include <stdlib.h>
#include <fstream>
#include <io.h>
#include <string.h>
#include <stdio.h>


using namespace  std;

#define INTE_PER_SEC 100
#define MAX_THREAD_NUM 64
#define Writer 'W'
#define Reader 'R'

struct ThreadInfo {
	int serial;		//线程标号
	char entity;	//线程类型
	int delay;		//线程延迟时间
	int persist;	//读写持续时间
};

int read_count = 0;		//计数器对读进程计数
CRITICAL_SECTION mutex, w;		//临界区结构对象

void readAndWrite(char* file);		//读写函数
void Thread_Reader(void* p);		//线程读
void Thread_Writer(void* p);		//线程写

int main()
{
	char s[] = "rw_data.txt";
	readAndWrite(s);
	return 0;
}

void readAndWrite(char* file) {
	DWORD n_thread = 0;	//为线程标序号
	DWORD thread_ID;

	HANDLE h_thread[MAX_THREAD_NUM];
	ThreadInfo thread_info[MAX_THREAD_NUM];

	ifstream InFile;
	InFile.open(file);
	puts("Read Data File\n");

	while (InFile) {
		InFile >> thread_info[n_thread].serial;
		InFile >> thread_info[n_thread].entity;
		InFile >> thread_info[n_thread].delay;
		InFile >> thread_info[n_thread].persist;
		n_thread++;
		InFile.get();
	}
	n_thread--;
	for (int i = 0; i < (int)n_thread; i++) {
		if (thread_info[i].entity == Writer) {
			h_thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(Thread_Writer), &thread_info[i], 0, &thread_ID);//创建一个新的线程
		}
		else if (thread_info[i].entity == Reader) {
			h_thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(Thread_Reader), &thread_info[i], 0, &thread_ID);
		}
		else {
			puts("Bad File\n");
			exit(0);
		}
	}

	InitializeCriticalSection(&mutex);	//初始化一个临界资源对象
	InitializeCriticalSection(&w);

	WaitForMultipleObjects(n_thread, h_thread, TRUE, -1);	//多个内核对象被触发时，WaitForMultipleObjects选择其中序号最小的返回；WaitForMultipleObjects(句柄数量,指定对象句柄组合中的第一个元素；TRUE，表示除非对象都发出信号，否则就一直等待下去；要等候的毫秒数，-1表示立即返回)
	printf("Task is finished!\n");
	_getch();

}

// 读者优先、可以连续读
void Thread_Reader(void* p) {
	DWORD	m_delay;	//读线程的延迟时间
	DWORD	m_persist;	//读线程的持续时间
	int		m_serial;	//读线程标号

	m_serial = ((ThreadInfo*)(p))->serial;
	m_delay = (DWORD)(((ThreadInfo*)(p))->delay * INTE_PER_SEC);
	m_persist = (DWORD)(((ThreadInfo*)(p))->persist * INTE_PER_SEC);

	while (true) {
		Sleep(m_delay);		//延迟时间
		printf("R thread %d send R require\n", m_serial);	//读进程发送读请求
		EnterCriticalSection(&mutex);	//进入临界区mutex
		read_count++;//读者读进程标志 
		//printf("%d\n", read_count);
		if (read_count == 1)
			EnterCriticalSection(&w);	//进入临界区w
		LeaveCriticalSection(&mutex);	//离开临界区mutex
		printf("R thread %d begin to read\n", m_serial);	//R开始读
		Sleep(m_persist);	//读的持续时间
		printf("R thread %d finish R\n", m_serial);		//R读完了
		EnterCriticalSection(&mutex);
		read_count--;
		if (read_count == 0)
			LeaveCriticalSection(&w);
		LeaveCriticalSection(&mutex);
	}

}

void Thread_Writer(void* p) {
	DWORD	m_delay;
	DWORD	m_persist;
	int		m_serial;

	m_serial = ((ThreadInfo*)(p))->serial;
	m_delay = (DWORD)(((ThreadInfo*)(p))->delay * INTE_PER_SEC);
	m_persist = (DWORD)(((ThreadInfo*)(p))->persist * INTE_PER_SEC);

	while (true) {
		Sleep(m_delay);
		printf("W thread %d send W require\n", m_serial);
		EnterCriticalSection(&w);
		printf("W thread %d begin to write\n", m_serial);
		Sleep(m_persist);
		printf("W thread %d finish W\n", m_serial);
		LeaveCriticalSection(&w);
	}
}

