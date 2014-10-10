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

bool triedSpots[35];
int triedSpotsTime[35];

std::map<int,int> length;
std::map<int,int> parent;
std::vector<int> fastPath;

bool once = true;

void makeFastPath(int startLocation) {
	fastPath.clear();
	auto maxElement = std::max_element(std::begin(probability),std::end(probability));
	int maxIndex = std::distance(std::begin(probability),maxElement);
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

void fillProbability(double value){
	int i;
	for(i = 0; i < 35; i++){
		probability[i] = value;
	}
}

//sets the start probabilities while accounting for backpackers
void accountForBackpackersStart(){
	int t;
	if(backpacker1Activity > 0 && backpacker2Activity > 0){
		if(backpacker1Activity != backpacker2Activity){ //both alive and at different places
			fillProbability(1.0/33.0);
			probability[backpacker1Activity-1] = 0;
			probability[backpacker2Activity-1] = 0;
		}
		else{ //both alive at the same place
			fillProbability(1.0/34.0);
			probability[backpacker1Activity-1] = 0;
		}
	}
	else{ //one or both are being eaten / have been eaten
		if(backpacker1Activity < 0){
			fillProbability(0.0);
			probability[abs(backpacker1Activity)-1] = 1.0;
		}
		if(backpacker2Activity < 0){
			fillProbability(0.0);
			probability[abs(backpacker2Activity)-1] = 1.0;
		}
	}
}

//change the probability depending on backpackers
void accountForBackpackersDuring(){
	if(backpacker1Activity > 0){
		probability[backpacker1Activity-1] = 0.0;	
	}
	if(backpacker2Activity > 0){
		probability[backpacker2Activity-1] = 0.0;	
	}
	if(backpacker1Activity < 0){
		fillProbability(0.0);
		probability[abs(backpacker1Activity)-1] = 1.0;
	}
	if(backpacker2Activity < 0){
		fillProbability(0.0);
		probability[abs(backpacker2Activity)-1] = 1.0;
	}
}
//return probability for reading given mean and stard deviation
double valueProbability(double reading, double mean, double std_dev){
	//normal distribution -> pdf
	double p;
	double pi = 3.141592653589793;
	p = (1 / (std_dev * sqrt(2* pi))) * exp(-((reading-mean)*(reading-mean))/(2*std_dev*std_dev));
	return p;
}

//calculates the probability for croc being at each waterhole and saves it in probability.
void calculateProbability (double readingCalcium, double readingSalinity, double readingAlkalinity) {
  double newProbability[35] = {};
  for (int i = 0; i < 35; i++) {
    for (int j = 0; j < paths[i].size(); j++) {
      int adjNode = paths[i][j];
      newProbability[i] += (1.0 / (paths[adjNode-1].size() + 1)) * probability[adjNode-1]; // Croc comes from an adjacent waterhole
    }
    newProbability[i] += (1.0 / (paths[i].size() + 1)) * probability[i]; // Croc stays at the same waterhole

    double mean;
    double std_dev;
    double dataProb;

    for (int c = 0; c < 3; c++) { //0 = calcium, 1 = salinity, 2 = alkalinity
      switch (c) {
      case 0:
	mean = calcium[i].first;
	std_dev = calcium[i].second;
	dataProb = valueProbability(readingCalcium, mean, std_dev);
	break;
      case 1:
	mean = salinity[i].first;
	std_dev = salinity[i].second;
	dataProb = valueProbability(readingSalinity, mean, std_dev);
	break;
      case 2:
	mean = alkalinity[i].first;
	std_dev = alkalinity[i].second;
	dataProb = valueProbability(readingAlkalinity, mean, std_dev);
	break;
      }
      newProbability[i] *= dataProb;
    }
  }

  for (int i = 0; i < 35; i++) {
    probability[i] = newProbability[i];
  }

  //normalize
  double sum = 0;
  for (int i = 0; i < 35; i++) {
    sum += probability[i];
  }
  for(int i = 0; i < 35; i++){
    probability[i] = probability[i] / sum;
  }
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::wstring name = L"Sverrir, Sander, Martin och Malin";
	bool OK;
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
				std::fill(std::begin(triedSpots),std::end(triedSpots),false);
				std::fill(std::begin(triedSpotsTime),std::end(triedSpotsTime),-1);
				session.GetGameDistributions(calcium, salinity, alkalinity);
				accountForBackpackersStart();
			}
			calculateProbability(calciumReading, salineReading, alkalinityReading);
			accountForBackpackersDuring();

			breadthFirstSearch(playerLocation-1);
			makeFastPath(playerLocation-1);

			for(int i=0;i<35;i++) {
				if(triedSpotsTime[i]<score+10) {
					triedSpots[i]=0;
				}
				if(triedSpots[i]) {    //makes a player not look at a certain already inspected spot unless it's been a long time since it was inspected
					probability[i]=0;
				}
			}

			auto maxElement = std::max_element(std::begin(probability),std::end(probability));
			int maxIndex = std::distance(std::begin(probability),maxElement);

			std::wcout << L"Score: " << score << L"\n";

			std::wcout << L"Location: " << playerLocation-1 << L"\n";
			std::wcout << L"Goal: " << maxIndex << L"\n";

			std::wcout << L"probability[maxIndex] = " << probability[maxIndex] << L"\n";

			for(int i=0;i<fastPath.size();i++) {
				std::wcout << L"fastPath[" << i << L"] = " << fastPath[i] << L"\n";
				if(i==fastPath.size()-1) std::wcout << L"\n";
			}

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

				Sleep(3000);
		}
	}
	session.PostResults();
	std::wcout << L"Average: " << session.getAverage() << "\n"; //
	while(true);
	return 0;
}
