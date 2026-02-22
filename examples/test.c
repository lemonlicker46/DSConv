#include <stdio.h>
#define DeclareColor( _V , _I , _N ) ColorTable _V = { .ColorID=_I , .ColorName[] = _N };

typedef struct ColorTable
{
    int ColorID;
    char* ColorName;
}ColorTable;

int main(void)
{
    DeclareColor(BLACK, 0, "Black")
        ColorTable BLACK = { .ColorID = 0,  .ColorName = "Black" };
    ColorTable BLUE = { .ColorID = 1,  .ColorName = "Blue" };
    ColorTable GREEN = { .ColorID = 2,  .ColorName = "Green" };
    ColorTable DARK_TURQUOISE = { .ColorID = 3,  .ColorName = "Dark Turquoise" };
    ColorTable RED = { .ColorID = 4,  .ColorName = "Red" };
    ColorTable DARK_PINK = { .ColorID = 5,  .ColorName = "Dark Pink" };
    ColorTable BROWN = { .ColorID = 6,  .ColorName = "Brown" };
    ColorTable LIGHT_GRAY = { .ColorID = 7,  .ColorName = "Light Gray" };
    ColorTable DARK_GRAY = { .ColorID = 8,  .ColorName = "Dark Gray" };
    ColorTable LIGHT_BLUE = { .ColorID = 9,  .ColorName = "Light Blue" };
    ColorTable BRIGHT_GREEN = { .ColorID = 10, .ColorName = "Bright Green" };
    ColorTable LIGHT_TURQUIOSE = { .ColorID = 11, .ColorName = "Light Turquoise" };
    ColorTable SALMON = { .ColorID = 12, .ColorName = "Salmon" };
    ColorTable PINK = { .ColorID = 13, .ColorName = "Pink" };
    ColorTable YELLOW = { .ColorID = 14, .ColorName = "Yellow" };
    ColorTable WHITE = { .ColorID = 15, .ColorName = "White" };
    ColorTable LIGHT_GREEN = { .ColorID = 16, .ColorName = "Light Green" };
    ColorTable LIGHT_YELLOW = { .ColorID = 17, .ColorName = "Light Yellow" };
    ColorTable TAN = { .ColorID = 19, .ColorName = "Tan" };
    ColorTable LIGHT_VIOLET = { .ColorID = 20, .ColorName = "Light Violet" };
    ColorTable PURPLE = { .ColorID = 22, .ColorName = "Purple" };
    ColorTable DARK_BLUE_VIOLET = { .ColorID = 23, .ColorName = "Dark Blue Violet" };
    ColorTable ORANGE = { .ColorID = 25, .ColorName = "Orange" };
    ColorTable MAGENTA = { .ColorID = 26, .ColorName = "Magenta" };
    ColorTable LIME = { .ColorID = 27, .ColorName = "Lime" };
    ColorTable DARK_TAN = { .ColorID = 28, .ColorName = "Dark Tan" };
    ColorTable BRIGHT_PINK = { .ColorID = 29, .ColorName = "Bright Pink" };
    ColorTable MEDIUM_LAVENDER = { .ColorID = 30, .ColorName = "Medium Lavender" };
    ColorTable LAVENDER = { .ColorID = 31, .ColorName = "Lavender" };
    ColorTable VERY_LIGHT_ORANGE = { .ColorID = 68, .ColorName = "Very Light Orange" };
    ColorTable BRIGHT_REDDISH_LILAC = { .ColorID = 69, .ColorName = "Bright Reddish Lilac" };
    ColorTable REDDISH_BROWN = { .ColorID = 70, .ColorName = "Reddish Brown" };

    short unsigned int Color;
    scanf("%hu", &Color);
    ColorTable* P_Color;
    for (P_Color = &BLACK; P_Color <= &BLACK.ColorName; P_Color++) //loop through all struct variables using pointer (ColorID, ColorName)
    {
        printf("%hu", (struct) * P_Color);
        if (Color == P_Color->ColorID)
        {

        }
    }
    return 0;
}
