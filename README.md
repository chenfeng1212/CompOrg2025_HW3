# CompOrg2025_HW3

## Directory Structure
```
E94121038_HW3.zip/
└──E94121038_HW3/
    ├── README.md
    ├── exercise1
    │   ├── cachesim.cc
    │   └──  cachesim.h
    └── exercise2
        ├── exercise2-1
        │   ├── matrix_transpose.c
        │   ├── matrix_transpose_improved.c
        │   └── testbench_driver.c
        ├── exercise2-2
        │   ├── matrix_multiply.c
        │   ├── matrix_multiply_improved.c
        │   └── testbench_driver.c
        ├── makefile
        ├── test_exercise2_1.py
        └── test_exercise2_2.py
```
## Exercise 1 
**Set-associative cache（cache_sim_t）:**  
std::vector<std::queue<size_t>> 每組 set 一個 queue。  
在 victimize() 中，當 cache 還有空位時，直接找第一個無效的 block 塞入。  
若已滿，從 queue 踢出最早進來的 block ，並將新 block 放入相同位置。  
**Fully-associative cache（fa_cache_sim_t） :**  
std::queue<uint64_t>記錄每個 block 的 tag key 插入順序。  
若 cache 已滿，從 queue 踢出最早進來的 block，再把新 block 加入 map 並推入 queue。

**Spike 如何整合並模擬 cache ?**  
- Spike 在執行每條 RISC-V 指令時，會同時呼叫 memtracer_t（記憶體追蹤器)。    
- 我們透過 cache_memtracer_t 將自己的快取模擬器包裝成一個 memtracer。  
- icache_sim_t（指令快取）與 dcache_sim_t（資料快取）都繼承自它。  
- 當 Spike 要讀寫一塊記憶體時，先呼叫 check_tag() 檢查命中與否  
如果 miss 則呼叫 victimize() 找出 victim way 。若 dirty 則 writeback（+1），再從下一層 miss_handler 要資料  
最後更新統計數據（read_misses, writebacks 等）。
- 接下來就是上面做的事
- Spike 在結束模擬後會呼叫 cache_sim_t::print_stats()
輸出：Bytes Read / Written、Accesses、Misses、Writebacks、Miss Rate、Memory subsystem access overhead

**總結 :**  
Spike 模擬 RISC-V 程式時，會透過 memtracer 將每個記憶體操作傳給自己寫的 cache_sim_t。在裡面用 FIFO 邏輯判斷是否命中、替換、計數，Spike 再根據你的模擬結果統計 cache 效能與記憶體存取成本。  
## Exercise 2-1
一開始，我將內外迴圈對調，讓讀取方向變成連續 ( row-wise )，但 Write Miss 上升，效果有限。  
接下來我加入 tile block size B = 32，將矩陣切成小區塊，效果改善但WriteBack 和 Write Miss 還是太多。
最後將 B 縮到剩下 8 就成功將 Improved Ratio 提升到 1.6 以上了   

**我認為這樣可以提升 Improved Ratio 的原因 :**  
- 將矩陣切成 B×B 的區塊，使得每個區塊存取時具有空間區域性
- 减少 cache miss，因為一個 tile 可以 fit 進 cache（尤其是 read）
- 每行 8 個 int（int = 4 bytes）剛好是一個 cache line（32 bytes）
- 避免了 dirty block 在 tile 間被踢出 ➜ 減少 Write Miss 與 Writeback

**最終程式 :**
```c
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
```

## Exercise 2-2
我先將 y , z 兩個 for 迴圈對調位置，讓 output 和 b 讀取方向變成連續 ( row-wise ) ，再把 a 移到第二層迴圈裡使它的讀取方向也變成連續  
因為 output 和 b 是 [x * j + y] 、 [z * j + y]， 所以放在 for(y) 會是 row-wise  
a 是 [x * k + z] ，所以放在 for(z) 會是 row-wise ，而且 a 與 y 無關，因此才可以把它從原本第三層迴圈移到第二層，還可以減少 a 的 load 次數，暫存器幫忙省時間

**最終程式 :**
```c
void matrix_multiply(int *a, int *b, int *output, int i, int k, int j) {
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
```

## Homework 2 description

- complex_add.c 和 complex_sub.c :
    - 就單純的加和減而已
- complex_mul.c :
    - 計算(a+bi)*(c+di) = (ab - bd) + (bc + ad)i
    - 這邊有一個重點是把答案最後再一起放進去
- log2.c
    - t0累加log次數、t1暫存比較結果
    - 首先如果N < 2 t1 = 1，否則t1 = 0。若t1 != 0就跳到2.結束
    - 接下來把 N 右移一位，t0 = t0 + 1，回到上一步直到N < 2為止
    - 2.將t0結果放到log裡
- bit_reverse.c
    - t0存輸入b，t1為位數m的倒數計數，t2存輸入&1
    - 首先把b存到t0裡，把b歸0，把m存到t1裡
    - 1.若t1 == 0就跳到2. 結束
    - t2 = t0 & 1，記住t0的最末位
    - 把b向左位移一位，再把t2填入最末位，t1 = t1 - 1，回到1.
- pi.c
    - pi/4 = 1 - 1/3 + 1/5 - 1/7 ......
    - t1存索引值k，t2存(-1)^k，t3存(2k+1)
    - 首先初始化pi = 0.0f, t1 = 0, t2 = 1
    - 1.判斷t1是否為預設迭代次數，是就跳到2. 結束
    - 計算t3 = t1 << 1 + 1， f1存t3轉成浮點數，f2存t2轉成浮點數，f3存f2/f1
    - 計算pi = pi + f3， t2 = -t2， t1 = t1 + 1，跳回1.
- arraymul_baseline.c
  ```c
  for (int i = 0; i < arr_size; i++) {
    y[i] = h[i] * x[i] + id;
  }
  ```
    - 1.如果arr_size == 0 就跳到2. 結束
    - 讀取h陣列的第i位到f0，x陣列的第i位到f1，f0 = f0 * f1 + id，把f0存到y陣列的第i位(i為當下指向位置)
    - h、x、y陣列指標都+4，也就是到下一個元素，arr_size = arr_size - 1，跳回1.
- arraymul_improved.c
    - 這個跟arraymul_baseline.c比起來是差在它一次平行處理4個float
    - 1.如果arr_size == 0 就跳到2. 結束
    - 設定向量長度VL=min(arr_size,4)， v1存h陣列裡VL個資料，v2存x陣列裡VL個資料
    - v0 = v1 * v2 + id，把v0裡VL個資料存到y裡
    - h、x、y陣列指標都+16，也就是到4個元素後，arr_size = arr_size - 4，跳回1.
- arraymul_float.c
  ```c
  for (int i = 0; i < arr_size; i++){
    single_floating_result = single_floating_result * p_h[i] * p_x[i];
  }
  ```
    - 1.如果arr_size == 0 就跳到2. 結束
    - 讀取h陣列的第i位到f0，result = result * f0，x陣列的第i位到f0，result = result * f0
    - h、x陣列指標都+4，也就是到下一個元素，arr_size = arr_size - 1，跳回1.
- arraymul_double.c
  ```c
  for (int i = 0; i < arr_size; i++){
    double_floating_result = double_floating_result * p_h[i] * p_x[i];
  }
  ```
    - 1.如果arr_size == 0 就跳到2. 結束
    - 讀取h陣列的第i位到f0，result = result * f0，x陣列的第i位到f0，result = result * f0
    - h、x陣列指標都+8，也就是到下一個元素，arr_size = arr_size - 1，跳回1.
