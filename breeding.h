#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <omp.h>
using namespace std;

const int GENE_SIZE = 50;
const int ARRAY_SIZE = 201;

vector<vector<int>> selectTop(vector<vector<int>>& arr)
{
	int size = 201;
	sort(arr.begin(), arr.end(), [](const vector<int>& a, const vector<int>& b) {return a[0] > b[0]; });

	vector<vector<int>> top_arrays(10, vector<int>(size));
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < size; j++)
		{
			top_arrays[i][j] = arr[i][j];

		}
		int len = sizeof(top_arrays[i]) / sizeof(int);
		if (len < size)
		{
			size = len;
			top_arrays[i].resize(size);
		}
	}

	return top_arrays;
}

int** breeding_serial(vector<int*>& top_arrays)
{
	const int NUM_EXCHANGES = 100;
	int half_A[NUM_EXCHANGES], half_B[NUM_EXCHANGES];
	int a[ARRAY_SIZE], b[ARRAY_SIZE];
	int** c = new int* [GENE_SIZE];
	for (int i = 0; i < GENE_SIZE; i++)
		c[i] = new int[ARRAY_SIZE];

	vector<int> detectIndex_a, detectIndex_b;
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> rand(1, ARRAY_SIZE - 1);
	uniform_int_distribution<int> toprand(0, top_arrays.size());


	int idx1 = toprand(gen);
	int idx2 = toprand(gen);
	while (idx2 == idx1)
	{
		idx2 = toprand(gen);
	}

	int A[ARRAY_SIZE], B[ARRAY_SIZE];
	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		A[i] = top_arrays[idx1][i];
		B[i] = top_arrays[idx2][i];
	}

	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		a[i] = A[i];
		b[i] = B[i];
	}

	int randIndex;
	int childcount = 0;

	for (int count = 0; count < 25; count++)
	{
		for (int i = 0; i < NUM_EXCHANGES; i++)
		{
			do {
				randIndex = rand(gen);
			} while (find(detectIndex_a.begin(), detectIndex_a.end(), randIndex) != detectIndex_a.end());

			detectIndex_a.push_back(randIndex);

			half_A[i] = A[randIndex];
			b[randIndex] = half_A[i];
			c[childcount][i] = b[i];
		}
		childcount++;
		detectIndex_a.clear();

		for (int i = 0; i < NUM_EXCHANGES; i++)
		{
			do {
				randIndex = rand(gen);
			} while (find(detectIndex_b.begin(), detectIndex_b.end(), randIndex) != detectIndex_b.end());

			detectIndex_b.push_back(randIndex);

			half_B[i] = B[randIndex];
			a[randIndex] = half_B[i];
			c[childcount][i] = a[i];
		}
		childcount++;
		detectIndex_b.clear();
			
		if (childcount == 49)
			break;
	}

	return c;
}

int** breeding_parallel(vector<int*>& top_arrays)
{
	const int NUM_EXCHANGES = 100;
	int half_A[NUM_EXCHANGES], half_B[NUM_EXCHANGES];
	int a[ARRAY_SIZE], b[ARRAY_SIZE];
	int** c = new int* [GENE_SIZE];
	for (int i = 0; i < GENE_SIZE; i++)
		c[i] = new int[ARRAY_SIZE];

	vector<int> detectIndex_a, detectIndex_b;
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> rand(1, ARRAY_SIZE - 1);
	uniform_int_distribution<int> toprand(0, top_arrays.size());


	int idx1 = toprand(gen);
	int idx2 = toprand(gen);
	while (idx2 == idx1)
	{
		idx2 = toprand(gen);
	}

	int A[ARRAY_SIZE], B[ARRAY_SIZE];
	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		A[i] = top_arrays[idx1][i];
		B[i] = top_arrays[idx2][i];
	}

	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		a[i] = A[i];
		b[i] = B[i];
	}

	int randIndex;
	int childcount = 0;

	#pragma omp_parallel num_threads(4)
	{

		for (int count = 0; count < 25; count++)
		{
			#pragma omp for
			for (int i = 0; i < NUM_EXCHANGES; i++)
			{
				do {
					randIndex = rand(gen);
				} while (find(detectIndex_a.begin(), detectIndex_a.end(), randIndex) != detectIndex_a.end());

				detectIndex_a.push_back(randIndex);

				half_A[i] = A[randIndex];
				b[randIndex] = half_A[i];
				c[childcount][i] = b[i];
			}
			childcount++;
			detectIndex_a.clear();

			#pragma omp for
			for (int i = 0; i < NUM_EXCHANGES; i++)
			{
				do {
					randIndex = rand(gen);
				} while (find(detectIndex_b.begin(), detectIndex_b.end(), randIndex) != detectIndex_b.end());

				detectIndex_b.push_back(randIndex);

				half_B[i] = B[randIndex];
				a[randIndex] = half_B[i];
				c[childcount][i] = a[i];
			}
			childcount++;
			detectIndex_b.clear();

			if (childcount == 49)
				break;
		}
	}

	return c;
}

/* main
vector<vector<int>> top_arrays = selectTop( - );
breeding(top_arrays);
*/
