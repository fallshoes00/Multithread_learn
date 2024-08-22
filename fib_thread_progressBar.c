#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_TERMS 20 // Fibonacci 項數

typedef struct {
    int totalTerms;
    int currentTerm;
    pthread_mutex_t lock;
    int done; // 確認是否完成
} Progress;

void printProgress(Progress *progress) {
    while (1) {
        pthread_mutex_lock(&(progress->lock));
        int progressPercent = (progress->currentTerm * 100) / progress->totalTerms;
        int isDone = progress->done; // shared variable
        pthread_mutex_unlock(&(progress->lock));

        printf("\rProgress: [");
        for (int i = 0; i < progressPercent / 2; i++) printf("#");
        for (int i = progressPercent / 2; i < 50; i++) printf(" ");
        printf("] %d%%", progressPercent);
        fflush(stdout);

        if (isDone && progressPercent == 100) break; // 做完退出
        usleep(100000); 
    }
}

void *fibonacciThread(void *arg) {
    Progress *progress = (Progress *)arg;
    long long fib[NUM_TERMS];

    fib[0] = 0;
    fib[1] = 1;

    for (int i = 0; i < NUM_TERMS; i++) {
        if(i>1)
            fib[i] = fib[i-1] + fib[i-2];

        pthread_mutex_lock(&(progress->lock));
        progress->currentTerm += 1;
        pthread_mutex_unlock(&(progress->lock));

        usleep(200000);
    }

    pthread_mutex_lock(&(progress->lock));
    progress->done = 1;  // 全部做完了，修改變數
    pthread_mutex_unlock(&(progress->lock));

    printf("\nFibonacci Sequence: ");
    for (int i = 0; i < NUM_TERMS; i++) {
        printf("%lld ", fib[i]);
    }
    printf("\n");

    return NULL;
}

void *progressThread(void *arg) {
    Progress *progress = (Progress *)arg;
    printProgress(progress);
    return NULL;
}

int main() {
    pthread_t fib_tid, progress_tid;
    Progress progress = {
        .totalTerms = NUM_TERMS,
        .currentTerm = 0,
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .done = 0 // 初始化
    };

    // 建Fib和ProgressBar的thread
    pthread_create(&fib_tid, NULL, fibonacciThread, &progress);
    pthread_create(&progress_tid, NULL, progressThread, &progress);

    // 等待thread完成
    pthread_join(fib_tid, NULL);
    pthread_join(progress_tid, NULL);

    return 0;
}
