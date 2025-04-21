#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

#define TOTAL_ROUNDS 10

volatile sig_atomic_t signal_received = 0;
volatile sig_atomic_t last_guess = 0;
volatile sig_atomic_t is_correct_guess = 0;


void handle_guess_signal(int sig, siginfo_t *info, void *context) {
    if (sig == SIGRTMIN) {
        last_guess = info->si_value.sival_int;
        signal_received = 1;
    }
}

void handle_result_signal(int sig) {
    is_correct_guess = (sig == SIGUSR1) ? 1 : 0; 
}

void first_player(pid_t second_player_pid, int max_number, int round_number) {
    srand(time(NULL));
    int secret_number = 1 + rand() % max_number; числа
    printf("Раунд %d \n Загаданное число: %d\n", round_number, secret_number);
    fflush(stdout);

    struct sigaction sa = {0};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_guess_signal;
    sigaction(SIGRTMIN, &sa, NULL); 

    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGRTMIN);
    
    sigset_t old_mask; 
    sigprocmask(SIG_BLOCK, &block_mask, &old_mask); 

    int attempts_count = 0;

    while (1) {
        signal_received = 0;

        siginfo_t info;
        sigwaitinfo(&block_mask, &info); игрока

        int guessed_number = info.si_value.sival_int;
        attempts_count++;
        printf("Первый игрок получил - %d\n", guessed_number);
        fflush(stdout);

        if (guessed_number == secret_number) {
            kill(second_player_pid, SIGUSR1);
            break;
        } else {
            kill(second_player_pid, SIGUSR2); 
        }
    }

    printf("Первый игрок - Количество попыток: %d \n\n", attempts_count);
}

void second_player(pid_t first_player_pid, int max_number) {
    struct sigaction sa = {0};
    sa.sa_handler = handle_result_signal; 
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigaddset(&block_mask, SIGUSR2);
    
    sigset_t old_mask; 
    sigprocmask(SIG_BLOCK, &block_mask, &old_mask); 

    for (int round = 1; round <= TOTAL_ROUNDS; ++round) {
        int attempts_count = 0;

        do {
            int guessed_number = 1 + rand() % max_number; 
            printf("Второй игрок угадал - %d\n", guessed_number);
            fflush(stdout);

            union sigval value;
            value.sival_int = guessed_number;
            if (sigqueue(first_player_pid, SIGRTMIN, value) == -1) { 
                perror("Ошибка при отправке сигнала");
                exit(EXIT_FAILURE);
            }

            int signal;
            sigwait(&block_mask, &signal); 
            attempts_count++;
        } while (!is_correct_guess);

        printf("Второй игрок - Количество попыток: %d\n\n", attempts_count);
        is_correct_guess = 0; 
        sleep(1); 
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "%s <MAX>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int max_value = atoi(argv[1]);
    
    if (max_value < 1) {
        fprintf(stderr, "Укажите положительное число\n");
        return EXIT_FAILURE;
    }
    
    printf("Игра началась! \n");

    pid_t child_pid = fork();

    if (child_pid == 0) { 
        srand(time(NULL)); 
        second_player(getppid(), max_value);
        exit(EXIT_SUCCESS);
    } else {
        for (int round = 1; round <= TOTAL_ROUNDS; ++round) {
            first_player(child_pid, max_value, round);
        }

        kill(child_pid, SIGTERM); 
        wait(NULL);
        printf("Конец игры! Дружба победила!\n");
    }

    return EXIT_SUCCESS;
}
