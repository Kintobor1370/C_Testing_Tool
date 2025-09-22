/*
0 - Not a triangle
1 - Equilateral
2 - Isosceles
3 - Scalene
*/

int triangleCounter(int a, int b, int c) {
    if (a + b <= c || a + c <= b || b + c <= a)
        return 0;
    else {
        int numEqualPairs = 0;
        if (a == b)
            numEqualPairs++;
        if (a == c)
            numEqualPairs++;
        if (b == c)
            numEqualPairs++;
        
        if (numEqualPairs == 0)
            return 3;
        else if (numEqualPairs == 1)
            return 2;
        else
            return 1;
    }
}