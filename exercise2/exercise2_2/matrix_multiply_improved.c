void matrix_multiply(int *a, int *b, int *output, int i,
                           int k, int j) {
    // Inplement your code here
    for (int x = 0; x < i; x++) {
        for (int z = 0; z < k; z++) {
            int a_val = a[x * k + z];
            for (int y = 0; y < j; y++) {
                output[x * j + y] += a_val * b[z * j + y]; 
            }
        }
    }
}