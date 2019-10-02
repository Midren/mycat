#include <iostream>
#include <boost/program_options.hpp>
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
            size_t st = 0;
            for (size_t i = 0; i < read_n; i++) {
                if (!isprint(buf[i]) && !isspace(buf[i])) {
                    if (st != i)
                        write(STDOUT_FILENO, buf + st, i - st);
                    char c[5];
                    sprintf(c, "\\x%02x", static_cast<unsigned char>(buf[i]));
                    write(STDOUT_FILENO, c, 4);
                    st = i + 1;
                }
            }
            if (read_n - st)
                write(STDOUT_FILENO, buf, read_n - st);
        } else {
            if (write(STDOUT_FILENO, buf, read_n) != read_n) {
                write(STDERR_FILENO, "Couldn`t write the whole buffer", strlen("Couldn`t write the whole buffer"));
                return -1;
            }
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
        exit(1);
    }

    return vm;
}

int main(int argc, char **argv) {
    auto vm = parse_args(argc, argv);
    if (vm.count("input-file")) {
        std::vector<std::string> input_files = vm["input-file"].as<std::vector<std::string> >();
    } else {

    }
    fd_to_stdout("../main.cpp", true);
    return 0;
}