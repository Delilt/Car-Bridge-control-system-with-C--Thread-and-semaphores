#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h> 

#define MaksimumAracSayisi 20
#define MaksimumBekleyenArac 3
#define MaksimumGecisSayisi 10

int KuzeyKuyruk = 0;
int GuneyKuyruk = 0;
int KopruAracSayisi = 0;
int GecisSayisi = 0;
int Yon = -1; // -1: Hiçbir yön, 0: Kuzey -> Güney, 1: Güney -> Kuzey

sem_t Mutex, Yol;

void ArayuzGuncelle() {
    printf("  -->                              <--\n");
    printf(" Kuyruk             K O P R U          Kuyruk\n");
    printf(" ======    =================      ======\n");
    if (Yon == 0) {
        printf(" %2d                %d -->            %2d\n", KuzeyKuyruk, KopruAracSayisi, GuneyKuyruk);
    } else if (Yon == 1) {
        printf(" %2d               <-- %d             %2d\n", KuzeyKuyruk, KopruAracSayisi, GuneyKuyruk);
    } else {
        printf(" %2d                %d                %2d\n", KuzeyKuyruk, KopruAracSayisi, GuneyKuyruk);
    }
}

void* AracGiris(void* Id) {
    int AracId = (intptr_t)Id;
    while (1) {
        int AracYon = rand() % 2; // 0: Kuzey -> Güney, 1: Güney -> Kuzey
        if (AracYon == 0) {
            sem_wait(&Mutex);
            KuzeyKuyruk++;
            ArayuzGuncelle();
            sem_post(&Mutex);
        } else {
            sem_wait(&Mutex);
            GuneyKuyruk++;
            ArayuzGuncelle();
            sem_post(&Mutex);
        }

        while (1) {
            sem_wait(&Yol);
            sem_wait(&Mutex);
            
            // Köprü yönü belirlenmemişse veya araç yönü köprü yönüyle aynıysa
            if (Yon == -1 || Yon == AracYon) {
                if (KopruAracSayisi < MaksimumBekleyenArac && GecisSayisi < MaksimumGecisSayisi) {
                    KopruAracSayisi++;
                    GecisSayisi++;
                    Yon = AracYon;
                    if (AracYon == 0) {
                        KuzeyKuyruk--;
                    } else {
                        GuneyKuyruk--;
                    }
                    ArayuzGuncelle();
                    printf("Arac %d, Kopruye Giris Yapiyor... [Kopru Arac Sayisi: %d, Yon: %s]\n", AracId, KopruAracSayisi, AracYon == 0 ? "-->" : "<--");
                    sem_post(&Mutex);
                    sem_post(&Yol);
                    usleep(rand() % 500000 + 500000); // Rastgele bekleme süresi
                    printf("Arac %d, Kopruden Geçiyor... [Kopru Arac Sayisi: %d, Yon: %s]\n", AracId, KopruAracSayisi, AracYon == 0 ? "-->" : "<--");
                    sem_wait(&Mutex);
                    KopruAracSayisi--;
                    ArayuzGuncelle();
                    printf("Arac %d, Kopruden Cikis Yapiyor... [Kopru Arac Sayisi: %d, Yon: %s]\n", AracId, KopruAracSayisi, AracYon == 0 ? "-->" : "<--");
                    sem_post(&Mutex);

                 
                    if (GecisSayisi >= MaksimumGecisSayisi) {
                        sem_wait(&Mutex);
                        GecisSayisi = 0; 
                        Yon = -1; // Yönü sıfırla
                        sem_post(&Mutex);
                    }

                    
                    pthread_exit(0);
                }
            }
            
            // Karşı yönden bekleyen araç yoksa, geçişe devam et
            if ((AracYon == 0 && GuneyKuyruk == 0) || (AracYon == 1 && KuzeyKuyruk == 0)) {
                if (KopruAracSayisi < MaksimumBekleyenArac) {
                    KopruAracSayisi++;
                    GecisSayisi++;
                    Yon = AracYon;
                    if (AracYon == 0) {
                        KuzeyKuyruk--;
                    } else {
                        GuneyKuyruk--;
                    }
                    ArayuzGuncelle();
                    printf("Arac %d, Kopruye Giris Yapiyor... [Kopru Arac Sayisi: %d, Yon: %s]\n", AracId, KopruAracSayisi, AracYon == 0 ? "-->" : "<--");
                    sem_post(&Mutex);
                    sem_post(&Yol);
                    usleep(rand() % 500000 + 500000); // Rastgele bekleme süresi
                    printf("Arac %d, Kopruden Geçiyor... [Kopru Arac Sayisi: %d, Yon: %s]\n", AracId, KopruAracSayisi, AracYon == 0 ? "-->" : "<--");
                    sem_wait(&Mutex);
                    KopruAracSayisi--;
                    ArayuzGuncelle();
                    printf("Arac %d, Kopruden Cikis Yapiyor... [Kopru Arac Sayisi: %d, Yon: %s]\n", AracId, KopruAracSayisi, AracYon == 0 ? "-->" : "<--");
                    sem_post(&Mutex);

                   
                    if (GecisSayisi >= MaksimumGecisSayisi) {
                        sem_wait(&Mutex);
                        GecisSayisi = 0; // Geçiş sayısını sıfırla
                        Yon = -1; // Yönü sıfırla
                        sem_post(&Mutex);
                    }

                  
                    pthread_exit(0);
                }
            }
            
            sem_post(&Mutex);
            sem_post(&Yol);
            usleep(rand() % 1000000 + 1000000); // Rastgele bekleme süresi
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL)); 

    sem_init(&Mutex, 0, 1); 
    sem_init(&Yol, 0, 1);   

    pthread_t AracThreadleri[MaksimumAracSayisi];
    for (int i = 0; i < MaksimumAracSayisi; i++) {
        pthread_create(&AracThreadleri[i], NULL, AracGiris, (void*)(intptr_t)(i + 1)); 
    }

    
    for (int i = 0; i < MaksimumAracSayisi; i++) {
        pthread_join(AracThreadleri[i], NULL);
    }

    sem_destroy(&Mutex); 
    sem_destroy(&Yol);   

    return 0;
}

