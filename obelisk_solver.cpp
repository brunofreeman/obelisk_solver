#include <iostream>
#include <vector>
#include <stack>

#define HOLE_POSITIONS 12
#define BROWN_TILES 5
#define GREEN_TILES 4
#define TOTAL_TILES (BROWN_TILES + GREEN_TILES)
#define PEG_TYPES 3
#define ONE_PEGS 1
#define TWO_PEGS 9
#define THREE_PEGS 4

#define BOOL_SWAP(a, b) temp = a; a = b; b = temp;

typedef struct tile {
    /**
     * Hole indices correspond to a clockwise
     * winding with 0 being the upper left corner
     */
    bool holes[HOLE_POSITIONS];
    bool used;

    // clockwise rotation
    void rotate() {
        bool temp[3] = {
                holes[HOLE_POSITIONS - 3],
                holes[HOLE_POSITIONS - 2],
                holes[HOLE_POSITIONS - 1],
        };
        for (uint8_t i = HOLE_POSITIONS - 1; i >= 3; i--) {
            holes[i] = holes[i - 3];
        }
        for (uint8_t i = 0; i <= 2; i++) {
            holes[i] = temp[i];
        }
    }

    void flip(bool vertical) {
        bool temp;
        if (vertical) {
            BOOL_SWAP(holes[0], holes[ 2])
            BOOL_SWAP(holes[3], holes[11])
            BOOL_SWAP(holes[4], holes[10])
            BOOL_SWAP(holes[5], holes[ 9])
            BOOL_SWAP(holes[6], holes[ 8])
        } else {
            BOOL_SWAP(holes[0], holes[ 8])
            BOOL_SWAP(holes[1], holes[ 7])
            BOOL_SWAP(holes[2], holes[ 6])
            BOOL_SWAP(holes[3], holes[ 5])
            BOOL_SWAP(holes[9], holes[11])
        }
    }
} tile_t;

enum state_t {
    INVALID = 0,
    VALID   = 1,
    SOLVED  = 2
};

/**
 * pegs[i][j] -->
 * j == 0 -->  number used of size i + 1
 * j == 1 --> total number of size i + 1
 */
uint8_t pegs[PEG_TYPES][2] = { {0, ONE_PEGS}, {0, TWO_PEGS}, {0, THREE_PEGS} };

/**
 * 0 = unoccupied
 * 1 = occupied by 1-length
 * 2 = occupied by 2-length
 * 3 = occupied by 3-length
 */
uint8_t peg_matrix[TOTAL_TILES][HOLE_POSITIONS] = {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

std::vector<tile_t> tile_groups[2];

std::stack<tile_t> obelisk;
bool on_green = false;
bool on_pegs = false;
bool solved = false;

/**
 * Tile starting arrangements prioritize placing holes
 * in lowest index positions
 */
void set_browns() {
    // single hole
    tile_groups[0][0] = {
            {
                    true, false, false,
                    false, false, false,
                    false, false, false,
                    false, false, false
            },
            false
    };

    // triangle
    tile_groups[0][1] = {
            {
                    true, false, false,
                    false,  true, false,
                    false, false,  true,
                    false, false, false
            },
            false
    };

    // rectangle
    tile_groups[0][2] = {
            {
                    true, false,  true,
                    false, false, false,
                    true, false,  true,
                    false, false, false
            },
            false
    };

    // footprint 1
    tile_groups[0][3] = {
            {
                    true, false,  true,
                    false,  true, false,
                    false, false,  true,
                    false, false, false
            },
            false
    };

    // footprint 2
    tile_groups[0][4] = {
            {
                    true, false,  true,
                    false,  true, false,
                    false, false,  true,
                    false, false, false
            },
            false
    };
}

void set_greens() {
    // diagonal
    tile_groups[1][0] = {
            {
                    true, false, false,
                    false, false, false,
                    true, false, false,
                    false, false, false
            },
            false
    };

    // flipped (uppercase) gamma
    tile_groups[1][1] = {
            {
                    true, false,  true,
                    false, false, false,
                    true, false, false,
                    false, false, false
            },
            false
    };

    // skewed trapezoid
    tile_groups[1][2] = {
            {
                    true, false,  true,
                    false,  true, false,
                    true, false, false,
                    false, false, false
            },
            false
    };

    // hexagon
    tile_groups[1][3] = {
            {
                    true, false,  true,
                    false,  true, false,
                    true, false,  true,
                    false,  true, false,
            },
            false
    };
}

void print_solution() {
    std::vector<tile_t> reverse_stack(TOTAL_TILES);

    for (uint8_t i = TOTAL_TILES; i > 0; i--) {
        reverse_stack[i - 1] = obelisk.top();
        obelisk.pop();
    }

    for (uint8_t i = 0; i < TOTAL_TILES; i++) {
        printf("Tile %d:\n", i + 1);
        for (uint8_t row = 0; row < 5; row++) {
            std::cout << "---------------------" << std::endl;
            for (uint8_t col = 0; col < 5; col++) {
                if ((row == 0 && col == 0) || (row == 0 && col == 4) ||
                    (row == 4 && col == 0) || (row == 4 && col == 4)) {
                    std::cout << '|';
                    std::cout << "|||";
                } else if ((0 < row && row < 4) && (0 < col && col < 4)) {
                    if (col == 1) std::cout << "|   ";
                    else std::cout << "    ";
                } else {
                    std::cout << '|';
                    /**
                     * 0 --> (0,1)
                     * 1 --> (0,2)
                     * 2 --> (0,3)
                     * 3 --> (1,4)
                     * 4 --> (2,4)
                     * 5 --> (3,4)
                     * 6 --> (4,3)
                     * 7 --> (4,2)
                     * 8 --> (4,1)
                     * 9 --> (3,0)
                     * 10 -> (2,0)
                     * 11 -> (1,0)
                     */
                    uint8_t pos;
                    if (row == 0) {
                        pos = col - 1;
                    } else if (row == 4) {
                        pos = 9 - col;
                    } else {
                        if (col == 0) {
                            pos = 12 - row;
                        } else {
                            pos = row + 2;
                        }
                    }

                    if (peg_matrix[i][pos] == 0) {
                        std::cout << "   ";
                    } else {
                        std::cout << ' ' << (int) peg_matrix[i][pos] << ' ';
                    }
                }
            }
            std::cout << '|' << std::endl;
        }
        std::cout << "---------------------" << std::endl << std::endl;
    }
}

state_t check_valid() {
    bool all_filled = true;

    for (uint8_t i = 0; i < HOLE_POSITIONS; i++) {
        if (!obelisk.top().holes[i] && peg_matrix[obelisk.size() - 1][i] != 0) {
            return INVALID;
        }
        if (obelisk.top().holes[i]) {
            all_filled &= peg_matrix[obelisk.size() - 1][i] != 0;
        }
    }

    if (all_filled) {
        if (obelisk.size() == TOTAL_TILES) {
            print_solution();
            return SOLVED;
        } else {
            bool next_layer_empty = true;
            for (uint8_t i = 0; i < HOLE_POSITIONS; i++) {
                next_layer_empty &= peg_matrix[obelisk.size()][i] == 0;
            }
            return next_layer_empty ? INVALID : VALID;
        }
    } else {
        return on_pegs ? INVALID : VALID;
    }
}

void try_tiles();
void try_pegs();

void try_pegs() {
    on_pegs = true;

    uint8_t pegs_restore[PEG_TYPES][2];
    memcpy(pegs_restore, pegs, sizeof(pegs));

    uint8_t peg_matrix_restore[TOTAL_TILES][HOLE_POSITIONS];
    memcpy(peg_matrix_restore, peg_matrix, sizeof(peg_matrix));

    for (uint8_t i = 0; i < HOLE_POSITIONS; i++) {
        if (obelisk.top().holes[i] && peg_matrix[obelisk.size() - 1][i] == 0) {
            if (obelisk.size() + 2 <= TOTAL_TILES && pegs[2][0] < pegs[2][1]) {
                peg_matrix[obelisk.size() - 1][i] = 3;
                peg_matrix[obelisk.size()][i]     = 3;
                peg_matrix[obelisk.size() + 1][i] = 3;
                pegs[2][0]++;
                try_pegs();
                if (solved) return;
                pegs[2][0]--;
                peg_matrix[obelisk.size() - 1][i] = 0;
                peg_matrix[obelisk.size()][i]     = 0;
                peg_matrix[obelisk.size() + 1][i] = 0;
            }
            if (obelisk.size() + 1 <= TOTAL_TILES && pegs[1][0] < pegs[1][1]) {
                peg_matrix[obelisk.size() - 1][i] = 2;
                peg_matrix[obelisk.size()][i]     = 2;
                pegs[1][0]++;
                try_pegs();
                if (solved) return;
                pegs[1][0]--;
                peg_matrix[obelisk.size() - 1][i] = 0;
                peg_matrix[obelisk.size()][i]     = 0;
            }
            if (pegs[0][0] < pegs[0][1]) {
                peg_matrix[obelisk.size() - 1][i] = 1;
                pegs[0][0]++;
                try_pegs();
                if (solved) return;
                pegs[0][0]--;
                peg_matrix[obelisk.size() - 1][i] = 0;
            }
        }
    }

    uint8_t state = check_valid();
    if (state == SOLVED) {
        solved = true;
    } else if (state == INVALID) {
        memcpy(pegs, pegs_restore, sizeof(pegs));
        memcpy(peg_matrix, peg_matrix_restore, sizeof(peg_matrix));
    } else {
        try_tiles();
    }
}

void try_tiles() {
    on_pegs = false;

    for (uint8_t i = 0; i < tile_groups[on_green].size(); i++) {
        if (tile_groups[on_green][i].used) continue;


        tile_groups[on_green][i].used = true;
        obelisk.push(tile_groups[on_green][i]);

        for (uint8_t rotation = 0; rotation < 4; rotation++) {
            uint8_t state = check_valid();

            if (state == SOLVED) {
                solved = true;
                return;
            } else if (state == VALID) {
                on_green = !on_green;
                try_pegs();
                if (solved) {
                    return;
                } else {
                    obelisk.top().flip(rotation % 2 == 0);
                    state = check_valid();
                    if (state == SOLVED) {
                        solved = true;
                        return;
                    } else if (state == VALID) {
                        on_green = !on_green;
                        try_pegs();
                        if (solved) return;
                    }
                }
            } else {
                obelisk.top().flip(rotation % 2 == 0);
                state = check_valid();
                if (state == SOLVED) {
                    solved = true;
                    return;
                } else if (state == VALID) {
                    on_green = !on_green;
                    try_pegs();
                    if (solved) return;
                }
            }

            obelisk.top().rotate();
        }

        obelisk.pop();
        tile_groups[on_green][i].used = false;
    }
}

void solve_obelisk() {
    try_tiles();
}

int main(int argc, char* argv[]) {
    tile_groups[0] = std::vector<tile_t>(BROWN_TILES);
    tile_groups[1] = std::vector<tile_t>(GREEN_TILES);

    set_browns();
    set_greens();

    solve_obelisk();

    return 0;
}
