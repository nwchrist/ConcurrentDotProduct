/* ------------------------------------------------------------------
* Author: Nicholas Christensen					User ID: nwchrist
* Due Date: 12/11/2017
* Class: CS3331
* Program Assignment #6
* File Name: thread-main.cpp
* File Purpose:
*		This file contains the main program for assignment 6
* ------------------------------------------------------------------  */

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ThreadClass.h"
#include "thread.h"

using namespace std;

int main (int argc, char* argv[])
{
	//The first step is to read in matrices A and B
	
	//We will read in matrix A first, starting with the size of the matrix
	int numRowsA;
	int numColumnsA;
	cin >> numRowsA;
	cin >> numColumnsA;
	
	//Next we need to create a 2D array to store matrix A
	int i;
	int** APointer =  new int*[numRowsA];
	for (i = 0; i < numRowsA; i++)
	{
		APointer[i] = new int[numColumnsA];
	}
	
	//Then we need to add the values into the 2D array
	
	int j;
	for (i = 0; i < numRowsA; i++)
	{
		for (j = 0; j < numColumnsA; j++)
		{
			cin >> APointer[i][j];
		}
	}
	
	//And then we need to repeat the process for the second matrix
	int numRowsB;
	int numColumnsB;
	cin >> numRowsB;
	cin >> numColumnsB;
	int** BPointer = new int*[numRowsB];
	for (i = 0; i < numRowsB; i++)
	{
		BPointer[i] = new int[numColumnsB];
	}
	
	for (i = 0; i < numRowsB; i++)
	{
		for (j = 0; j < numColumnsB; j++)
		{
			cin >> BPointer[i][j];
		}
	}
	
	//Now that we have loaded in the data, we need to check and make sure that we can multiply the matricies together
	//The matrices can only be multiplied if the number of columns of matrix A equals the number of rows of matrix B
	//If the matrices are not compatible, the program prints out an error message and terminate
	if (numColumnsA != numRowsB)
	{
		cout << "Critical Error: Matrice dimensions not compatible for multiplication. This program will now terminate." << endl;
		Exit();
	}
	
	//Next we will create the result matrix
	int** Result = new int*[numRowsA];
	for (i = 0; i < numRowsA; i++)
	{
		Result[i] = new int [numColumnsB];
	}
	
	//Now we must create all of the necesary channels, and store them in an array
	//There will be twice as many channels as there are grid processes
	
	//SPECIFICATION: channel verticalChannels[A][B] connects threadIDs A*10+B+1 and A*10+10+B+1, simplified: A*10+B+1, A*10+B+11
	//Example1: verticalChannels[0][0] connects threadIDs 1 (The first vertical master channel) and 11 (the first processor)
	//Example2: verticalChannels[2][2] connects threadIDs 23 and 33
	
	
	//SPECIFICATION: channel horizontalChannels[A][B] connects threadIDs ((A+1)*10 + B) and ((A+1)*10 + B + 1)
	//Example1: horizontalChannels[0][0] connects threadIDs 10 (The first horizontal master channel) and 11 (the first processor)
	//Example2: horizontalChannels[2][2] connects threadIDs 32 and 33
	
	//First we will populate the vertical channel array
	SynOneToOneChannel* verticalChannels[10][10];
	for (i = 0; i < numRowsA; i++)
	{
		for (j = 0; j < numColumnsB; j++)
		{
			int channelID = i + j;
			char temp [10];
			sprintf(temp, "V%d", channelID);
			const char* FChannelName = temp;
			
			verticalChannels[i][j] = new SynOneToOneChannel(FChannelName, (i*10 + j + 1), (i*10 + j + 11));
		}
	}
	
	//Secondly we will populate the horizontal channel array
	SynOneToOneChannel* horizontalChannels[10][10];
	for (i = 0; i < numRowsA; i++)
	{
		for (j = 0; j < numColumnsB; j++)
		{
			int channelID = i + j;
			char temp [10];
			sprintf(temp, "H%d", channelID);
			const char* FChannelName = temp;
			
			horizontalChannels[i][j] = new SynOneToOneChannel(FChannelName, ((i+1)*10 + j), ((i+1)*10 + j + 1) );
		}
	}
	
	//Now that we have all of the channels that we need, we can create our threads
	//We will start with our vertical masterThreads
	masterThread* verticalMThreads[numColumnsB];
	
	for (i = 0; i < numColumnsB; i++)
	{
		int threadID = i + 1;
		
		//Each masterThread requires a threadID (which it can use to determine whether it is horizontal or vertical), the matrix for 
		//it to pull data from and a channel for it to send its message to, and how many data points to read in
		SynOneToOneChannel* Down = verticalChannels[0][i];
		
		//verticalMasterThreads require the B matrix
		verticalMThreads[i] = new masterThread(threadID, Down, BPointer, numRowsB);
		verticalMThreads[i]->Begin();
	}
	
	//And then our horizontal masterThreads
	masterThread* horizontalMThreads[numRowsA];
	
	for (i = 0; i < numRowsA; i++)
	{
		int threadID = (i+1) * 10;
		SynOneToOneChannel* Right = horizontalChannels[i][0];
		horizontalMThreads[i] = new masterThread(threadID, Right, APointer, numColumnsA);
		horizontalMThreads[i]->Begin();
	}
	
	//Next we can create our normal proceses
	sortThread* processors[numRowsA][numColumnsB];
	
	for (i = 0; i < numRowsA; i++)
	{
		for (j = 0; j < numColumnsB; j++)
		{
			int threadID = (i+1)*10 + j + 1;
			
			//Each sortThread needs to be passed its threadID; upper, lower, left and right channels (or NULL if it does not have one); and the result matrix for it to store its end result
			//First we need to get all of our channels to be passed
			SynOneToOneChannel* Up;
			SynOneToOneChannel* Left;
			SynOneToOneChannel* Right;
			SynOneToOneChannel* Down;
			
			Up = verticalChannels[i][j];
			Left = horizontalChannels[i][j];
			
			if (j == numColumnsB - 1)
			{
				Right = NULL;
			} else {
				Right = horizontalChannels[i][j+1];
			}
			
			if (i == numRowsA - 1)
			{
				Down = NULL;
			} else {
				Down = verticalChannels[i+1][j];
			}
			
			processors[i][j] = new sortThread(threadID, Up, Down, Left, Right, Result);
			processors[i][j]->Begin();
		}
	}
	
	//Now that everything is started we just need to join with all of the threads
	for (i = 0; i < numColumnsB; i++)
	{
		verticalMThreads[i]->Join();
	}
	
	for (i = 0; i < numRowsA; i++)
	{
		horizontalMThreads[i]->Join();
	}
	
	for (i = 0; i < numRowsA; i++)
	{
		for (j = 0; j < numColumnsB; j++)
		{
			processors[i][j]->Join();
		}
	}
	
	//Now we will print out all of the info for the main thread
	cout << endl;
	
	//First we will print out matrix A
	char outputFinal[1000];
	char * outputPointer = outputFinal;
	char buffer [100];
	sprintf(outputFinal, "Matrix A: %d rows and %d columns\n", numRowsA, numColumnsA);
	write(1, outputFinal, strlen(outputFinal));
	memset(outputPointer, '\0', sizeof(outputFinal));
	
	for (i = 0; i < numRowsA; i++)
	{
		for (j = 0; j < numColumnsA; j++)
		{
			sprintf(buffer, "%10d", APointer[i][j]);
			strcat(outputPointer, buffer);
		}
		sprintf(buffer, "\n");
		strcat(outputPointer, buffer);
		write(1, outputFinal, strlen(outputFinal));
		memset(outputPointer, '\0', sizeof(outputFinal));
	}
	cout << endl;
	
	//Then we will print out matrix B
	sprintf(outputFinal, "Matrix B: %d rows and %d columns\n", numRowsB, numColumnsB);
	write(1, outputFinal, strlen(outputFinal));
	memset(outputPointer, '\0', sizeof(outputFinal));
	
	for (i = 0; i < numRowsB; i++)
	{
		for (j = 0; j < numColumnsB; j++)
		{
			sprintf(buffer, "%10d", BPointer[i][j]);
			strcat(outputPointer, buffer);
		}
		sprintf(buffer, "\n");
		strcat(outputPointer, buffer);
		write(1, outputFinal, strlen(outputFinal));
		memset(outputPointer, '\0', sizeof(outputFinal));
	}
	cout << endl;
	
	//Finally we will print out Matrix C
	sprintf(outputFinal, "Matrix C = A * B: %d rows and %d columns\n", numRowsA, numColumnsB);
	write(1, outputFinal, strlen(outputFinal));
	memset(outputPointer, '\0', sizeof(outputFinal));
	
	for (i = 0; i < numRowsA; i++)
	{
		for (j = 0; j < numColumnsB; j++)
		{
			sprintf(buffer, "%10d", Result[i][j]);
			strcat(outputPointer, buffer);
		}
		sprintf(buffer, "\n");
		strcat(outputPointer, buffer);
		write(1, outputFinal, strlen(outputFinal));
		memset(outputPointer, '\0', sizeof(outputFinal));
	}
	
	
	
	//And then terminate
	Exit();
}


