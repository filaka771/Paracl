#include <fstream>
#include <string>
#include <memory>
#include <unordered_map>
#include <stdexcept>

#include "../../parser/include/parser.h"

class CodeGen {
public: 
    CodeGen(std::string &file_name, Parser::ParseResult& parse_result)
    : asm_file_(file_name), parse_result_(std::move(parse_result)) {}

private:
    struct function_sign {
        std::size_t param_count;
        std::string function_label;
    };

    std::ofstream asm_file_;
    Parser::ParseResult parse_result_;
    std::unordered_map<std::string, function_sign> function_table_;

    //-----------------Helpers-----------------
    void print_error(const Node& node, std::string error_msg) {
        std::cout << error_msg;
        const auto line = parse_result_.token_list[node.span_.begin].line;
        const auto column = parse_result_.token_list[node.span_.begin].column; 
        std::cout << " " << line << ":" << column;
        
    }

    //-----------------Functions-----------------
    void visit_program(const Program& program) {
        for (const auto& child : program.children_nodes) {
            const auto* function_def = static_cast<FunctionDef*>(child.get());
            visit_function(*function_def);

        }
    }

    void visit_function(const FunctionDef& function_def) {
        const std::string function_id = function_def.get_function_id();
        // Add new function declaration in the function table
        if(function_table_.find(function_id) == function_table_.end()) {
            if(function_id == "main")
                function_table_[function_id].function_label = "start_";
            else {
                function_table_[function_id].function_label = "_int_" + function_id;
            }
            //
            visit_arg_list(function_def);
        }
        else {
            const std::string err_msg = "Multiple definitions of function "
                                        + function_def.get_function_id();
            print_error(function_def, err_msg);
            throw std::runtime_error("Multiple definition!");
        }
    }

    void visit_arg_list(const FunctionDef& function_def) {
        const std::string function_id = function_def.get_function_id();
        const auto* param_list =
            static_cast<ParameterList*>(function_def.children_nodes[0].get());

        const std::size_t param_count = param_list->children_nodes.size();
        function_table_[function_id].param_count = param_count;
    }


};
