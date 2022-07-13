#include <infos.h>
#include <mutex.h>

mutex m;
unsigned int n = 0;

const long THREADS = 4;
const long ITER = 500;
bool inside = false;

void work(void *arg) {
    for (int i = 0; i < ITER; i++) {
        unique_lock<mutex> lock(m);
        if (inside) {
            printf("test failed\n");
            return ;
        }
        inside = true;
        usleep(1);
        n++;
        inside = false;
    }
}

int main() {
    HTHREAD threads[THREADS];
    for (int i = 0; i < THREADS; i++) {
        threads[i] = create_thread(work, NULL);
    }
    for (int i = 0; i < THREADS; i++) {
        join_thread(threads[i]);
    }
    printf("expected: %ld\n", THREADS * ITER);
    printf("actual: %ld\n", n);
    return 0;
}