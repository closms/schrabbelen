/* $Id: main.c,v 1.20 2013/04/14 06:50:39 mclosson Exp $ */
/* vim: cin et ts=4 sw=4
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <search.h>
#include <time.h>

#include "hs.h"

enum {
    HORIZ,
    VERT,
};

#define LEN 15  /* Can't change len. */
#define BLANK 26
#define INC 10

typedef char board_t[LEN][LEN];
typedef char map_t[27];
typedef struct {
    board_t board;
    char word[LEN+1];
    int score;
} score_t;


char Version[] = "$Revision: 1.20 $";
char Date[] = "$Date: 2013/04/14 06:50:39 $";

board_t board;
board_t backup_board;
score_t *best_boards;
int best_boards_num = 0;
int best_boards_size = 0;
board_t blanks;
char *tray = NULL;
char *used_tray = NULL;
int tray_size = 0;
char **words = NULL;
int num_words = 0;
int empty_board = 1;
char wordfile[1024];

map_t boardmap;
map_t rowmap[15];
map_t colmap[15];

int debug = 0;

#define EMPTY_TRAY_BONUS 35

int letter_scores[26] = {
 1, 4, 4, 2, 1,  /* A, B, C, D, E */
 4, 3, 3, 1,10,  /* F, G, H, I, J */
 5, 2, 4, 2, 1,  /* K, L, M, N, O */
 4,10, 1, 1, 1,  /* P, Q, R, S, T */
 2, 5, 4, 8, 3,  /* U, V, W, X, Y */
10               /* Z */
};

board_t letter_mult = {
    {1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1},
    {1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1},
    {1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1},
    {1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1},
    {1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 1},
    {1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 1, 1},
    {3, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {3, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 3},
    {1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 1, 1, 1},
    {1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 1},
    {1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1},
    {1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1},
    {1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1},
    {1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1},
};

board_t word_mult = {
    {1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1},
    {1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {3, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {3, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1},
    {1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1},
};

#define SCORE(x) ((x)=='?'?0:(letter_scores[(x)-'a']))

#define FLAG_LETTER(x,b) ((x)=='?'?((b)[BLANK]++):((b)[(x)-'a']++))

int
cmp_score(const void *v1, const void *v2)
{
    const score_t *s1 = v1;
    const score_t *s2 = v2;
    return s1->score - s2->score;
}

int
score(int row, int col)
{
    if (blanks[row][col]) {
      return 0;
    }
    return SCORE(board[row][col]);
}

void
reset_blanks()
{
    memset(blanks, 0, sizeof(blanks));
}

int
enough_letters(char *word, char *letter_map)
{
    char *pc;
    int rv = 0;
    int ret = 1;
    char local_map[27];

    memcpy(local_map, letter_map, 27);
    for (pc = word; *pc!=0; pc++) {
        local_map[*pc-'a']--;
        if (local_map[*pc-'a'] < 0) {
            ret = 0;
            rv += local_map[*pc-'a'];
        }
    }
    /* Special handling for blank tiles. (rv*-1) if the number of blank
     * tiles we need to place this word. */
    if (local_map[BLANK] >= (rv*-1))
        ret = 1;

    return ret;
}

void
read_board()
{
    char buf[1024];
    int ix, ix2, ix3;
    char *pc;

    /* Read board. */
    for (ix = 0; ix < LEN; ix++) {
        if (fgets(buf, 1024, stdin) == NULL) {
            fprintf(stderr, "Board must be %d columns.\n", LEN);
            exit(99);
        }
        if (strlen(buf) < LEN) {
            fprintf(stderr, "Line %d is wrong length, should be %d.\n",
                    ix+1, LEN);
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
                FLAG_LETTER(buf[ix2], boardmap);
                FLAG_LETTER(buf[ix2], rowmap[ix]);
                FLAG_LETTER(buf[ix2], colmap[ix2]);
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
        if (buf[strlen(buf)-1] == '\r')
            buf[strlen(buf)-1] = 0;
        tray = strdup(buf);
        assert(tray);
    }
    tray_size = strlen(tray);
    used_tray = calloc(tray_size, sizeof(char));
    assert(used_tray);

    /* Add tray letters to maps */
    for (ix2 = 0; ix2 < tray_size; ix2++) {
        FLAG_LETTER(tray[ix2], boardmap);
        for (ix3 = 0; ix3 < LEN; ix3++) {
            FLAG_LETTER(tray[ix2], rowmap[ix3]);
            FLAG_LETTER(tray[ix2], colmap[ix3]);
        }
    }

    printf("Read %d blank(s).\n", boardmap[BLANK]);
    printf("Read %lu tray letters.\n", strlen(tray));
}

void
load_words()
{
    FILE *f;
    int n;
    char buf[1024];
    int size = 1000;

    f = fopen(wordfile, "r");
    if (f == NULL) {
        perror("Can't open word file. fopen() failed");
        exit(99);
    }

    n = 0;
    while(fscanf(f, "%s", buf)==1) {
        n++;
    }
    rewind(f);

    num_words = 0;
    words = malloc(n * sizeof(char *));
    if (words == NULL) {
        perror("malloc() failed");
        exit(99);
    }
    if (mhcreate(n) == 0) {
        perror("mhcreate()");
        exit(99);
    }
    while(fscanf(f, "%s", buf) == 1) {
        char *pc, *pcb;
        struct entry hent;
        pc = strdup(buf);
        if (num_words+1 == size) {
            size += 1000;
            words = realloc(words, size * sizeof(char*));
            if (words == NULL) {
                perror("realloc() failed");
                exit(99);
            }
        }

        words[num_words++] = pc;
        pcb = pc;
        for (pc=words[num_words-1]; *pc!=0; pc++) {
            char c;
            c = tolower(*pc);
            *pc = c;
        }
        /* Load word into word hash table */
        hent.key = pcb;
        hent.data = (void*)1;
        if (mhsearch(hent, ENTER) == 0) {
            perror("hearch()");
            exit(99);
        }
    }
    fclose(f);
    printf("Read %d words from dictionary.\n", num_words);
}

void
save_best_board(int score, char *word)
{
    if (best_boards_num >= best_boards_size) {
        score_t *tmp;
        best_boards_size += INC;
        tmp = realloc(best_boards, best_boards_size * sizeof(score_t));
        assert(tmp);
        best_boards = tmp;
    }

    memcpy(&best_boards[best_boards_num].board, board, sizeof(board_t));
    best_boards[best_boards_num].score = score;
    strcpy(best_boards[best_boards_num].word, word);
    best_boards_num++;
}

void
reset_board()
{
    memcpy(board, backup_board, sizeof(board));
}

void
reset_tray()
{
    memset(used_tray, 0, tray_size * sizeof(char));
}

int
letter_avail(char c)
{
    int ix;
    int blank = -1;
    /* Check for the actual letter */
    for (ix=0; ix < tray_size; ix++) {
        if (tray[ix] == '?' && used_tray[ix] == 0) {
            blank = ix;
        }
        if (tray[ix] == c && used_tray[ix] == 0)
            return 1;
    }

    /* Letter wasn't available, but a blank is, so use it. */
    if (blank > -1)
        return 2;

    return 0;
}

void
mark_used(char c)
{
    int ix;
    int blank = -1;
    for (ix=0; ix < tray_size; ix++) {
        if (tray[ix] == '?' && used_tray[ix] == 0)
            blank = ix;
        if (tray[ix] == c && used_tray[ix] == 0) {
            used_tray[ix] = 1;
            return;
        }
    }
    assert(blank > -1);
    used_tray[blank] = 1;
    return;
}

int
score_word(int row, int col, int dir)
{
    int ix;
    int s = 0;
    int mult = 1;

    if (debug)
        printf("SCORE: (%d, %d) %s\n", row, col, (dir==HORIZ?"HORIZ":"VERT"));

    if (dir == HORIZ) {
        /* Horizontal word */
        for (ix = col; ix < LEN && board[row][ix] != '_'; ix++) {
            if (backup_board[row][ix] == '_') {
                s += (score(row, ix) * letter_mult[row][ix]);
            }
            else
                s += (score(row, ix));
            if (debug)
                printf("(%c): %d += %d * %d\n",
                    board[row][ix],
                    s,
                    score(row, ix),
                    letter_mult[row][ix]);
            if (backup_board[row][ix] == '_')
                mult *= word_mult[row][ix];
            else
                mult *= 1;
        }
    }
    else {
        /* Vertical word */
        for (ix = row; ix < LEN && board[ix][col] != '_'; ix++) {
            if (backup_board[ix][col] == '_')
                s += (score(ix, col) * letter_mult[ix][col]);
            else
                s += (score(ix, col));

            if (debug)
                printf("(%c): %d += %d * %d\n",
                    board[ix][col],
                    s,
                    score(ix, col),
                    letter_mult[ix][col]);
            if (backup_board[ix][col] == '_')
               mult *= word_mult[ix][col];
            else
               mult *= 1;
        }
    }

    if (debug)
        printf("mult: %d = %d * %d\n", s*mult, s, mult);
    return s * mult;
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
    while(row_num > 0 && board[row_num-1][col_num] != '_')
        row_num--;
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
    while(col_num > 0 && board[row_num][col_num-1] != '_')
        col_num--;
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
    return mhsearch(hent, FIND)!=NULL;
}


void
print_board(score_t score)
{
    int col, row;

    for (row=0; row<LEN; row++) {
        for (col=0; col<LEN; col++) {
            putc(score.board[row][col], stdout);
        }
        putc('\n', stdout);
    }
}


void
search_horiz()
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
        if (debug) {
            printf("try %s\n", w);
        }
        word_len = strlen(w);

        /* Output some indication that we're making progress. */
        if (word_num % 10000 == 0) {
            fputc('.', stdout);
        }
        if (!enough_letters(w, boardmap)) {
            continue;
        }
    
        rstop = LEN;

        for (row_num=0; row_num<rstop; row_num++) {
            /* Try to place the word in a row */
            if (debug) {
               printf("try row %d ( < %d )\n", row_num, rstop);
            }

            if (!enough_letters(w, rowmap[row_num])) {
                if (debug) {
                    printf("not enough letters.\n");
                }
                continue;
            }

            cstop = LEN-word_len+1;

            for (col_num=0; col_num<cstop; col_num++) {
                if (debug) {
                    printf("try col %d ( < %d )\n", col_num, cstop);
                }

                /* When we try anew to place the word, we need to reset
                 * the letter tray and the game board back to the original
                 * state.
                 */
                reset_board();
                reset_tray();
                reset_blanks();
                attached_flag = 0;
                placed_flag = 0;
                /* Try to place word horizontally in row row_num and
                 * column col_num.
                 */

                /* words can't start right after another word. */
                if (col_num > 0 && board[row_num][col_num-1] != '_')
                    continue;

                /* words can't end right in-front of another word. */
                if (((col_num+word_len)<LEN)
                        && (board[row_num][col_num+word_len] != '_')) {
                    continue;
                }

                /* Match up letters, iterate over the letters of the word. */
                for (ltr = 0; ltr<word_len; ltr++) {
                    if (row_num==(LEN/2)&&(col_num+ltr)==(LEN/2)) {
                        attached_flag = 1;
                    }
                    if (board[row_num][col_num+ltr] != '_') {
                        /* board space is in use. */
                        if (board[row_num][col_num+ltr]==w[ltr]) {
                            /* Good, the letter is already in place. */
                            attached_flag = 1;
                        }
                        else {
                            /* mismatch, move on to next column. */
                            goto next_col;
                        }
                    }
                    else {
                        int r;
                        /* Board space is empty, is the needed letter in our
                         * tray? */
                        if ((r=letter_avail(w[ltr]))) {
                            /* Place the letter and move on. */
                            mark_used(w[ltr]);
                            board[row_num][col_num+ltr] = w[ltr];
                            if (r==2)
                                blanks[row_num][col_num+ltr] = 1;
                            placed_flag = 1;
                        }
                        else {
                            /* Letter isn't in our tray, move to next column
                             * and keep looking. */
                            goto next_col;
                        }
                    }
                }

                if (debug) {
                    printf("word %splaced.\n", (placed_flag?"":"not "));
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
                    /* New word is horiz, so check for vert words. */
                    if ((pc=is_vert_word(row_num, col_num+ltr,
                                  &new_words_r[num_new_words],
                                  &new_words_c[num_new_words]))!=NULL) {
                        new_words[num_new_words++] = pc;
                        attached_flag = 1;
                        if (debug) {
                            printf("got new vert word '%s'\n", pc);
                        }
                    }
                }

                if (debug) {
                    printf("word %sattached\n", (attached_flag?"":"not "));
                }

                if (attached_flag == 0 || placed_flag == 0) {
                    /* Too bad, word isn't attached to the rest of the
                     * puzzle.
                     */
                    goto next_col;
                }

                /* From that list, check if the words are real words */
                for (nix=0; nix<num_new_words; nix++) {
                    int isword = is_word(new_words[nix]);
                    if (!isword) {
                        /* Bummer, free temp data, move on to next
                         * position. */
                        if (debug) {
                            printf("new word '%s', not a word. NEXT\n",
                                new_words[nix]);
                        }
                        for(ltr=0; ltr<num_new_words; ltr++) {
                            free(new_words[ltr]);
                        }
                        goto next_col;
                    }
                    if (debug) {
                      printf("Got new word: %s\n", new_words[nix]);
                    }
                }
                if (debug) {
                    printf("and the %d new words are OK.\n", num_new_words);
                }

                /* Looks like everything is nice and legal, so compute the
                 * total score. */
                if (debug) {
                    printf("SCORE: %s\n", w);
                }
                score = score_word(row_num, col_num, HORIZ);
                if (debug) {
                    printf("score(%s) = %d\n", w, score);
                }
                for(nix = 0; nix < num_new_words; nix++) {
                    score += score_word(new_words_r[nix],
                                        new_words_c[nix],
                                        VERT);
                    if (debug) {
                        printf("score(%s) += %d\n", new_words[nix], score);
                    }
                    free(new_words[nix]);
                }

                /* Check for empty tray bonus. */
		bonus = 0;
		if (tray_size == 7) {
                bonus = EMPTY_TRAY_BONUS;
                    for (nix=0; nix < tray_size; nix++) {
                        if (used_tray[nix] == 0) {
                            bonus = 0;
                        }
                    }
                }
                score += bonus;
                if (debug)
                    printf("Empty tray bonus: %d += %d\n", score, bonus);

                printf("%s(%d)", w, score);
                save_best_board(score, w);
next_col:
                ;
            }
        }
    }
    fputc('\n', stdout);
}






void
search_vert()
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
        if (debug) {
            printf("try %s\n", w);
        }
        word_len = strlen(w);

        /* Output some indication that we're making progress. */
        if (word_num % 10000 == 0) {
            fputc('.', stdout);
        }
        if (!enough_letters(w, boardmap)) {
            continue;
        }
    
        cstop = LEN;

        for (col_num=0; col_num<cstop; col_num++) {
            /* Try to place the word in a column */
            if (debug) {
               printf("try col %d ( < %d )\n", col_num, cstop);
            }

            if (!enough_letters(w, colmap[col_num])) {
                if (debug) {
                    printf("not enough letters.\n");
                }
                continue;
            }

            rstop = LEN-word_len+1;

            for (row_num=0; row_num<rstop; row_num++) {
                if (debug) {
                    printf("try row %d ( < %d )\n", row_num, rstop);
                }

                /* When we try anew to place the word, we need to reset
                 * the letter tray and the game board back to the original
                 * state.
                 */
                reset_board();
                reset_tray();
                reset_blanks();
                attached_flag = 0;
                placed_flag = 0;

                /* words can't start right after another word. */
                if (row_num > 0 && board[row_num-1][col_num] != '_')
                    continue;

                /* words can't end right in-front of another word. */
                if (((row_num+word_len)<LEN)
                        && (board[row_num+word_len][col_num] != '_')) {
                    continue;
                }

                /* Match up letters, iterate over the letters of the word. */
                for (ltr = 0; ltr<word_len; ltr++) {
                    if ((row_num+ltr)==(LEN/2)&&col_num==(LEN/2)) {
                        attached_flag = 1;
                    }
                    if (board[row_num+ltr][col_num] != '_') {
                        /* board space is in use. */
                        if (board[row_num+ltr][col_num]==w[ltr]) {
                            /* Good, the letter is already in place. */
                            attached_flag = 1;
                        }
                        else {
                            /* mismatch, move on to next row. */
                            goto next_row;
                        }
                    }
                    else {
                        int r;
                        /* Board space is empty, is the needed letter in our
                         * tray? */
                        if ((r=letter_avail(w[ltr]))) {
                            /* Place the letter and move on. */
                            mark_used(w[ltr]);
                            board[row_num+ltr][col_num] = w[ltr];
                            if (r==2)
                                blanks[row_num+ltr][col_num] = 1;
                            placed_flag = 1;
                        }
                        else {
                            /* Letter isn't in our tray, move to next column
                             * and keep looking. */
                            goto next_row;
                        }
                    }
                }

                if (debug) {
                    printf("word %splaced.\n", (placed_flag?"":"not "));
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
                    /* New word is vert so check for horiz words. */
                    if ((pc=is_horiz_word(row_num+ltr, col_num,
                                  &new_words_r[num_new_words],
                                  &new_words_c[num_new_words]))!=NULL) {
                        new_words[num_new_words++] = pc;
                        attached_flag = 1;
                    }
                }

                if (debug) {
                    printf("word %sattached\n", (attached_flag?"":"not "));
                }

                if (attached_flag == 0 || placed_flag == 0) {
                    /* Too bad, word isn't attached to the rest of the
                     * puzzle.
                     */
                    goto next_row;
                }

                /* From that list, check if the words are real words */
                for (nix=0; nix<num_new_words; nix++) {
                    int isword = is_word(new_words[nix]);
                    if (!isword) {
                        /* Bummer, free temp data, move on to next
                         * position. */
                        if (debug) {
                            printf("new word '%s', not a word. NEXT\n",
                                new_words[nix]);
                        }
                        for(ltr=0; ltr<num_new_words; ltr++) {
                            free(new_words[ltr]);
                        }
                        goto next_row;
                    }
                    if (debug) {
                      printf("Got new word: %s\n", new_words[nix]);
                    }
                }
                if (debug) {
                    printf("and the %d new words are OK.\n", num_new_words);
                }

                /* Looks like everything is nice and legal, so compute the
                 * total score. */
                if (debug) {
                    printf("SCORE: %s\n", w);
                }
                score = score_word(row_num, col_num, VERT);
                if (debug) {
                    printf("score(%s) = %d\n", w, score);
                }
                for(nix = 0; nix < num_new_words; nix++) {
                    score += score_word(new_words_r[nix],
                                        new_words_c[nix],
                                        HORIZ);
                    if (debug) {
                        printf("score(%s) += %d\n", new_words[nix], score);
                    }
                    free(new_words[nix]);
                }

                /* Check for empty tray bonus. */
		bonus = 0;
		if (tray_size == 7) {
                bonus = EMPTY_TRAY_BONUS;
                    for (nix=0; nix < tray_size; nix++) {
                        if (used_tray[nix] == 0) {
                            bonus = 0;
                        }
                    }
                }
                score += bonus;
                if (debug)
                    printf("Empty tray bonus: %d += %d\n", score, bonus);

                printf("%s(%d)", w, score);
                save_best_board(score, w);
next_row:
                ;
            }
        }
    }
    fputc('\n', stdout);
}


int
main(int argc, char *argv[])
{
    int opt;

    strcpy(wordfile, "words");

    while ((opt = getopt(argc, argv, "vxd:")) != -1) {
        switch(opt) {
            case 'v':
                fprintf(stderr, "%s\n", Version);
                fprintf(stderr, "%s\n", Date);
                exit(0);
            case 'x':
                debug=1;
                break;
            case 'd':
                strcpy(wordfile, optarg);
                fprintf(stderr, "reading words from %s\n", wordfile);
                break;
            default:
                exit(99);
        }
    }

    read_board();
    load_words();

    search_horiz();
    search_vert();

    qsort(best_boards, best_boards_num, sizeof(score_t), cmp_score);

    for (opt = 0; opt < best_boards_num; opt++) {
        printf("Board: %d\n", opt);
        printf("Score: %d\n", best_boards[opt].score);
        printf("Word : %s\n", best_boards[opt].word);
        print_board(best_boards[opt]);
        printf("\n\n");
    }

    mhdestroy();
    for (opt = 0; opt < num_words; opt++) {
        free(words[opt]);
    }
    free(words);
    free(best_boards);
    free(used_tray);
    free(tray);

    return 0;
}


