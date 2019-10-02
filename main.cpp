#include <iostream>
#include <boost/program_options.hpp>
#include <sys/types.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <cctype>

const size_t BUFFER_SIZE = 4096;

int stdout_write(const char* buffer, ssize_t size){
    ssize_t written_bytes = 0;
    while(written_bytes < size){
        ssize_t written_chunk = write(STDOUT_FILENO, buffer, size);
        if(written_chunk == -1){
            if(errno == EINTR)
                continue;
            else{
                perror("Can`t write to file");
                exit(3);
            }
        }
        else{
            written_bytes += written_chunk;
        }
    }
    return 0;
}

int fd_to_stdout(int fd, bool A) {
    ssize_t read_n;
    char buf[BUFFER_SIZE + 1];
    while ((read_n = read(fd, buf, BUFFER_SIZE)) > 0) {
        if (A) {
            size_t st = 0;
            for (size_t i = 0; i < read_n; i++) {
                if (!isprint(buf[i]) && !isspace(buf[i])) {
                    if (st != i)
                        stdout_write(buf + st, i - st);
                    char c[5];
                    sprintf(c, "\\x%02x", static_cast<unsigned char>(buf[i]));
                    stdout_write(c, 4);
                    st = i + 1;
                }
            }
            if (read_n - st)
                stdout_write(buf, read_n - st);
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
    for (auto fd : fds)
        fd_to_stdout(fd, vm.count("show-all"));
    return 0;
}