#define _XOPEN_SOURCE 700 //Neka magija neophodna za nftw() ==> man nftw
#define __USE_XOPEN_EXTENDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>

#define check_error(expr,userMsg) \
	do { \
		if (!(expr)) { \
			perror(userMsg); \
			exit(EXIT_FAILURE) ; \
		} \
	} while(0)

#define SECONDS_IN_DAY (24*60*60)
#define KILOBYTE (1U << 10)
#define FULL_MODE (S_IRWXU | S_IRWXG | S_IRWXO)

bool osIsPublicFile(const char* fpath){
  struct stat st;
  check_error(stat(fpath, &st) == 0, "stat");
  check_error(S_ISREG(st.st_mode), "dir");
  bool retval = (st.st_mode & (S_IROTH | S_IWOTH)) == (S_IROTH | S_IWOTH);
  return retval;
}

void osMkPublicDir(const char* dname){
    mode_t oldMask = umask(0);
    check_error(mkdir(dname, 0777) == 0, "mkdir");
    umask(oldMask);
}

unsigned osNumOfDaysFileModified(const char *fpath){
    struct stat st;
    check_error(stat(fpath, &st) == 0, "stat");
    unsigned daysSince = time(0)/SECONDS_IN_DAY - st.st_mtime/SECONDS_IN_DAY;
    return daysSince;
}

void osMoveFile(const char* srcPath, const char* destPath){
    struct stat stSrc, stDest;
    stat(srcPath, &stSrc);
    stat(destPath, &stDest);
    check_error(stSrc.st_ino != stDest.st_ino, "ino");
    check_error(rename(srcPath, destPath) == 0, "rename");
    return;
}

unsigned strSize(const char* str){
    unsigned i;
    for(i = 0; str[i] != '\0'; i++);
    return i;
}

char* concatWithChar(const char* s1, const char* s2, const char specChar){
    char* retStr = malloc(strSize(s1) + strSize(s2) + 2);
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

void osSpecialFunc(const char* srcPath, const char* backupPath){
    DIR* srcDir;
    check_error((srcDir = opendir(srcPath)) != NULL, "opendir");
    struct dirent* dirEnt;
    while((dirEnt = readdir(srcDir)) != NULL){
        struct stat stDirEnt;
        check_error(stat(dirEnt->d_name, &stDirEnt) == 0, "dirEnt stat");
        if(S_ISREG(stDirEnt.st_mode)){
            if(osNumOfDaysFileModified(dirEnt->d_name) > 30)
                remove(dirEnt->d_name);
            else
                osMoveFile(srcPath, backupPath);
        }
        if(S_ISDIR(stDirEnt.st_mode)){
            osSpecialFunc(concatWithChar(srcPath, dirEnt->d_name, '/'), backupPath);
        }
    }
}

char* fileToModeString(const char* filePath){
    char* retString = malloc(11);
    struct stat st;
    check_error(stat(filePath, &st) == 0, "fileToModeStat");
    mode_t mode = st.st_mode;
    for(int i = 0; i < 10; i++) retString[i] = '-';
    retString[10] = '\n';
    if(S_ISDIR(mode))   retString[0] = 'd';
    if(mode & S_IRUSR)   retString[1] = 'r';
    if(mode & S_IWUSR)   retString[2] = 'w';
    if(mode & S_IXUSR)   retString[3] = 'x';
    if(mode & S_IRGRP)   retString[4] = 'r';
    if(mode & S_IWGRP)   retString[5] = 'w';
    if(mode & S_IXGRP)   retString[6] = 'x';
    if(mode & S_IROTH)   retString[7] = 'r';
    if(mode & S_IWOTH)   retString[8] = 'w';
    if(mode & S_IXOTH)   retString[9] = 'x';
    return retString;
}

char* fileToOwnerString(const char* filePath){
    struct stat st;
    check_error(stat(filePath, &st) == 0, "fileToOwnerString stat");
    struct passwd* pw;
    struct group* gp;
    check_error((pw = getpwuid(st.st_uid)) != NULL, "fileToOwnerString getpwuid");
    check_error((gp = getgrgid(st.st_gid)) != NULL, "fileToOwnerString getgrgid");
    return concatWithChar(pw->pw_name, gp->gr_name, ' ');
}

void changeOrMakeRegFile(const char* filepath, const mode_t mode){
    check_error(open(filepath, O_WRONLY | O_CREAT, mode) != -1, "changeOrMakeRegFile");
    return;
}

void changeOrMakeDir(const char* filepath, const mode_t mode){
    errno = 0;
    if(chmod(filepath, mode) == -1){
        if(errno == ENOENT){
            mode_t oldMask = umask(0);
            mkdir(filepath, mode);
            umask(oldMask);
        }
        else    check_error(false, "changeOrMakeDir");
    }
    return;
}

void mkRegOrDir(const char* ft, const char* filepath, const char* strmode){
    mode_t mode = strtol(strmode, NULL, 8);
    if(ft[1] == 'f'){
        changeOrMakeRegFile(filepath, mode);
    }
    else{
        changeOrMakeDir(filepath, mode);
    }
    return;
}

void osR(const char* filepath){
    int fd;
    check_error((fd = open(filepath, O_RDONLY)) != -1, "osR open");
    ssize_t buffSize = KILOBYTE;
    char* membuff = malloc(buffSize * sizeof(char));
    check_error(membuff != NULL, "osR malloc");
    ssize_t bytes_read = 0;
    while((bytes_read = read(fd, membuff, buffSize)) > 0)
        check_error(write(STDOUT_FILENO, membuff, bytes_read) != -1, "osR write");
    free(membuff);
    return;
}

void osW(const char* filepath){
    int fd;
    check_error((fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, FULL_MODE)) != -1, "osW open");
    ssize_t buffSize = KILOBYTE;
    char* membuff = malloc(buffSize * sizeof(char));
    check_error(membuff != NULL, "osW malloc");
    ssize_t bytes_read = 0;
    while((bytes_read = read(STDIN_FILENO, membuff, buffSize)) > 0){
        check_error(write(fd, membuff, bytes_read) != -1, "osW write");
    }
    free(membuff);
    return;
}

void osA(const char* filepath){
    int fd;
    check_error((fd = open(filepath, O_WRONLY | O_APPEND)) != -1, "osA open");
    ssize_t buffSize = KILOBYTE;
    char* membuff = malloc(buffSize * sizeof(char));
    check_error(membuff != NULL, "osA malloc");
    ssize_t bytes_read = 0;
    while((bytes_read = read(STDIN_FILENO, membuff, buffSize)) > 0){
        check_error(write(fd, membuff, bytes_read) != -1, "osW write");
    }
    free(membuff);
    return;
}

int nFajlova[8] = {0,0,0,0,0,0,0,0};
int fnPrebrojFajl(const char* fpath, const struct stat* sb, int typeflag, struct FTW *ftwbuf){
    if(S_ISREG(sb->st_mode))        nFajlova[0]++;
    else if(S_ISDIR(sb->st_mode))   nFajlova[1]++;
    else if(S_ISCHR(sb->st_mode))   nFajlova[2]++;
    else if(S_ISBLK(sb->st_mode))   nFajlova[3]++;
    else if(S_ISLNK(sb->st_mode))   nFajlova[4]++;
    else if(S_ISFIFO(sb->st_mode))  nFajlova[5]++;
    else if(S_ISSOCK(sb->st_mode))  nFajlova[6]++;
    nFajlova[7]++;
    return 0;
}

int main(int argc, char** argv){
    check_error(argc == 3, "argc");
    osMoveFile(argv[1], argv[2]);
    return 0;
}