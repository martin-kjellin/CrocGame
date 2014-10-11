#include "stdafx.h"
#include <iostream>
#include <string>
#include <array>
#include <math.h>
#include "CrocGame.h"
#include <map>
#include <windows.h>
#define _USE_MATH_DEFINES
#pragma comment(lib,"wherescrocengine")
//stays the same throughout session
std::vector<std::vector<int>> paths;
//change from game to game
int score;
int playerLocation;
int backpacker1Activity;
int backpacker2Activity;
double calciumReading;
double salineReading;
double alkalinityReading;
std::vector<std::pair<double,double>> calcium;
std::vector<std::pair<double,double>> salinity;
std::vector<std::pair<double,double>> alkalinity;
//probability: fill with 1/35
double probability[35] = {}; //index 0 is waterhole 1
double diffProbability[35] = {};

bool triedSpots[35];
int triedSpotsTime[35];

std::map<int,int> length;
std::map<int,int> parent;
std::vector<int> fastPath;

bool once = true;

double totalScore = 0;
double totalGamesPlayed = 0;

void makeFastPath(int startLocation) {
	fastPath.clear();
	//auto maxElement = std::max_element(std::begin(probability),std::end(probability));
	//int maxIndex = std::distance(std::begin(probability),maxElement);

	auto maxElement = std::max_element(std::begin(diffProbability),std::end(diffProbability));
	int maxIndex = std::distance(std::begin(diffProbability),maxElement);

	int currentIndex=maxIndex;

	fastPath.push_back(currentIndex);

	while(parent.count(currentIndex)>0) {
		fastPath.push_back(parent[currentIndex]);
		currentIndex = parent[currentIndex];
	}
}

void breadthFirstSearch(int currentLocation) {

	if(currentLocation==playerLocation-1) {  //starting place
		//std::wcout << L"Starting node = " << (currentLocation) << L"\n";
		length[currentLocation] = 0;

		for(int i=0;i<paths[currentLocation].size();i++) {
			//std::wcout << L"Neighbour " << (i+1) << L" = " << (paths[currentLocation][i]-1) << L"\n";
		}


		for(int i=0;i<paths[currentLocation].size();i++) {
			int currentNode = paths[currentLocation][i]-1;
			parent[currentNode]=currentLocation;
			length[currentNode]=length[parent[currentNode]]+1;
			//std::wcout << L"The node " << currentNode << L" has the parent node " << parent[currentNode] << L"\n";
			//std::wcout << L"The node " << currentNode << L" has the length " << length[currentNode] << L"\n";
		}

		for(int i=0;i<paths[currentLocation].size();i++) {
			int currentNode = paths[currentLocation][i]-1;
			breadthFirstSearch(currentNode);
		}

	} else { //if we are not at the starting place
		for(int i=0;i<paths[currentLocation].size();i++) {
			int currentNode = paths[currentLocation][i]-1;
			if(length.count(currentNode)>0) {
				if((length[currentLocation]+1)<length[currentNode]) {
					parent[currentNode]=currentLocation;
					length[currentNode]=length[parent[currentNode]]+1;
					breadthFirstSearch(currentNode);
					//std::wcout << L"The node " << currentNode << L" has the parent node " << parent[currentNode] << L"\n";
					//std::wcout << L"The node " << currentNode << L" has the length " << length[currentNode] << L"\n";
				}
			} else {
				parent[currentNode]=currentLocation;
				length[currentNode]=length[parent[currentNode]]+1;
				breadthFirstSearch(currentNode);
			}
		}
	}
}

//void fillProbability(double value){
//	int i;
//	for(i = 0; i < 35; i++){
//		probability[i] = value;
//	}
//}

void fillDiffProbability(double value){
	int i;
	for(i = 0; i < 35; i++){
		diffProbability[i] = value;
	}
}

//sets the start probabilities while accounting for backpackers
void accountForBackpackersStart(){
	int t;
	if(backpacker1Activity > 0 && backpacker2Activity > 0){
		if(backpacker1Activity != backpacker2Activity){ //both alive and at different places
			fillDiffProbability(1.0/33.0);
			diffProbability[backpacker1Activity-1] = 0;
			diffProbability[backpacker2Activity-1] = 0;
		}
		else{ //both alive at the same place
			fillDiffProbability(1.0/34.0);
			diffProbability[backpacker1Activity-1] = 0;
		}
	}
	else{ //one or both are being eaten / have been eaten
		if(backpacker1Activity < 0){
			fillDiffProbability(0.0);
			diffProbability[abs(backpacker1Activity)-1] = 1.0;
		}
		if(backpacker2Activity < 0){
			fillDiffProbability(0.0);
			diffProbability[abs(backpacker2Activity)-1] = 1.0;
		}
	}
}

//change the probability depending on backpackers
void accountForBackpackersDuring(){
	if(backpacker1Activity > 0){
		diffProbability[backpacker1Activity-1] = 0.0;	
	}
	if(backpacker2Activity > 0){
		diffProbability[backpacker2Activity-1] = 0.0;	
	}
	if(backpacker1Activity < 0){
		fillDiffProbability(0.0);
		diffProbability[abs(backpacker1Activity)-1] = 1.0;
	}
	if(backpacker2Activity < 0){
		fillDiffProbability(0.0);
		diffProbability[abs(backpacker2Activity)-1] = 1.0;
	}
}

double diffValueProbability(double reading1, double reading2, double reading3, double mean1, double std_dev1, double mean2, double std_dev2, double mean3, double std_dev3) {
	double pi = 3.141592653589793;
	
	double value1 = (1/sqrt(2*pi))*(1/std_dev1)*exp(-(reading1-mean1)*(reading1-mean1)/(2*std_dev1*std_dev1));
	double value2 = (1/sqrt(2*pi))*(1/std_dev2)*exp(-(reading2-mean2)*(reading2-mean2)/(2*std_dev2*std_dev2));
	double value3 = (1/sqrt(2*pi))*(1/std_dev3)*exp(-(reading3-mean3)*(reading3-mean3)/(2*std_dev3*std_dev3));   //change to logarithms or inverses possibly if bad result

	return (value1*value2*value3);
}

void diffCalculateProbability(double readingCalcium, double readingSalinity, double readingAlkalinity, bool justStarted) {
	double initValue = 1.0/35.0;
	if(justStarted) { fillDiffProbability(initValue); }
	double oldProbability[35];

	double sumNeighbours = 0;
	for(int i=0;i<35;i++) {
		oldProbability[i] = diffProbability[i];
	}

	fillDiffProbability(0.0);

	for(int i=0;i<35;i++) {
		diffProbability[i]=diffValueProbability(readingCalcium,readingSalinity,readingAlkalinity,calcium[i].first,calcium[i].second,salinity[i].first,salinity[i].second,
			alkalinity[i].first,alkalinity[i].second);
	}

	//normalize:

	double sum = 0;
	for(int i=0;i<35;i++) {
		sum += diffProbability[i];
	}

	for(int i=0;i<35;i++) {
		diffProbability[i] = diffProbability[i]/sum;
	}

	for(int i=0;i<35;i++) {
		int size = paths[i].size();
		std::array<int,35> neighbours;

		double neighboursSum = 0;
		for(int j=0;j<size;j++) {
			int neighbourID = paths[i][j]-1;
			neighboursSum += oldProbability[paths[i][j]-1]/(paths[neighbourID].size()+1);
		}

		neighboursSum += oldProbability[i]/(paths[i].size()+1);

		diffProbability[i] = diffProbability[i] * neighboursSum;

		neighboursSum = 0;
	}

	//normalize again:

	sum = 0;
	for(int i=0;i<35;i++) {
		sum += diffProbability[i];
	}

	for(int i=0;i<35;i++) {
		diffProbability[i] = diffProbability[i]/sum;
	}

}
int _tmain(int argc, _TCHAR* argv[])
{
	std::wstring name = L"Sverrir, Sander, Martin och Malin";
	bool OK;
	while(true) {
		CrocSession session(name, OK);
		paths = session.getPaths();
		for(int i = 0; i < 100; i++){
			bool gameStillGoingOn = true;
			std::fill(std::begin(triedSpots),std::end(triedSpots),false);
			session.StartGame();
			//session.GetGameState(score, playerLocation, backpacker1Activity, backpacker2Activity, calciumReading, salineReading, alkalinityReading); //run here to get info for accountForBackpackersStart
			//session.GetGameDistributions(calcium, salinity, alkalinity);
			//accountForBackpackersStart();
			while(gameStillGoingOn){
				session.GetGameState(score, playerLocation, backpacker1Activity, backpacker2Activity, calciumReading, salineReading, alkalinityReading);
				int t;

				if(score==0) {

					calcium.clear();
					salinity.clear();
					alkalinity.clear();

					//std::wcout << L"Games played = " << session.getPlayed() << L"\n";
					//std::wcout << L"Get average = " << session.getAverage() << L"\n";

					std::fill(std::begin(triedSpots),std::end(triedSpots),false);
					std::fill(std::begin(triedSpotsTime),std::end(triedSpotsTime),-1);
					session.GetGameDistributions(calcium, salinity, alkalinity);

					accountForBackpackersStart();
				}
				//calculateProbability(calciumReading, salineReading, alkalinityReading);

				
				//std::wcout << L"Calcium = " << calcium[0].first << "\n";

				if(score==0) {
					diffCalculateProbability(calciumReading,salineReading,alkalinityReading,true);
				} else {
					diffCalculateProbability(calciumReading,salineReading,alkalinityReading,false);
				}

				//std::wcout << L"Calcium = " << calcium[0].first << "\n";

				accountForBackpackersDuring();

				breadthFirstSearch(playerLocation-1);
				makeFastPath(playerLocation-1);

				//for(int k=0;k<35;k++) {
				//	if(triedSpotsTime[k]<score+10) {
				//		triedSpots[k]=0;
				//	}
				//	if(triedSpots[k]) {    //makes a player not look at a certain already inspected spot unless it's been a long time since it was inspected
				//		probability[k]=0;
				//		//diffProbability[k]=0;
				//	}
				//}

				//auto maxElement = std::max_element(std::begin(probability),std::end(probability));
				//int maxIndex = std::distance(std::begin(probability),maxElement);

				auto maxElement = std::max_element(std::begin(diffProbability),std::end(diffProbability));
				int maxIndex = std::distance(std::begin(diffProbability),maxElement);

				//std::wcout << L"Score: " << score << L"\n";

				///std::wcout << L"probability[maxIndex] = " << diffProbability[maxIndex] << L"\n";
				//std::wcout << "salineReading = " << salineReading << ", calciumReading = " << calciumReading << ", alkalinityReading = " << alkalinityReading << "\n";
				//std::wcout << "salineReading[maxIndex] = " << salinity[maxIndex].first << ", calciumReading[maxIndex] = " << calcium[maxIndex].first << ", alkReading[maxIndex] = " << alkalinity[maxIndex].first << "\n"  << "\n";

				//std::wcout << L"Location: " << playerLocation-1 << L"\n";
				//std::wcout << L"Goal: " << maxIndex << L"\n";

				//for(int i=0;i<fastPath.size();i++) {
				//	std::wcout << L"fastPath[" << i << L"] = " << fastPath[i] << L"\n";
				//	if(i==fastPath.size()-1) std::wcout << L"\n";
				//}

				if(fastPath.size()==1) {
					std::wstring s = L"S";
					_ULonglong theMove = maxIndex+1;
					std::wstring theRealMove = L"" + std::to_wstring(theMove);
					gameStillGoingOn = session.makeMove(s,theRealMove,score);
					triedSpots[maxIndex]=true;
					triedSpotsTime[maxIndex]=score;

				} else if(fastPath.size()==2) {
					std::wstring s = L"S";
					_ULonglong theMove = maxIndex+1;
					std::wstring theRealMove = L"" + std::to_wstring(theMove);
					gameStillGoingOn = session.makeMove(theRealMove,s,score);
					triedSpots[maxIndex]=true;
					triedSpotsTime[maxIndex]=score;

				} else if(fastPath.size()>2) {
					_ULonglong theMove = fastPath[fastPath.size()-2]+1;
					std::wstring firstMove = L"" + std::to_wstring(theMove);
					theMove = fastPath[fastPath.size()-3]+1;
					std::wstring secondMove = L"" + std::to_wstring(theMove);
					gameStillGoingOn = session.makeMove(firstMove,secondMove,score);
				}

				length.clear();
				parent.clear();

				//if(session.getAverage() > 110) {
				//	std::wcout << L"Games finished before reboot = " << session.getPlayed() << L"\n";
				//	session.ClearRecord();
				//	i=0;
				//}
				//Sleep(1000);
			}
		}
		session.PostResults();
		std::wcout << L"Average: " << session.getAverage() << "\n";
		totalGamesPlayed++;

		}
	return 0;
	}
