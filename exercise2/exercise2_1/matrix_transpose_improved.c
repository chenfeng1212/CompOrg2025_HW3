void matrix_transpose(int n, int *dst, int *src) {
    // Inplement your code here
    int B = 8;

    for (int i = 0; i < n; i += B) {
        for (int j = 0; j < n; j += B) {
            for (int x = i; x < i + B && x < n; ++x) {
                for (int y = j; y < j + B && y < n; ++y) {
                    dst[y * n + x] = src[x * n + y]; 
                }
            }
        }
    }
}