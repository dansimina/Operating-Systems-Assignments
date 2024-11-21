#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>


const int NUMBER = 33729;
const int REQ_SIZE = 255;
const int SECTION_SIZE = 22;
const int LINEOUT = 3072;

#define FIFO_RESP_NAME "RESP_PIPE_33729"
#define FIFO_REQ_NAME "REQ_PIPE_33729"

int main() {

    int fdRESP = -1;
    int fdREQ = -1;
    if(mkfifo(FIFO_RESP_NAME, 0644) != 0) {
        printf("ERROR\ncannot create the response pipe");
        return 1;
    }

    fdREQ = open(FIFO_REQ_NAME, O_RDONLY);
    fdRESP = open(FIFO_RESP_NAME, O_WRONLY);

    if(fdREQ == -1) {
        printf("ERROR\ncannot open the request pipe");
        close(fdRESP);
        unlink(FIFO_RESP_NAME);
        unlink(FIFO_REQ_NAME);
        return 1;
    }

    char *hello = "HELLO#";
    for(int i = 0; i < strlen(hello); i++) {
        write(fdRESP, &hello[i], sizeof(char));
    }

    printf("SUCCESS\n");

    int shmFd = -1;
    volatile char *shared = NULL;
    unsigned int sizeShm;
    int fileFd = -1;
    int fileSize = 0;
    char *file = NULL;

    for(;;) {
        char reqName[REQ_SIZE];
        for(int i = 0; i < REQ_SIZE; i++) {
            read(fdREQ, &reqName[i], sizeof(char));
            if(reqName[i] == '#') {
                reqName[i] = 0;
                break;
            }
        }

        if(strcmp(reqName, "PING") == 0) {
            write(fdRESP, "PING#", strlen("PING#") * sizeof(char));
            write(fdRESP, &NUMBER, sizeof(int));
            write(fdRESP, "PONG#", strlen("PONG#") * sizeof(char));
        } 
        else if(strcmp(reqName, "CREATE_SHM") == 0) {
            read(fdREQ, &sizeShm, sizeof(unsigned int));

            shmFd = shm_open("/WKaBMj8", O_CREAT | O_RDWR, 0664);
            if(shmFd < 0) {
                write(fdRESP, "CREATE_SHM#ERROR#", strlen("CREATE_SHM#ERROR#") * sizeof(char));
                continue;
            }
            ftruncate(shmFd, sizeof(char) * sizeShm);
            
            shared = (volatile char*) mmap(0, sizeof(char) * sizeShm, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
            if(shared == (void*)-1) {
                write(fdRESP, "CREATE_SHM#ERROR#", strlen("CREATE_SHM#ERROR#") * sizeof(char));
                shm_unlink("/WKaBMj8");
                continue;
            }

            write(fdRESP, "CREATE_SHM#SUCCESS#", strlen("CREATE_SHM#SUCCESS#") * sizeof(char));

        }
        else if(strcmp(reqName, "WRITE_TO_SHM") == 0) {
            unsigned int offset = 0;
            unsigned int value = 0;

            read(fdREQ, &offset, sizeof(unsigned int));
            read(fdREQ, &value, sizeof(unsigned int));

            if(offset >= 0 && (offset + sizeof(unsigned int)) < sizeShm) {
                lseek(shmFd, offset, SEEK_SET);
                write(shmFd, &value, sizeof(unsigned int));
                lseek(shmFd, 0, SEEK_SET);
                write(fdRESP, "WRITE_TO_SHM#SUCCESS#", strlen("WRITE_TO_SHM#SUCCESS#") * sizeof(char));
            }
            else {
                write(fdRESP, "WRITE_TO_SHM#ERROR#", strlen("WRITE_TO_SHM#ERROR#") * sizeof(char));
            }
        }
        else if(strcmp(reqName, "MAP_FILE") == 0) {
            char path[REQ_SIZE];
            for(int i = 0; i < REQ_SIZE; i++) {
                read(fdREQ, &path[i], sizeof(char));
                if(path[i] == '#') {
                    path[i] = 0;
                    break;
                }
            }

            fileFd = open(path, O_RDWR);
            if(fileFd == -1) {
                write(fdRESP, "MAP_FILE#ERROR#", strlen("MAP_FILE#ERROR#") * sizeof(char));
            }

            fileSize = lseek(fileFd, 0, SEEK_END);
            lseek(fileFd, 0, SEEK_SET);
            file = (char*)mmap(0, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileFd, 0);
            if(file == (void*)-1) {
                write(fdRESP, "MAP_FILE#ERROR#", strlen("MAP_FILE#ERROR#") * sizeof(char));
                close(fileFd);
                continue;
            }

            write(fdRESP, "MAP_FILE#SUCCESS#", strlen("MAP_FILE#SUCCESS#") * sizeof(char));
        }
        else if(strcmp(reqName, "READ_FROM_FILE_OFFSET") == 0) {
            unsigned int offset = 0;
            unsigned int noOfBytes = 0;

            read(fdREQ, &offset, sizeof(unsigned int));
            read(fdREQ, &noOfBytes, sizeof(unsigned int));

            if(shmFd == -1 || shared == NULL || file == NULL || (offset + noOfBytes) > fileSize) {
                write(fdRESP, "READ_FROM_FILE_OFFSET#ERROR#", strlen("READ_FROM_FILE_OFFSET#ERROR#") * sizeof(char));
                continue;
            }

            for (int i = 0; i < noOfBytes; i++) {
                shared[i] = file[offset + i];
            }
            write(fdRESP, "READ_FROM_FILE_OFFSET#SUCCESS#", strlen("READ_FROM_FILE_OFFSET#SUCCESS#") * sizeof(char));
        }
        else if(strcmp(reqName, "READ_FROM_FILE_SECTION") == 0) {
            unsigned int sectionNo = 0;
            unsigned int offset = 0;
            unsigned int noOfBytes = 0;

            read(fdREQ, &sectionNo, sizeof(unsigned int));
            read(fdREQ, &offset, sizeof(unsigned int));
            read(fdREQ, &noOfBytes, sizeof(unsigned int));

            unsigned int headerSize = (((0x000000FF & file[fileSize - 2]) << 8) + (0x000000FF & file[fileSize - 3]));
            unsigned int noOfSections = file[fileSize - headerSize + 1];

            if(noOfSections < sectionNo || sectionNo <= 0) {
                write(fdRESP, "READ_FROM_FILE_SECTION#ERROR#", strlen("READ_FROM_FILE_SECTION#ERROR#") * sizeof(char));
                continue;
            }
            
            unsigned int offsetHeader = fileSize - headerSize + 2 + (sectionNo - 1) * SECTION_SIZE + 14;
            unsigned int sectionOffset = (((0x000000FF & file[offsetHeader + 3]) << 24) + ((0x000000FF & file[offsetHeader + 2]) << 16) + ((0x000000FF & file[offsetHeader + 1]) << 8) + (0x000000FF & file[offsetHeader]));
            offsetHeader += 4;
            unsigned int sectionSize = (((0x000000FF & file[offsetHeader + 3]) << 24) + ((0x000000FF & file[offsetHeader + 2]) << 16) + ((0x000000FF & file[offsetHeader + 1]) << 8) + (0x000000FF & file[offsetHeader]));

            if(offset + noOfBytes > sectionSize || offset + noOfBytes < 0) {
                write(fdRESP, "READ_FROM_FILE_SECTION#ERROR#", strlen("READ_FROM_FILE_SECTION#ERROR#") * sizeof(char));
                continue;
            }

            for(int i = 0; i < noOfBytes; i++) {
                shared[i] = file[sectionOffset + offset + i];
            }

            write(fdRESP, "READ_FROM_FILE_SECTION#SUCCESS#", strlen("READ_FROM_FILE_SECTION#SUCCESS#") * sizeof(char));
        }
        else if(strcmp(reqName, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0) {
            int i = 0;

            unsigned int logicalOffset = 0;
            unsigned int noOfBytes = 0;

            read(fdREQ, &logicalOffset, sizeof(unsigned int));
            read(fdREQ, &noOfBytes, sizeof(unsigned int));

            unsigned int headerSize = (((0x000000FF & file[fileSize - 2]) << 8) + (0x000000FF & file[fileSize - 3]));
            unsigned int noOfSections = file[fileSize - headerSize + 1];
            
            unsigned int currentOffset = 0;
            
            for(i = 0; i < noOfSections; i++) {
                unsigned int offsetHeader = fileSize - headerSize + 2 + i * SECTION_SIZE + 18;
                unsigned int sectionSize = (((0x000000FF & file[offsetHeader + 3]) << 24) + ((0x000000FF & file[offsetHeader + 2]) << 16) + ((0x000000FF & file[offsetHeader + 1]) << 8) + (0x000000FF & file[offsetHeader]));
                
                unsigned int nextOffset = currentOffset + (sectionSize / LINEOUT + 1) * LINEOUT;

                if(currentOffset <= logicalOffset && logicalOffset < nextOffset) {
                    break;
                }
                currentOffset = nextOffset; 
            }
            
            if(i == noOfSections) {
                write(fdRESP, "READ_FROM_LOGICAL_SPACE_OFFSET#ERROR#", strlen("READ_FROM_LOGICAL_SPACE_OFFSET#ERROR#") * sizeof(char));
                continue;
            }
        
            unsigned int offset = logicalOffset - currentOffset;

            unsigned int offsetHeader = fileSize - headerSize + 2 + i * SECTION_SIZE + 14;
            unsigned int sectionOffset = (((0x000000FF & file[offsetHeader + 3]) << 24) + ((0x000000FF & file[offsetHeader + 2]) << 16) + ((0x000000FF & file[offsetHeader + 1]) << 8) + (0x000000FF & file[offsetHeader]));
            
            for(int j = 0; j < noOfBytes; j++) {
                shared[j] = file[sectionOffset + offset + j];
            }

            write(fdRESP, "READ_FROM_LOGICAL_SPACE_OFFSET#SUCCESS#", strlen("READ_FROM_LOGICAL_SPACE_OFFSET#SUCCESS#") * sizeof(char));
        }
        else {
            munmap((void*)file, sizeof(char) * fileSize);
            close(fileFd);
            
            shm_unlink("/WKaBMj8");
            munmap((void*)shared, sizeof(char) * sizeShm);

            close(fdRESP);
            close(fdREQ);
            unlink(FIFO_RESP_NAME);
            unlink(FIFO_REQ_NAME);

            return 0;
        }
    }

    return 0;
}