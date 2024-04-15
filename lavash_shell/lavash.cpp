#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sched.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

class Command {
public:
    void AddArg(const std::string& arg) {
        args.push_back(arg);
    }

    void AddPipeIn(const std::string& pipe) {
        pipe_in_ = pipe;
    }
    bool IsPipeInEmpty() {
        return pipe_in_.empty();
    }
    std::string GetPipeIn() {
        return pipe_in_;
    }
    void AddPipeOut(const std::string& pipe) {
        pipe_out_ = pipe;
    }
    bool IsPipeOutEmpty() {
        return pipe_out_.empty();
    }
    std::string GetPipeOut() {
        return pipe_out_;
    }
    void PrintInfo() {
        std::cout << "Args: ";
        for (auto x : args) {
            std::cout << x << ' ';
        }
        std::cout << std::endl;
        std::cout << "Pipe_in: #" << pipe_in_ << ' ' << fd_in_ << "#" << std::endl;
        std::cout << "Pipe_out: #" << pipe_out_<< ' ' << fd_out_ << "#" << std::endl;
    }

    void Clear() {
        args.clear();
        pipe_in_.clear();
        pipe_out_.clear();
    }

    bool IsEmpty() {
        return args.empty() && pipe_in_.empty() && pipe_out_.empty();
    }

    void Run() {
        if (args.size() == 0) {
            exit(13);
        }
        if (args[0] == "1984") {
            args[0] = "true";
        }
        char ** args_char = new char* [args.size() + 1];
        for (int i = 0; i < args.size(); ++i) {
            args_char[i] = new char[args[i].size() + 1];
            for (int j = 0; j < args[i].size(); ++j) {
                args_char[i][j] = args[i][j];
            }
            args_char[i][args[i].size()] = '\0';
        }
        args_char[args.size()] = 0;
        if (fd_in_ != -2) {
            if (dup2(fd_in_, STDIN_FILENO) == -1) {
                close(fd_in_);
                exit(1);
            }
            close(fd_in_);
        }
        if (fd_out_ != -2) {
            if (dup2(fd_out_, STDOUT_FILENO) == -1) {
                close(fd_out_);
                std::cerr << "Dup2 crashed" << '\n';
                exit(1);
            }
            close(fd_out_);
        }
        execvp(args_char[0], args_char);
        std::cerr << "./lavash: line 1: " << args[0] << ": command not found\n";
        exit(127);
    }

    void OpenFiles() {
        if (!this->IsPipeInEmpty()) {
            fd_in_ = open(const_cast<char*>(pipe_in_.c_str()), O_RDONLY);
            if (fd_in_ == -1) {
                std::cerr << "./lavash: line 1: " << pipe_in_ << ": No such file or directory\n";
                exit(1);
            }
        }
        if (!this->IsPipeOutEmpty()) {
            fd_out_ = open(const_cast<char*>(pipe_out_.c_str()), O_WRONLY | O_CREAT);
            if (fd_out_ == -1) {
                std::cerr << "./lavash: line 1: " << pipe_out_ << ": No such file or directory\n";
                exit(1);
            }
        }
    }

    int GetFdIn() {
        return fd_in_;
    }
    int GetFdOut() {
        return fd_out_;
    }

    bool SetFdIn(int fd) {
        if (fd_in_ < 0) {
            fd_in_ = fd;
            return true;
        }
        return false;
    }

    bool SetFdOut(int fd) {
        if (fd_out_ < 0) {
            fd_out_ = fd;
            return true;
        }
        return false;
    }
private:
    std::vector<std::string> args;
    std::string pipe_in_ = "";
    std::string pipe_out_ = "";
    // int append_flag = 0; // >> flag to add to a file, not overwrite it
    int fd_in_ = -2;
    int fd_out_ = -2;

};

class CommandBlock {
public:
    void AddCommand(Command& command) {
        commands_.push_back(command);
    }

    inline void ChangeConnectionType(int x) {
        connection_flag_ = x;
    }
    bool IsEmpty() {
        return commands_.empty();
    }

    void Clear() {
        commands_.clear();
        connection_flag_ = 0;
        return;
    }

    int GetCon() {
        return connection_flag_;
    }

    int Run() {
        for (size_t i = 0; i < commands_.size(); ++i) {
            commands_[i].OpenFiles();
        }
        int status = -1;
        std::vector<pid_t> pids(commands_.size());
        int fd[2];
        int fd_old = -1;
        for (int i = 0; i < commands_.size(); ++i) {
            if (i + 1 != commands_.size()) {
                pipe(fd);
            }
            pid_t pid = fork();
            if (pid == -1) {
                exit(1);
            } else if (pid == 0) {
                if (i + 1 != commands_.size()) {
                    if (!commands_[i].SetFdOut(fd[1])) {
                        close(fd[1]);
                    }
                    close(fd[0]);
                }
                commands_[i].Run();
                exit(127);
            } else {
                pids[i] = pid;
                if (fd_old != -1) {
                    close(fd_old);
                    fd_old = -1;
                }
                if (i + 1 != commands_.size()) {
                    if (!commands_[i + 1].SetFdIn(fd[0])) {
                        close(fd[0]);
                    }
                    close(fd[1]);
                    fd_old = fd[0];
                }
            }
        }
        for (size_t i = 0; i < pids.size(); ++i) { // Waiting for all commands to be executed
            int new_status = 0;
            waitpid(pids[i], &new_status, 0);
            status = new_status; // get last command's execute code
        }
        return WEXITSTATUS(status);
    }
private:
    std::vector<Command> commands_ = {};
    int connection_flag_ = 0; // 0 - terminal block, 1 - &&, 2 - ||
};

bool IsSpecialSymbol(char x) {
    static std::string special_symbols = "<>|& ";
    for (auto y : special_symbols) {
        if (x == y) {
            return true;
        }
    }
    return false;
}

void Parse(char *str, std::vector<CommandBlock>& args) {
    size_t str_len = strlen(str);
    if (str_len == 0) {
        return;
    }
    std::string cur_str;
    bool fl_quote = false; // ' flag
    bool fl_dquote = false; // " flag
    Command cur_command;
    CommandBlock cur_block;
    for (size_t i = 0; i < str_len; ++i) {
        switch (str[i]) {
            case '"':
                if (fl_quote) {
                    cur_str.push_back(str[i]);
                } else {
                    fl_dquote ^= true;
                }
                break;
            case '\'':
                if (fl_dquote) {
                    cur_str.push_back(str[i]);
                } else {
                    fl_quote ^= true;
                }
                break;
            case '\\':
                ++i;
                if (i >= str_len) {
                    std:: cerr << "Syntax error next to an unexpected marker \"\\\"" << '\n';
                    exit(1);
                }
                cur_str.push_back(str[i]);
                break;
            case ' ':
                if (fl_dquote || fl_quote) {
                    cur_str.push_back(str[i]);
                } else if (!cur_str.empty()) {
                    cur_command.AddArg(cur_str);
                    cur_str.clear();
                }
                break;
            case '&':
                if (fl_dquote || fl_quote) {
                    cur_str.push_back(str[i]);
                } else {
                    ++i;
                    if (i >= str_len || str[i] != '&') {
                        std::cerr << "Syntax error next to an unexpected marker \"&\"" << '\n';
                        exit(1);
                    }
                    if (!cur_str.empty()) {
                        cur_command.AddArg(cur_str);
                        cur_str.clear();
                    }
                    if (!cur_command.IsEmpty()) {
                        cur_block.AddCommand(cur_command);
                        cur_command.Clear();
                    }
                    if (cur_block.IsEmpty()) {
                        std::cerr << "Syntax error next to an unexpected marker \"&&\"" << '\n';
                        exit(1);
                    }
                    cur_block.ChangeConnectionType(1);
                    args.push_back(cur_block);
                    cur_block.Clear();
                }
                break;
            case '|':
                if (fl_dquote || fl_quote) {
                    cur_str.push_back(str[i]);
                } else {
                    ++i;
                    if (i >= str_len) {
                        std::cerr << "Syntax error next to an unexpected marker \"|\"" << '\n';
                        exit(1);
                    }
                    else if (str[i] == '|') {
                        if (!cur_str.empty()) {
                            cur_command.AddArg(cur_str);
                            cur_str.clear();
                        }
                        if (!cur_command.IsEmpty()) {
                            cur_block.AddCommand(cur_command);
                            cur_command.Clear();
                        }
                        if (cur_block.IsEmpty()) {
                            std::cerr << "Syntax error next to an unexpected marker \"||\"" << '\n';
                            exit(1);
                        }
                        cur_block.ChangeConnectionType(2);
                        args.push_back(cur_block);
                        cur_block.Clear();
                    } else {
                        --i;
                        if (!cur_str.empty()) {
                            cur_command.AddArg(cur_str);
                            cur_str.clear();
                        }
                        if (!cur_command.IsEmpty()) {
                            cur_block.AddCommand(cur_command);
                            cur_command.Clear();
                        } else {
                            std::cerr << "Syntax error next to an unexpected marker \"|\"" << '\n';
                            exit(1);
                        }
                    }
                }
                break;
            case '<':
                if (fl_dquote || fl_quote) {
                    cur_str.push_back(str[i]);
                } else {
                    if (!cur_str.empty()) {
                        cur_command.AddArg(cur_str);
                        cur_str.clear();
                    }
                    ++i;
                    if (i >= str_len) {
                        std::cerr << "Syntax error next to an unexpected marker \"<\"" << '\n';
                        exit(1);
                    }
                    bool fl_tmp = false;
                    if (str[i] == '<') {
                        ++i;
                        fl_tmp = true;
                    }
                    while (i < str_len && str[i] == ' ') {
                        ++i;
                    }
                    if (i >= str_len || IsSpecialSymbol(str[i])) {
                        if (fl_tmp) {
                            std::cerr << "Syntax error next to an unexpected marker \"<<\"" << '\n';
                        } else {
                            std::cerr << "Syntax error next to an unexpected marker \"<\"" << '\n';
                        }
                        exit(1);
                    }
                    bool fl_quote_fn = false; // flag ' for file name
                    bool fl_dquote_fn = false; // " flag for file name
                    bool fl_break = false;
                    while (i < str_len) {
                        switch (str[i]) {
                            case '"':
                                if (fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_dquote_fn ^= true;
                                }
                                break;
                            case '\'':
                                if (fl_dquote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_dquote_fn ^= true;
                                }
                                break;
                            case '|':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case '&':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case '<':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case '>':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case ' ':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case '\\':
                                ++i;
                                if (i >= str_len) {
                                    std::cerr << "Syntax error next to an unexpected marker \"\\\"" << '\n';
                                    exit(1);
                                }
                                cur_str.push_back(str[i]);
                                break;
                            default:
                                cur_str.push_back(str[i]);
                                break;
                        }
                        if (fl_break) {
                            break;
                        }
                        ++i;
                    }
                    if (cur_str.empty()) {
                        if (fl_tmp) {
                            std::cerr << "Syntax error next to an unexpected marker \"<<\"" << '\n';
                        } else {
                            std::cerr << "Syntax error next to an unexpected marker \"<\"" << '\n';
                        }
                        exit(1);
                    }
                    cur_command.AddPipeIn(cur_str);
                    cur_str.clear();
                    --i;
                }
                break;
            case '>':
                if (fl_dquote || fl_quote) {
                    cur_str.push_back(str[i]);
                    break;
                } else {
                    if (!cur_str.empty()) {
                        cur_command.AddArg(cur_str);
                        cur_str.clear();
                    }
                    ++i;
                    if (i >= str_len) {
                        std::cerr << "Syntax error next to an unexpected marker \">\"" << '\n';
                        exit(1);
                    }
                    bool fl_tmp = false;
                    if (str[i] == '>') {
                        ++i;
                        fl_tmp = true;
                    }
                    while (i < str_len && str[i] == ' ') {
                        ++i;
                    }
                    if (i >= str_len || IsSpecialSymbol(str[i])) {
                        if (fl_tmp) {
                            std::cerr << "Syntax error next to an unexpected marker \">>\"" << '\n';
                        } else {
                            std::cerr << "Syntax error next to an unexpected marker \">\"" << '\n';
                        }
                        exit(1);
                    }
                    bool fl_quote_fn = false; // flag ' for file name
                    bool fl_dquote_fn = false; // " flag for file name
                    bool fl_break = false;
                    while (i < str_len) {
                        switch (str[i]) {
                            case '"':
                                if (fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_dquote_fn ^= true;
                                }
                                break;
                            case '\'':
                                if (fl_dquote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_dquote_fn ^= true;
                                }
                                break;
                            case '|':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case '&':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case '<':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case '>':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case ' ':
                                if (fl_dquote_fn || fl_quote_fn) {
                                    cur_str.push_back(str[i]);
                                } else {
                                    fl_break = true;
                                }
                                break;
                            case '\\':
                                ++i;
                                if (i >= str_len) {
                                    std::cerr << "Syntax error next to an unexpected marker \"\\\"" << '\n';
                                    exit(1);
                                }
                                cur_str.push_back(str[i]);
                                break;
                            default:
                                cur_str.push_back(str[i]);
                                break;
                        }
                        if (fl_break) {
                            break;
                        }
                        ++i;
                    }
                    if (cur_str.empty()) {
                        if (fl_tmp) {
                            std::cerr << "Syntax error next to an unexpected marker \">>\"" << '\n';
                        } else {
                            std::cerr << "Syntax error next to an unexpected marker \">\"" << '\n';
                        }
                        exit(1);
                    }
                    cur_command.AddPipeOut(cur_str);
                    cur_str.clear();
                    --i;
                }
                break;
            default:
                cur_str.push_back(str[i]);
                break;
        }
    }
    if (fl_dquote || fl_quote) {
        std::cerr << "Invalid args" << '\n';
        exit(1);
    }
    if (!cur_str.empty()) {
        cur_command.AddArg(cur_str);
        cur_str.clear();
    }
    if (!cur_command.IsEmpty()) {
        cur_block.AddCommand(cur_command);
        cur_command.Clear();
    }
    if (cur_block.IsEmpty()) {
        std::cerr << "Syntax error" << '\n';
        exit(1);
    }
    cur_block.ChangeConnectionType(0);
    args.push_back(cur_block);
    cur_block.Clear();
    return;
}

int main(int argc, char **argv, char **envv) {
    if (argc < 3 || strcmp(argv[1], "-c")) {
        std::cerr << "Invalid args" << '\n';
        return 1;
    }
    std::vector<CommandBlock> commands;
    Parse(argv[2], commands);
    int status = 0;
    int last_status = 0;
    int nxt_con;
    for (int i = 0; i < commands.size(); ++i) {
        if ((nxt_con == 2 && !status) || (nxt_con == 1 && status)) {
            nxt_con = commands[i].GetCon();
            continue;
        }
        pid_t pid = fork();
        if (pid == 0) {
            int st = commands[i].Run();
            return st;
        } else {
            status = 0;
            waitpid(pid, &status, 0);
            nxt_con = commands[i].GetCon();
        }
    }
    return WEXITSTATUS(status);
}
