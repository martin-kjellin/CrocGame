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
double diffProbability[35] = {};
std::map<int,double> probMap;

bool triedSpots[35];
int triedSpotsTime[35];

std::map<int,int> length;
std::map<int,int> parent;
std::vector<int> fastPath;

bool once = true;

double totalScore = 0;
int totalGamesPlayed = 0;
std::vector<double> allScores;
double bestScoreSoFar = 100;

void makeDiffMap() {
	probMap.clear();
	for(int i=0;i<35;i++) {
		probMap[0]=diffProbability[i];
	}
}

void makeFastPath(int startLocation,int maxIndex) {
	fastPath.clear();

	//auto maxElement = std::max_element(std::begin(diffProbability),std::end(diffProbability));
	//int maxIndex = std::distance(std::begin(diffProbability),maxElement);

	int currentIndex=maxIndex;

	fastPath.push_back(currentIndex);

	while(parent.count(currentIndex)>0) {
		fastPath.push_back(parent[currentIndex]);
		currentIndex = parent[currentIndex];
	}
}

void breadthFirstSearch(int currentLocation) {

	if(currentLocation==playerLocation-1) {  //starting place
		length[currentLocation] = 0;

		for(int i=0;i<paths[currentLocation].size();i++) {
		}


		for(int i=0;i<paths[currentLocation].size();i++) {
			int currentNode = paths[currentLocation][i]-1;
			parent[currentNode]=currentLocation;
			length[currentNode]=length[parent[currentNode]]+1;
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
				}
			} else {
				parent[currentNode]=currentLocation;
				length[currentNode]=length[parent[currentNode]]+1;
				breadthFirstSearch(currentNode);
			}
		}
	}
}

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
		while(true){
			
			if(session.getPlayed()==100) break;

			bool gameStillGoingOn = true;
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
					session.GetGameDistributions(calcium, salinity, alkalinity);

					accountForBackpackersStart();
				}

				if(score==0) {
					diffCalculateProbability(calciumReading,salineReading,alkalinityReading,true);
				} else {
					diffCalculateProbability(calciumReading,salineReading,alkalinityReading,false);
				}

				accountForBackpackersDuring();

				auto maxElement = std::max_element(std::begin(diffProbability),std::end(diffProbability));
				int maxIndex = std::distance(std::begin(diffProbability),maxElement);

				//find max as double, not double*

				double maxProb=0;
				for(int i=0;i<35;i++) {
					if(diffProbability[i]>maxProb) {
						maxProb=diffProbability[i];
					}
				}

				if(maxProb<0.15 && ((playerLocation-1)!= maxIndex)) {
					int length1 = length[31];
					int length2 = length[32];
					int length3 = length[33];

					if(length1<length2 && length1<length3) {
						maxIndex = 31;
					}

					if(length2<length1 && length2<length3) {
						maxIndex = 32;
					}

					if(length3<length1 && length3<length2) {
						maxIndex = 33;
					}
				}

				breadthFirstSearch(playerLocation-1);
				makeFastPath(playerLocation-1,maxIndex);

				if(fastPath.size()==1) {

					diffProbability[maxIndex]=0.0;

					//makeDiffMap();

					////maxIndex=paths[playerLocation-1][0]-1;
					////int neighbourSize=paths[playerLocation-1].size();
					////for(int i=0;i<neighbourSize;i++) {
					////	if (probMap[(paths[playerLocation-1][i])-1]>probMap[maxIndex]) {
					////		maxIndex=paths[playerLocation-1][i]-1;
					////	}
					////}

					std::wstring s = L"S";
					_ULonglong theMove = maxIndex+1;
					std::wstring theRealMove = L"" + std::to_wstring(theMove);
					gameStillGoingOn = session.makeMove(s,theRealMove,score);

					diffProbability[maxIndex]=0.0;

				} else if(fastPath.size()==2) {
					std::wstring s = L"S";
					_ULonglong theMove = maxIndex+1;
					std::wstring theRealMove = L"" + std::to_wstring(theMove);
					gameStillGoingOn = session.makeMove(theRealMove,s,score);

					diffProbability[maxIndex]=0.0;

				} else if(fastPath.size()>2) {
					_ULonglong theMove = fastPath[fastPath.size()-2]+1;
					std::wstring firstMove = L"" + std::to_wstring(theMove);
					theMove = fastPath[fastPath.size()-3]+1;
					std::wstring secondMove = L"" + std::to_wstring(theMove);
					gameStillGoingOn = session.makeMove(firstMove,secondMove,score);
				}

				length.clear();
				parent.clear();

				int playedGames = session.getPlayed();
				double avrg = session.getAverage();

				if((playedGames>70 && (session.getAverage())>13.5) || (playedGames>40 && (session.getAverage())>14) || (playedGames>20 && (session.getAverage()>16)) || (playedGames>10 && (session.getAverage()>19))) {
					std::wcout << L"Abrt at game " << session.getPlayed() << ", avrg score " << session.getAverage() << ", best score so far: " << bestScoreSoFar << "\n";
					session.ClearRecord();
				}
			}
		}
		session.PostResults();
		std::wcout << L"Average: " << session.getAverage() << "\n";
		if(session.getAverage()<bestScoreSoFar) {
			bestScoreSoFar = session.getAverage();
		}
		totalGamesPlayed++;
		totalScore += session.getAverage();
		allScores.push_back(session.getAverage());

	//	if(totalGamesPlayed % 5 == 0) {
			//double mean = totalScore/totalGamesPlayed;
			//std::wcout << L"Average final score so far: " << totalScore/totalGamesPlayed << "\n";

			////std dv
			//double totalSum=0;
			//for(int i=0;i<totalGamesPlayed;i++) {
			//	totalSum += (allScores[i]-mean)*(allScores[i]-mean);
			//}
			//totalSum = totalSum/(totalGamesPlayed-1);
			//totalSum = sqrt(totalSum);
			//std::wcout << L"Standard deviation so far: " << totalSum << "\n";
	//	}

		}
	return 0;
	}
