#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define T 10

int akt_pojemnosc = 0;
int tasma[T];
int i = 0;
int cegla = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

//up worker
void *user_up(void *arg) {
	while (1) {
		sleep(rand() % 10);
		cegla = rand() % 2 + 1;
		if (akt_pojemnosc + cegla <= T) {
			pthread_mutex_lock(&mutex);
			pthread_cond_wait(&cond1, &mutex);
			// sprawdzamy, czy mamy wolne miejsce na tasmie, jesli nie, to sie przesuwamy
			if (tasma[i] != 0) {
				i++;
			}
			//dodajemy cegle na tasme
			if (akt_pojemnosc == 0 && i != 0) {
				i = 0;
			}
			tasma[i%T] = cegla;
			akt_pojemnosc += cegla;
			printf("+ %dkg\n", cegla);
			//wypisujemy zawartosc tasmy
			for (int j = 0; j<T; j++) {
				printf(" %d ", tasma[j]);
			}
			printf("Obciazenie tasmy: %d\n", akt_pojemnosc);
			pthread_mutex_unlock(&mutex);
		}
	}
}

//down worker
void *user_down(void *arg) {
	while (1) {
		sleep(rand() % 10);
		//sprawdzamy, czy miejsce na tasmie jest zajete i czy aktualna pojemnosc jest wieksza niz 0
		if (tasma[i%T] != 0 && akt_pojemnosc > 0) {
			pthread_mutex_lock(&mutex);
			pthread_cond_wait(&cond2, &mutex);
			printf("- %d kg\n", tasma[i%T]);
			akt_pojemnosc -= tasma[i%T];
			//zerujemy miejsce na tasmie, obowiazuje kolejnosc LIFO
			tasma[i%T] = 0;
			i--;
			for (int j = 0; j<T; j++) {
				printf(" %d ", tasma[j]);
			}
			printf("Obciazenie tasmy: %d\n", akt_pojemnosc);
			pthread_mutex_unlock(&mutex);

		}
	}
}

int main() {
	int randint, j, i;
	pthread_t pth1, pth2;
	pthread_create(&pth1, NULL, user_up, NULL);
	pthread_create(&pth2, NULL, user_down, NULL);

	printf("Zaczynamy: \n");

	while (1) {
		while (akt_pojemnosc > 0 && akt_pojemnosc <= T) {

			pthread_cond_signal(&cond1);
			pthread_cond_signal(&cond2);
		}
		while (akt_pojemnosc == 0) {
			//od tego zaczynamy, musimy cos dodac na tasme
			pthread_cond_signal(&cond1);
		}
		while (akt_pojemnosc == T) {
			//jesli osiagamy max, prosimy o sciagniecie elementu z tasmy
			pthread_cond_signal(&cond2);
		}

	}
	pthread_join(pth1, NULL);
	pthread_join(pth2, NULL);
	printf("Koniec programu\n");
	pthread_cancel(pth1);
	pthread_cancel(pth2);
}

