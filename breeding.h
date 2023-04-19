#include <iostream>
#include <random>
#include <vector>
using namespace std;

// 적합도 상위 10개 개체 중 두 개 선택
void breeding(char A[], char B[])
{
	const int ARRAY_SIZE = 200;
	const int NUM_EXCHANGES = 100;

	vector<int> detectIndex_a, detectIndex_b;
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> rand(0, ARRAY_SIZE - 1);

	char half_A[NUM_EXCHANGES], half_B[NUM_EXCHANGES];
	char a[ARRAY_SIZE], b[ARRAY_SIZE];

	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		a[i] = A[i];
		b[i] = B[i];
	}

	int randIndex;

	for (int i = 0; i < NUM_EXCHANGES; i++)
	{
		do {
			randIndex = rand(gen);
		} while (find(detectIndex_a.begin(), detectIndex_a.end(), randIndex) != detectIndex_a.end());

		detectIndex_a.push_back(randIndex);

		half_A[i] = A[randIndex];
		b[randIndex] = half_A[i];
	}

	for (int i = 0; i < NUM_EXCHANGES; i++)
	{
		do {
			randIndex = rand(gen);
		} while (find(detectIndex_b.begin(), detectIndex_b.end(), randIndex) != detectIndex_b.end());

		detectIndex_b.push_back(randIndex);

		half_B[i] = B[randIndex];
		a[randIndex] = half_B[i];
	}
}