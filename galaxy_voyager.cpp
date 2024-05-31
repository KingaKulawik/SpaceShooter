/*
    ########################################
    #                                      #
    #            Galaxy voyager            #
    #                                      #
    ########################################

    Simple Text User Interface based game where spaceship has to 
    collect all antimatter in a galaxy.

*/

#include <iostream>
#include <ctime>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <string>
#include <thread>
#include <algorithm>
#include <future>
#include <ncurses.h>
#include <unistd.h>

using namespace std;


const string GAME_TITLE = "Galaxy voyager";
const string DEFEAT = "DEFEAT!";
const string WIN = "YOU WIN!";
const int BOARD_WIDTH = 40;
const int BOARD_HEIGHT = 20;
const int NUM_STARS = 4;
const int PLAYER_SPEED = 1;
const int NUM_ANTIMATTER = 10;
const int SHOT_PERIOD = 5; //seconds
const int COMET_SPEED = 500; //milliseconds
const int BANNER_HEIGHT = 4;


struct Star {
    int x, y;
};

struct Comet{
    int x, y;
    int dx, dy;
    char covered_character;
    std::chrono::time_point<std::chrono::system_clock> last_position_change_time; 
};

Star stars[NUM_STARS];
vector<string> board(BOARD_HEIGHT, string(BOARD_WIDTH, ' '));

mutex board_access_mutex;
mutex print_board_mutex;

int player_x, player_y;
int score = 0;
std::atomic<bool> game_over(false);


int get_random_int(int left_bound, int right_bound){
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(left_bound, right_bound);

    return dis(gen);
}

// Reflect board status to the terminal screen
void print_board() {
    print_board_mutex.lock();
    
    char pixel;
    for (int h = 0; h < BOARD_HEIGHT; h++) {
        for(int w=0;w<BOARD_WIDTH;w++){
            pixel=board[h][w];
            mvaddch(h, w, pixel);
        }
 
    }
    refresh();
    
    print_board_mutex.unlock();
}

void print_end_banner(string message){
    print_board_mutex.lock();

    int margin = (BOARD_WIDTH - 2 - message.length())/2;

    string end_banner_message = string(margin, ' ') + message + string(margin, ' ');

    
    mvaddstr(BOARD_HEIGHT - 1 + BANNER_HEIGHT/2, 1, end_banner_message.c_str());

    refresh();

    print_board_mutex.unlock();
}

void update_board(int x, int y, char c) {
    board[x][y] = c;
}


void remove_antimatter(int x, int y) {
    update_board(x, y, ' ');
    score += 1;
    
    if(score == NUM_ANTIMATTER){
        game_over = true;
    }
}


void move_player(int dx, int dy) {
    int new_x = player_x + dx;
    int new_y = player_y + dy;
       
    if (new_x < 0 || new_x >= BOARD_HEIGHT  || new_y < 0 || new_y >= BOARD_WIDTH) {
        return;
    }

    char c = board[new_x][new_y];
    
    if (c == '#'){
       return;
    }
    else if (c == ' ') {
        update_board(player_x, player_y, ' ');
        player_x = new_x;
        player_y = new_y;
        update_board(player_x, player_y, '^');
    } else if (c == '.') {
        remove_antimatter(new_x, new_y);
        update_board(player_x, player_y, ' ');
        player_x = new_x;
        player_y = new_y;
        update_board(player_x, player_y, '^');
    } else if (c == 'O') {
        update_board(player_x, player_y, 'X');
        game_over = true;
    } else if (c == '*') {
        update_board(player_x, player_y, 'X');
        game_over = true;
    }

}

void shoot_comets(Star& star) {
    vector<Comet> comets;
    auto shoot_time = chrono::system_clock::now();

    while (!game_over) {
        auto curr_time = chrono::system_clock::now();
        
        vector<Comet>::iterator comet = comets.begin();

        while(comet != comets.end()){
            
            // Move comet
            chrono::duration<double> time_elapsed_from_last_comet_move = curr_time - (*comet).last_position_change_time;
            time_elapsed_from_last_comet_move.count();
            
            board_access_mutex.lock();

            if(time_elapsed_from_last_comet_move >= chrono::milliseconds(COMET_SPEED)){
                int new_x = (*comet).x + (*comet).dx;
                int new_y = (*comet).y + (*comet).dy;

                // Comet to disappear
                if(new_x == BOARD_HEIGHT - 1 || new_x == 0 || new_y == BOARD_WIDTH - 1 || new_y == 0 || board[new_x][new_y] == 'O' || board[new_x][new_y] == '*'){
                    update_board((*comet).x, (*comet).y, (*comet).covered_character);
                    
                    comet = comets.erase(comet);
                }
                // Comet hits the player
                else if (new_x == player_x && new_y == player_y) {
                    game_over = true;
                    board[new_x][new_y]='X';
                    board_access_mutex.unlock();
                    break;
                }
                // Comet moves forward
                else{
                    update_board((*comet).x, (*comet).y, (*comet).covered_character);
                    (*comet).x = new_x;
                    (*comet).y = new_y;
                    (*comet).last_position_change_time = curr_time;
                    (*comet).covered_character = board[new_x][new_y];
                    update_board(new_x, new_y, '*');
                    ++comet;
                }
            }
            else ++comet;

            board_access_mutex.unlock();
        }
        
        // Shoot new comet
        chrono::duration<double> time_elapsed_from_last_comet_shoot = curr_time - shoot_time;
        time_elapsed_from_last_comet_shoot.count();
        if(time_elapsed_from_last_comet_shoot >= chrono::seconds(SHOT_PERIOD)){
            shoot_time = curr_time;
            Comet comet;
            comet.x = star.x;
            comet.y = star.y;
            comet.dx = get_random_int(-1, 1);
            comet.dy = get_random_int(-1, 1);
            comet.covered_character = 'O';
            comet.last_position_change_time = curr_time;
            comets.emplace_back(comet);
        }

        print_board();
        
        // If there is no sleep function, then screen is too often refreshed and player move is not smooth
        sleep(0.5);
    }
}

void game_loop() {
    while (!game_over) {
        int c = getch();
        
        
        board_access_mutex.lock();
 
        if (c == 259) { // up
            move_player(-PLAYER_SPEED, 0);
        } else if (c == 258) { // down
            move_player(PLAYER_SPEED, 0);
        } else if (c == 260) { // left
            move_player(0, -PLAYER_SPEED);
        } else if (c == 261) { // right
            move_player(0, PLAYER_SPEED);
        }
        
        print_board();
        board_access_mutex.unlock();
    }
    if (game_over){
        if(score < NUM_ANTIMATTER) print_end_banner(DEFEAT);
        else print_end_banner(WIN);
    }
}

void initialize_game_board(){
    // Game board
    for (int x = 0; x < BOARD_HEIGHT; x++) {
        for (int y = 0; y < BOARD_WIDTH; y++) {
            if (x == 0 || x == BOARD_HEIGHT - 1 || y == 0 || y == BOARD_WIDTH - 1) {
                board[x][y] = '#';
            }
        }
    }

    // Game banner
    for (int x = BOARD_HEIGHT - 1; x < BOARD_HEIGHT + BANNER_HEIGHT; x++) {
        for (int y = 0; y < BOARD_WIDTH; y++) {
            if (x == BOARD_HEIGHT - 1 || x == BOARD_HEIGHT - 1 + BANNER_HEIGHT || y == 0 || y == BOARD_WIDTH - 1) {
                mvaddch(x, y, '#');
            }
        }
    }
    
    int margin = (BOARD_WIDTH - GAME_TITLE.length())/2;
    mvaddstr(BOARD_HEIGHT - 1 + BANNER_HEIGHT/2, margin, GAME_TITLE.c_str());
}

void place_antimatter(int number_of_antimatter){
    int i = 0;
    while (i < number_of_antimatter) {
        int x = get_random_int(1, BOARD_HEIGHT-1);
        int y = get_random_int(1, BOARD_WIDTH-1);

        if(board[x][y] == ' ') update_board(x, y, '.');
        else continue;

        i++;
    }
}

void place_stars(){
        for(auto & star : stars){
        int x,y;
        while (true){
            x = get_random_int(1, BOARD_HEIGHT-1);
            y = get_random_int(1, BOARD_WIDTH-1);
            if(board[x][y] != ' ') continue;
            star.x=x;
            star.y=y;
            break;
        }
        update_board(star.x, star.y, 'O');
    }
}

void place_player(){
    int x,y;
    while (true){
        x = get_random_int(1, BOARD_HEIGHT-1);
        y = get_random_int(1, BOARD_WIDTH-1);
        if(board[x][y] != ' ') continue;
        break;
    }
    update_board(x, y, '^');    
    player_x = x;
    player_y = y;
}

int main() {

    // init ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    srand(time(NULL));
 
    // initialize game board
    initialize_game_board();
    place_antimatter(NUM_ANTIMATTER);
    place_stars();
    place_player();

    print_board();

    // 1 thread for player
    thread player_thread(game_loop);

    // 4 threads for stars in a thread pool
    vector<thread> star_threads;

    for(int i=0; i<NUM_STARS; i++){
        star_threads.emplace_back(thread(shoot_comets, std::ref(stars[i])));
    }
    

    player_thread.join();
    
    for (thread &t: star_threads) {
        t.join();
    }

    // wait unitl player presses 'q' - prevent too quick game quit
    int c = getch();
    while(c != 'q'){
        c= getch();
    }

    // release ncurses
    endwin();
    

    return 0;
}