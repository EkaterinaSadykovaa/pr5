#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_ATTEMPTS 10
#define BUFFER_SIZE 32

void player_one(int read_fd, int write_fd, int upper_limit, int round) {
    srand(time(NULL) ^ getpid());
    int target_number = 1 + rand() % upper_limit;
    printf("Раунд %d \n Загаданное число: %d\n", round, target_number);
    fflush(stdout);

    char buffer[BUFFER_SIZE];
    int tries = 0;

    while (1) {
        read(read_fd, buffer, BUFFER_SIZE);
        int guess = atoi(buffer);
        tries++;
        printf("Первый игрок получил - %d\n", guess);
        fflush(stdout);

        if (guess == target_number) {
            write(write_fd, "1", 1);
            break;
        } else {
            write(write_fd, "0", 1);
        }
    }

    printf("Первый игрок - Количество попыток: %d\n\n", tries);
}

void player_two(int read_fd, int write_fd, int upper_limit) {
    char buffer[BUFFER_SIZE];
    for (int round = 1; round <= MAX_ATTEMPTS; ++round) {
        int tries = 0;

        while (1) {
            int current_guess = 1 + rand() % upper_limit;
            printf("Второй игрок угадал - %d\n", current_guess);
            fflush(stdout);

            snprintf(buffer, BUFFER_SIZE, "%d", current_guess);
            write(write_fd, buffer, strlen(buffer) + 1); 

            read(read_fd, buffer, 1);
            tries++;

            if (buffer[0] == '1') {
                printf("Второй игрок - Количество попыток: %d \n\n", tries);
                break;
            }
        }
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "%s <MAX>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int upper_limit = atoi(argv[1]);
    if (upper_limit < 1) {
        fprintf(stderr, "Число должно быть больше 0.\n");
        return EXIT_FAILURE;
    }
    
    printf("Игра началась! \n");

    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Ошибка создания канала");
        return EXIT_FAILURE;
    }

    pid_t child_process = fork();

    if (child_process < 0) {
        perror("Ошибка fork");
        return EXIT_FAILURE;
    }

    if (child_process == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        player_two(pipe1[0], pipe2[1], upper_limit);
        exit(EXIT_SUCCESS);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);

        for (int round = 1; round <= MAX_ATTEMPTS; ++round) {
            player_one(pipe2[0], pipe1[1], upper_limit, round);
        }

        kill(child_process, SIGTERM);
        wait(NULL);
        printf("Конец игры! Дружба победила!\n");
    }

    return EXIT_SUCCESS;
}

