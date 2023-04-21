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
#define NUM_PLAYERS 900

#define ROWS 25
#define COLS 25

#define GENERATION_THRESH 50
#define MADEIT_THRESH 10

const int GENE_SIZE = 50;
const int ARRAY_SIZE = 201;

using namespace std;

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

vector<vector<int>> generateArrays(int numArrays, int arraySize) {
	vector<vector<int>> arrays(numArrays, vector<int>(arraySize + 1, 0));
	srand(time(NULL)); // seed the random number generator with the current time
	for (int i = 0; i < numArrays; i++) {
		arrays[i][0] = 0;
		for (int j = 1; j < arraySize + 1; j++) {
			arrays[i][j] = rand() % 4 + 1; // generate random number between 1 and 4
		}
	}

	return arrays;
}

bool no_way(pair<int, int> pos, int** maze) {
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
        else if(maze[pos.first - 1][pos.second] == '#')
            count++;
    }
    

    if (count > 3)
        return true;
    else
        return false;
}

int Evaluate(int suitability, pair<int, int> pos, pair<int, int> next_pos, int** maze, pair<int, int> goal) {
    int now_length = pow((goal.first - next_pos.first), 2) + pow((goal.second - next_pos.second), 2);
    int next_length = pow((goal.first - next_pos.first), 2) + pow((goal.second - next_pos.second), 2);
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



// 적합도 상위 10개 개체 선정 -> top_arrays에 순서대로 넣음
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

vector<vector<int>> breeding_serial(vector<int*>& top_arrays)
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

class Player {
private:
	
public:

	vector<int> move_list;
	int fitness;
	vector<pair<int, int>> position;
	int id;
	bool is_made_goal;
	pair<int, int> start_position;
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
			&& find(this->position.begin(), this->position.end(), new_coordination) == this->position.end())
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

	
		for (int i = 0; i < ROWS; ++i) {
			string line;
			getline(infile, line);
			for (int j = 0; j < COLS; ++j) {
				
				maze[i][j] = line[i];
				cout << maze[i][j];

				if (maze[i][j] == '#') {
					this->collisions.push_back(make_pair(i, j));
				}
				else if (maze[i][j] == 'S') {
					this->spawn_pos = make_pair(i, j);
				}
				else if (maze[i][j] == 'E') {
					this->goal = make_pair(i, j);
				}
				
			}
			cout << "\n";
			
		}

		cout << "   " << this->spawn_pos.second << ", " << this->spawn_pos.first << " is start point.\n";
		cout << "   " << this->goal.second << ", " << this->goal.first << " is end point.\n";


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
	int mode;
	void Evaluate_serial() {
	    for (auto player : players) {
		for (auto i = player.move_list.begin() + 1; i != player.move_list.end(); ++i) {
		    while (player.is_collision(player.next_position, player.maze.maze))
			player.move_list[i] = player.new_move();
		    player.move_list[0] = Evaluate(player.move_list[0], player.position, player.next_position(player.position, player.move_list[i]), player.maze.maze, player.maze.goal);
		    player.movePlayer(player.move_list[i];
		}
	    }
	}
	void Evaludate_serial() {
	#pragma omp for num_threads(n)
		for (auto player : players) {
			for (auto i = player.move_list.begin() + 1; i != player.move_list.end(); ++i) {
	    			while (player.is_collision(player.next_position, player.maze.maze))
					player.move_list[i] = player.new_move();
	    			player.move_list[0] = Evaluate(player.move_list[0], player.position, player.next_position(player.position, player.move_list[i]), player.maze.maze, player.maze.goal);
	    			player.movePlayer(player.move_list[i];
			}
	    	}
	}
	App(int mode) {
		this->is_running = true;
		this->maze = Maze();
		this->mode = mode;
		
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

		this->moves_array = generateArrays(ROWS, 200);
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

	int calc_madeit_prop() {
		int madeit_sum = 0;
		for (auto& player: this->players) {
			if (player.is_made_goal) {
				madeit_sum += 1;
			}
		}

		return (madeit_sum / NUM_PLAYERS);
	}
	
	void on_execute() {
		while (this->is_running) {

			vector<vector<int>> moves_lists;
			// Evaluate fitnesses

			if (this->turn == this->num_moves || max_element(this->players.begin(), this->players.end(), [](const auto& a, const auto& b) {return a.speed < b.speed; })->speed == 0) {
				this->made_it_proportion = this->calc_madeit_prop();

				if (this->made_it_proportion > MADEIT_THRESH){
					printf("%d of players made it.\n", this->made_it_proportion);
					break;
				}

				if (this->generation == GENERATION_THRESH) {
					printf("All generations have passed.\n");
					printf("%d of players made it.\n", this->made_it_proportion);
					break;
				}





				// Breed


				if (this->mode == 0 || this->mode == 2)
				{
					 moves_lists = breeding_serial();
				}

				if (this->mode == 1 || this->mode == 3)
				{
					moves_lists = breeding_parallel();
				}


				this->restart(moves_lists);
			}


			// Players move

			

			for (auto& player : this->players) {
				int move = this->moves_array[player.id][static_cast<int32_t>(this->turn) - 1];
				int new_move = player.CheckMove(move, this->player_known_walls);
			
				if (new_move != 0) {
					this->moves_array[player.id][static_cast<int32_t>(this->turn) - 1] = new_move;
				}

				player.position.push_back(make_pair(player.x, player.y));
			
			}

			this->on_loop();
			this->turn += 1;
		}
	}

	
};



