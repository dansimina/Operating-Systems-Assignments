#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

struct __attribute__((packed)) section_header{
    char sect_name[13];
    char sect_type;
    int sect_offset;
    int sect_size;
 };
struct header{
    int version;
    int no_of_sections;
    struct section_header *section_headers;
    int header_size;
    char magic;
 };

void list(const int recursive, const char* path, const char* name_ends_with, const int permissions, int call_number) {
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[1024];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\ninvalid directory path");
        return;
    }

    if(call_number == 0)
        printf("SUCCESS\n");

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 1024, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                if((name_ends_with[0] == 0 || strcmp(entry->d_name + (strlen(entry->d_name) - strlen(name_ends_with)), name_ends_with) == 0) && (permissions == -1 || permissions == (statbuf.st_mode & 0777))) {
                    printf("%s\n", fullPath);
                }
                
                if(recursive > 0 && S_ISDIR(statbuf.st_mode)) {
                    list(recursive, fullPath, name_ends_with, permissions, call_number + 1);
                }
            }
        }
    }

    closedir(dir);
}

struct header parse(const char *path) {
    struct header header = {0};
    header.section_headers = NULL;

    int fd = open(path, O_RDONLY);
    
    if(fd == -1) {
        header.version = -5;
        return header;
    }

    char buf[1024] = {0};

    lseek(fd, -3, SEEK_END);
   
    if(read(fd, buf, 3) != 3) {
        header.version = -1;
        return header;
    }

    header.header_size = (((0x000000FF & buf[1]) << 8) + (0x000000FF & buf[0]));
    header.magic = 0x000000FF & buf[2];

    if(header.magic != 'n') {
        header.version = -1;
        return header;
    }

    int size = -header.header_size;
    lseek(fd, size, SEEK_END);
    if(read(fd, buf, 2) != 2) {
        header.version = -1;
        return header;
    }

    header.version = 0x000000FF & buf[0];

    if(!(header.version >= 63 && header.version <= 133)) {
        header.version = -2;
        return header;
    }

    header.no_of_sections = 0x000000FF & buf[1];

    if(!(header.no_of_sections == 2 || (header.no_of_sections >= 7 && header.no_of_sections <= 20))) {
        header.version = -3;
        return header;
    }

    header.section_headers = (struct section_header*) malloc (header.no_of_sections * sizeof(struct section_header));

    if(read(fd, header.section_headers, header.no_of_sections * sizeof(struct section_header)) != header.no_of_sections * sizeof(struct section_header)) {
        header.version = -3;
        return header;
    }

    for(int i = 0; i < header.no_of_sections; i++) {
        if(header.section_headers[i].sect_type != 94 && header.section_headers[i].sect_type != 17 && header.section_headers[i].sect_type != 22 && header.section_headers[i].sect_type != 27 && header.section_headers[i].sect_type != 93 && header.section_headers[i].sect_type != 91 && header.section_headers[i].sect_type != 11) {
            header.version = -4;
            return header;
        }
    }

    close(fd);
    return header;
}

void print_header(struct header header) {
    printf("SUCCESS\n");
    printf("version=%d\n", header.version);
    printf("nr_sections=%d\n", header.no_of_sections);

    for(int i = 0; i < header.no_of_sections; i++) {
        char name[14] = {0};
        strncpy(name, header.section_headers[i].sect_name, 13);
        printf("section%d: %s %d %d\n", i + 1, name, header.section_headers[i].sect_type, header.section_headers[i].sect_size);
    }
}

int stringToInteger(char *string) {
    int integer = 0;

    for(int i = 0; i < strlen(string); i++) {
        if(string[i] >= '0' && string[i] <= '9') {
            integer = integer * 10 + string[i] - '0';
        }
        else {
            return -1;
        }
    }

    return integer;
}

char* extract(const char *path, const int section, const int line, struct header header) {
    char *result = NULL;
    int fd = open(path, O_RDONLY);
    
    if(fd == -1) {
        perror("ERROR\ninvalid directory path");
        exit(1);
    }
    
    char *buf = (char*) malloc (header.section_headers[section - 1].sect_size * sizeof(char));
    lseek(fd, header.section_headers[section - 1].sect_offset, SEEK_SET);
    if(read(fd, buf, header.section_headers[section - 1].sect_size * sizeof(char)) != header.section_headers[section - 1].sect_size * sizeof(char)) {
        printf("ERROR\ninvalid section");
        goto final;
    }
    
    int cnt_lines = 0, line_start = header.section_headers[section - 1].sect_size - 1, i;

    for(i = header.section_headers[section - 1].sect_size - 1; i >= 0; i--) {
        if(buf[i] == '\n') {
            cnt_lines++;

            if(cnt_lines == line) {
                break;
            }
            else {
                line_start = i - 1;
            }
        }
    }

    if(i < 0)
        cnt_lines++;

    if(cnt_lines != line) {
        printf("ERROR\ninvalid line");
        goto final;
    }

    result = (char*) malloc ((line_start - i + 1) * sizeof(char));

    int line_end = i;
    int j = 0;

    for(i = line_start; i > line_end; i--) {
        result[j] = buf[i];
        j++;
    }

    result[j] = 0;
    
final:
    free(buf);
    close(fd);

    return result;
}

int isType27(struct header header) {
    for(int i = 0; i < header.no_of_sections; i++) {
        if(header.section_headers[i].sect_type == 27)
            return 1;
    }

    return 0;
}

void findall(const char* path, int call_number) {
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[1024];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\ninvalid directory path");
        return;
    }

    if(call_number == 0)
        printf("SUCCESS\n");

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 1024, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                if(!S_ISDIR(statbuf.st_mode)) {
                    struct header header = parse(fullPath);
                    if(header.version >= 0 && isType27(header) == 1) {
                        printf("%s\n", fullPath);
                    }

                    if(header.section_headers != NULL)
                        free(header.section_headers);
                }
                if(S_ISDIR(statbuf.st_mode)) {
                    findall(fullPath, call_number + 1);
                } 
            }
        }
    }

    closedir(dir);
}

int main(int argc, char **argv)
{
    if(argc >= 2) {
        char cmd[32] = {0};

        for(int i = 1; i < argc; i++) {
            if(strcmp(argv[i], "variant") == 0 || strcmp(argv[i], "list") == 0 || strcmp(argv[i], "parse") == 0 || strcmp(argv[i], "extract") == 0 || strcmp(argv[i], "findall") == 0) {
                strcpy(cmd, argv[i]);
                break;
            }
        }

        if(strcmp(cmd, "variant") == 0) {
            printf("33729\n");
        }
        else if(strcmp(cmd, "list") == 0) {
            int recursive = 0;
            char path[1024] = {0};
            char name_ends_with[1024] = {0};
            char permi_string[16] = {0};
            int permissions = -1;

            for(int i = 1; i < argc; i++) {
                if(strcmp(argv[i], "recursive") == 0) {
                    recursive = 1;
                }
                else if(strncmp(argv[i], "path=", 5) == 0) {
                    strcpy(path, argv[i] + 5);
                }
                else if(strncmp(argv[i], "name_ends_with=", 15) == 0) {
                    strcpy(name_ends_with, argv[i] + 15);
                }
                else if(strncmp(argv[i], "permissions=", 12) == 0) {
                    strcpy(permi_string, argv[i] + 12);
                }
            }

            int len_permi_string = strlen(permi_string);
            if(len_permi_string == 9) {
                permissions = 0;
                for(int i = 0; i < len_permi_string; i++) {
                    if(permi_string[i] != '-') {
                        permissions += (1 << (len_permi_string - i - 1));
                    }
                }
            }

            list(recursive, path, name_ends_with, permissions, 0);
        } 
        else if(strcmp(cmd, "parse") == 0) {
            char path[1024] = {0};

            for(int i = 1; i < argc; i++) {
                if(strncmp(argv[i], "path=", 5) == 0) {
                    strcpy(path, argv[i] + 5);
                }
            }

            struct header header = parse(path);

            if(header.version == -1) {
                printf("ERROR\nwrong magic\n");
            }
            else if(header.version == -2) {
                printf("ERROR\nwrong version\n");
            }
            else if(header.version == -3) {
                printf("ERROR\nwrong sect_nr\n");
            }
            else if(header.version == -4) {
                printf("ERROR\nwrong sect_types\n");
            }
            else if(header.version == -5) {
                printf("ERROR\ninvalid directory path");
            }
            else {
                print_header(header);
            }

            if(header.section_headers != NULL)
                free(header.section_headers);
        } 
        else if(strcmp(cmd, "extract") == 0) {
            char path[1024] = {0};
            char section_string[1024] = {0};
            char line_string[1024] = {0};

            for(int i = 1; i < argc; i++) {
                if(strncmp(argv[i], "path=", 5) == 0) {
                    strcpy(path, argv[i] + 5);
                } 
                else if(strncmp(argv[i], "section=", 8) == 0) {
                    strcpy(section_string, argv[i] + 8);
                }
                else if(strncmp(argv[i], "line=", 5) == 0) {
                    strcpy(line_string, argv[i] + 5);
                }
            }

            struct header header = parse(path);
            int section = stringToInteger(section_string);
            int line = stringToInteger(line_string);
            char *line_resulted = NULL;

            if(header.version < 0) {
                printf("ERROR\ninvalid file");
            }
            else if(section == -1 || section > header.no_of_sections) {
                printf("ERROR\ninvalid section");
            }
            else if(line == -1) {
                printf("ERROR\ninvalid line");
            }
            else {
                line_resulted = extract(path, section, line, header);

                if(line_resulted != NULL) {
                    printf("SUCCESS\n%s\n", line_resulted);
                    free(line_resulted);
                }
            }

            if(header.section_headers != NULL)
                free(header.section_headers);
        }
        else if(strcmp(cmd, "findall") == 0) {
            char path[1024] = {0};

            for(int i = 1; i < argc; i++) {
                if(strncmp(argv[i], "path=", 5) == 0) {
                    strcpy(path, argv[i] + 5);
                }
            }

            findall(path, 0);
        }
        else {
            printf("./a1 [OPTIONS] [PARAMETERS]\n");
        }
    }
    else {
            printf("./a1 [OPTIONS] [PARAMETERS]\n");
    }
    return 0;
}