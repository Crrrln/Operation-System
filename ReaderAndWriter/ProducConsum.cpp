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
	int serial;		//�̱߳��
	char entity;	//�߳�����
	int delay;		//�߳��ӳ�ʱ��
	int persist;	//��д����ʱ��
};

int read_count = 0;		//�������Զ����̼���
CRITICAL_SECTION mutex, w;		//�ٽ����ṹ����

void readAndWrite(char* file);		//��д����
void Thread_Reader(void* p);		//�̶߳�
void Thread_Writer(void* p);		//�߳�д

int main()
{
	char s[] = "rw_data.txt";
	readAndWrite(s);
	return 0;
}

void readAndWrite(char* file) {
	DWORD n_thread = 0;	//Ϊ�̱߳����
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
			h_thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(Thread_Writer), &thread_info[i], 0, &thread_ID);//����һ���µ��߳�
		}
		else if (thread_info[i].entity == Reader) {
			h_thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(Thread_Reader), &thread_info[i], 0, &thread_ID);
		}
		else {
			puts("Bad File\n");
			exit(0);
		}
	}

	InitializeCriticalSection(&mutex);	//��ʼ��һ���ٽ���Դ����
	InitializeCriticalSection(&w);

	WaitForMultipleObjects(n_thread, h_thread, TRUE, -1);	//����ں˶��󱻴���ʱ��WaitForMultipleObjectsѡ�����������С�ķ��أ�WaitForMultipleObjects(�������,ָ������������еĵ�һ��Ԫ�أ�TRUE����ʾ���Ƕ��󶼷����źţ������һֱ�ȴ���ȥ��Ҫ�Ⱥ�ĺ�������-1��ʾ��������)
	printf("Task is finished!\n");
	_getch();

}

// �������ȡ�����������
void Thread_Reader(void* p) {
	DWORD	m_delay;	//���̵߳��ӳ�ʱ��
	DWORD	m_persist;	//���̵߳ĳ���ʱ��
	int		m_serial;	//���̱߳��

	m_serial = ((ThreadInfo*)(p))->serial;
	m_delay = (DWORD)(((ThreadInfo*)(p))->delay * INTE_PER_SEC);
	m_persist = (DWORD)(((ThreadInfo*)(p))->persist * INTE_PER_SEC);

	while (true) {
		Sleep(m_delay);		//�ӳ�ʱ��
		printf("R thread %d send R require\n", m_serial);	//�����̷��Ͷ�����
		EnterCriticalSection(&mutex);	//�����ٽ���mutex
		read_count++;//���߶����̱�־ 
		//printf("%d\n", read_count);
		if (read_count == 1)
			EnterCriticalSection(&w);	//�����ٽ���w
		LeaveCriticalSection(&mutex);	//�뿪�ٽ���mutex
		printf("R thread %d begin to read\n", m_serial);	//R��ʼ��
		Sleep(m_persist);	//���ĳ���ʱ��
		printf("R thread %d finish R\n", m_serial);		//R������
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

