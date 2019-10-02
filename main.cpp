#include <iostream>
#include <boost/program_options.hpp>

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
    return 0;
}