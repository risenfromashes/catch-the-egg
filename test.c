#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
char colors[][64] = {"black	0,0,0",
                     "silver	192,192,192",
                     "gray	128,128,128",
                     "white	255,255,255",
                     "maroon	128,0,0",
                     "red 	255,0,0",
                     "purple	128,0,128",
                     "fuchsia	255,0,255",
                     "green	0,128,0",
                     "lime	0,255,0",
                     "olive	128,128,0",
                     "yellow	255,255,0",
                     "blue	0,0,255",
                     "navy	0,0,128",
                     "aqua	0,255,255",
                     "teal	0,128,128"};

int comp(const void* p1, const void* p2) { return strcmp((const char*)p1, (const char*)p2); }
#define PI 3.141593265358979
int inAngleRange(double x, double l, double r) { return (l <= x && x <= r || l >= x && x >= r) ^ (fabs(l - r) > PI); }
int main()
{
    printf("%d\n", inAngleRange(3 * PI / 2, PI / 4, 3 * PI / 4));
    return 0;
}