#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <string>
#include <Python.h>

#define PLAYER_SPEED 1
#define NUM_MOVES 200
#define DEAD_END_PENALTY 200

using namespace std;

const vector<string> MOVE_OPTIONS = {"right", "left", "up", "down"};


string calc_move(pair<int, int> old_pos, pair<int, int> new_pos) {
	int diff_x = old_pos.first - new_pos.first;
	int diff_y = old_pos.second - new_pos.second;

	if (diff_x < 0) return "right";
	else if (diff_x > 0) return "left";
	else if (diff_y < 0) return "up";
	else if (diff_y > 0) return "down";
	else if (diff_x == 0 && diff_y == 0) return "";
	else {
		cout << "Mistakes were made.\n" << endl;
		printf("old pos: (%d, %d), new pos: (%d, %d)\n", old_pos.first, old_pos.second, new_pos.first, new_pos.second);
		return "";
	}
}


class Player {
private:
	vector<string> move_list;
	int fitness;
	vector<pair<int, int>> position;
	int id;
	bool is_made_goal;
	pair<int, int> start_position;
	set<pair<int, int>> unique_position;
	int speed = PLAYER_SPEED;
	

	int x;
	int y;
public:
	Player(pair<int, int> start_position) {
		this->move_list = vector<string>();
		this->fitness = 0;
		this->id = -1;
		this->position = vector<pair<int, int>>();
		this->is_made_goal = false;
		this->start_position = start_position;
		this->unique_position = set<pair<int, int>>();
		this->unique_position.insert(start_position);
		this->x = start_position.first + 2;
		this->y = start_position.second + 2;


	}
	void MovePlayer(string direction) {
		if (direction == "right") {
			x += speed;
		}
		else if (direction == "left") {
			x -= speed;
		}
		else if (direction == "up") {
			y += speed;
		}
		else if (direction == "down") {
			y -= speed;
		}
		else {
			cout << "Unknown Move.\n" << endl;
		}
	}
	string CheckMove(string move, set<pair<int, int>> known_walls_set) {
		if (speed == 0) return "";

		pair<int, int> new_coordination;

		if (move == "right") {
			new_coordination = make_pair(x + speed, y);
		}
		else if (move == "left") {
			new_coordination = make_pair(x - speed, y);
		}
		else if (move == "up") {
			new_coordination = make_pair(x, y + speed);
		}
		else if (move == "left") {
			new_coordination = make_pair(x - speed, y);
		}
		else {
			cout << move << endl;
			return "";
		}
	

		vector<pair<int, int>> possible_moves = { 
			make_pair(x + speed, y),
			make_pair(x - speed, y), 
			make_pair(x, y - speed), 
			make_pair(x, y + speed)
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
			move_list.push_back(move);
			position.push_back(make_pair(x, y));
			unique_position.insert(new_coordination);
			MovePlayer(move);
			return move;
		}

		else {
			if (applicable_moves.size() == 1) {
				speed = 0;
				fitness += DEAD_END_PENALTY;
				return "";
			}

			else {
				vector<pair<int, int>> remainder;
				set_difference(applicable_moves.begin(), applicable_moves.end(), unique_position.begin(), unique_position.end(), back_inserter(remainder));

				if (remainder.empty()) {
					speed = 0;
					return "";
				}

				else if (remainder.size() == 1) {
					move_list.push_back(move);
					position.push_back(make_pair(x, y));
					unique_position.insert(remainder[0]);
					MovePlayer(move);
					return move;
				}

				else {
					int rand_idx = rand() % remainder.size();
					new_coordination = remainder[rand_idx];
					string new_move = calc_move(make_pair(x, y), new_coordination);
					move_list.push_back(new_move);
					position.push_back(make_pair(x, y));
					unique_position.insert(new_coordination);
					MovePlayer(new_move);
					return new_move;
				}
			}
		}

	}

};

