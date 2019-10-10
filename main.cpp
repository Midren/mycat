#include <iostream>
#include <boost/program_options.hpp>
#include <sys/types.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <winnt.h>
#endif

#ifndef _WIN32
typedef char BYTE;
typedef size_t DWORD;
#endif
const size_t BUFFER_SIZE = 4096;
BYTE buf[BUFFER_SIZE + 1];
BYTE buf_hex[BUFFER_SIZE * 4 + 1];

//#define USING_LIB_C
#ifdef USING_LIB_C
#define FILE_DESC FILE*
#elif __linux__
#define FILE_DESC int
#elif _WIN32
#define FILE_DESC HANDLE
#endif


int stdout_write(const BYTE *buffer, DWORD size) {
    ssize_t written_bytes = 0;
    while (written_bytes < size) {
#ifdef USING_LIB_C
        ssize_t written_chunk = fwrite(buffer, sizeof(char), size, stdout);
        if (ferror(stdout)) {
#elif __linux__
        ssize_t written_chunk = write(STDOUT_FILENO, buffer, static_cast<unsigned int>(size));
        if (written_chunk == -1) {
            if (errno == EINTR)
                continue;
#elif _WIN32
            DWORD written_chunk;
            HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (!WriteFile(hStdOut, buffer, size, &written_chunk, nullptr)) {
#endif
            perror("Can`t write to stdout");
            exit(3);
        } else {
            written_bytes += static_cast<ssize_t>(written_chunk);
        }
    }

    return 0;
}

int file_read(FILE_DESC fd, BYTE *buf, DWORD sz) {
    while (true) {
#ifdef USING_LIB_C
        size_t read_n = fread(buf, sizeof(char), BUFFER_SIZE, fd);
        if (ferror(fd) && !feof(fd)) {
#elif __linux__
        ssize_t read_n = read(reinterpret_cast<int>(fd), buf, BUFFER_SIZE);
        if (read_n == -1) {
            if (errno == EINTR)
                continue;
#elif _WIN32
            DWORD read_n;
            if (!ReadFile(fd, buf, BUFFER_SIZE, &read_n, NULL)) {
#endif
            perror("Cannot read file");
            exit(4);
        } else {
            return static_cast<int>(read_n);
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


std::vector<FILE_DESC> open_files(std::vector<std::string> &files) {
    std::vector<FILE_DESC> fds;
    for (auto &file_name : files) {
#ifdef USING_LIB_C
        auto fd = fopen(file_name.c_str(), "r");
        if (fd == nullptr) {
#elif __linux__
        int fd = open(file_name.c_str(), O_RDONLY);
        if (fd == -1) {
#elif _WIN32
            auto fd = CreateFile(file_name.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY,
                                 nullptr);
            if (fd == INVALID_HANDLE_VALUE) {
#endif
            perror("Cannot open file");
        }
        fds.push_back(fd);
    }
    return fds;
}

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
        fd_to_stdout(fd, static_cast<bool>(vm.count("show-all")));
#ifdef USING_LIB_C
        fclose(fd);
#elif __linux__
        close(fd);
#elif _WIN32
        CloseHandle(fd);
#endif
    }
    return 0;
}