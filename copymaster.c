#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "options.h"
#include <time.h>
#include <dirent.h>

void FatalError(char c, const char *msg, int exit_status);
void PrintCopymasterOptions(struct CopymasterOptions *cpm_options);
void without(struct CopymasterOptions *cpm_options);
bool fals_flag(struct CopymasterOptions cpm_options);

int main(int argc, char *argv[])
{
    struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    //-------------------------------------------------------------------
    // Kontrola hodnot prepinacov
    //-------------------------------------------------------------------

    // Vypis hodnot prepinacov odstrante z finalnej verzie

    PrintCopymasterOptions(&cpm_options);

    //-------------------------------------------------------------------
    // Osetrenie prepinacov pred kopirovanim
    //-------------------------------------------------------------------

    if (cpm_options.fast && cpm_options.slow)
    {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    if (cpm_options.overwrite && cpm_options.create)
    {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    if (cpm_options.overwrite && cpm_options.append)
    {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    if (cpm_options.append && cpm_options.create)
    {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    if (cpm_options.truncate && cpm_options.delete_opt)
    {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    if (cpm_options.sparse && (cpm_options.fast || cpm_options.slow || cpm_options.create || cpm_options.overwrite || cpm_options.append || cpm_options.lseek || cpm_options.directory || cpm_options.delete_opt || cpm_options.chmod || cpm_options.inode || cpm_options.umask || cpm_options.link))
    {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    if (cpm_options.link && (cpm_options.fast || cpm_options.slow || cpm_options.create || cpm_options.overwrite || cpm_options.append || cpm_options.lseek || cpm_options.directory || cpm_options.delete_opt || cpm_options.chmod || cpm_options.inode || cpm_options.umask || cpm_options.sparse))
    {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    if (cpm_options.directory && (cpm_options.fast || cpm_options.slow || cpm_options.create || cpm_options.overwrite || cpm_options.append || cpm_options.lseek || cpm_options.link || cpm_options.delete_opt || cpm_options.chmod || cpm_options.inode || cpm_options.umask || cpm_options.sparse))
    {
        fprintf(stderr, "CHYBA PREPINACOV\n");
        exit(EXIT_FAILURE);
    }

    // TODO Nezabudnut dalsie kontroly kombinacii prepinacov ...

    //-------------------------------------------------------------------
    // Kopirovanie suborov
    //-------------------------------------------------------------------

    // TODO Implementovat kopirovanie suborov

    // cpm_options.infile
    // cpm_options.outfile

    //-------------------------------------------------------------------
    // Vypis adresara
    //-------------------------------------------------------------------

    if (fals_flag(cpm_options))
    {
        struct stat reprave;
        stat(cpm_options.infile, &reprave);
        mode_t mode = reprave.st_mode & 0777;

        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_CREAT | O_WRONLY | O_TRUNC, mode);

        char buf;
        int c;

        if (fstat(outfile, &reprave) < 0)
            FatalError('B', "INA CHYBA", 21);

        if (infile < 0)
            FatalError('B', "– SUBOR NEEXISTUJE", 21);

        if ((S_IWUSR & reprave.st_mode) == 0)
            FatalError('B', "INA CHYBA", 21);

        while ((c = read(infile, &buf, 1)) > 0)
            write(outfile, &buf, c);

        close(infile);
        close(outfile);
    }

    if (cpm_options.fast)
    {
        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_WRONLY);

        char buf[1000000];
        int c;

        if ((c = read(infile, buf, 1000000)) > 0)
            write(outfile, buf, c);
        else
            FatalError('f', "INA CHYBA", -1);

        close(infile);
        close(outfile);
    }

    if (cpm_options.slow)
    {
        struct stat stats;
        stat(cpm_options.infile, &stats);
        mode_t mode = stats.st_mode & 0777;

        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_CREAT | O_WRONLY | O_TRUNC, mode);

        char buf;
        int c;

        while ((c = read(infile, &buf, 1)) > 0)
            write(outfile, &buf, c);

        if ((c = read(infile, &buf, 1)) == -1)
            FatalError('s', "INA CHYBA", -1);

        close(infile);
        close(outfile);
    }

    if (cpm_options.create)
    {
        struct stat stats;
        stat(cpm_options.infile, &stats);
        mode_t create_mode = cpm_options.create_mode;

        if (create_mode < 1 || create_mode > 777)
            FatalError('c', "ZLE PRAVA", 23);

        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile;

        if ((outfile = open(cpm_options.outfile, O_EXCL)) > 0)
            FatalError('c', " SUBOR EXISTUJE", 23);

        if (outfile < 0)
        {
            outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT, create_mode);

            int size = stats.st_size;
            char buf[size];
            int c;

            while ((c = read(infile, &buf, size)) > 0)
                write(outfile, &buf, c);

            if ((c = read(infile, &buf, size)) == -1 || c == -1)
                FatalError('c', "INA CHYBA", 23);
        }

        close(infile);
        close(outfile);
    }

    if (cpm_options.overwrite)
    {
        struct stat stats;
        stat(cpm_options.infile, &stats);

        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_WRONLY);

        int size = stats.st_size;
        char buf[size];
        int c;

        if (outfile < 0)
            FatalError('o', " SUBOR NEXISTUJE", 24);

        if (outfile > 0)
        {
            outfile = open(cpm_options.outfile, O_RDONLY | O_WRONLY | O_TRUNC);

            while ((c = read(infile, &buf, size)) > 0)
                write(outfile, &buf, c);

            if ((c = read(infile, &buf, size)) == -1 || c == -1)
                FatalError('o', "INA CHYBA", 24);

            return 0;
        }

        close(infile);
        close(outfile);
    }

    if (cpm_options.append)
    {
        struct stat stats;
        stat(cpm_options.infile, &stats);

        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_WRONLY | O_APPEND);

        int c;
        int size = stats.st_size;
        char buf[size];

        if (outfile < 0)
            FatalError('a', "SUBOR NEEXISTUJE", 22);

        if (outfile > 0)
        {
            lseek(outfile, 0L, SEEK_END);
            c = read(infile, &buf, size);
            write(outfile, &buf, c);

            if ((c = read(infile, &buf, size)) == -1 || c == -1)
                FatalError('a', "INA CHYBA", 22);
        }

        close(infile);
        close(outfile);
    }

    if (cpm_options.lseek)
    {
        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_WRONLY);

        char buf;
        int c;

        if (cpm_options.lseek_options.pos1 < 0 && cpm_options.lseek_options.pos1 > 0)
            FatalError('l', "CHYBA POZICIE infile", 33);

        if (cpm_options.lseek_options.pos2 < 0 && cpm_options.lseek_options.pos2 > 0)
            FatalError('l', "CHYBA POZICIE outfile", 33);

        lseek(infile, cpm_options.lseek_options.pos1, SEEK_SET);

        if (cpm_options.lseek_options.x == 0)
            lseek(outfile, cpm_options.lseek_options.pos2, SEEK_SET);
        if (cpm_options.lseek_options.x == 1)
            lseek(outfile, cpm_options.lseek_options.pos2, SEEK_END);
        if (cpm_options.lseek_options.x == 2)
            lseek(outfile, cpm_options.lseek_options.pos2, SEEK_CUR);

        for (size_t i = 0; i < cpm_options.lseek_options.num; i++)
        {
            if ((c = read(infile, &buf, 1)) > 0)
                write(outfile, &buf, c);
            else
                FatalError('l', "INA CHYBA", 33);
        }

        close(infile);
        close(outfile);
    }

    if (cpm_options.delete_opt)
    {
        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_WRONLY | O_TRUNC);

        struct stat reprave;
        stat(cpm_options.infile, &reprave);

        int size = reprave.st_size;
        char buf[size];
        int c;

        if (!S_ISDIR(reprave.st_mode))
            remove(cpm_options.infile);
        else
            FatalError('d', "SUBOR NEBOL ZMAZANY", 26);

        while ((c = read(infile, &buf, size)) > 0)
        {
            if (outfile < 0)
                outfile = open(cpm_options.outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

            if (outfile == -1)
                FatalError('d', "INA CHYBA", 26);

            write(outfile, &buf, c);
            remove(cpm_options.infile);
        }
        if ((c = read(infile, &buf, size)) == -1 || c == -1)
            FatalError('d', "INA CHYBA", 26);

        close(infile);
        close(outfile);
    }

    if (cpm_options.chmod_mode)
    {
        int decimal_chmod_mode = atoi(argv[2]);

        mode_t chmod_mode = cpm_options.chmod_mode;

        if (decimal_chmod_mode < 1 || decimal_chmod_mode > 777)
            FatalError('m', "ZLE PRAVA", 34);

        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_CREAT | O_WRONLY, S_IRUSR | S_IROTH | S_IWUSR, chmod_mode, 0777);

        struct stat reprave;
        stat(cpm_options.infile, &reprave);

        int size = reprave.st_size;
        char buf[size];
        int c;

        chmod(cpm_options.outfile, chmod_mode);

        while ((c = read(infile, &buf, size)) > 0)
            write(outfile, &buf, c);

        if ((c = read(infile, &buf, size)) == -1 || c == -1)
            FatalError('m', "INA CHYBA", 34);

        close(infile);
        close(outfile);
    }

    if (cpm_options.inode)
    {
        // 1339891

        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_WRONLY);

        struct stat reprave;
        stat(cpm_options.infile, &reprave);

        int size = reprave.st_size;
        char buf[size];
        int c;

        if (reprave.st_ino != cpm_options.inode_number)
            FatalError('i', "ZLY INODE", 27);

        if (!S_ISREG(reprave.st_mode))
            FatalError('i', "ZLY TYP VSTUPNEHO SUBORU", 27);

        if ((reprave.st_ino = cpm_options.inode_number))
        {
            while ((c = read(infile, &buf, size)) > 0)
            {
                if ((outfile = open(cpm_options.outfile, O_CREAT | O_WRONLY, S_IRUSR | S_IROTH | S_IWUSR)))
                    write(outfile, &buf, c);
            }

            if ((c = read(infile, &buf, size)) == -1 || c == -1)
                FatalError('i', "INA CHYBA", 27);
        }

        close(infile);
        close(outfile);
    }

    if (cpm_options.umask)
    {
    }

    if (cpm_options.link)
    {
        int infile = open(cpm_options.infile, O_RDONLY);

        if (infile < 0)
            FatalError('K', "VSTUPNY SUBOR NEEXISTUJE", 30);

        if (link(cpm_options.infile, cpm_options.outfile) != 0)
            FatalError('K', "VYSTUPNY SUBOR NEVYTVORENY", 30);

        if (link(cpm_options.infile, cpm_options.outfile) == 0)
            return 0;

        close(infile);
    }

    if (cpm_options.truncate)
    {
        int infile = open(cpm_options.infile, O_RDONLY);
        int outfile = open(cpm_options.outfile, O_WRONLY);

        struct stat reprave;
        stat(cpm_options.infile, &reprave);

        int size = reprave.st_size;
        char buf[size];
        int c;

        if (cpm_options.truncate_size < 0)
            FatalError('t', "ZAPORNA VELKOST", 31);

        while ((c = read(infile, &buf, size)) > 0)
            write(outfile, &buf, c);

        if ((c = read(infile, &buf, size)) == -1 || c == -1)
            FatalError('t', "INA CHYBA", 31);

        if (S_ISREG(reprave.st_mode) != 0)
        {
            truncate(cpm_options.infile, cpm_options.truncate_size);
        }

        close(infile);
        close(outfile);
    }

    if (cpm_options.sparse)
    {
    }

    if (cpm_options.directory)
    {
        // TODO Implementovat vypis adresara
        FILE *output;
        output = fopen(cpm_options.outfile, "w");

        DIR *dir = opendir(cpm_options.infile);
        struct dirent *ent;

        struct stat buf;
        stat(cpm_options.infile, &buf);

        struct tm *info;
        info = gmtime(&buf.st_mtime);

        if (dir == NULL)
            return 1;
        if (output == NULL)
            FatalError('D', "VYSTUPNY SUBOR - CHYBA", 28);
        if (!S_ISDIR(buf.st_mode))
            FatalError('D', "VSTUPNY SUBOR NIE JE ADRESAR", 28);
        if (!dir)
            FatalError('D', "VSTUPNY SUBOR NEEXISTUJE", 28);

        while ((ent = readdir(dir)) != NULL)
        {
            fprintf(output, "\n");

            fprintf(output, (S_ISDIR(buf.st_mode)) ? "d" : "-");
            fprintf(output, (buf.st_mode & S_IRUSR) ? "r" : "-");
            fprintf(output, (buf.st_mode & S_IWUSR) ? "w" : "-");
            fprintf(output, (buf.st_mode & S_IXUSR) ? "x" : "-");
            fprintf(output, (buf.st_mode & S_IRGRP) ? "r" : "-");
            fprintf(output, (buf.st_mode & S_IWGRP) ? "w" : "-");
            fprintf(output, (buf.st_mode & S_IXGRP) ? "x" : "-");
            fprintf(output, (buf.st_mode & S_IROTH) ? "r" : "-");
            fprintf(output, (buf.st_mode & S_IWOTH) ? "w" : "-");
            fprintf(output, (buf.st_mode & S_IXOTH) ? "x" : "-");

            fprintf(output, " %ld %d %d %ld", buf.st_nlink, buf.st_uid, buf.st_gid, buf.st_size);
            fprintf(output, " %d-0%d-%d ", info->tm_mday, info->tm_mon, info->tm_year + 1900);
            fprintf(output, cpm_options.infile, &buf);
        }

        fclose(output);
        closedir(dir);
    }

    //-------------------------------------------------------------------
    // Osetrenie prepinacov po kopirovani
    //-------------------------------------------------------------------

    // TODO Implementovat osetrenie prepinacov po kopirovani

    return 0;
}

void FatalError(char c, const char *msg, int exit_status)
{
    fprintf(stderr, "%c:%d\n", c, errno);
    fprintf(stderr, "%c:%s\n", c, strerror(errno));
    fprintf(stderr, "%c:%s\n", c, msg);
    exit(exit_status);
}

bool fals_flag(struct CopymasterOptions cpm_options)
{

    return !cpm_options.fast && !cpm_options.slow && !cpm_options.create &&
           !cpm_options.overwrite && !cpm_options.append && !cpm_options.lseek &&
           !cpm_options.directory && !cpm_options.delete_opt && !cpm_options.chmod &&
           !cpm_options.inode && !cpm_options.umask && !cpm_options.link &&
           !cpm_options.truncate && !cpm_options.sparse;
}

void PrintCopymasterOptions(struct CopymasterOptions *cpm_options)
{
    if (cpm_options == 0)
        return;

    printf("infile:        %s\n", cpm_options->infile);
    printf("outfile:       %s\n", cpm_options->outfile);

    printf("fast:          %d\n", cpm_options->fast);
    printf("slow:          %d\n", cpm_options->slow);
    printf("create:        %d\n", cpm_options->create);
    printf("create_mode:   %o\n", (unsigned int)cpm_options->create_mode);
    printf("overwrite:     %d\n", cpm_options->overwrite);
    printf("append:        %d\n", cpm_options->append);
    printf("lseek:         %d\n", cpm_options->lseek);

    printf("lseek_options.x:    %d\n", cpm_options->lseek_options.x);
    printf("lseek_options.pos1: %ld\n", cpm_options->lseek_options.pos1);
    printf("lseek_options.pos2: %ld\n", cpm_options->lseek_options.pos2);
    printf("lseek_options.num:  %lu\n", cpm_options->lseek_options.num);

    printf("directory:     %d\n", cpm_options->directory);
    printf("delete_opt:    %d\n", cpm_options->delete_opt);
    printf("chmod:         %d\n", cpm_options->chmod);
    printf("chmod_mode:    %o\n", (unsigned int)cpm_options->chmod_mode);
    printf("inode:         %d\n", cpm_options->inode);
    printf("inode_number:  %lu\n", cpm_options->inode_number);

    printf("umask:\t%d\n", cpm_options->umask);
    for (unsigned int i = 0; i < kUMASK_OPTIONS_MAX_SZ; ++i)
    {
        if (cpm_options->umask_options[i][0] == 0)
        {
            // dosli sme na koniec zoznamu nastaveni umask
            break;
        }
        printf("umask_options[%u]: %s\n", i, cpm_options->umask_options[i]);
    }

    printf("link:          %d\n", cpm_options->link);
    printf("truncate:      %d\n", cpm_options->truncate);
    printf("truncate_size: %ld\n", cpm_options->truncate_size);
    printf("sparse:        %d\n", cpm_options->sparse);
}
