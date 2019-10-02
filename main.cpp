#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <cctype>

const size_t BUFFER_SIZE = 4096;



int fd_to_stdout(const std::string &file_name, bool A) {
    int fd = open(file_name.c_str(), O_RDONLY);
    ssize_t read_n;
    if (fd == -1) {
        write(STDERR_FILENO, "Can`t open file\n", 16);
        return -1;
    }
    char buf[BUFFER_SIZE + 1];
    while ((read_n = read(fd, buf, BUFFER_SIZE)) > 0) {
        if (A) {
            size_t st = 0, cur_buf_len = 0;
            for(size_t i = 0; i < BUFFER_SIZE; i++) {
                if(isprint(buf[i]) || isspace(buf[i])) {
                    cur_buf_len++;
                    continue;
                }
                else{
                    if(st != i){
                        write(STDOUT_FILENO, buf + st, cur_buf_len);
                        cur_buf_len = 0;
                    } else{
                        char c[2];
                        int x = sprintf(c, "%x", buf[i]);
                        write(STDOUT_FILENO, c, x);

                    }
                    st = i + 1;
                }
            }
            if(cur_buf_len)
                write(STDOUT_FILENO, buf, cur_buf_len);
        } else {
            if (write(STDOUT_FILENO, buf, read_n) != read_n) {
                write(STDERR_FILENO, "Couldn`t write the whole buffer", strlen("Couldn`t write the whole buffer"));
                return -1;
            }
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    fd_to_stdout("mycat", true);
    return 0;
}