#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <ctype.h>

//CONSTANTS AND DEFINITIONS
#define MAP_HEIGHT 20
#define MAP_WIDTH 40

#define LOCATION_IS_WALL 0
#define LOCATION_OK 1
#define LOCATION_IS_HEALTH_PACK 2
#define LOCATION_IS_TRAP -1
#define LOCATION_IS_PLAYER 3

#define MOVE_UP_CAPS 'W'
#define MOVE_UP_LOWER 'w'

#define MOVE_DOWN_CAPS 'S'
#define MOVE_DOWN_LOWER 's'

#define MOVE_LEFT_CAPS 'A'
#define MOVE_LEFT_LOWER 'a'

#define MOVE_RIGHT_CAPS 'D'
#define MOVE_RIGHT_LOWER 'd'

#define QUIT_KEY 27

#define HEALTH_COUNTER_LOCATION 13,20
#define MESSAGE_LOCATION 18,22
#define BUILD_MODE_MESSAGE_LOCATION 15,20

#define PLAYER_IS_ALIVE 1
#define PLAYER_IS_DEAD 0

#define DEFAULT_HEALTH_COUNT 0

#define DEFAULT_COORD_Y 1
#define DEFAULT_COORD_X 1

#define NUMBER_OF_ITEMS_GENERATED 10

#define CHAR_TRAP '*'
#define CHAR_WALL '#'
#define CHAR_EMPTY ' '
#define CHAR_HEALTH_PACK '+'
#define CHAR_NEWLINE '\n'
#define CHAR_PLAYER 1

#define USER_PROFILE_STORE_FILENAME "user.txt"
#define USER_SCOREBOARD_FILE "score.txt"
#define MODE_READ "r"
#define MODE_WRITE "w"
#define MODE_APPEND "a"

#define USER_MAP_FILENAME_TEMPLATE "%s_map.txt"

#define MODE_BUILD 1
#define MODE_PLAY 0

#define SILENCE_MSVC -999

using namespace std;

//Login
typedef struct user_credential {
    char *username;
    char *password;
};

int userCount = 0;
user_credential users[100];
user_credential currentUser;

int init_login();
void register_account();
int authenticate_login(char *username, char *password);
int query_username_exists(const char *username);
int is_valid_username(const char *username, int length);
int is_valid_password(const char *password, int length);

//Main menu and bootstrapper
int run();
void main_menu();

//Game engine
char map[MAP_HEIGHT][MAP_WIDTH];
int playerY = DEFAULT_COORD_Y;
int playerX = DEFAULT_COORD_X;
int player_health_count = DEFAULT_HEALTH_COUNT;
int isPlayerAlive = PLAYER_IS_ALIVE;

//Board
void init_board();
void build_default_map(int build_mode);
void build_map_from_file(char *filename);
void render_board(int mode);
int place_traps(int Y, int X);
int place_health_packs(int Y, int X);

//Game Engine

//Start Game
int play_game();
void reset_player();

//Gameplay and Movement Parser
int parse_movement(char move_char);
int check_location_condition(int new_player_Y, int new_player_X);
void move_character(int new_player_Y, int new_player_X);
void add_player_health();
void deduct_player_health(int new_player_Y, int new_player_X);

//Map Creator
int current_console_pointer_X = 1;
int current_console_pointer_Y = 1;
int items_placed = 0;

void create_map();
int parse_movement_map_creator(char move_char);
void place_item(int Y, int X, char item_char);
void delete_item(int Y, int X);
void write_map_to_file(char *username);


//Scoreboard
void write_score();
void show_scoreboard();

//Misc
int file_exists(const char *fname);
void cleanup();
void setCursorPosition(int x, int y);

int main()
{
    int exit = 0;
    do {
        exit = run();
    } while (exit != 1);
    cleanup();
    return 0;
}

int run() {
    char username[40], password[40];
    char userInput = 0;
    int user_cancels_login = 0;
    init_login();
    do {
        user_cancels_login = 0;
        system("cls");
        puts("SweeMan");
        puts("=================");
        puts("1. Login");
        puts("2. Register");
        puts("3. Exit");

        userInput = getch();

        switch(userInput)
        {
        case '1':
            fflush(stdin);
            do {
                printf("%s", "Please input your username : ");
                scanf("%s", &username);
                if (strcmp(username, "exit") == 0){
                    user_cancels_login = 1;
                    break;
                }
            } while (strlen(username) == 0);

            if (!user_cancels_login) {
                do {
                    printf("%s", "Please input your password : ");
                    scanf("%s", &password);
                } while (strlen(password) == 0);

                if (authenticate_login(username, password)) {
                    currentUser.username = strdup(username);
                    currentUser.password = strdup(password);
                    printf("Welcome, %s", currentUser.username);
                    getch();
                    main_menu();
                }
                else {
                    printf("%s", "Login fail. Please try again.");
                    getch();
                }
            }
            break;
        case '2':
            //register the account and repopulate the user profiles
            register_account();
            init_login();
            break;
        case '3':
            return 1;
            break;
        default:
            continue;
        }
    } while(userInput != 3);
    return 0;
}

int file_exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, MODE_READ)))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void register_account() {
    fflush(stdin);
    FILE *fp;
    if (file_exists(USER_PROFILE_STORE_FILENAME))
        fp = fopen(USER_PROFILE_STORE_FILENAME, MODE_APPEND);
    else
        fp = fopen(USER_PROFILE_STORE_FILENAME, MODE_WRITE);

    char username[30], password[30];

    do {
        printf("%s", "Input username : ");
        scanf("%40[^\n]", &username); fflush(stdin);
    } while (
                strlen(username) < 5 ||
                strlen(username) > 10 ||
                query_username_exists(username) ||
                (!is_valid_username(username, strlen(username)))
             );


    do {
        printf("%s", "Input password : ");
        scanf("%s", &password);
    } while (
                strlen(password) < 8 ||
                strlen(password) > 20 ||
                (!is_valid_password(password, strlen(password)))
             );

    fprintf(fp, "%s#%s\n", username, password);
    fclose(fp);
}

int query_username_exists(const char *username) {
    for (int i = 0; i < (userCount); i++) {
        if(strcmp(users[i].username, username) == 0)
            return 1;
    }
    return 0;
}

int is_valid_username(const char *username, int length) {
    for (int i = 0; i < length; i++) {
        if(!isalnum(username[i]))
            return 0;
    }

    return 1;
}

int is_valid_password(const char *password, int length) {
    int numberExists = 0;
    for (int i = 0; i < length; i++) {
        if(isalnum(password[i])) {
            if (isdigit(password[i]))
                numberExists = 1;
        }

        else {
            return 0;
        }
    }

    if (numberExists)
        return 1;
    else
        return 0;
}

void cleanup() {
    //TODO: Free used ptrs on user storage.
    for (int i = 0; i < (userCount); i++) {
        free(users[i].username);
        free(users[i].password);
    }
    free(currentUser.username);
    free(currentUser.password);
}

//BUGBUG : garbled string went into the user profile storage!
int init_login() {
    FILE *fp;

    int indexor = 0;
    if (!file_exists(USER_PROFILE_STORE_FILENAME)){
        printf("No one registered yet, please register first");
        fp = fopen(USER_PROFILE_STORE_FILENAME, MODE_WRITE);
        fclose(fp);
        return 0;
    }

    fp = fopen(USER_PROFILE_STORE_FILENAME, MODE_READ);

    char username_buf[40], password_buf[40];
    while (fscanf(fp, "%[^#]#%[^#,\n]\n", &username_buf, password_buf)!= EOF) {
        users[indexor].username = strdup((const char*)username_buf);
        users[indexor].password = strdup((const char*)password_buf);
        indexor++;
        userCount++;
    }

    fclose(fp);
    return 1;
}

int authenticate_login(char *username, char *password) {
    for (int i = 0; i <= (userCount-1); i++) {
        if ((strcmp(username, users[i].username) == 0) && (strcmp(password, users[i].password) == 0)) return 1;
    }
    return 0;
}

void main_menu() {
    char choice = 0;
    do {
        system("cls");
        puts("1. Play Game");
        puts("2. Create Map");
        puts("3. High Score");
        puts("4. Logout");


        choice = getch();
        switch(choice) {
        case '1':
            play_game();
            break;
        case '2':
            create_map();
            break;
        case '3':
            show_scoreboard();
            break;
        case '4':
            return;
            break;
        default:
            continue;
            break;
        }

    } while (choice != '4');

}

void show_scoreboard() {
    if (!file_exists(USER_SCOREBOARD_FILE))
        return;
    char name[40];
    int score = 0;
    FILE *fp = fopen(USER_SCOREBOARD_FILE, MODE_READ);
    system("cls");
    puts("Scoreboard");
    puts("===================\n");

    while (!feof(fp)) {
        fscanf(fp, "%40[^#]#%d\n", &name, &score);
        printf("%s %d\n", name, score);
    }

    fclose(fp);
    puts("\nPress any key to continue...");
    getch();
}

void place_item(int Y, int X, char item_char) {
    if (items_placed < 50 && check_location_condition(Y, X) == LOCATION_OK) {
        putchar(item_char);
        map[Y][X] = item_char;

        items_placed++;
        setCursorPosition(BUILD_MODE_MESSAGE_LOCATION);
        printf("%d", items_placed);

        setCursorPosition(X, Y);
    }
}

void delete_item(int Y, int X) {
    if (check_location_condition(Y,X) != LOCATION_OK) {
        items_placed--;
        setCursorPosition(BUILD_MODE_MESSAGE_LOCATION);
        printf("%d", items_placed);

        setCursorPosition(X, Y);
    }
    putchar(CHAR_EMPTY);
    map[Y][X] = CHAR_EMPTY;

}

int parse_movement_map_creator(char move_char) {
    int new_cursor_position_Y = current_console_pointer_Y;
    int new_cursor_position_X = current_console_pointer_X;

    switch(move_char) {
    case MOVE_UP_CAPS:
    case MOVE_UP_LOWER:
        new_cursor_position_Y--;
        break;
    case MOVE_LEFT_CAPS:
    case MOVE_LEFT_LOWER:
        new_cursor_position_X--;
        break;
    case MOVE_DOWN_CAPS:
    case MOVE_DOWN_LOWER:
        new_cursor_position_Y++;
        break;
    case MOVE_RIGHT_CAPS:
    case MOVE_RIGHT_LOWER:
        new_cursor_position_X++;
        break;
    case CHAR_WALL:
    case '3':
        place_item(current_console_pointer_Y, current_console_pointer_X, CHAR_WALL);
        break;
    case CHAR_TRAP:
    case '8':
        place_item(current_console_pointer_Y, current_console_pointer_X, CHAR_TRAP);
        break;
    case CHAR_HEALTH_PACK:
    case '=':
        place_item(current_console_pointer_Y, current_console_pointer_X, CHAR_HEALTH_PACK);
        break;
    case 127:
    case 8:
        delete_item(current_console_pointer_Y, current_console_pointer_X);
        break;
    }

    if (new_cursor_position_Y == 0 || new_cursor_position_Y == MAP_HEIGHT-1)
        new_cursor_position_Y = current_console_pointer_Y;
    if (new_cursor_position_X == 0 || new_cursor_position_X == MAP_WIDTH-1)
        new_cursor_position_X = current_console_pointer_X;

    current_console_pointer_Y = new_cursor_position_Y;
    current_console_pointer_X = new_cursor_position_X;
    setCursorPosition(current_console_pointer_X, current_console_pointer_Y);
    return 0;
}

void write_map_to_file(char *username) {
    FILE *fp;
    char filename[50];

    sprintf(filename, USER_MAP_FILENAME_TEMPLATE, username);

    fp = fopen(filename, MODE_WRITE);

    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            fprintf(fp, "%c", map[i][j]);
        }
        fprintf(fp, "%c", CHAR_NEWLINE);
    }

    fclose(fp);
}

void create_map() {
    char input_code = 0;
    system("cls");
    build_default_map(1);
    system("cls");
    render_board(MODE_BUILD);
    do {
        input_code = getch();
        parse_movement_map_creator(input_code);
    } while (input_code != 27);
    write_map_to_file(currentUser.username);
}



void reset_player() {
    playerY = DEFAULT_COORD_Y;
    playerX = DEFAULT_COORD_X;
    isPlayerAlive = PLAYER_IS_ALIVE;
    player_health_count = DEFAULT_HEALTH_COUNT;
}

int play_game() {
    reset_player();
    init_board();
    char move_char = 0;
    int quit = 0;

    system("cls");
    render_board(MODE_PLAY);

    do {
        move_char = getch();
        switch(move_char) {
        case QUIT_KEY:
            quit = 1;
            break;
        default:
            parse_movement(move_char);
            break;
        }
    }while (isPlayerAlive == 1 && quit == 0 );

    switch(isPlayerAlive){
    case 1:
        setCursorPosition(MESSAGE_LOCATION);
        puts("You quitted!");
        getch();
        break;
    case 0:
        setCursorPosition(MESSAGE_LOCATION);
        puts("You died!");
        getch();
        break;
    }
    write_score();
    return 0;
}

void write_score() {
    FILE *fp;
    if (!file_exists(USER_SCOREBOARD_FILE)) {
        fp = fopen(USER_SCOREBOARD_FILE, MODE_WRITE);
    }

    else {
        fp = fopen(USER_SCOREBOARD_FILE, MODE_APPEND);
    }

    fprintf(fp, "%s#%d\n", currentUser.username, player_health_count);
    fclose(fp);
}

void build_map_from_file(char *filename) {
    FILE *fp;
    char curChar = 0;
    fp = fopen(filename, MODE_READ);
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            curChar = fgetc(fp);
            //if we encounter '\n', then do another fgetc()
            if (curChar == CHAR_NEWLINE)
                curChar = fgetc(fp);
            map[i][j] = curChar;
        }
    }
}

//MODE_PLAY 0
void build_default_map(int build_mode) {
    for (int y=0; y<MAP_HEIGHT; y++) {
        for (int x = 0; x<MAP_WIDTH; x++) {
            if (x == 0 || x == 39 || y == 0 || y == 19) {
                map[y][x] = CHAR_WALL; //fprintf(fout, "%c", '#');
            }
            else {
                map[y][x] = CHAR_EMPTY; //fprintf(fout, "%c", ' ');
            }
        }
        //fprintf(fout, "%c", '\n');
    }

    if (build_mode == 0){
        for (int i = 0; i < NUMBER_OF_ITEMS_GENERATED; i++) {
            while (!place_traps(rand() % MAP_HEIGHT, rand() % MAP_WIDTH)) {}
        }

        for (int i = 0; i < NUMBER_OF_ITEMS_GENERATED; i++) {
            while (!place_health_packs(rand() % MAP_HEIGHT, rand() % MAP_WIDTH)) {}
        }
    }
    setCursorPosition(playerX, playerY);
}

void init_board() {
    srand(time(NULL));
    //FILE *fp;
    char user_map[50];
    //char curChar = 0;
    sprintf(user_map, USER_MAP_FILENAME_TEMPLATE, currentUser.username);
    int user_map_file_exists = file_exists(user_map);
    //FILE *fout = fopen("map.txt", "w");

    //build map from custom file!
    if (user_map_file_exists) {
        build_map_from_file(user_map);
    }

    //build default map
    else {
        build_default_map(0);
    }

    //fclose(fout);
    map[playerY][playerX] = CHAR_PLAYER;
    setCursorPosition(playerX, playerY);
}

int place_traps(int coord_Y, int coord_X) {
    switch(check_location_condition(coord_Y, coord_X))
    {
    case LOCATION_IS_WALL:
    case LOCATION_IS_TRAP:
    case LOCATION_IS_PLAYER:
    case LOCATION_IS_HEALTH_PACK:
        return 0;
        break;
    case LOCATION_OK:
        map[coord_Y][coord_X] = CHAR_TRAP;
        return 1;
    }

    return SILENCE_MSVC;
}

int place_health_packs(int coord_Y, int coord_X) {
    switch(check_location_condition(coord_Y, coord_X))
    {
    case LOCATION_IS_WALL:
    case LOCATION_IS_TRAP:
    case LOCATION_IS_PLAYER:
    case LOCATION_IS_HEALTH_PACK:
        return 0;
        break;
    case LOCATION_OK:
        map[coord_Y][coord_X] = CHAR_HEALTH_PACK;
        return 1;
    }
    return SILENCE_MSVC;
}

void render_board(int mode) {
    for (int y=0; y<MAP_HEIGHT; y++) {
        for (int x = 0; x<MAP_WIDTH; x++) {
            if (map[y][x] == CHAR_TRAP)
                    putchar(CHAR_EMPTY);
            else
                putchar(map[y][x]);
        }
        putchar(CHAR_NEWLINE);
    }
    if (mode == MODE_PLAY)
        printf("Your Score : %d\n", player_health_count);
    else if (mode == MODE_BUILD) {
        puts("Items Placed : \n");
        puts("Press '+' to add score point        Press '*' to add trap");
        puts("Press '#' to add wall    Press Backspace to delete item        Press W/A/S/D to move on map");
        puts("Press ESC to save and exit");
    }
}

void add_player_health() {
    player_health_count++;
    setCursorPosition(HEALTH_COUNTER_LOCATION);
    printf("%d", player_health_count);
}

void deduct_player_health(int new_player_Y, int new_player_X) {
    isPlayerAlive = 0;
}

int parse_movement(char move_char) {
    int new_player_Y = playerY;
    int new_player_X = playerX;

    switch(move_char) {
    case MOVE_UP_CAPS:
    case MOVE_UP_LOWER:
        new_player_Y--;
        break;
    case MOVE_LEFT_CAPS:
    case MOVE_LEFT_LOWER:
        new_player_X--;
        break;
    case MOVE_DOWN_CAPS:
    case MOVE_DOWN_LOWER:
        new_player_Y++;
        break;
    case MOVE_RIGHT_CAPS:
    case MOVE_RIGHT_LOWER:
        new_player_X++;
        break;
    }

    int location_condtion = check_location_condition(new_player_Y, new_player_X);
    switch(location_condtion) {
    case LOCATION_IS_WALL:
        return 0;
        break;
    case LOCATION_OK:
        move_character(new_player_Y, new_player_X);
        return 1;
        break;
    case LOCATION_IS_HEALTH_PACK:
        move_character(new_player_Y, new_player_X);
        add_player_health();
        return 1;
        break;
    case LOCATION_IS_TRAP:
        //TODO when player hits trap
        deduct_player_health(new_player_Y, new_player_X);
        return -1;
        break;
    }
    return 0;
}

void move_character(int new_player_Y, int new_player_X) {
    setCursorPosition(playerX, playerY); putchar(CHAR_EMPTY);

    map[playerY][playerX] = CHAR_EMPTY;
    playerY = new_player_Y;
    playerX = new_player_X;
    map[playerY][playerX] = CHAR_PLAYER;

    setCursorPosition(playerX, playerY);
    putchar(CHAR_PLAYER);
}

//NOTE: this functions now not only checks player movement,
//but also checks for the condition on given coordinate
//TODO: rename function name to check_location_condition()
//returns:
//  1 if movement is successful
//  0 if player hits wall
//  -1 if hits trap and died
//  2 if it hits health pack
//  3

int check_location_condition(int new_player_Y, int new_player_X) {
    if (map[new_player_Y][new_player_X] == CHAR_WALL)
        return LOCATION_IS_WALL;
    else if (map[new_player_Y][new_player_X] == CHAR_EMPTY)
        return LOCATION_OK;
    else if (map[new_player_Y][new_player_X] == CHAR_HEALTH_PACK)
        return LOCATION_IS_HEALTH_PACK;
    else if (map[new_player_Y][new_player_X] == CHAR_TRAP)
        return LOCATION_IS_TRAP;
    else if (map[new_player_Y][new_player_X] == CHAR_PLAYER)
        return LOCATION_IS_PLAYER;

    return SILENCE_MSVC;
}

void setCursorPosition(int x, int y) {
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    COORD coord = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hOut, coord);
}
