/* sample.c: example input demonstrating complex C declarations */

typedef void (*function_pointer_t)(int);

struct s {
    int s[10];
    union { unsigned u; int i; } ; /* anonymous union */
    struct { int a; } anon_struct; /* anonymous struct as member */
};

typedef struct {
    int x;
    int y;
} point_t;

enum Colors { RED=0, GREEN=1, BLUE=2 };
