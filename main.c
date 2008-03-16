#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN 11  /* Can't change len. */

char board[LEN][LEN];
char backup_board[LEN][LEN];
char *tray = NULL;
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
        if (fgets(buf, 1024, f) != NULL) {
            if (buf[strlen(buf)-1] == '\n')
                buf[strlen(buf)-1] = 0;
            words[num_words++] = strdup(buf);
        }
    }
    fclose(f);
}

void
save_best_board()
{

}

void
reset_board()
{
}

void
reset_tray()
{
}

int
letter_avail(char c)
{
}

void
mark_used(char c)
{
}

char *
is_vert_word(int row, int col)
{
    /* (row,col) is some position on the board with a letter in it.
     * Find the boundries of the word (only if its longer then a single
     * letter. */

    /* strdup() and return. */
}

int
is_word(char *w)
{
}

int
score_word(char *w)
{
}

void
find_horiz()
{
    int word_num, row_num, col_num;
    int score = 0;
    int word_len, stop, ltr, nix;
    char *w;
    char *new_words[LEN];
    int num_new_words;

    for (word_num=0; word_num<num_words; word_num++) {
        w = words[word_num];
        word_len = strlen(w);

        /* Output some indication that we're making progress. */
        if (word_num % 10000 == 0) {
            fputc('.', stderr);
            fflush(stderr);
        }

        for (row_num=0; row_num<LEN; row_num++) {
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
                    if (board[row_num][col_num+ltr] != '_') {
                        /* board space is in use. */
                        if (board[row_num][col_num+ltr] == w[ltr]) {
                            /* Good, the letter is already in place. */
                            attached_flag = 1;
                        }
                        else {
                            /* mismatch, move on to next column. */
                            goto next_col;
                        }
                    }
                    else {
                        /* Board space is empty, is the needed letter in our
                         * tray? */
                        if (letter_avail(w[ltr])) {
                            /* Place the letter and move on. */
                            mark_used(w[ltr]);
                        }
                        else {
                            /* Letter isn't in our tray, move to next column
                             * and keep looking. */
                            goto next_col;
                        }
                    }
                }

                if (attached_flag == 0) {
                    /* Too bad, word isn't attached to the rest of the puzzle.
                     */
                    goto next_col;
                }

                /* Is it legal? */

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

                /* Looks like everything is nice and legal, so compute the
                 * total score. */
                score = 0;
                for (nix=0; nix<num_new_words; nix++) {
                    score += score_word(new_words[nix]);
                }

                /* Is this the best words we've found? */
                if (score > best_score) {
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
find_vert()
{
}

void
print_result()
{
}


int
main()
{

    read_board();
    load_words();

    find_horiz();
    find_vert();

    print_result();

}


