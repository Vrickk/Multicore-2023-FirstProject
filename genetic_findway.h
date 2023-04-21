#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <random>
#include <string>
#include <algorithm>
#include <math.h>
#include <ctime>
#include <omp.h>

#define PLAYER_SPEED 1
#define NUM_MOVES 200
#define DEAD_END_PENALTY 200
#define NUM_PLAYERS 50

#define ROWS 25
#define COLS 25

#define GENERATION_THRESH 50
#define MADEIT_THRESH 10

const int GENE_SIZE = 50;
const int ARRAY_SIZE = 201;

using namespace std;

vector<vector<int>> generateArrays(int numArrays, int arraySize) {
	vector<vector<int>> arrays(numArrays, vector<int>(arraySize + 1, 0));
	srand((unsigned)time(NULL)); // seed the random number generator with the current time
	for (int i = 0; i < numArrays; i++) {
		arrays[i][0] = 0;
		for (int j = 1; j < arraySize + 1; j++) {
			arrays[i][j] = rand() % 4 + 1; // generate random number between 1 and 4
		}
	}

	return arrays;
}

void saveToFile(vector<vector<int>> arr, string filename) {
	ofstream file(filename);

	if (file.is_open()) {
		for (int i = 0; i < arr.size(); i++) {
			for (int j = 0; j < arr[0].size(); j++) {
				file << arr[i][j] << " ";
			}
			file << endl;
		}
		file.close();
	}
}

bool no_way(pair<int, int> pos, vector<vector<char>> maze) {
	int count = 0;
	if (maze[pos.first][pos.second + 1] == '#')
		count++;
	if (maze[pos.first][pos.second - 1] == '#')
		count++;
	if (maze[pos.first + 1][pos.second] == '#')
		count++;
	if (pos.first < 0) {
		if (maze[0][pos.second] == '#')
			count++;
		else if (maze[pos.first - 1][pos.second] == '#')
			count++;
	}


	if (count > 3)
		return true;
	else
		return false;
}

int Evaluate(int suitability, pair<int, int> pos, pair<int, int> next_pos, vector<vector<char>> maze, pair<int, int> goal) {
	double now_length = pow((goal.first - next_pos.first), 2) + pow((goal.second - next_pos.second), 2);
	double next_length = pow((goal.first - next_pos.first), 2) + pow((goal.second - next_pos.second), 2);
	if (maze[next_pos.first][next_pos.second] == 'V' || maze[next_pos.first][next_pos.second] == 'S')
		suitability -= 2;
	else if (now_length > next_length)
		suitability -= 1;
	else if (now_length < next_length)
		suitability += 1;
	else if (no_way(next_pos, maze))
		suitability -= 10;
	else if (next_pos == goal)
		suitability += 30;

	return suitability;
}

const int NUM_OFFSPRING = 50;
const int NUM_NORMAL_OFFSPRING = 45;
const int NUM_MUTATED_OFFSPRING = 5;

vector<vector<int>> breeding(vector<vector<int>>& population)
{
	vector<vector<int>> offspring(NUM_OFFSPRING, vector<int>(population[0].size(), 0));

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> rand(1, population.size() - 1);

	for (int i = 0; i < NUM_NORMAL_OFFSPRING; i += 2) {
		int idx1 = rand(gen);
		int idx2 = rand(gen);
		while (idx2 == idx1) {
			idx2 = rand(gen);
		}

		for (int j = 1; j < population[0].size(); j++) {
			if (rand(gen) % 2 == 0) {
				offspring[i][j] = population[idx1][j];
				offspring[i + 1][j] = population[idx2][j];
			}
			else {
				offspring[i][j] = population[idx2][j];
				offspring[i + 1][j] = population[idx1][j];
			}
		}
	}

	uniform_int_distribution<int> rand_gene(1, 4);
	for (int i = NUM_NORMAL_OFFSPRING; i < NUM_OFFSPRING; i++) {
		int idx = rand(gen);
		for (int j = 1; j < population[0].size(); j++) {
			offspring[i][j] = population[idx][j];
		}
		int mutation_pos = rand(gen) % (population[0].size() - 1) + 1;
		offspring[i][mutation_pos] = rand_gene(gen);
	}

	return offspring;
}

// 적합도 상위 10개 개체 선정 -> top_arrays에 순서대로 넣음
vector<vector<int>> selectTop(vector<vector<int>> arr) {
	const int TOP_SIZE = 10;
	vector<vector<int>> top(TOP_SIZE, vector<int>(arr[0].size()));

	sort(arr.begin(), arr.end(), [](const vector<int>& a, const vector<int>& b) {
		return a[0] > b[0];
		});

	int min_size = arr[0].size();
	for (int i = 0; i < TOP_SIZE; i++) {
		top[i] = arr[i];
		if (arr[i].size() < min_size) {
			min_size = arr[i].size();
		}
	}

	for (int i = 0; i < TOP_SIZE; i++) {
		top[i].resize(min_size);
	}

	arr.erase(arr.begin() + TOP_SIZE, arr.end());

	return top;
}
vector<vector<int>> breeding_serial(vector<vector<int>> top_arrays)
{
	int NUM_EXCHANGES = top_arrays[0].size() / 2;
	vector<int> half_A(NUM_EXCHANGES), half_B(NUM_EXCHANGES);

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> rand(1, top_arrays[0].size() - 1);
	uniform_int_distribution<int> toprand(0, top_arrays.size() - 1);
	uniform_int_distribution<int> mut_gen(0, top_arrays[0].size() - 1);
	uniform_int_distribution<int> mut_arr(1, 4);

	int idx1 = toprand(gen);
	int idx2 = toprand(gen);

	// Exchange random halves of the selected arrays
	for (int i = 0; i < NUM_EXCHANGES; i++) {
		int randIndex = rand(gen);
		half_A[i] = top_arrays[idx1][randIndex];
		half_B[i] = top_arrays[idx2][randIndex];
		top_arrays[idx1][randIndex] = half_B[i];
		top_arrays[idx2][randIndex] = half_A[i];
	}

	// Mutate some genes in the resulting arrays
	vector<vector<int>> new_arrays = top_arrays;
	for (int i = 0; i < 5; i++) {
		int randIndex_mug, randIndex_mua;
		vector<int> detectIndex_mug, detectIndex_mua;
		for (int j = 0; j < 10; j++) {
			do {
				randIndex_mug = mut_gen(gen);
			} while (find(detectIndex_mug.begin(), detectIndex_mug.end(), randIndex_mug) != detectIndex_mug.end());
			detectIndex_mug.push_back(randIndex_mug);

			do {
				randIndex_mua = rand(gen);
			} while (find(detectIndex_mua.begin(), detectIndex_mua.end(), randIndex_mua) != detectIndex_mua.end());
			detectIndex_mua.push_back(randIndex_mua);

			new_arrays[i][randIndex_mug] = mut_arr(gen);
		}
	}

	return new_arrays;
}

vector<vector<int>> breeding_parallel(vector<int*>& top_arrays)
{
	const int NUM_EXCHANGES = 100;
	vector<int> half_A(NUM_EXCHANGES);
	vector<int> half_B(NUM_EXCHANGES);
	vector<int> a(ARRAY_SIZE);
	vector<int> b(ARRAY_SIZE);
	vector<vector<int>> c(GENE_SIZE, vector<int>(ARRAY_SIZE));

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

	vector<int> A(ARRAY_SIZE);
	vector<int> B(ARRAY_SIZE);
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

#pragma omp parallel num_threads(4)
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


/* main에서
vector<int*> top_arrays = selectTopArrays(배열);
breeding(top_arrays);
*/



class Maze {
private:

public:
	int rows = ROWS;
	int cols = COLS;
	vector<pair<int, int>> collisions;
	pair<int, int> goal, spawn_pos;
	vector<vector<char>> use_maze;
	Maze() {
		ifstream infile("maze.txt");
		vector<char> line_vec;
		for (int i = 0; i < ROWS; ++i) {
			string line;
			getline(infile, line);
			line_vec.clear();
			for (int j = 0; j < COLS; ++j) {
				char c = line[j];
				line_vec.push_back(c);
				if (c == '#') {
					this->collisions.push_back(make_pair(i, j));
				}
				else if (c == 'S') {
					this->spawn_pos = make_pair(i, j);
				}
				else if (c == 'E') {
					this->goal = make_pair(i, j);
				}
			}
			this->use_maze.push_back(line_vec);
		}
	}
};

class Player {
private:

public:
	vector<int> move_list;
	int fitness;
	//vector<pair<int, int>> position;
	int id;
	bool is_made_goal;
	pair<int, int> start_position;
	int speed = PLAYER_SPEED;
	pair<int, int> position;
	Maze maze;

	int x;
	int y;


	Player(vector<int> gene) {
		this->move_list = gene;
		this->fitness = 0;
		this->id = -1;
		this->maze = Maze();
		//this->position = vector<pair<int, int>>();
		this->is_made_goal = false;
		this->start_position = this->maze.spawn_pos;
		this->position = this->maze.spawn_pos;
		this->x = start_position.first;
		this->y = start_position.second;


	}

	pair<int, int> check_next_position(pair<int, int> position, int move) {
		printf("%d\n", move);
		if (move == 1) {
			position.first += 1;
			if (position.first >= ROWS)
				position.first = ROWS - 1;
			return make_pair(position.first, position.second);
		}
		else if (move == 2) {
			position.first -= 1;
			if (position.first < 0)
				position.first = 0;
			return make_pair(position.first, position.second);
		}
		else if (move == 3) {
			position.second += 1;
			if (position.second >= ROWS)
				position.second = ROWS -1;
			return make_pair(position.first, position.second);
		}
		else if (move == 4) {
			position.second -= 1;
			if (position.second <0)
				position.second = 0;
			return make_pair(position.first, position.second);
		}
		return make_pair(0,0);
	}
	bool is_collision(pair<int, int> next_position, vector<vector<char>> maze){
		printf("%d %d\n", next_position.first, next_position.second);
		if (maze[next_position.first][next_position.second] == '#')
			return true;
		else
			return false;
	}
	int new_move() {
		srand((unsigned)time(NULL));
		return rand() % 4 + 1;
	}

	bool is_goal() {
		if (this->position == this->maze.goal)
			return true;
		return false;
	}

	int getId() {
		return this->id;
	}

	void setId(int newId) {
		this->id = newId;
	}

	// MOVE: 1 RIGHT 2 LEFT 3 UP 4 DOWN

	void MovePlayer(int direction) {
		this->maze.use_maze[position.first][position.second] = 'V';
		if (direction == 1) {
			this->position.first += 1;
		}
		else if (direction == 2) {
			this->position.first -= 1;
		}
		else if (direction == 3) {
			this->position.second -= 1;
		}
		else if (direction == 4) {
			this->position.second -= 1;
		}
		else {
			cout << "Unknown Move.\n" << endl;
		}
	}

};

class App {
private:

public:
	int num_players = NUM_PLAYERS;
	int generation = 1;
	vector<int> average_fitness;
	vector<vector<int>> best_fitnesses;
	vector<Player> players;
	vector<vector<int>> genes;
	set<pair<int, int>> player_known_walls;
	int made_it_proportion;
	int num_moves;
	bool is_running = false;
	int mode;
	
	App(int mode) {
		this->is_running = true;
		this->mode = mode;
		this->genes = generateArrays(50, 201);
		for (int i = 0; i < this->num_players; i++)
		{
			this->players.push_back(Player(genes[i]));
		}
		cout << "player make end" << endl;
		int id = 0;
		for (auto& player : this->players) {
			player.setId(id);
			id += 1;
		}

		this->made_it_proportion = 0;
		this->num_moves = NUM_MOVES;
		on_execute();
	}

	void Evaluate_serial() {
		for (auto player : players) {
			for (int i = 1; i < player.move_list.size(); i++) {
				while (player.is_collision(player.check_next_position(player.position, player.move_list[i]), player.maze.use_maze))
					player.move_list[i] = player.new_move();
				player.move_list[0] = Evaluate(player.move_list[0], player.position, player.check_next_position(player.position, player.move_list[i]), player.maze.use_maze, player.maze.goal);
				player.MovePlayer(player.move_list[i]);
				if (player.is_goal()) {
					player.move_list.erase(player.move_list.begin() + i, player.move_list.end());
				}
			}
			this->genes[player.id] = player.move_list;
		}
	}
	void Evaluate_parallel() {
	#pragma omp parallel for num_threads(n)
		for (auto player : players) {
			for (int i = 1; i < player.move_list.size(); i++) {
				while (player.is_collision(player.check_next_position(player.position, player.move_list[i]), player.maze.use_maze))
					player.move_list[i] = player.new_move();
				player.move_list[0] = Evaluate(player.move_list[0], player.position, player.check_next_position(player.position, player.move_list[i]), player.maze.use_maze, player.maze.goal);
				player.MovePlayer(player.move_list[i]);
				if (player.is_goal()) {
					player.move_list.erase(player.move_list.begin() + i, player.move_list.end());
				}
			}
			this->genes[player.id] = player.move_list;
		}
	}

	void restart(vector<vector<int>> moves_list) {
		cout << this->generation << "st is end" << endl;
		this->generation += 1;
		this->genes = moves_list;

		this->players.clear();

		for (int i = 0; i < this->num_players; i++) {
			this->players.push_back(Player(genes[i]));
		}
		cout << "new player end" << endl;
		int id = 0;
		for (auto& player : this->players) {
			player.setId(id);
			id += 1;
		}

		cout << "Game restarted.\n" << endl;
	}

	

	int calc_madeit_prop() {
		int madeit_sum = 0;
		for (auto& player : this->players) {
			if (player.is_made_goal) {
				madeit_sum += 1;
			}
		}

		return (madeit_sum / NUM_PLAYERS);
	}

	void on_execute() {
		while (this->is_running) {
			vector<vector<int>> new_genes;
			if (this->mode == 0 || this->mode == 1)
			{
				Evaluate_serial();
			}

			if (this->mode == 2 || this->mode == 3)
			{
				Evaluate_parallel();
			}
			cout << "evaluate end" << endl;
			vector<vector<int>> top_arrays = selectTop(this->genes);
			saveToFile(top_arrays, "top_arrays.txt");
			cout << "select end" << endl;
			this->best_fitnesses.push_back(top_arrays[0]);
			if (this->mode == 0 || this->mode == 2)
				new_genes = breeding(top_arrays);
			if (this->mode == 1 || this->mode == 3)
				new_genes = breeding_serial(top_arrays);
			saveToFile(new_genes, "new_gens.txt");
			cout << "breeding end" << endl;
			saveToFile(this->best_fitnesses, "best_fitnesses.txt");
			restart(new_genes);
			
			if (this->generation > 5){
				is_running = false;
			}
		}
	}

};