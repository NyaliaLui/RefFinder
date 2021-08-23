#include <iostream>
#include <cassert>
#include <vector>
#include <coroutine>
#include <string>
#include <sstream>

//The Smart pointer aroud the coroutine handle
template<typename T>
struct Parser {
    //Forward declare promise type
    struct promise_type;

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;

    Parser(handle_type h) : coro(h) { }
    ~Parser(void) {
        if (coro) {
            coro.destroy();
        }
    }

    T value(void) {
        return coro.promise().current_value;
    }

    bool next() {
        coro.resume();
        return not coro.done();
    }
};

template<typename T>
struct Parser<T>::promise_type {
    using handle_type = std::coroutine_handle<promise_type>;

    auto get_return_object() {
        return handle_type::from_promise(*this);
    }

    auto initial_suspend() { return std::suspend_always(); }
    auto final_suspend() noexcept(true) { return std::suspend_always(); }
    auto return_void() {}
    auto unhandled_exception() {
        std::terminate();
    }

    auto yield_value(const T value) {
        current_value = value;
        return std::suspend_always();
    }

    T current_value;
};

Parser<std::string>
create_parser(std::string file_names) {
    std::stringstream stream(file_names);

    while(stream.good()) {
        std::string substr;
        std::getline(stream, substr, ',');
        co_yield substr;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <func name> <file1[,file2,...]> " << std::endl;
        std::exit(1);
    }

    std::string func_name(argv[1]);
    auto parser = create_parser(argv[2]);
    std::vector<std::string> input_files;
    while(parser.next()) {
        std::string f_name = parser.value();
        input_files.push_back(f_name);
    }

    for(std::string f_name : input_files) {
        std::cout << f_name << std::endl;
    }
    return 0;
}
