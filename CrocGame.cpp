#include "stdafx.h"
#include <iostream>
#include <string>
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

			/*
			std::wcout << L"score: "<< score << L"\n";
			std::wcout << L"playerLocation: "<< playerLocation << L"\n";
			std::wcout << L"backpacker1Activity: "<< backpacker1Activity << L"\n";
			std::wcout << L"backpacker2Activity: "<< backpacker2Activity << L"\n";
			std::wcout << L"calciumReading: "<< calciumReading << L"\n";
			std::wcout << L"salineReading: "<< salineReading << L"\n";
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

