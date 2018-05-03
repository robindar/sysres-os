#include "test.h"

typedef struct {
    int * m;
    int row;
    int col;
} matrix;

matrix new_matrix (int row_nb, int col_nb) {
    matrix m;
    m.row = row_nb;
    m.col = col_nb;
    m.m = (int *) kmalloc(row_nb * col_nb * sizeof(int));
    if (m.m == NULL){
        uart_error("Memory Allocation failed");
        abort();
    }
    return m;
}

matrix new_identity(int row_nb, int col_nb) {
    int i,j;
    matrix m = new_matrix(row_nb, col_nb);
    for (i=0; i < m.row; i++){
        for (j=0; j < m.col; j++){
            *(m.m + i*m.col + j) = (i == j);
        }
    }
    return m;
}

matrix new_zero(int row_nb, int col_nb) {
    int i,j;
    matrix m = new_matrix(row_nb, col_nb);
    for (i=0; i < m.row; i++){
        for (j=0; j < m.col; j++){
            *(m.m + i*m.col + j) = 0;
        }
    }
    return m;
}


void free_matrix (matrix * m) {
    kfree(m->m);
}

/* Ok I don't have rand numbers yet... */
void fill_rand(matrix * m) {
    int i,j;
    for (i=0; i < m->row; i++){
        for (j=0; j < m->col; j++){
            *(m->m + i*m->col + j) = i * j + 42;
        }
    }
}

void print_matrix(const matrix * m) {
    int i, j;
    for (i=0; i<m->row; i++){
        for (j=0; j<m->col; j++){
            uart_printf("%d ", *(m->m + i*m->col + j));
        }
        uart_printf("\r\n");
    }
}

bool equal(const matrix a, const matrix b) {
    bool res = (a.col == b.col) && (a.row == b.row);
    assert((a.col == b.col) && (a.row == b.row));
    int i,j;
    for (i=0; i < a.row; i++){
        for (j=0; j < a.col; j++){
            res &= (*(a.m + i*a.col + j) == *(b.m + i*b.col + j));
        }
    }
    return res;
}

matrix new_product(matrix * a, matrix * b) {
    if (a->col != b->row){
        uart_error("Impossible product");
        abort();
    }
    int n = a->col;
    matrix c = new_zero(a->row, b->col);
    int i, j, k;
    for(i = 0; i < c.row; i++){
        for(j = 0; j < c.col; j++){
            for(k = 0; k < n; k++){
                *(c.m + i*c.col + j) += *(a->m + i*a->col + k) * *(b->m + k*b->col + j);
            }
        }
    }
    return c;
}


void matrix_main() {
    uart_debug("Entering matrix test\r\n");
    #define SZ 10

    matrix a, b, c;

    a = new_identity(SZ ,SZ);
    b = new_matrix(SZ ,SZ);
    fill_rand(&b);
    c = new_product(&a, &b);
    assert(equal(b, c));
    uart_debug("Done matrix test\r\n");
}
