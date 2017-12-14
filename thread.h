/* ------------------------------------------------------------------
* Author: Nicholas Christensen					User ID: nwchrist
* Due Date: 12/11/2017
* Class: CS3331
* Program Assignment #6
* File Name: thread.h
* File Purpose:
*		This file contains the class definitions for my threads.
* ------------------------------------------------------------------  */

#include "ThreadClass.h"

const int EOD = -1;

class sortThread : public Thread
{
	public:
		sortThread(int threadID, SynOneToOneChannel* Up, SynOneToOneChannel* Down, SynOneToOneChannel* Left, SynOneToOneChannel* Right, int** Result);
	private:
		void ThreadFunc();
		SynOneToOneChannel* LChannel;
		SynOneToOneChannel* UChannel;
		SynOneToOneChannel* RChannel;
		SynOneToOneChannel* DChannel;
		int Number;
		int threadID;
		int** Matrix;
};

class masterThread : public Thread
{
	public:
		masterThread(int threadID, SynOneToOneChannel* outputChannel, int** matrix, int iterations);
	private:
		void ThreadFunc();
		int threadID;
		int Iterations;
		int** dataMatrix;
		SynOneToOneChannel* channel;
		
};