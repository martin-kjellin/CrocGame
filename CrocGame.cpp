#include "stdafx.h"
#include <iostream>
#include <string>
#include <array>
#include "CrocGame.h"  
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
double probability[4][35] = {{},{},{},{}}; //calcium, salinity, alkalinity, total

//return probability for reading given mean and stard deviation
double valueProbability(double reading, double mean, double std_dev){
	return 0;
}

void calculateProbability (double reading){
	int c, i, j, t;
	for(c = 0; c < 3; c++){
		double oldProbability[35];// = probability[c]; //put the values of probability in oldProbability
		
		memcpy(probability[c], oldProbability, 35);

		for(t = 0; t <35; t++){
			probability[c][t] = 0;//probability[c].fill(0); //fill with zeros
		}

		for(i=0; i < 35; i++){
			for(j=0; j < paths[i].size(); j++){
				probability[c][i] += (1.0 / paths[j-1].size()) * oldProbability[j-1]; //P(Xt+1 | Xt)* P(Xt | E1:t)
			}

			//get the mean and standard deviation
			double mean;
			double std_dev;
			switch(c){
			case 0:
				mean = calcium[i].first;
				std_dev = calcium[i].second;
			case 1:
				mean = salinity[i].first;
				std_dev = salinity[i].second;
			case 2:
				mean = alkalinity[i].first;
				std_dev = alkalinity[i].second;
			}

			double dataProb =  valueProbability(reading, mean, std_dev);    // P(Et+1 | X)
			probability[c][i] *= dataProb;
		}
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
		while(gameStillGoingOn){
			session.GetGameState(score, playerLocation, backpacker1Activity, backpacker2Activity, calciumReading, salineReading, alkalinityReading);
			session.GetGameDistributions(calcium, salinity, alkalinity);

			/*int i;
			for(i = 0; i < 35; i++){
			std::wcout << i << L": calcium mean: " << calcium[i].first << "    calcium std dev: " << calcium[i].second << "\n";
			}
			*/
			/*
			std::wcout << L"score: "<< score << L"\n";
			std::wcout << L"playerLocation: "<< playerLocation << L"\n";
			std::wcout << L"backpacker1Activity: "<< backpacker1Activity << L"\n";
			std::wcout << L"backpacker2Activity: "<< backpacker2Activity << L"\n";*/
			std::wcout << L"calciumReading: "<< calciumReading << L"\n";
			/*std::wcout << L"salineReading: "<< salineReading << L"\n";
			std::wcout << L"alkalinityReading: "<< alkalinityReading << L"\n";
			*/
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

