#include <iostream>
#include <boost/program_options.hpp>
#include <sys/types.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <cctype>

const size_t BUFFER_SIZE = 4096;
char buf[BUFFER_SIZE + 1];
char buf_hex[BUFFER_SIZE * 4 + 1];

//#define USING_LIB_C
#ifdef USING_LIB_C
#define FILE_DESC FILE*
#else
#define FILE_DESC int
#endif

int stdout_write(const char *buffer, ssize_t size) {
    ssize_t written_bytes = 0;
    while (written_bytes < size) {
#ifdef USING_LIB_C
        ssize_t written_chunk = fwrite(buffer, sizeof(char), size, stdout);
        if (ferror(stdout)) {
#else
        ssize_t written_chunk = write(STDOUT_FILENO, buffer, size);
        if (written_chunk == -1) {
            if (errno == EINTR)
                continue;
#endif
            perror("Can`t write to stdout");
            exit(3);
        } else {
            written_bytes += written_chunk;
        }
    }
    return 0;
}

int file_read(FILE_DESC fd, char *buf, size_t sz) {
    while (true) {
#ifdef USING_LIB_C
        size_t read_n = fread(buf, sizeof(char), BUFFER_SIZE, fd);
        if (ferror(fd) && !feof(fd)) {
#else
            ssize_t read_n = read(fd, buf, BUFFER_SIZE);
            if (read_n == -1) {
                if (errno == EINTR)
                    continue;
#endif
            perror("Cannot read file");
            exit(4);
        } else {
            return read_n;
        }
    }

}

int fd_to_stdout(FILE_DESC fd, bool A) {
    ssize_t read_n;
    while ((read_n = file_read(fd, buf, BUFFER_SIZE)) > 0) {
        if (A) {
            size_t j = 0;
            for (size_t i = 0; i < read_n; i++) {
                if (!isprint(buf[i]) && !isspace(buf[i])) {
                    char c[5];
                    sprintf(c, "\\x%02x", static_cast<unsigned char>(buf[i]));
                    memcpy(buf_hex + j, c, 4);
                    j += 4;
                } else {
                    buf_hex[j] = buf[i];
                    j += 1;
                }
            }
            stdout_write(buf_hex, j);
        } else {
            stdout_write(buf, read_n);
        }
    }
    return 0;
}

namespace po = boost::program_options;

po::variables_map parse_args(int argc, char **argv) {
    po::options_description allowed("Allowed options");
    allowed.add_options()
            ("help,h", "Show help")
            ("show-all,A", "Show invisible characters");

    po::options_description hidden("Hidden options");
    hidden.add_options()
            ("input-file", po::value<std::vector<std::string> >(), "input file");

    po::options_description cmdline_options;
    cmdline_options.add(allowed).add(hidden);

    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(cmdline_options).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        allowed.print(std::cout);
        exit(0);
    }

    return vm;
}

#ifdef USING_LIB_C

std::vector<FILE *> open_files(std::vector<std::string> &files) {
    std::vector<FILE *> fds;
    for (auto &file_name : files) {
        auto fd = fopen(file_name.c_str(), "r");
        if (fd == nullptr) {
            perror("Cannot open file");
            exit(1);
        }
        fds.push_back(fd);
    }
    return fds;
}

#else
std::vector<int> open_files(std::vector<std::string> &files) {
    std::vector<int> fds;
    for (auto &file_name : files) {
        int fd = open(file_name.c_str(), O_RDONLY);
        if (fd == -1) {
            perror("Cannot open file");
        }
        fds.push_back(fd);
    }
    return fds;
}
#endif

int main(int argc, char **argv) {
    auto vm = parse_args(argc, argv);
    std::vector<std::string> input_files;
    if (vm.count("input-file")) {
        input_files = vm["input-file"].as<std::vector<std::string> >();
    } else {
        errno = EINVAL;
        perror("No input files");
        exit(2);
    }
    auto fds = open_files(input_files);
    for (auto fd : fds) {
        fd_to_stdout(fd, vm.count("show-all"));
#ifdef USING_LIB_C
        fclose(fd);
#else
        close(fd);
#endif
    }
    return 0;
}