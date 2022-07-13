#include <infos.h>
#include <vector.h>

int main() {
    const int n = 100000;
    int *ptr = (int *)malloc(n * sizeof(*ptr));
    for (int i = 0; i < n; i++) {
        ptr[i] = i;
    }
    for (int i = 0; i < n; i += 1000) {
        printf("%d ", ptr[i]);
    }
    printf("\ndone\n");
    free(ptr);
    printf("vector test\n");
    vector<int> vec;
    for (int i = 0; i < n; i++) {
        vec.push_back(i);
    }
    for (int i = 0; i < n; i++) {
        my_assert(vec[i] == i);
    }
    my_assert(vec.length() == n);
    printf("leaving\n");
    return 0;
}