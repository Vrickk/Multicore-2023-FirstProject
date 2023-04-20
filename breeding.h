#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <omp.h>
using namespace std;

const int GENE_SIZE = 50;
const int ARRAY_SIZE = 201;

// 적합도 상위 10개 개체 선정 -> top_arrays에 순서대로 넣음
vector<int*> selectTopArrays(int arr[GENE_SIZE][ARRAY_SIZE])
{
	sort(arr, arr + 50, [](const int* a, const int* b) {return a[0] > b[0]; });

	vector<int*> top_arrays;
	for (int i = 0; i < 10; i++)
	{
		top_arrays.push_back(arr[i]);
	}

	return top_arrays;
}

void breeding(vector<int*>& top_arrays)
{
	const int NUM_EXCHANGES = 100;
	int half_A[NUM_EXCHANGES], half_B[NUM_EXCHANGES];
	int a[ARRAY_SIZE], b[ARRAY_SIZE];
	int c[GENE_SIZE][ARRAY_SIZE];

	vector<int> detectIndex_a, detectIndex_b;
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> rand(1, ARRAY_SIZE - 1);
	uniform_int_distribution<int> toprand(0, top_arrays.size());

	// top_arrays에서 무작위로 두 개의 배열 선택
	int idx1 = toprand(gen);
	int idx2 = toprand(gen);
	while (idx2 == idx1)
	{
		idx2 = toprand(gen);
	}

	// 선택된 배열을 각각 A, B에 저장
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
		#pragma omp for
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
	}
	
}

/* main에서
vector<int*> top_arrays = selectTopArrays(배열);
breeding(top_arrays);
*/
