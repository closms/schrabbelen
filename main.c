/* $Id: main.c,v 1.8 2008/03/21 02:22:10 mike Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <search.h>

enum {
    HORIZ,
    VERT,
};

#define LEN 15  /* Can't change len. */

#define GREY   "[00;37m"
#define LBLUE  "[00;36m"
#define PURPLE "[00;35m"
#define BLUE   "[00;34m"
#define ORANGE "[00;33m"
#define GREEN  "[00;32m"
#define RED    "[00;31m"
#define NORM   "[00m"

char Version[] = "$Revision: 1.8 $";
char Date[] = "$Date: 2008/03/21 02:22:10 $";

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

char letter_map[26];
char backup_letter_map[26];

#define EMPTY_TRAY_BONUS 50

int letter_scores[26] = {
 1, 3, 3, 2, 1,  /* A, B, C, D, E */
 4, 2, 4, 1, 8,  /* F, G, H, I, J */
 5, 1, 3, 1, 1,  /* K, L, M, N, O */
 3,10, 1, 1, 1,  /* P, Q, R, S, T */
 1, 4, 4, 8, 4,  /* U, V, W, X, Y */
 10              /* Z */
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

#define FLAG_LETTER(x) (letter_map[(x)-'a']++)

int
reset_letter_map()
{
    memcpy(letter_map, backup_letter_map, 26);
}

int
enough_letters(char *word)
{
    char *pc;
    int ret = 1;
    for (pc = word; *pc!=0; pc++) {
        letter_map[*pc-'a']--;
        if (letter_map[*pc-'a'] < 0) {
            ret = 0;
            goto done;
        }
    }
done:
    reset_letter_map();
    return ret;
}

void
read_board()
{
    char buf[1024];
    int ix, ix2;
    char *pc;

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
            if (buf[ix2] != '_') {
                FLAG_LETTER(buf[ix2]);
            }
        }
    }
    fprintf(stderr, "Read game board.\n");

    /* Read tray letters */
    pc = fgets(buf, 1024, stdin);
    if (pc == NULL) {
        fprintf(stderr, "Autogenerating tray.\n");
        srand(time(NULL));
        tray = malloc(8+1);
        assert(tray);
        for (ix2=0; ix2<8; ix2++) {
            tray[ix2] = (rand() % 26) + 'a';
        }
        tray[8] = 0;
    }
    else {
        if (buf[strlen(buf)-1] == '\n')
            buf[strlen(buf)-1] = 0;
        tray = strdup(buf);
        assert(tray);
    }
    used_tray = calloc(strlen(tray)+1, sizeof(int));
    assert(used_tray);
    tray_size = strlen(tray);
    for (ix2 = 0; ix2 < tray_size; ix2++) {
        FLAG_LETTER(tray[ix2]);
    }
    fprintf(stderr, "Read %d tray letters.\n", strlen(tray));

    memcpy(backup_letter_map, letter_map, 26);
}

void
load_words()
{
    FILE *f;
    int n, ix;
    char buf[1024];

    f = popen("/usr/bin/wc -l words", "r");
    if (f == NULL) {
        perror("popen(/usr/bin/wc -l words) failed");
        exit(99);
    }
    if (fscanf(f, "%d", &n) != 1) {
        fprintf(stderr, "Failed to read number of words in dict/words.\n");
        exit(99);
    }
    pclose(f);

    f = fopen("words", "r");
    if (f == NULL) {
        perror("fopen(words) failed");
        exit(99);
    }

    num_words = 0;
    words = malloc(n * sizeof(char *));
    if (words == NULL) {
        perror("malloc() failed");
        exit(99);
    }
    if (hcreate(n) == 0) {
        perror("hcreate()");
        exit(99);
    }
    for (ix = 0; ix < n; ix++) {
        char *pc;
        struct entry hent;
        if (fgets(buf, 1024, f) != NULL) {
            if (buf[strlen(buf)-1] == '\n')
                buf[strlen(buf)-1] = 0;
            for (pc=buf; *pc!=0; pc++) {
                int b;
                b = isupper(*pc);
                if (b) {
                    goto again;
                }
            }
            pc = strdup(buf);
            words[num_words++] = pc;
#if 0
            for (pc=words[num_words-1]; *pc!=0; pc++) {
                *pc = tolower(*pc);
            }
#endif
            /* Load word into word hash table */
            hent.key = pc;
            hent.data = (void*)1;
            if (hsearch(hent, ENTER) == 0) {
                perror("hearch()");
                exit(99);
            }

        }
again:
        ;
    }
    fclose(f);
    fprintf(stderr, "Read %d words from dictionary.\n", num_words);
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

int
score_word(int row, int col, int dir)
{
    int ix;
    int score = 0;
    int mult = 1;
    int bonus;

    if (dir == HORIZ) {
        /* Horizontal word */
        for (ix = col; ix < LEN && board[row][ix] != '_'; ix++) {
            score += (SCORE(board[row][ix]) * letter_mult[row][ix]);
            mult *= word_mult[row][ix];
        }
    }
    else {
        /* Vertical word */
        for (ix = row; ix < LEN && board[ix][col] != '_'; ix++) {
            score += (SCORE(board[ix][col]) * letter_mult[ix][col]);
            mult *= word_mult[ix][col];
        }
    }

    return score * mult;
}

char *
is_vert_word(int row_num, int col_num, int *pr, int *pc)
{
    /* If this cell was empty on the original board, and one of the cells
     * above or below (or both) were occupied in the original board, then
     * this is a new word.
     */
    char buf[LEN+1];
    int bix = 0;

    if ( /* Case 1: word was already on the board. */
        (backup_board[row_num][col_num] != '_') ||
         /* Case 2: vert word starts at top of board. */
        (row_num==0 && backup_board[row_num+1][col_num] == '_') ||
         /* Case 3: vert word ends at bottom of board. */
        (row_num+1==LEN && backup_board[row_num-1][col_num] == '_') ||
         /* Case 4: Word is in the middle but neighbours are blank. */
        ((backup_board[row_num-1][col_num] == '_' &&
        backup_board[row_num+1][col_num] == '_')))
    {
        return NULL;
    }

    /* Find the beginning of the word */
    while(row_num > 0 && board[row_num][col_num] != '_')
        row_num--;
    row_num++;
    *pr = row_num;
    *pc = col_num;

    /* Copy word */
    while(row_num < LEN && board[row_num][col_num] != '_')
        buf[bix++] = board[row_num++][col_num];

    buf[bix] = 0;
    return strdup(buf);
}

char *
is_horiz_word(int row_num, int col_num, int *pr, int *pc)
{
    /* If this cell was empty on the original board, and one of the cells
     * left or right (or both) were occupied in the original board, then
     * this is a new word.
     */
    char buf[LEN+1];
    int bix = 0;

    if ( /* Case 1: word was already on the board. */
        (backup_board[row_num][col_num] != '_') ||
         /* Case 2: horiz word starts at left of board. */
        (col_num==0 && backup_board[row_num][col_num+1] == '_') ||
         /* Case 3: horiz word ends at right of board. */
        (col_num+1==LEN && backup_board[row_num][col_num-1] == '_') ||
         /* Case 4: Word is in the middle but neighbours are blank. */
        ((backup_board[row_num][col_num-1] == '_' &&
        backup_board[row_num][col_num+1] == '_')))
    {
        return NULL;
    }
    
    /* Find the beginning of the word */
    while(col_num > 0 && board[row_num][col_num] != '_')
        col_num--;
    col_num++;
    *pr = row_num;
    *pc = col_num;

    /* Copy word */
    while(col_num < LEN && board[row_num][col_num] != '_')
        buf[bix++] = board[row_num][col_num++];

    buf[bix] = 0;
    return strdup(buf);
}

int
is_word(char *word)
{
    struct entry hent;
    hent.key = word;
    return hsearch(hent, FIND)!=NULL;
}

void
search(int dir)
{
    int word_num, row_num, col_num;
    int score = 0;
    int word_len, rstop, cstop, ltr, nix;
    char *w;
    char *new_words[LEN];
    int new_words_r[LEN];
    int new_words_c[LEN];
    int num_new_words;
    int attached_flag, placed_flag;
    int bonus;

    for (word_num=0; word_num<num_words; word_num++) {
        w = words[word_num];
        if (!enough_letters(w)) {
            continue;
        }
        word_len = strlen(w);

        /* Output some indication that we're making progress. */
        if (word_num % 10000 == 0) {
            fputc('.', stderr);
        }
    
        if (dir == HORIZ)
            rstop = LEN;
        else
            rstop = LEN-word_len+1;

        for (row_num=0; row_num<rstop; row_num++) {
            /* Try to place the word in a row */

            if (dir == HORIZ )
                cstop = LEN-word_len+1;
            else
                cstop = LEN;

            for (col_num=0; col_num<cstop; col_num++) {

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
                if (dir == HORIZ) {
                    if (col_num > 0 && board[row_num][col_num-1] != '_')
                        goto next_col;
                }
                else {
                    if (row_num > 0 && board[row_num-1][col_num] != '_')
                        goto next_col;
                }

                /* words can't end right in-front of another word. */
                if (dir == HORIZ) {
                    if ((col_num+word_len)<(LEN-1)
                            && board[row_num][col_num+word_len+1] != '_')
                        goto next_col;
                }
                else {
                    if ((row_num+word_len)<(LEN-1)
                            && board[row_num+word_len+1][col_num] != '_')
                        goto next_col;
                }

                /* Match up letters, iterate over the letters of the word. */
                for (ltr = 0; ltr<word_len; ltr++) {
                    if (dir == HORIZ) {
                        if (row_num==(LEN/2)&&(col_num+ltr)==(LEN/2)) {
                            attached_flag = 1;
                        }
                    }
                    else {
                        if ((row_num+ltr)==(LEN/2)&&col_num==(LEN/2)) {
                            attached_flag = 1;
                        }
                    }
                    if ((dir == HORIZ && board[row_num][col_num+ltr] != '_')
                     || (dir == VERT && board[row_num+ltr][col_num] != '_')) {
                        /* board space is in use. */
                        if ((dir==HORIZ&&board[row_num][col_num+ltr]==w[ltr])
                          ||(dir==VERT&&board[row_num+ltr][col_num]==w[ltr])) {
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
                            if (dir == HORIZ)
                                board[row_num][col_num+ltr] = w[ltr];
                            else
                                board[row_num+ltr][col_num] = w[ltr];
                            placed_flag = 1;
                        }
                        else {
                            /* Letter isn't in our tray, move to next column
                             * and keep looking. */
                            goto next_col;
                        }
                    }
                }

                if (attached_flag == 0 || placed_flag == 0) {
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
                /* Part 1, build the list of vert/hotiz words. */
                for (ltr=0; ltr<word_len; ltr++) {
                    char *pc;
                    if (dir==HORIZ) {
                        /* New word is horiz, so check for vert words. */
                        if ((pc=is_vert_word(row_num, col_num+ltr,
                                        &new_words_r[num_new_words],
                                        &new_words_c[num_new_words]))!=NULL) {
                            new_words[num_new_words++] = pc;
                        }
                    }
                    else {
                        /* New word is vert so check for horiz words. */
                        if ((pc=is_horiz_word(row_num+ltr, col_num,
                                        &new_words_r[num_new_words],
                                        &new_words_c[num_new_words]))!=NULL) {
                            new_words[num_new_words++] = pc;
                        }
                    }
                }

                /* From that list, check if the words are real words */
                for (nix=0; nix<num_new_words; nix++) {
                    int isword = is_word(new_words[nix]);
                    if (!isword) {
                        /* Bummer, free temp data, move on to next position. */
                        for(ltr=0; ltr<num_new_words; ltr++) {
                            free(new_words[ltr]);
                        }
                        goto next_col;
                    }
                }

                /* Looks like everything is nice and legal, so compute the
                 * total score. */
                score = score_word(row_num, col_num, dir);
                for(nix = 0; nix < num_new_words; nix++) {
                    score += score_word(new_words_r[nix],
                                        new_words_c[nix],
                                        dir);
                    free(new_words[nix]);
                }

                /* Check for empty tray bonus. */
                bonus = EMPTY_TRAY_BONUS;
                for (nix=0; nix < tray_size; nix++) {
                    if (used_tray[nix] == 0) {
                        bonus = 0;
                    }
                }
                score += bonus;

                /* Is this the best words we've found? */
                if (score > best_score) {
                    fprintf(stderr, "%s(%d)", w, score);
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
    int istty;

    /*istty = (access("/dev/tty", F_OK)==0);*/
    istty = 0;
    for (row=0; row<LEN; row++) {
        for (col=0; col<LEN; col++) {
            if (istty && word_mult[row][col] == 3)
                fputs(RED, stdout);
            if (istty && word_mult[row][col] == 2)
                fputs(BLUE, stdout);
            if (istty && letter_mult[row][col] == 3)
                fputs(GREEN, stdout);
            if (istty && letter_mult[row][col] == 2)
                fputs(ORANGE, stdout);
            putc(best_board[row][col], stdout);
            if (istty)
                fputs(NORM, stdout);
        }
        putc('\n', stdout);
    }
}


int
main(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch(opt) {
            case 'v':
                fprintf(stderr, "Version: %s\n", Version);
                fprintf(stderr, "Date: %s\n", Date);
                exit(0);
            default:
                fprintf(stderr, "Huh(%c)\n", opt);
                exit(99);
        }
    }

    read_board();
    load_words();

    search(HORIZ);
    search(VERT);

    print_result();

}


