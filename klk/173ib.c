#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>

#define SECONDS_IN_DAY (24*60*60)
#define KILOBYTE (1U << 10)
#define MEGABYTE (1U << 20)
#define GIGABYTE (1U << 30)

#define check_error(expr, msg) \
    do{ \
        if(!(expr)){ \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    }while(0)

void zad1Days(const char* fpath);
void zad2Size(char* fpath, const char* b);
void zad3Make(const char* type, const char* fpath, const char* mode);
void zad4SpecRead(const char* fpath, const size_t offset, const size_t count);
void zad5Count(const char* fpath, const char* fext, int* count);



int main(int argc, char** argv){
    check_error(argc == 3, "argc");
    int ret = 0;
    zad5Count(argv[1], argv[2], &ret);
    printf("%d\n", ret);
    return 0;
}



void zad1Days(const char* fpath){
    struct stat* st = malloc(sizeof(struct stat));
    check_error(st != NULL, "zad1Dani malloc");
    check_error(stat(fpath, st) == 0, "zad1Dani stat");
    check_error(S_ISREG(st->st_mode) == true, "zad1Dani !reg");
    int retval = labs(st->st_atime - st->st_mtime) / SECONDS_IN_DAY;
    printf("%d\n", retval);
    free(st);
    return;
}

void zad2Size(char* fpath, const char* b){
    char* filename = basename(fpath);
    struct stat *st = malloc(sizeof(struct stat));
    check_error(st != NULL, "zad2Size malloc");
    check_error(stat(fpath, st) == 0, "zad2Size stat");
    off_t sizeBytes = st->st_size;
    unsigned formatedSize = 0;
    switch(b[0]){
        case 'K':
            formatedSize = sizeBytes / KILOBYTE;
            if(sizeBytes % KILOBYTE)
                formatedSize++;
            printf("%s %uKB\n", filename, formatedSize);
            break;
        case 'M':
            formatedSize = sizeBytes / MEGABYTE;
            if(sizeBytes % MEGABYTE)
                formatedSize++;
            printf("%s %uMB\n", filename, formatedSize);
            break;
        case 'G':
            formatedSize = sizeBytes / GIGABYTE;
            if(sizeBytes % GIGABYTE)
                formatedSize++;
            printf("%s %uGB\n", filename, formatedSize);
            break;
        default:
            check_error(false, "zad2Size format");
    }
    free(st);
    return;
}

void zad3Make(const char* type, const char* fpath, const char* smode){
    check_error(access(fpath, F_OK) != 0, "zad3Make already exists");
    mode_t mode = strtol(smode, NULL, 8);
    if(type[1] == 'f'){
        int fd = open(fpath, O_CREAT, mode);
        check_error(fd != -1, "zad3Make open");
    }
    else if(type[1] == 'd'){
        mode_t old_mask = umask(0);
        check_error(mkdir(fpath, mode) == 0, "zad3Make mkdir");
        umask(old_mask);
    }
    else    check_error(false, "zad3Make file type");
    return;
}

void zad4SpecRead(const char* fpath, const size_t offset, const size_t count){
    size_t newOffset = offset;

    //proveri da li je fajl direktorijum
    struct stat* st = malloc(sizeof(struct stat));
    check_error(st != NULL, "zad4 stat malloc");
    check_error(stat(fpath, st) == 0, "zad4 stat");
    check_error(S_ISREG(st->st_mode) == true, "zad4 !reg");
    free(st);

    //otvaramo fajl na fpath za citanje
    int fd = open(fpath, O_RDONLY);
    check_error(fd != -1, "zad4 open");

    //pravimo membuff koji ce biti 1KB
    size_t buffsize = KILOBYTE * sizeof(char);
    char* membuff = malloc(buffsize);
    check_error(membuff != NULL, "zad4 membuff malloc");

    //citamo u membuff do zadatog offseta karakter po karakter
    int bytes_read = 0;
    while(newOffset--){
        bytes_read = read(fd, membuff, sizeof(char));
        check_error(bytes_read > 0, "zad4 offset too off set");
    }

    //citamo count karaktera
    bytes_read = read(fd, membuff, count);
    check_error(bytes_read != -1, "zad4 read");

    //ispisujemo membuff na STDOUT_FILENO
    bytes_read = write(STDOUT_FILENO, membuff, count);
    check_error(bytes_read != -1, "zad4 write");
}

void zad5Count(const char* fpath, const char* fext, int* count){
    // ako je reg fajl, povecavamo count
    struct stat* file_info = malloc(sizeof(struct stat));
    check_error(file_info != NULL, "zad5Count malloc");
    check_error(stat(fpath, file_info) == 0, "zad5Count stat");
    if(!S_ISDIR(file_info->st_mode)){
        if(S_ISREG(file_info->st_mode)){
            char* dot = strrchr(fpath, '.');
            if(dot != NULL && strcmp(dot, fext) == 0)    (*count)++;
        }
        return;
    }
    free(file_info); //ne treba nam vise, pa cemo osloboditi tu memoriju pre nego sto napravimo rekurentni poziv

    //ako je direktorijum, ali ne . ili .. krecemo rekurentni poziv od prvog nadjenog dir_entryja
    DIR* dir = opendir(fpath);
    check_error(dir != NULL, "zad5Count opendir");
    check_error(chdir(fpath) == 0, "zad5Count chdir1"); //menjamo working directorium u fpath
    struct dirent* ent = NULL;
    while((ent = readdir(dir)) != NULL){
        if((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
            continue;
        zad5Count(ent->d_name, fext, count);
    }
    check_error(chdir("..") == 0, "zad5Count chdir 2");
    check_error(closedir(dir) == 0, "zad5Count closedir");
}

















