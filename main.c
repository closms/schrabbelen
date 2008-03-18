#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

enum {
    HORIZ,
    VERT,
};

#define LEN 15  /* Can't change len. */

char board[LEN][LEN];
char backup_board[LEN][LEN];
char best_board[LEN][LEN];
char *tray = NULL;
char *used_tray = NULL;
int tray_size = 0;
char **words = NULL;
int num_words = 0;
int best_score = 0;
int empty_board = 1;

int letter_scores[26] = {
 1, 1, 1, 1, 1,  /* A, B, C, D, E */
 1, 1, 1, 1, 1,  /* F, G, H, I, J */
 1, 1, 1, 1, 1,  /* K, L, M, N, O */
 1, 1, 1, 1, 1,  /* P, Q, R, S, T */
 1, 1, 1, 1, 1,  /* U, V, W, X, Y */
 1               /* Z */
};

int letter_mult[LEN][LEN] = {
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1},
    {1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1},
    {2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 3, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 3, 1},
    {1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 1},
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1},
    {1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 1},
    {1, 3, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 3, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2},
    {1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 1, 1},
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1},
};

int word_mult[LEN][LEN] = {
    {3, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 3},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1},
    {1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1},
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1},
    {1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1},
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1},
    {1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1},
    {3, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 3},
};

#define SCORE(x) (letter_scores[(x)-'a'])

void
read_board()
{
    char buf[1024];
    int ix, ix2;

    /* Read board. */
    for (ix = 0; ix < LEN; ix++) {
        if (fgets(buf, 1024, stdin) == NULL) {
            fprintf(stderr, "Board must be %d columns.\n", LEN);
            exit(99);
        }
        if (strlen(buf)-1 != LEN) {
            fprintf(stderr, "Line %d is wrong length, should be %d.\n",
                    ix+1, LEN);
            exit(99);
        }
        if (strlen(buf) != (LEN+1)) {
            fprintf(stderr, "Error in input, line %d\n", ix+1);
            exit(99);
        }
        for (ix2=0; ix2<LEN; ix2++) {
            if (!islower(buf[ix2]) && buf[ix2] != '_') {
                fprintf(stderr, "Invalid Character, line %d, char %d\n",
                        ix+1, ix2+1);
                exit(99);
            }
            board[ix][ix2] = buf[ix2];
            backup_board[ix][ix2] = buf[ix2];
        }
    }
    fprintf(stderr, "Read game board.\n");

    /* Read tray letters */
    fgets(buf, 1024, stdin);
    if (buf[strlen(buf)-1] == '\n')
        buf[strlen(buf)-1] = 0;
    tray = strdup(buf);
    assert(tray);
    used_tray = calloc(strlen(tray)+1, sizeof(int));
    assert(used_tray);
    tray_size = strlen(tray);
    fprintf(stderr, "Read %d tray letters.\n", strlen(tray));
}

void
load_words()
{
    FILE *f;
    int n, ix;
    char buf[1024];

    f = popen("/usr/bin/wc -l /usr/share/dict/words", "r");
    if (f == NULL) {
        perror("popen(/usr/bin/wc -l /usr/share/dict/words) failed");
        exit(99);
    }
    if (fscanf(f, "%d", &n) != 1) {
        fprintf(stderr, "Failed to read number of words in dict/words.\n");
        exit(99);
    }
    pclose(f);
    fprintf(stderr, "Read %d words from dictionary.\n", n);

    f = fopen("/usr/share/dict/words", "r");
    if (f == NULL) {
        perror("fopen(/usr/share/dict/words) failed");
        exit(99);
    }

    num_words = 0;
    words = malloc(n * sizeof(char *));
    if (words == NULL) {
        perror("malloc() failed");
        exit(99);
    }
    for (ix = 0; ix < n; ix++) {
        char *pc;
        if (fgets(buf, 1024, f) != NULL) {
            if (buf[strlen(buf)-1] == '\n')
                buf[strlen(buf)-1] = 0;
            words[num_words++] = strdup(buf);
            for (pc=words[num_words-1]; *pc!=0; pc++) {
                *pc = tolower(*pc);
            }
        }
    }
    fclose(f);
}

void
save_best_board()
{
    int row;
    for (row = 0; row < LEN; row++) {
        memcpy(best_board[row], board[row], LEN*sizeof(char));
    }
}

void
reset_board()
{
    memcpy(board, backup_board, sizeof(board));
}

void
reset_tray()
{
    memset(used_tray, 0, tray_size * sizeof(int));
}

int
letter_avail(char c)
{
    int ix;
    for (ix=0; ix < tray_size; ix++) {
        if (tray[ix] == c && used_tray[ix] == 0)
            return 1;
    }
    return 0;
}

void
mark_used(char c)
{
    int ix;
    for (ix=0; ix < tray_size; ix++) {
        if (tray[ix] == c && used_tray[ix] == 0) {
            used_tray[ix] = 1;
            return;
        }
    }
    assert(0);
}

char *
is_vert_word(int row, int col)
{
    /* (row,col) is some position on the board with a letter in it.
     * Find the boundries of the word (only if its longer then a single
     * letter. */

    /* strdup() and return. */
    return NULL;
}

int
is_word(char *w)
{
}

int
score_word(int row, int col, int direction)
{
    int ix;
    int score = 0;
    int mult = 1;
    assert(direction == HORIZ);

    /* Horizontal word */
    for (ix = col; ix < LEN && board[row][ix] != '_'; ix++) {
        score += (SCORE(board[row][ix]) * letter_mult[row][ix]);
        mult *= word_mult[row][ix];
    }
    return score * mult;
}

void
search(int dir)
{
    int word_num, row_num, col_num;
    int score = 0;
    int word_len, stop, ltr, nix;
    char *w;
    char *new_words[LEN];
    int num_new_words;
    int attached_flag, placed_flag;

    for (word_num=0; word_num<num_words; word_num++) {
        w = words[word_num];
        word_len = strlen(w);
//        printf("Try word %s\n", w);

        /* Output some indication that we're making progress. */
        if (word_num % 10000 == 0) {
            fputc('.', stderr);
            fflush(stderr);
        }

        for (row_num=0; row_num<LEN; row_num++) {
//            printf("row: %d\n", row_num);
            /* Try to place the word in a row */

            stop = LEN-word_len;
            for (col_num=0; col_num<stop; col_num++) {

                /* When we try anew to place the word, we need to reset
                 * the letter tray and the game board back to the original
                 * state.
                 */
                reset_board();
                reset_tray();
                attached_flag = 0;
                placed_flag = 0;
                /* Try to place word horizontally in row row_num and
                 * column col_num.
                 */

                /* words can't start right after another word. */
                if (col_num > 0 && board[row_num][col_num-1] != '_')
                    goto next_col;

                /* words can't end right in-front of another word. */
                if ((col_num+word_len)<(LEN-1)
                        && board[row_num][col_num+word_len+1] != '_')
                    goto next_col;

                /* Match up letters, iterate over the letters of the word. */
                for (ltr = 0; ltr<word_len; ltr++) {
                    if (row_num==((LEN+1)/2) && (col_num+ltr)==((LEN+1)/2)) {
                        attached_flag = 1;
                    }
                    if (board[row_num][col_num+ltr] != '_') {
                        /* board space is in use. */
                        if (board[row_num][col_num+ltr] == w[ltr]) {
                            /* Good, the letter is already in place. */
                            attached_flag = 1;
//                            printf("using existing letter '%c'\n", w[ltr]);
                        }
                        else {
                            /* mismatch, move on to next column. */
//                            printf("existing word is blocking this word.\n");
                            goto next_col;
                        }
                    }
                    else {
                        /* Board space is empty, is the needed letter in our
                         * tray? */
                        if (letter_avail(w[ltr])) {
                            /* Place the letter and move on. */
                            mark_used(w[ltr]);
                            board[row_num][col_num+ltr] = w[ltr];
                            placed_flag = 1;
//                            printf("good, using letter from tray\n");
                        }
                        else {
                            /* Letter isn't in our tray, move to next column
                             * and keep looking. */
//                            printf("needed letter isn't avail.\n");
                            goto next_col;
                        }
                    }
                }

//                printf("OK?\n");
                if (attached_flag == 0 || placed_flag == 0) {
                    /* Too bad, word isn't attached to the rest of the puzzle.
                     */
                    goto next_col;
                }
//                printf("OK\n");

                /* Is it legal? */

#if 0
                /* First build a list of all the new words we made. */
                /* We know that the word is not bordered on the left and
                 * right by another word.  So we just need to check for
                 * vertical words.
                 */

                num_new_words = 0;
                /* Part 1, build the list of vertical words. */
                for (ltr=0; ltr<word_len; ltr++) {
                    char *pc;
                    if ((pc=is_vert_word(row_num, col_num+ltr))!=NULL) {
                        new_words[num_new_words] = pc;
                    }
                }

                /* From that list, check if the words are real words */
                for (nix=0; nix<num_new_words; nix++) {
                    if (!is_word(new_words[nix])) {
                        /* Bummer, free temp data, move on to next position. */
                        for(ltr=0; ltr<num_new_words; ltr++) {
                            free(new_words[ltr]);
                        }
                        goto next_col;
                    }
                }
#endif
                /* Looks like everything is nice and legal, so compute the
                 * total score. */
                score = score_word(row_num, col_num, HORIZ);

                /* Is this the best words we've found? */
//                printf("%s, %d, %d\n", w, score, best_score);
                if (score > best_score) {
                    fprintf(stderr, "new best score: %d, %s\n", score, w);
                    best_score = score;
                    save_best_board();
                }
next_col:
                ;
            }
next_row:
            ;
        }
next_word:
        ;
    }
    fputc('\n', stderr);
}

void
print_result()
{
    int col, row;
    for (row=0; row<LEN; row++) {
        for (col=0; col<LEN; col++) {
            putc(best_board[row][col], stdout);
        }
        putc('\n', stdout);
    }
}


int
main()
{

    read_board();
    load_words();

    search(HORIZ);
    search(VERT);

    print_result();

}


