#include "stdafx.h"
#include <iostream>
#include <string>
#include <array>
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


//probability: fill with 1/35
double probability[35] = {}; //index 0 is waterhole 1

//return probability for reading given mean and stard deviation
double valueProbability(double reading, double mean, double std_dev){
	//normal distribution -> pdf 
	double p;
	double pi = 3.141592653589793;

	p = (1 / (std_dev * sqrt(2* pi))) * exp(-((reading-mean)*(reading-mean))/(2*std_dev*std_dev));
	return p;
}

//calculates the probability for croc being at each waterhole and saves it in probability. 
void calculateProbability (double readingCalcium, double readingSalinity, double readingAlkalinity){
	int c, i, j, t;
	double oldProbability[35];	//put the values of probability in oldProbability
	memcpy(probability, oldProbability, 35);

	double newProbability[3][35] = {{},{},{}};
	for(c = 0; c < 3; c++){ //0 = calcium, 1 = salinity, 2 = alkalinity 
		for(i=0; i < 35; i++){
			for(j=0; j < paths[i].size(); j++){
				newProbability[c][i] += (1.0 / (paths[j-1].size() + 1)) * oldProbability[j-1]; //P(Xt+1 | Xt)* P(Xt | E1:t)
			}
			newProbability[c][i] += (1.0 / (paths[i-1].size() + 1)) * oldProbability[i]; //P(croc stay at the waterhole)

			//get the mean and standard deviation
			double mean;
			double std_dev;
			double dataProb;
			switch(c){
			case 0:
				mean = calcium[i].first;
				std_dev = calcium[i].second;
				dataProb =  valueProbability(readingCalcium, mean, std_dev);    // P(Et+1 | X)
			case 1:
				mean = salinity[i].first;
				std_dev = salinity[i].second;
				dataProb =  valueProbability(readingSalinity, mean, std_dev);    // P(Et+1 | X)
			case 2:
				mean = alkalinity[i].first;
				std_dev = alkalinity[i].second;
				dataProb =  valueProbability(readingAlkalinity, mean, std_dev);    // P(Et+1 | X)
			}

			newProbability[c][i] *= dataProb;
		}
	}

	for(t = 0; t < 35; t++){
		//multiply the probabilities given the different observations
		probability[t] = newProbability[0][t] * newProbability[1][t] * newProbability[2][t];
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


int _tmain(int argc, _TCHAR* argv[])
{
	std::wstring name = L"Sverrir, Sander, Martin och Malin";
	bool OK;
	CrocSession session(name, OK);
	paths = session.getPaths();

	for(int i = 0; i < 100; i++){
		bool gameStillGoingOn = true;

		session.StartGame();
		int t;
		for(t = 0; t <35; t++){
			probability[t] = 1.0/35.0;	 //fill probability with prob 1/35 ... ta bort hitchhikers position?
		}
		while(gameStillGoingOn){
			session.GetGameState(score, playerLocation, backpacker1Activity, backpacker2Activity, calciumReading, salineReading, alkalinityReading);
			session.GetGameDistributions(calcium, salinity, alkalinity);

			

			/*int i;
			for(i = 0; i < 35; i++){
			std::wcout << i << L": calcium mean: " << calcium[i].first << "    calcium std dev: " << calcium[i].second << "\n";
			}
			*/

			/*for(t = 0; t < 35; t++){
				std::wcout << t +1 << ": " << probability[t] << "\n";
			}
			int wait;
			std::cin >> wait;
			*/
			calculateProbability(calciumReading, salineReading, alkalinityReading);
			
			/*
			std::wcout << L"score: "<< score << L"\n";
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
			int size = paths[playerLocation-1].size();
			int random = rand() % size;

			_ULonglong theMove = paths[playerLocation-1][random];
			std::wstring playerMove  = L"" + std::to_wstring(theMove);
			std::wstring playerMove2 = L"S";

			gameStillGoingOn = session.makeMove (playerMove ,playerMove2, score);

		}
	}
	session.PostResults();
	std::wcout << L"Average: " << session.getAverage() << "\n"; //
	while(true); 
	return 0;
}

