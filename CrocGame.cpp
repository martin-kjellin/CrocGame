#include "stdafx.h"
#include <iostream>
#include <string>
#include <array>
#include <set>
#include <math.h>
#include "CrocGame.h"  
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

/////
class Waterhole {
public:
	int id;
	int cost;
	int estCost;
	Waterhole *parent;

	Waterhole() {
		id = 0;
		cost = 0;
		estCost = 0;
		parent = NULL;
	}

	Waterhole(int id_, int pathCost, Waterhole* parent_){
		id = id_;
		cost = pathCost;
		estCost = 1;
		parent = parent_;
	}

	bool operator() (const Waterhole& n1, const Waterhole& n2) const {
		return n1.estCost < n2.estCost;
	}
};

// Help function to aStar, returns the path to node
std::vector<int> solution(Waterhole *node, int start) {
	std::vector<int> solutionPath;
	while ((*node).id != start) {
		int currentLocation = (*node).id;
		solutionPath.insert(solutionPath.begin(), currentLocation);
		Waterhole *oldNode = node;
		node = (*node).parent;
		delete oldNode;
	}
	return solutionPath;
}


std::vector<int> aStar(int start, int end){
	Waterhole startWaterhole(start, 0, NULL);
	std::multiset<Waterhole, Waterhole> frontier;
	frontier.insert(startWaterhole);
	std::set<int> explored;
	std::vector<int> returnValue;
	while (true) {
		if (frontier.empty()) { // No solution to the problem was found (this should never happen)
			break;
		} else {
			// Get the lowest-cost node of the frontier
			std::multiset<Waterhole, Waterhole>::iterator lowestIterator = frontier.begin();
			Waterhole *waterhole = new Waterhole;
			*waterhole = *lowestIterator;
			frontier.erase(lowestIterator);

			if ((*waterhole).id == end) { // The goal is reached
				std::vector<int> solutionNodes = solution(waterhole, start);
				for (std::vector<int>::iterator it = solutionNodes.begin(); (it /*+ 1*/) != solutionNodes.end(); it++) {
					returnValue.push_back(*it);
				}
				break;

			} else {
				int current = (*waterhole).id;
				explored.insert(current);
				int i;
				for(i = 0; i < paths[current - 1].size(); i++){
					int neighbour = paths[current-1][i];

					int pathCostToNode = (*waterhole).cost + 1;
					Waterhole insertNode(neighbour, pathCostToNode, waterhole);
					bool frontierHasInsertNode = false;
					for (std::multiset<Waterhole, Waterhole>::iterator it = frontier.begin(); it != frontier.end(); it++) {
						Waterhole currentNode = *it;
						if (currentNode.id == insertNode.id) {
							frontierHasInsertNode = true;
						}
					}
					if (!frontierHasInsertNode) {
						if (!explored.count(insertNode.id)) {
							frontier.insert(insertNode);
						}
					} else {
						std::multiset<Waterhole, Waterhole>::iterator findIterator = frontier.find(insertNode);
						if (findIterator != frontier.end()) {
							Waterhole oldInsertNode = *findIterator;
							if (oldInsertNode.cost > insertNode.cost) {
								frontier.erase(findIterator);
								frontier.insert(insertNode);
							}
						}
					}
				}
			}
		}
	}
	return returnValue;
}

////////////////






//probability: fill with 1/35
double probability[35] = {}; //index 0 is waterhole 1

void fillProbability(double value){
	int i;
	for(i = 0; i < 35; i++){
		probability[i] = value;
	}
}

//sets the start probabilities while accounting for backpackers
void accountForBackpackersStart(){
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
		if(backpacker1Activity == 0 && backpacker2Activity == 0){
			fillProbability(1.0/35.0);
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


//return "probability" for reading given mean and stard deviation
double valueProbability(double reading, double mean, double std_dev){
	//normal distribution -> pdf 
	double p;
	double pi = 3.14159265358979323846264338;

	p = (1 / (std_dev * sqrt(2* pi))) * exp(-((reading-mean)*(reading-mean))/(2*std_dev*std_dev));
	return p;
}

//calculates the probability for croc being at each waterhole and saves it in probability. 
void calculateProbability (double readingCalcium, double readingSalinity, double readingAlkalinity){
	int c, i, j, t;

	double newProbability[4][35] = {{},{},{},{}};
	for(i=0; i < 35; i++){
		newProbability[3][i] = (1.0 / (paths[i].size() + 1)) * probability[i]; //P(croc stay at the waterhole)
		for(j=0; j < paths[i].size(); j++){
			int adjNode = paths[i][j];
			newProbability[3][i] += (1.0 / (paths[adjNode-1].size() + 1)) * probability[adjNode-1]; //P(Xt+1 | Xt)* P(Xt | E1:t)  //P(croc comes from any adjecent waterhole)
		}

	}

	for(c = 0; c < 3; c++){ //0 = calcium, 1 = salinity, 2 = alkalinity 
		for(i=0; i < 35; i++){

			//get the mean and standard deviation
			double mean;
			double std_dev;
			double dataProb;
			switch(c){
			case 0:
				mean = calcium[i].first;
				std_dev = calcium[i].second;
				dataProb =  valueProbability(readingCalcium, mean, std_dev);    // P(Et+1 | X)
				break;
			case 1:
				mean = salinity[i].first;
				std_dev = salinity[i].second;
				dataProb =  valueProbability(readingSalinity, mean, std_dev);    // P(Et+1 | X)
				break;
			case 2:
				mean = alkalinity[i].first;
				std_dev = alkalinity[i].second;
				dataProb =  valueProbability(readingAlkalinity, mean, std_dev);    // P(Et+1 | X)
				break;
			}

			newProbability[c][i] = dataProb;
		}
	}
	
	for(c=0;c<3;c++){
		double sumisum = 0;
	for(t = 0; t < 35; t++){
		sumisum += newProbability[c][i];
	}
	for(t = 0; t < 35; t++){
		newProbability[c][i] = newProbability[c][i] / sumisum;
	}

	}
	
	for(t = 0; t < 35; t++){
		//multiply the probabilities given the different observations
		probability[t] = newProbability[0][t] * newProbability[1][t] * newProbability[2][t] * newProbability[3][t];
	}
	//normalize
	double sum = 0;
	for(t = 0; t < 35; t++){
		sum += probability[t];
	}

	for(t = 0; t < 35; t++){
		probability[t] = probability[t] / sum;
	}
}

int score2= 0;
int _tmain(int argc, _TCHAR* argv[])
{
	std::wstring name = L"Sverrir, Sander, Martin och Malin";
	bool OK;
	CrocSession session(name, OK);
	paths = session.getPaths();
	while(true){
		session.ClearRecord();
		for(int i = 0; i < 100; i++){
			bool gameStillGoingOn = true;
			score2 = 0;

			session.StartGame();
			

			while(gameStillGoingOn){

				if(score2 == 0){
					
					session.GetGameState(score, playerLocation, backpacker1Activity, backpacker2Activity, calciumReading, salineReading, alkalinityReading); //run here to get info for accountForBackpackersStart
					calcium.clear();
					salinity.clear();
					alkalinity.clear();
					session.GetGameDistributions(calcium, salinity, alkalinity);
					accountForBackpackersStart();
					std::wstring playerMove = L"S";
					std::wstring playerMove2 = L"S";
					gameStillGoingOn = session.makeMove (playerMove ,playerMove2, score2);	
				}
				else{
					session.GetGameState(score, playerLocation, backpacker1Activity, backpacker2Activity, calciumReading, salineReading, alkalinityReading);
					calculateProbability(calciumReading, salineReading, alkalinityReading);
					accountForBackpackersDuring();
					double maxValue = 0.0;
				int index, maxIndex; // index of min in probability - 1 
				for(index = 0; index < 35; index++){
					if(probability[index] > maxValue){
						maxValue = probability[index];
						maxIndex = index;
					}
				}

				std::vector<int> path = aStar(playerLocation, maxIndex + 1);
				_ULonglong theMove1;
				_ULonglong theMove2;
				std::wstring playerMove;
				std::wstring playerMove2;
				if(path.size() > 1){
					theMove1 = path.front();
					theMove2 = path.at(1);

					playerMove  = L"" + std::to_wstring(theMove1);
					playerMove2 = L"" + std::to_wstring(theMove2);
				}
				else if(path.size() > 0){
					theMove1 = path.front();
					playerMove  = L"" + std::to_wstring(theMove1);
					playerMove2 = L"S";
				}
				else{
					playerMove = L"S";
					playerMove2 = L"S";
				}



				/*std::wcout << L"score: "<< score << L"\n";
				std::wcout << L"playerLocation: "<< playerLocation << L"\n";
				std::wcout << L"backpacker1Activity: "<< backpacker1Activity << L"\n";
				std::wcout << L"backpacker2Activity: "<< backpacker2Activity << L"\n";
				std::wcout << L"calciumReading: "<< calciumReading << L"\n";
				std::wcout << L"salineReading: "<< salineReading << L"\n";
				std::wcout << L"alkalinityReading: "<< alkalinityReading << L"\n";
				int wait;
				std::cin >> wait;
				*/

				/*move to random location*/
				/*int size = paths[playerLocation-1].size();
				int random = rand() % size;

				_ULonglong theMove = paths[playerLocation-1][random];*/


				gameStillGoingOn = session.makeMove (playerMove ,playerMove2, score2);	
				}

				
			}
			//std::wcout << L"score: " << score2 << "\n"; //
		}
		session.PostResults();
		std::wcout << L"Average: " << session.getAverage() << "\n"; //
	}
	while(true); 
	return 0;
}
