#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

#define check_error(expr, msg) \
    do { \
        if(!(expr)) { \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

void zad1Time       (const char* filepath, const char* stime);
void zad2RenameDir  (const char* oldpath, const char* newpath);
void zad3Swap       (const char* filepath);
void zad4Write      (const char* infilepath, const char* outfilepath);
int zad5MinMax     (const char* rootpath);
char* concatWithChar(const char* s1, const char* s2, const char specChar);

int main(int argc, char** argv) {
    check_error(argc == 2, "argc");
    printf("%d\n", zad5MinMax(argv[1]));
    return 0;
}

void zad1Time(const char* filepath, const char* stime) {
    //prevodimo string stime u unsigned time
    unsigned time = strtoul(stime, NULL, 10);

    //koristeci utime strukturu menjamo atime i mtime prosledjenog fajla
    struct utimbuf* new_times = malloc(sizeof(struct utimbuf));
    check_error(new_times != NULL, "zad1Time utimbuf malloc"); 
    new_times->modtime = time;
    new_times->actime = time;
    check_error(utime(filepath, new_times) == 0, "zad1Time utime");
    return;
}

void zad2RenameDir(const char* oldpath, const char* newpath) {
    //proveravamo da li je prosledjeni fajl direktorijum i ukoliko nije, zavrsavamo program
    struct stat* dirinfo = malloc(sizeof(struct stat));
    check_error(dirinfo != NULL, "zad2RenameDir malloc");
    check_error(stat(oldpath, dirinfo) == 0, "zad2RenameDir stat");
    check_error(S_ISDIR(dirinfo->st_mode), "zad2RenameDir !dir");
    free(dirinfo);

    //nadjemo poslednji '/' u oldpath i postavimo radni direktorijum tu. ukoliko '/' ne postoji, ne menjamo radni direktorijum
    char* slash = strrchr(oldpath, '/');
    if (slash != NULL) {
        //cuvamo putanju do fajla (od pocetka oldpath do poslednjeg '/')
        char* oldpathpointer = oldpath;
        char* oldpathroot = malloc(strlen(oldpath) * sizeof(char));
        check_error(oldpathroot != NULL, "zad2RenameDir oldpathroot malloc");
        while (oldpathpointer != slash) {
            oldpathroot[oldpathpointer-oldpath] = *(oldpathpointer);
            oldpathpointer++;
        }
        oldpathroot[oldpathpointer-oldpath] = '\0';
        printf("%s\n", oldpathroot);

        //cuvamo ime fajla (od poslednjeg '/' do kraja oldpath)
        char* oldpathfilename = malloc(strlen(oldpath) * sizeof(char));
        int i = 0;
        while (*(slash++) != '\0') {
            oldpathfilename[i] = *(slash);
            i++;
        }
        oldpathfilename[i] = '\0';

        //menjamo radni direktorijum i menjamo ime direktorijumu
        check_error(chdir(oldpathroot) == 0, "zad2RenameDir chdir");
        check_error(rename(oldpathfilename, newpath) == 0, "zad2RenameDir rename");
        free(oldpathroot);
        free(oldpathfilename);
    }
    else {
        rename(oldpath, newpath);
    }
    return;
}

void zad3Swap(const char* filepath) {
    //probamo da otvorimo stat prosledjenog fajla, i ako ne postoji, vracamo gresku
    struct stat* fstat = malloc(sizeof(struct stat));
    check_error(fstat != NULL, "zad3Swap malloc");
    check_error(stat(filepath, fstat) == 0, "zad3Swap stat");
    mode_t oldmode = fstat->st_mode;
    free(fstat);

    //formiramo novi mod od starog moda tako sto menjamo mesta GRP i OTH pravima pristupa
    mode_t newmode = (S_IRWXU & oldmode) |
                     (((S_IRGRP & oldmode) | (S_IWGRP & oldmode) | (S_IXGRP & oldmode)) >> 3) |
                     (((S_IROTH & oldmode) | (S_IWOTH & oldmode) | (S_IXOTH & oldmode)) << 3);
    mode_t old_umask = umask(0);
    check_error(chmod(filepath, newmode) == 0, "zad3Swap chmod");
    umask(old_umask);
    return;
}

void zad4Write(const char* infilepath, const char* outfilepath) {
    //otvaramo oba fajla, prvi za citanje, drugi za pisanje
    FILE* infile = fopen(infilepath, "r");
    int outfile = open(outfilepath, O_CREAT | O_WRONLY, 0777);
    check_error(infile  != NULL, "zad4Write infile open");
    check_error(outfile != -1, "zad4Write outfile open");

    char* line = malloc(256 * sizeof(char));
    char* text = malloc(256 * sizeof(char));
    check_error(line != NULL, "zad4Write line malloc");
    check_error(text != NULL, "zad4Write text malloc");
    int offset;
    while (fgets(line, 256, infile)) {
        sscanf(line, "%d %s", &offset, text);
        check_error(lseek(outfile, offset, SEEK_SET) == offset, "zad4Write lseek");
        check_error(write(outfile, text, strlen(text)) != -1, "zad4Write write");
    }
    free(line);
    free(text);

    fclose(infile);
    close(outfile);
    return;
}

int zad5MinMax(const char* rootpath) {
    DIR* dir = opendir(rootpath);
    check_error(dir != NULL, "zad5MinMax opendir");
    struct dirent* ent = NULL;
    static size_t minsize = -1, maxsize = -1;
    struct stat* fileinfo = malloc(sizeof(struct stat));
    check_error(fileinfo != NULL, "zad5MinMax stat malloc");
    while ((ent = readdir(dir)) != NULL) {
        if((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
            continue;

        check_error(ent != NULL, "zad5MinMax readdir");
        char* filepath = concatWithChar(rootpath, ent->d_name, '/');
        check_error(stat(filepath, fileinfo) == 0, "zad5MinMax stat");
        if (S_ISREG(fileinfo->st_mode)) {
            size_t filesize = fileinfo->st_size;
            if (minsize == -1 || maxsize == -1) {
                minsize = filesize;
                maxsize = filesize;
            }
            if (filesize < minsize) minsize = filesize;
            if (filesize > maxsize) maxsize = filesize;
        }
        else { //ako je dir
            zad5MinMax(concatWithChar(rootpath, ent->d_name, '/'));
        }
    }

    closedir(dir);
    free(ent);
    free(fileinfo);    

    check_error(minsize != -1, "zad5MinMax min err");
    check_error(maxsize != -1, "zad5MinMax max err");
    return maxsize-minsize;
}

char* concatWithChar(const char* s1, const char* s2, const char specChar){
    char* retStr = malloc(strlen(s1) + strlen(s2) + 2);
    int i = 0, j = 0;
    while(s1[i] != '\0'){
        retStr[i] = s1[i];
        i++;
    }
    retStr[i] = specChar;
    i++;
    while(s2[j] != '\0'){
        retStr[i] = s2[j];
        i++;
        j++;
    }
    retStr[i] = '\0';
    return retStr;
}












