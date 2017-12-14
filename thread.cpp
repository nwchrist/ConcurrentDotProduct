/* ------------------------------------------------------------------
* Author: Nicholas Christensen					User ID: nwchrist
* Due Date: 12/11/2017
* Class: CS3331
* Program Assignment #6
* File Name: thread.cpp
* File Purpose:
*		This file contains the class implementations for my threads.
* ------------------------------------------------------------------  */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sstream>
#include "thread.h"

//IMPORTANT NOTE: The threadID of each process reflects its position in the array
//The tens place is the row of the process and the ones place is the column

sortThread::sortThread(int threadIDIn, SynOneToOneChannel* Up, SynOneToOneChannel* Down, SynOneToOneChannel* Left, SynOneToOneChannel* Right, int** Result)
{
	UserDefinedThreadID = threadIDIn;
	threadID = threadIDIn;
	Number = 0;
	
	LChannel = Left;
	UChannel = Up;
	RChannel = Right;
	DChannel = Down;
	Matrix = Result;
}


void sortThread::ThreadFunc()
{
	Thread::ThreadFunc();

	int tempRow = threadID / 10;
	int tempColumn = threadID % 10;
	
	char outputoo[1000];
	sprintf(outputoo, "      Thread P[%d, %d] started\n", tempRow, tempColumn);
	write(1, outputoo, strlen(outputoo));
	
	int LInput = 0;
	int UInput = 0;
	int tmp1 = 0;
	int tmp2 = 0;
	int done = 0;
	while (true)
	{
		
		//First we want to receive our inputs from our neighbor above and to the left
		LChannel->Receive(&LInput, sizeof(int));
		UChannel->Receive(&UInput, sizeof(int));
		
		char outputho[1000];
		sprintf(outputho, "      Thread P[%d, %d] received %d from above and %d from left\n", tempRow, tempColumn, UInput, LInput);
		write(1, outputho, strlen(outputho));
		
		if (LInput == EOD || UInput == EOD)
		{
			//If we receive an EOD, then we want to update the result matrix and send the EOD to our neighbors
			tmp1 = EOD;
			tmp2 = EOD;
			
			Matrix[threadID / 10 - 1][threadID % 10 - 1] = Number;
			done = 1;
			
		} else {
			//If we do not reveice an EOD, then we need to send the two numbers we reveived to our neighbors
			tmp1 = LInput;
			tmp2 = UInput;
			
			//And we need to update our number to be the product of our two input numbers
			Number = Number + tmp1 * tmp2;
		}
		
		//Here we send out the values to our neighbors if they are there
		if (RChannel == NULL && DChannel == NULL)
		{
			//In this case we are the bottom right process and we don't have to send info to anyone
		} else if (RChannel == NULL) {
			//In this case we only have a DChannel
			DChannel->Send(&UInput, sizeof(int));
			
			char outputko[1000];
			sprintf(outputko, "      Thread P[%d, %d] sent %d below\n", tempRow, tempColumn, UInput);
			write(1, outputko, strlen(outputko));
			
		} else if (DChannel == NULL) {
			//In this case we only have a RCHannel
			RChannel->Send(&LInput, sizeof(int));
			
			char outputko[1000];
			sprintf(outputko, "      Thread P[%d, %d] sent %d right\n", tempRow, tempColumn, LInput);
			write(1, outputko, strlen(outputko));
			
		} else {
			//If we made it here, than we have both a lower and right neighbor
			RChannel->Send(&LInput, sizeof(int));
			DChannel->Send(&UInput, sizeof(int));
			
			char outputko[1000];
			sprintf(outputko, "      Thread P[%d, %d] sent %d below and %d to the right\n", tempRow, tempColumn, UInput, LInput);
			write(1, outputko, strlen(outputko));
		}
		
		//If we have sent forward our EOD, we can exit our loop
		if (done == 1)
		{
			break;
		}
	}
	
	//Once we get here we know we have finished our task and can exit
	
	char outputhoh[1000];
	sprintf(outputhoh, "      Thread P[%d, %d] received EOD, saved result %d and terminated\n", tempRow, tempColumn, Number);
	write(1, outputhoh, strlen(outputhoh));
	
	Exit();
}


masterThread::masterThread(int inthreadID, SynOneToOneChannel* outputChannel, int** matrix, int iterations)
{
	UserDefinedThreadID = inthreadID;
	channel = outputChannel;
	dataMatrix = matrix;
	Iterations = iterations;
	threadID = inthreadID;
}


void masterThread::ThreadFunc()
{
	Thread::ThreadFunc();
	
	//The masterThread needs to read in data from either its row of matrix A or column of Matrix B
	//First we will find out what type of master thread we are based on our threadID
	int isVertical;
	if ((threadID / 10) >= 1)
	{
		isVertical = 0;
		
		char outputoo[1000];
		sprintf(outputoo, "   Column thread c[%d] started\n", threadID);
		write(1, outputoo, strlen(outputoo));
	} else {
		isVertical = 1;
		
		char outputoo[1000];
		sprintf(outputoo, "Row thread r[%d] started\n", (threadID % 10));
		write(1, outputoo, strlen(outputoo));
	}
	
	//If we are a vertical masterThread, we need to iterate through the rows of our column of matrix B and send the data to our channel
	int i;
	int data;
	if (isVertical)
	{
		
		int columnNumber = threadID % 10;
		for (i = 0; i < Iterations; i++)
		{
			data = dataMatrix[i][columnNumber - 1];
			channel->Send(&data, sizeof(int));
			
			char outputvo[1000];
			int tempID = threadID % 10;
			sprintf(outputvo, "   Column thread c[%d] sent %d to P[1, %d]\n", tempID, data, i + 1);
			write(1, outputvo, strlen(outputvo));
		}
		
		//Once we have sent all the data from the matrix, we then send EOD
		data = EOD;
		channel->Send(&data, sizeof(int));
		
	} else {
		
		//If we are a horizontal masterThread, then we iterate through the columns of our row in matrix and send the data to our channel
		int rowNumber = threadID / 10;
		
		for (i = 0; i < Iterations; i++)
		{
			data = dataMatrix[rowNumber - 1][i];
			channel->Send(&data, sizeof(int));
			
			char outputho[1000];
			int tempID = threadID / 10;
			sprintf(outputho, "Row thread r[%d] sent %d to P[%d, 1]\n", tempID, data, i + 1);
			write(1, outputho, strlen(outputho));
		}
		
		//Once we have sent all the data from the matrix, we then send EOD
		data = EOD;
		channel->Send(&data, sizeof(int));
	}
	
	//After we finish sending all the data and the EOD, we can exit
	
	//But first we need to print out that we are exiting
	int tempRow;
	int tempColumn;
	int idtemp;
	if (isVertical)
	{
		idtemp = threadID;
		tempRow = 1;
		tempColumn = threadID % 10;
		char output3 [10000];
		sprintf(output3, "   Column thread [%d] sent EOD to P[%d, %d] and terminated\n", idtemp, tempRow, tempColumn);
		write(1, output3, strlen(output3));
	} else {
		idtemp = threadID / 10;
		tempRow = threadID / 10;
		tempColumn = 1;
		char output3 [10000];
		sprintf(output3, "Row thread [%d] sent EOD to P[%d, %d] and terminated\n", idtemp, tempRow, tempColumn);
		write(1, output3, strlen(output3));
	}
	
	Exit();
}









