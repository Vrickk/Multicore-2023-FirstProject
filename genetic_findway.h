#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <random>
#include <string>
#include <algorithm>
#include <math.h>
#include <ctime>

#define PLAYER_SPEED 1
#define NUM_MOVES 200
#define DEAD_END_PENALTY 200
#define NUM_PLAYERS 900

#define ROWS 25
#define COLS 25

using namespace std;


vector<int> create_random_moves(int turns) {
	vector<int> options{1,2,3,4};
	vector<int> moves;

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dis(0, options.size() - 1);

	for (int i = 0; i < turns; i++) {
		int rand_index = dis(gen);
		moves.push_back(options[rand_index]);
	}

	return moves;
}

int calc_move(pair<int, int> old_pos, pair<int, int> new_pos) {
	int diff_x = old_pos.first - new_pos.first;
	int diff_y = old_pos.second - new_pos.second;

	if (diff_x < 0) return 1;
	else if (diff_x > 0) return 2;
	else if (diff_y < 0) return 3;
	else if (diff_y > 0) return 4;
	else if (diff_x == 0 && diff_y == 0) return 0;
	else {
		cout << "Mistakes were made.\n" << endl;
		printf("old pos: (%d, %d), new pos: (%d, %d)\n", old_pos.first, old_pos.second, new_pos.first, new_pos.second);
		return 0;
	}
}

void generateArrays(int arrays[][201], int numArrays, int arraySize) {
	srand(time(NULL)); // seed the random number generator with the current time
	for (int i = 0; i < numArrays; i++) {
		arrays[i][0] = 0;
		for (int j = 1; j < arraySize + 1; j++) {
			arrays[i][j] = rand() % 4 + 1; // generate random number between 1 and 4
		}
	}
}

bool no_way(int coordination[2], int** maze) {
	int count = 0;
	if (maze[coordination[0]][coordination[1] + 1] == '#')
		count++;
	if (maze[coordination[0]][coordination[1] - 1] == '#')
		count++;
	if (maze[coordination[0] + 1][coordination[1]] == '#')
		count++;
	if (maze[coordination[0] - 1][coordination[1]] == '3')
		count++;

	if (count == 3)
		return true;
	else
		return false;
}

vector<vector<int>> create_move_array(int x = NUM_PLAYERS, int y = NUM_MOVES)
{
	vector<vector<int>> moves;

	for (int i = 0; i < x; i++)
	{
		moves.push_back(create_random_moves(y));
	}

	return moves;
}

int Evaluate(int suitability, int coordination[2], int next_coordination[2], int** maze, int End_coordination[2]) {
	int now_length = pow((End_coordination[0] - coordination[0]), 2) + pow((End_coordination[1] - coordination[1]), 2);
	int next_length = pow((End_coordination[0] - next_coordination[0]), 2) + pow((End_coordination[1] - next_coordination[1]), 2);
	if (maze[next_coordination[0]][next_coordination[1]] == 'V')
		suitability -= 2;
	else if (now_length > next_length)
		suitability -= 1;
	else if (now_length < next_length)
		suitability += 1;
	else if (no_way(next_coordination, maze))
		suitability -= 10;
	else if (next_coordination == End_coordination)
		suitability += 30;

	return suitability;
}

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

class Player {
private:
	
public:

	vector<int> move_list;
	int fitness;
	vector<pair<int, int>> position;
	int id;
	bool is_made_goal;
	pair<int, int> start_position;
	set<pair<int, int>> unique_position;
	int speed = PLAYER_SPEED;


	int x;
	int y;


	Player(pair<int, int> start_position) {
		this->move_list = vector<int>();
		this->fitness = 0;
		this->id = -1;
		this->position = vector<pair<int, int>>();
		this->is_made_goal = false;
		this->start_position = start_position;
		this->unique_position = set<pair<int, int>>();
		this->unique_position.insert(start_position);
		this->x = start_position.first;
		this->y = start_position.second;


	}

	int getId() {
		return this->id;
	}

	void setId(int newId) {
		this->id = newId;
	}

	// MOVE: 1 RIGHT 2 LEFT 3 UP 4 DOWN

	void MovePlayer(int direction) { 
		if (direction == 1) {
			this->x += this->speed;
		}
		else if (direction == 2) {
			this->x -= this->speed;
		}
		else if (direction == 3) {
			this->y += this->speed;
		}
		else if (direction == 4) {
			this->y -= this->speed;
		}
		else {
			cout << "Unknown Move.\n" << endl;
		}
	}


	int CheckMove(int move, set<pair<int, int>> known_walls_set) {
		if (this->speed == 0) return 0;

		pair<int, int> new_coordination;

		if (move == 1) {
			new_coordination = make_pair(this->x + this->speed, this->y);
		}
		else if (move == 2) {
			new_coordination = make_pair(this->x - this->speed, this->y);
		}
		else if (move == 3) {
			new_coordination = make_pair(this->x, this->y + this->speed);
		}
		else if (move == 4) {
			new_coordination = make_pair(this->x, this->y - this->speed);
		}
		else {
			cout << move << endl;
			return 0;
		}
	

		vector<pair<int, int>> possible_moves = { 
			make_pair(this->x + this->speed, this->y),
			make_pair(this->x - this->speed, this->y),
			make_pair(this->x, this->y + this->speed),
			make_pair(this->x, this->y - this->speed)
		};

		vector<pair<int, int>> applicable_moves;
		for (auto p : possible_moves) {
			if (known_walls_set.find(p) == known_walls_set.end()) {
				applicable_moves.push_back(p);
			}
		}

		if (find(applicable_moves.begin(), applicable_moves.end(), new_coordination) != applicable_moves.end()
			&& unique_position.find(new_coordination) == unique_position.end())
		{
			this->MovePlayer(move);
			return 0;
		}

		else {
			if (applicable_moves.size() == 1) {
				this->speed = 0;
				this->fitness += DEAD_END_PENALTY;
				return 0;
			}

			else {
				vector<pair<int, int>> remainder;
				set_difference(applicable_moves.begin(), applicable_moves.end(), known_walls_set.begin(), known_walls_set.end(), back_inserter(remainder));
				set_difference(remainder.begin(), remainder.end(), position.begin(), position.end(), back_inserter(remainder));

				if (remainder.empty()) {
					this->speed = 0;
					return 0;
				}

				else if (remainder.size() == 1) {
					pair<int, int> old_pos = make_pair(this->x, this->y);
					pair<int, int> new_coord = remainder[0];
					this->x = new_coord.first;
					this->y = new_coord.second;

					pair<int, int> new_pos = make_pair(this->x, this->y);

					int what_move = calc_move(old_pos, new_pos);
					return what_move;
				}

				else {
					pair<int, int> old_pos = make_pair(this->x, this->y);
					pair<int, int> new_coord = remainder[rand() % remainder.size()];
					this->x = new_coord.first;
					this->y = new_coord.second;

					pair<int, int> new_pos = make_pair(this->x, this->y);

					int what_move = calc_move(old_pos, new_pos);
					return what_move;
				}
			}
		}

	}

};

class Maze {
private:

public:
	int rows = ROWS;
	int cols = COLS;
	vector<pair<int, int>> collisions;
	pair<int, int> goal, spawn_pos;
	Maze() {
		char maze[ROWS][COLS];

		ifstream infile("maze.txt");

		int bx = 0;
		int by = 0;

		for (int i = 0; i < ROWS; ++i) {
			string line;
			getline(infile, line);
			for (int j = 0; j < COLS; ++j) {
				
				maze[i][j] = line[j];
				cout << maze[i][j];

				if (maze[i][j] == '#') {
					this->collisions.push_back(make_pair(j, i));
				}
				else if (maze[i][j] == 'S') {
					this->spawn_pos = make_pair(j, i);
				}
				else if (maze[i][j] == 'E') {
					this->goal = make_pair(j, i);
				}
				
			}
			cout << "\n";
			
		}

		cout << "   " << this->spawn_pos.first << ", " << this->spawn_pos.second << " is start point.\n";
		cout << "   " << this->goal.first << ", " << this->goal.second << " is end point.\n";


		cout << endl;

	}
};

class App {
private:
	

public:
	int num_players = NUM_PLAYERS;
	int generation = 1;
	vector<int> average_fitness;
	vector<int> best_fitness;
	vector<Player> players;
	Maze maze;
	int turn;
	vector<vector<int>> moves_array;
	set<pair<int, int>> player_known_walls;
	int made_it_proportion;
	int num_moves;
	bool is_running = false;

	App() {
		this->is_running = true;
		this->maze = Maze();
		
		for (int i = 0; i < this->num_players; i++)
		{
			this->players.push_back(Player(make_pair(0, 23)));
		}

		int id = 0;
		for (auto& player : this->players) {
			player.setId(id);
			id += 1;
		}
		this->turn = 1;

		this->moves_array = create_move_array();
		this->made_it_proportion = 0;
		this->num_moves = NUM_MOVES;
	}

	void restart(vector<vector<int>> moves_list) {
		this->generation += 1;
		this->turn = 1;
		this->moves_array = moves_list;

		this->players.clear();

		for (int i = 0; i < this->num_players; i++) {
			this->players.push_back(Player(make_pair(0, 23)));
		}

		int id = 0;
		for (auto& player : this->players) {
			player.setId(id);
			id += 1;
		}

		cout << "Game restarted.\n" << endl;
	}

	bool is_collision(int x1, int x2, int y1, int y2)
	{
		if (x1 >= x2 && x1 < ROWS)
		{
			if (y1 >= y2 && y1 < COLS) {
				return true;
			}
		}
		else return false;
	}

	void on_loop() {
		for (auto i : this->maze.collisions)
		{
			for (auto player : this->players) {
				if (is_collision(player.x, i.first, player.y, i.second)) {
					player.speed = 0;
					this->player_known_walls.insert(make_pair(player.x, player.y));
				}
			}
		}

		for (auto player : this->players) {
			if (is_collision(player.x, this->maze.goal.first, player.y, this->maze.goal.second)) {
				player.is_made_goal = true;
				player.speed = 0;
				printf("Player %d made it.\n", player.id);
			}
		}
	}

	/*void on_execute() {
		while (this->is_running) {

			for (auto& player : this->players) {
				vector<int> move = this->moves_array[player.id, this->turn - 1];
				vector<int> new_move = player.CheckMove(move, player_known_walls);
			}
		}
	}*/

	
};



