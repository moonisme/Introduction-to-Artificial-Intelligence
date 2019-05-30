/*********************************************************
 *  agent.c
 *  Nine-Board Tic-Tac-Toe Agent
 *  COMP3411/9414/9814 Artificial Intelligence
 *  Alan Blair, CSE, UNSW
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "agent.h"
#include "common.h"
#include "game.h"

#define MAX_MOVE 81

#define INF 0xffffff // should be large enough
#define DEFAULT_DEPTH 8 // default search depth

int board[10][10];
int move[MAX_MOVE + 1];
int player;
int m;
int DEPTH; // search depth,

// int cur_player;

/*********************************************************/ /*
    Print usage information and exit
 */
void usage(char argv0[])
{
    printf("Usage: %s\n", argv0);
    printf("       [-p port]\n"); // tcp port
    printf("       [-h host]\n"); // tcp host
    exit(1);
}

/*********************************************************/ /*
    Parse command-line arguments
 */
void agent_parse_args(int argc, char* argv[])
{
    int i = 1;

    DEPTH = DEFAULT_DEPTH;
    while (i < argc) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
            }
            port = atoi(argv[i + 1]);
            i += 2;
        } else if (strcmp(argv[i], "-h") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
            }
            host = argv[i + 1];
            i += 2;
        } else if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
            }
            DEPTH = atoi(argv[i + 1]);
            i += 2;
        } else {
            usage(argv[0]);
        }
    }
}

/*********************************************************
   Return TRUE if the game has been won on this sub-board
*/
int gamewon2(int p, int bb[10])
{
    return ((bb[1] == p && bb[2] == p && bb[3] == p) || (bb[4] == p && bb[5] == p && bb[6] == p) || (bb[7] == p && bb[8] == p && bb[9] == p) || (bb[1] == p && bb[4] == p && bb[7] == p) || (bb[2] == p && bb[5] == p && bb[8] == p) || (bb[3] == p && bb[6] == p && bb[9] == p) || (bb[1] == p && bb[5] == p && bb[9] == p) || (bb[3] == p && bb[5] == p && bb[7] == p));
}

/*********************************************************
   Return 10000 if player occupied two of bb[c1],bb[c2],bb[c3],
   and the rest one is EMPTY.
   Return -10000 if !player matches that.
*/
int evaluate_line(int player, int bb[10], int c1, int c2, int c3, int* flg1,
    int* flg2)
{
    int val;

    val = 0;

    if (*flg1) {
        if ((bb[c1] == player && bb[c2] == player && bb[c3] == EMPTY) || (bb[c1] == player && bb[c2] == EMPTY && bb[c3] == player) || (bb[c1] == EMPTY && bb[c2] == player && bb[c3] == player)) {
            val += 10000;
            *flg1 = 0;
            return val;
        }
    }

    if (*flg2) {
        player = !player;
        if ((bb[c1] == player && bb[c2] == player && bb[c3] == EMPTY) || (bb[c1] == player && bb[c2] == EMPTY && bb[c3] == player) || (bb[c1] == EMPTY && bb[c2] == player && bb[c3] == player)) {
            val -= 10000;
            *flg2 = 0;
        }
    }
    return val;
}

/*********************************************************
   Return its value of sub-board bb from player's point of view
*/
int evaluate_subborad(int player, int bb[10])
{
    int val, flg1, flg2;

    val = 0;
    flg1 = flg2 = 1;
    val += evaluate_line(player, bb, 1, 2, 3, &flg1, &flg2);
    val += evaluate_line(player, bb, 4, 5, 6, &flg1, &flg2);
    val += evaluate_line(player, bb, 7, 8, 9, &flg1, &flg2);
    val += evaluate_line(player, bb, 1, 4, 7, &flg1, &flg2);
    val += evaluate_line(player, bb, 2, 5, 8, &flg1, &flg2);
    val += evaluate_line(player, bb, 3, 6, 9, &flg1, &flg2);
    val += evaluate_line(player, bb, 1, 5, 9, &flg1, &flg2);
    val += evaluate_line(player, bb, 3, 5, 7, &flg1, &flg2);

    return val;
}

/*********************************************************
   Return its value of the whole board from player's point of view
*/
int evaluate_whole_board(int player)
{
    int i;
    int val;

    val = 0;
    for (i = 1; i <= 9; i++) {
        val += evaluate_subborad(player, board[i]);
    }

    return val;
}

/*********************************************************
   Alpha-beta search algorithm, return the best move for player
*/
int alpha_beta_search(int alpha, int beta, int depth, int player)
{
    int i, val, maxval, idx;
    int* subborad = board[move[m - 1]]; // board[pre_move];  //

    if (depth == 0) {
        // printf("EV %d\n", player);
        return evaluate_whole_board(player);
    }

    idx = 0;
    maxval = -INF;
    for (i = 1; i <= 9; i++) {
        if (subborad[i] == EMPTY) {
            subborad[i] = player;

            if (gamewon2(player, subborad)) {
                subborad[i] = EMPTY;

                if (depth == DEPTH) {
                    // printf("move %d I%d\n", i, INF);
                    return i;
                }
                return INF;
            }

            move[m++] = i;
            val = -alpha_beta_search(-beta, -alpha, depth - 1, !player);
            --m;

            subborad[i] = EMPTY;

            if (val > maxval) {
                maxval = val;
                idx = i;
            }

            if (val >= beta) {
                if (depth == DEPTH) {
                    // printf("move %d B%d\n", i, beta);
                    return i;
                }
                return beta;
            }

            if (val > alpha) {
                alpha = val;
            }

            if (idx == 0) {
                idx = i;
            }
        }
    }

    if (depth == DEPTH) {
        // printf("move %d A%d\n", idx, maxval);
        return idx;
    }
    return alpha;
}

/*********************************************************/ /*
    Called at the beginning of a series of games
 */
void agent_init()
{
    struct timeval tp;

    // generate a new random seed each time
    gettimeofday(&tp, NULL);
    srandom((unsigned int)(tp.tv_usec));
}

/*********************************************************/ /*
    Called at the beginning of each game
 */
void agent_start(int this_player)
{
    reset_board(board);
    m = 0;
    move[m] = 0;
    player = this_player;
}

/*********************************************************/ /*
    Choose second move and return it
 */
int agent_second_move(int board_num, int prev_move)
{
    int this_move;
    move[0] = board_num;
    move[1] = prev_move;
    board[board_num][prev_move] = !player;
    m = 2;

    this_move = alpha_beta_search(-INF * 10, INF * 10, DEPTH, player);

    move[m] = this_move;
    board[prev_move][this_move] = player;
    return (this_move);
}

/*********************************************************/ /*
    Choose third move and return it
 */
int agent_third_move(int board_num, int first_move, int prev_move)
{
    int this_move;
    move[0] = board_num;
    move[1] = first_move;
    move[2] = prev_move;
    board[board_num][first_move] = player;
    board[first_move][prev_move] = !player;
    m = 3;

    this_move = alpha_beta_search(-INF * 10, INF * 10, DEPTH, player);

    move[m] = this_move;
    board[move[m - 1]][this_move] = player;
    return (this_move);
}

/*********************************************************/ /*
    Choose next move and return it
 */
int agent_next_move(int prev_move)
{
    int this_move;
    m++;
    move[m] = prev_move;
    board[move[m - 1]][move[m]] = !player;
    m++;

    this_move = alpha_beta_search(-INF * 10, INF * 10, DEPTH, player);

    move[m] = this_move;
    board[move[m - 1]][this_move] = player;
    return (this_move);
}

/*********************************************************/ /*
    Receive last move and mark it on the board
 */
void agent_last_move(int prev_move)
{
    m++;
    move[m] = prev_move;
    board[move[m - 1]][move[m]] = !player;
}

/*********************************************************/ /*
    Called after each game
 */
void agent_gameover(int result, // WIN, LOSS or DRAW
    int cause // TRIPLE, ILLEGAL_MOVE, TIMEOUT or FULL_BOARD
)
{
    // nothing to do here
}

/*********************************************************/ /*
    Called after the series of games
 */
void agent_cleanup()
{
    // nothing to do here
}
