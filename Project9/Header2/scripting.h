#pragma once


#include "../utils/string/include.h"

// this is a proper scripting engine.

// Checklist
// parse tokens[x]
// create types[x]
// create objects[x]
// create expression tree[x]
// compile expressions[x] 
// compile functions[ TODO ]
// write entry point [ TODO ]
// comments
// semantic validation: assignment ?
// multiple files ?


// I think this should be able to run.
// I just need to resolve function addresses in the code.
// and compile all functions together into one program.
// ....


// remove sequences and all related code.
// this will make it run again...

#include <iostream>
#include <vector>
#include <stack>
#include <cstdint>
#include <cstring> // For memcpy
#include <cstdarg>
#include <fstream>
#include <regex>
#include <list>
#include <sstream>

#undef printf


struct object;
struct function_reference;
namespace v2
{
    struct scope;
    struct token;
}


enum primitives
{
    primitive_none,
    primitive_int8,
    primitive_int16,
    primitive_int32,
    primitive_int64,

    primitive_uint8,
    primitive_uint16,
    primitive_uint32,
    primitive_uint64,

    primitive_float32,
    primitive_float64,
    primitive_count
};

enum opcode {
    opcode_none,
    // opcode_init, // push starting memory (precalculated constants,string literals, etc)

    // memory
    opcode_copy_gg,   // src dest size
    opcode_copy_gl,   // src dest size
    opcode_copy_lg,   // src dest size
    opcode_copy_ll,   // src dest size
    opcode_push,  // pushes a stack frame
    opcode_pop,   // pops a stack frame
    opcode_push_value_g,   // pushes a value onto the current stack frame
    opcode_push_value_l,   // pushes a value onto the current stack frame
    opcode_push_value_c,   // pushes a value onto the current stack frame
    opcode_pop_value,    // pops a value from the current stack frame



    // arithmetics (int)
    opcode_set_int_gg, // a = b
    opcode_set_int_gl,
    opcode_set_int_gc,
    opcode_set_int_lg,
    opcode_set_int_ll,
    opcode_set_int_lc,

    opcode_add_int_ggg, // a = b + c
    opcode_add_int_ggl,
    opcode_add_int_ggc,
    opcode_add_int_glg,
    opcode_add_int_gll,
    opcode_add_int_glc,
    opcode_add_int_gcg,
    opcode_add_int_gcl,
    opcode_add_int_lgg,
    opcode_add_int_lgl,
    opcode_add_int_lgc,
    opcode_add_int_llg,
    opcode_add_int_lll,
    opcode_add_int_llc,
    opcode_add_int_lcg,
    opcode_add_int_lcl,

    opcode_sub_int_ggg, // a = b - c
    opcode_sub_int_ggl,
    opcode_sub_int_ggc,
    opcode_sub_int_glg,
    opcode_sub_int_gll,
    opcode_sub_int_glc,
    opcode_sub_int_gcg,
    opcode_sub_int_gcl,
    opcode_sub_int_lgg,
    opcode_sub_int_lgl,
    opcode_sub_int_lgc,
    opcode_sub_int_llg,
    opcode_sub_int_lll,
    opcode_sub_int_llc,
    opcode_sub_int_lcg,
    opcode_sub_int_lcl,

    opcode_mul_int_ggg, // a = b * c
    opcode_mul_int_ggl,
    opcode_mul_int_ggc,
    opcode_mul_int_glg,
    opcode_mul_int_gll,
    opcode_mul_int_glc,
    opcode_mul_int_gcg,
    opcode_mul_int_gcl,
    opcode_mul_int_lgg,
    opcode_mul_int_lgl,
    opcode_mul_int_lgc,
    opcode_mul_int_llg,
    opcode_mul_int_lll,
    opcode_mul_int_llc,
    opcode_mul_int_lcg,
    opcode_mul_int_lcl,

    opcode_div_int_ggg, // a = b / c
    opcode_div_int_ggl,
    opcode_div_int_ggc,
    opcode_div_int_glg,
    opcode_div_int_gll,
    opcode_div_int_glc,
    opcode_div_int_gcg,
    opcode_div_int_gcl,
    opcode_div_int_lgg,
    opcode_div_int_lgl,
    opcode_div_int_lgc,
    opcode_div_int_llg,
    opcode_div_int_lll,
    opcode_div_int_llc,
    opcode_div_int_lcg,
    opcode_div_int_lcl,
    
    
    // arithmetics (float)
    opcode_set_float_gg, // a = b
    opcode_set_float_gl,
    opcode_set_float_gc,
    opcode_set_float_lg,
    opcode_set_float_ll,
    opcode_set_float_lc,

    opcode_add_float_ggg, // a = b + c
    opcode_add_float_ggl,
    opcode_add_float_ggc,
    opcode_add_float_glg,
    opcode_add_float_gll,
    opcode_add_float_glc,
    opcode_add_float_gcg,
    opcode_add_float_gcl,
    opcode_add_float_lgg,
    opcode_add_float_lgl,
    opcode_add_float_lgc,
    opcode_add_float_llg,
    opcode_add_float_lll,
    opcode_add_float_llc,
    opcode_add_float_lcg,
    opcode_add_float_lcl,

    opcode_sub_float_ggg, // a = b - c
    opcode_sub_float_ggl,
    opcode_sub_float_ggc,
    opcode_sub_float_glg,
    opcode_sub_float_gll,
    opcode_sub_float_glc,
    opcode_sub_float_gcg,
    opcode_sub_float_gcl,
    opcode_sub_float_lgg,
    opcode_sub_float_lgl,
    opcode_sub_float_lgc,
    opcode_sub_float_llg,
    opcode_sub_float_lll,
    opcode_sub_float_llc,
    opcode_sub_float_lcg,
    opcode_sub_float_lcl,

    opcode_mul_float_ggg, // a = b * c
    opcode_mul_float_ggl,
    opcode_mul_float_ggc,
    opcode_mul_float_glg,
    opcode_mul_float_gll,
    opcode_mul_float_glc,
    opcode_mul_float_gcg,
    opcode_mul_float_gcl,
    opcode_mul_float_lgg,
    opcode_mul_float_lgl,
    opcode_mul_float_lgc,
    opcode_mul_float_llg,
    opcode_mul_float_lll,
    opcode_mul_float_llc,
    opcode_mul_float_lcg,
    opcode_mul_float_lcl,

    opcode_div_float_ggg, // a = b / c
    opcode_div_float_ggl,
    opcode_div_float_ggc,
    opcode_div_float_glg,
    opcode_div_float_gll,
    opcode_div_float_glc,
    opcode_div_float_gcg,
    opcode_div_float_gcl,
    opcode_div_float_lgg,
    opcode_div_float_lgl,
    opcode_div_float_lgc,
    opcode_div_float_llg,
    opcode_div_float_lll,
    opcode_div_float_llc,
    opcode_div_float_lcg,
    opcode_div_float_lcl,
    
    
   


    // compare
    opcode_cmp_eq,  // a == b?
    opcode_cmp_neq, // a != b?
    opcode_cmp_geq, // a >= b?
    opcode_cmp_leq, // a <= b?

    // control flow
    // opcode_goto,  
    opcode_jmp,      // addr
    opcode_jmp_if_0, // addr
    opcode_jmp_if_1, // addr
    opcode_call,     // bytecode address, memory address of args, return address
    opcode_return,
    opcode_return_g,
    opcode_return_l,
    opcode_return_c,

    // state
    opcode_save, // save sp and ip in preparation for a function call
    opcode_load, // load sp and ip after a function call


    opcode_halt,



    // debugging
    opcode_print_int,
    opcode_print_flt,
    // opcode_print_str,

    opcode_count
};


enum { invalid = ~std::uint64_t(0) };

enum storage
{
    storage_none,      // 5.storage == none
    storage_layout,    // type.storage == layout
    storage_static,    // root.myint.storage == static
    storage_dynamic,   // func.myint.storage == dynamic
    storage_temporary, // (x + y).storage == temporary
};




inline bool is_identifier(const std::string& str)
{
    if (str.empty()) return false;

    if (not std::isalpha(str.front())) return false;
    for (auto x : str)
    {
        if (not std::isalnum(x)) return false;
    }
    return true;
}
inline uint64_t primitive_size(primitives prim)
{

    switch (prim)
    {
    case primitive_int8: 
    case primitive_uint8:
        return 1;

    case primitive_int16:
    case primitive_uint16:
        return 2;

    case primitive_int32: 
    case primitive_uint32:
    case primitive_float32:
        return 4;

    case primitive_int64: 
    case primitive_uint64:
    case primitive_float64:
        return 8;

    default:
        return 0;
        break;
    }

    return 0;
}


struct function;

// an expression results in an output value.
// conditions are expressions, not if statements. which are statements...
struct expression
{
    expression* parent = nullptr;

    expression* lhs = nullptr; 
    std::string operation = ""; 
    expression* rhs = nullptr; 

    std::shared_ptr<object> result = std::make_shared<object>();

    

    void print()
    {
        if (lhs) lhs->print();
        
        printf(" %s ", operation.c_str());

        if (rhs) rhs->print();
    }
    void print2(uint64_t Depth = 0)
    {
        if (lhs) lhs->print2(Depth + 1);

        printf("%*s",Depth * 4,"");
        printf("%s\n",operation.c_str());
        
        if (rhs) rhs->print2(Depth + 1);
    }

    // displace nodes based on a function.
    // see transform_parens_L
    // see transform_parens_R
    bool transform(void(*transformer)(expression*))
    {
        transformer(this);

        if (lhs) lhs->transform(transformer);
        if (rhs) rhs->transform(transformer);
        return true;
    }

    bool ExpressionContainsParens()
    {
        if (operation == "(" || operation == ")")
        {
            return true;
        }
        if (lhs && lhs->ExpressionContainsParens()) return true;
        if (rhs && rhs->ExpressionContainsParens()) return true;
        return false;
    }

    bool compile_into(function* inout, v2::scope* containing_scope);

    bool is_arithmetic()
    {
        return
            operation == "+" ||
            operation == "-" ||
            operation == "*" ||
            operation == "/" ||
            false;
    }
    bool is_func_call()
    {
        return false;
    }
    bool is_keyword()
    {
        return operation == "return";
    }


    // https://en.cppreference.com/w/c/language/operator_precedence
    int precedence()
    {
        if (operation == "return") return 20;
        
        if (operation == ",") return 17;

        // assignment
        if (operation == "=") return 14;
        
        // arithmetic
        if (operation == "+") return 4;
        if (operation == "-") return 4;
        if (operation == "*") return 3;
        if (operation == "/") return 3;
        
        // comparison
        if (operation == ">") return 6;
        if (operation == "<") return 6;


        // function call
        if (operation == "(" || operation == ")") return 19;
        // array access
        if (operation == "[" || operation == "]") return 0;
        // member access
        if (operation == ".") return 1;


        // no-op
        if (is_identifier(operation))
        {
            return 0;
        }
        
        return 0; // root token/literal
    }


    bool right_associative()
    {
        if (operation == "=") return true;
        if (operation == "(") return true;
        if (operation == ")") return true;

        return false;
    }
};


// todo: compile statements
// each statement needs a type (if,else,for,switch)
// this results in different code being emitted for the entry and exit of the statement...
// this also means that statements need more than just one root.
// i.e a vector of expressions is a statement.
class statement
{
    expression* root = nullptr;
    std::string type = "";
public:
    
    static void demo()
    {
        statement Statement1;
        Statement1.add("x");
        Statement1.add("=");
        Statement1.add("10");
        Statement1.add("+");
        Statement1.add("2");
        Statement1.add("*");
        Statement1.add("3");
        Statement1.add("+");
        Statement1.add("4");

        statement Statement2;
        Statement2.add("float");
        Statement2.add("x");
        Statement2.add("=");
        Statement2.add("5");

        Statement1.print();
        Statement2.print();
    }


    void print()
    {
        printf("\n------------------\n");
        if (root) root->print();
        printf("\n------------------\n");
    }

    void print_line()
    {
        if (root) 
        {
            root->print();
            printf("\n");
        }
        
    }

   
    // i also want to add (), []
    bool add(expression* add)
    {
        if (root == nullptr)
        {
            root = add;
            return true;
        }

        // proceed down the chain until comparing node has a lower precedence.

        // if i reach the end, i get placed on the right

        // if i dont reach the end, i take the place of the right hand node, which becomes my left hand node

        expression* last = root;
        expression* node = root;
        while (node && node->precedence() > add->precedence())
        {
            last = node;
            node = node->rhs;
        }


        if (node)
        {
            // i take the place of node
            add->parent = node->parent;
            if (add->parent)
            {
                add->parent->rhs = add;
            }

            // node becomes my lhs
            node->parent = add;
            add->lhs = node;
        }
        else
        {
            last->rhs = add;
            add->parent = last;
        }

        while (root->parent) root = root->parent;
        return root;
    }
    bool add(const std::string& token)
    {
        expression* add = new expression();
        add->operation = token;

        return this->add(add);
    }
    expression* get()
    {
        return root;
    }
    bool is_empty()
    {
        return root == nullptr;
    }


    bool compile_into(function* inout, v2::scope* containing_scope);


    expression* find(bool(*callback)(expression*))
    {
        // compute for all nodes 
        return find(callback, root);
    }
private:
    expression* find(bool(*callback)(expression*), expression* from)
    {
        if (callback(from)) return from;
        
        if (from->lhs)
        {
            expression* found = find(callback, from->lhs);
            if (found) return found;
        }
        if (from->rhs)
        {
            expression* found = find(callback, from->rhs);
            if (found) return found;
        }
    }

};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct object
{
    std::string _name = "<unnamed>";
    std::string _type = "<untyped>";
    storage _storage = storage_none;

    uint64_t _local_address = 0;
    uint64_t _global_address = 0;
    uint64_t _layout_address = 0;

    v2::scope* _resolved_type = nullptr;
    v2::scope* containing_scope = nullptr;
};
struct code
{
    std::vector<v2::token> _tokens;
    std::vector<statement> _intermediate;
    std::vector<uint64_t> _bytecode;
};
struct function
{
    std::string _name = "<unnamed>";
    std::string _return_type;
    std::vector<object> _args;
    std::vector<object> _locals;

    v2::scope* _resolved_return_type = nullptr;

    code _code;
    
    uint64_t _work_size = 0; // temporaries. each temporary gets its own address.
    uint64_t _stack_size = 0; // object sum
    uint64_t _global_address = 0;

    void* containing_scope = nullptr;

    object* find_object(const std::string& name)
    {
        for (auto& x : _args) if (x._name == name) return &x;
        for (auto& x : _locals) if (x._name == name) return &x;
        return nullptr;
    }


private:
    struct Temporary
    {
        v2::scope* _type = nullptr;
        uint64_t _local_address = 0;
    };

    std::vector<Temporary> _temporaries;
    uint64_t create_temporary(uint64_t Size);
public:
    uint64_t create_temporary(v2::scope* Type);

    void pop_temporary(); // pops the top temporary off the stack. this is used to align arguments for a function call.
};



namespace v2
{
    struct location
    {
        std::string filename;

        uint64_t line = 0;
        uint64_t character = 0;

        void increment(char Char)
        {
            if (Char == '\n')
            {
                ++line;
                character = 0;
            }
            else
            {
                ++character;
            }
        }
    };
    enum token_type
    {
        token_alpha,
        token_number,
        token_symbol,
        token_ws,
    };
    struct token
    {
        location begin;
        location end;
        token_type type;
        std::string _text;

        std::string text() const
        {
            return _text;
        }
    };
    struct scope
    {
        storage memory_policy = storage_none;
        std::string name = "<unnamed-scope>";

        uint64_t index = invalid;
        std::shared_ptr<scope> parent;
        std::vector<std::shared_ptr<scope>> children;


        std::vector<object> locals;
        std::vector<function> functions; // a scope becomes callable if it contains unnamed functions. idk if this is good.

        // the stack size of a type is the sum of the size of its members
        // the stack size of anything else is 0
        // even a function
        uint64_t stack_size = 0;

        // used to compile expressions...
        primitives alias_for = primitive_none;

        object* find_object(const std::string& name)
        {
            for (auto& obj : locals) if (obj._name == name) return &obj;
            return nullptr;
        }
        function* find_function(const std::string& name)
        {
            for (auto& fun : functions) if (fun._name == name) return &fun;
            return nullptr;
        }
      
        std::shared_ptr<scope> find_matching_type(const std::string& name)
        {
            // search children
            for (auto x : children) if (x->name == name) return x;

            // search parent
            if (parent)
            {
                return parent->find_matching_type(name);
            }
            return nullptr;
        }
        std::shared_ptr<scope> find_primitive_type(primitives prim)
        {
            // search children
            for (auto x : children) if (x->alias_for == prim) return x;

            // search parent
            if (parent)
            {
                return parent->find_primitive_type(prim);
            }
            return nullptr;
        }
        function* best_match_function(const std::string& name, const std::vector<std::string>& args)
        {
            for (auto& func : functions)
            {
                if (func._args.size() != args.size()) continue;
                if (func._name != name) continue;

                
                // match the args
                for (int i = 0; i < func._args.size(); ++i)
                {
                    object& obj = func._args[i];

                    auto obj_type = find_matching_type(obj._type);
                    auto arg_type = find_matching_type(args[i]);

                    if (obj_type.get() == arg_type.get())
                    {
                        return &func;
                    }
                }
                
            }
            if (parent) return parent->best_match_function(name, args);
            return nullptr;
        }

        object* find_type_member(const std::string& membername)
        {
            for (auto& x : locals)
            {
                if (x._name == membername && x._storage == storage_layout)
                    return &x;
            }
            return nullptr;
        }

        static std::shared_ptr<scope> type(const std::string _name, std::vector<std::shared_ptr<scope>>& _context)
        {
            scope ret;
            ret.name = _name;
            ret.index = _context.size();
            ret.memory_policy = storage_layout;

            return _context.emplace_back(std::make_shared<scope>(ret));
        }
        static std::shared_ptr<scope> package(const std::string& _name, std::vector<std::shared_ptr<scope>>& _context)
        {
            scope ret;
            ret.name = _name;
            ret.index = _context.size();
            ret.memory_policy = storage_static;

            return _context.emplace_back(std::make_shared<scope>(ret));
        }
    };


    class program
    {
        std::vector<std::string> _read_files;
        typedef std::vector<token> token_stream;

        std::shared_ptr<scope> _root = std::make_shared<scope>();
        std::shared_ptr<scope> current_scope_ptr = _root;

        // used to assign a stack address to each global
        uint64_t address_ptr_obj = 0;
        // used to assign a bytecode address to each function
        uint64_t address_ptr_fn = 0;
        std::vector<uint64_t> BYTECODE;


        std::shared_ptr<scope> current_scope()
        {
            return current_scope_ptr;
        }

        std::vector<uint64_t> initial_bytecode()
        {
            std::list<function*> FuncList;
            collect_all_funcs(FuncList);

            std::vector<object*> GlobalsList;
            collect_static_objects(GlobalsList);

            size_t GlobalsSize = 0;
            size_t MainAddress = 0;
            for (auto& X : GlobalsList) GlobalsSize += X->_resolved_type->stack_size;

            for (auto& func : FuncList)
            {
                // give function an address. already done?
                if (func->_name == "main")
                {
                    MainAddress = func->_global_address;
                    break;
                }
            }

            return
            {
                // create globals
               opcode_push, GlobalsSize,

               // goto main
               opcode_jmp, MainAddress
            };
        }

    public:

        program()
        {
            _root = std::make_shared<scope>();
            current_scope_ptr = _root;

            _root->memory_policy = storage_static;

        }
        bool parse_file(const std::string& _filename)
        {
            if (file_was_already_parsed(_filename))
                return true;

            
            token_stream filedata;
            read_file(_filename, filedata);


            process_file_data(filedata);

            mark_file_as_read(_filename);
            return true;
        }
        bool define_intrinsic(const std::string& _name, primitives alias_for)
        {
            auto Type = scope::type(_name, _root->children);
            Type->stack_size = primitive_size(alias_for);
            Type->parent = _root;
            Type->alias_for = alias_for;

            return true;
        }






    private:
        bool file_was_already_parsed(const std::string& _filename)
        {
            for (auto& x : _read_files) if (x == _filename) return true;
            return false;
        }
        bool read_file(const std::string& _filename, token_stream& _out)
        {
            assert_file_exists(_filename);

            std::string filedata;
            read_file_string(_filename, filedata);
            tokenise_string(filedata, _out);
            return true;
        }
        bool process_file_data(token_stream& _filedata)
        {
            printf("processing file data : %u tokens\n",_filedata.size());


            printf("---------------------------------------------------\n");
            printf("reading class layouts\n");
            printf("---------------------------------------------------\n");
            read_data_types(_filedata);
            

            printf("---------------------------------------------------\n");
            printf("assigning global object addresses\n");
            // also assigns resolved_type to each global
            printf("---------------------------------------------------\n");
            read_data_objects(_filedata);
            
            
            printf("---------------------------------------------------\n");
            printf("compiling functions\n");
            printf("---------------------------------------------------\n");
            read_data_functions(_filedata);
            

            printf("---------------------------------------------------\n");
            printf("linking functions\n");
            printf("---------------------------------------------------\n");
            link_data_functions();

            printf("---------------------------------------------------\n");
            printf("done.\n");
            printf("---------------------------------------------------\n");
            print_program();

            return true;
        }
        bool mark_file_as_read(const std::string& _filename)
        {
            _read_files.push_back(_filename);
            return true;
        }
    private:
        // read_file
        bool assert_file_exists(const std::string& _filename)
        {
            return false;
        }
        bool read_file_string(const std::string& _filename, std::string& _filedata)
        {
            // Open the file in binary mode to avoid newline translation issues
            std::ifstream file(_filename, std::ios::binary);
      
            // didnt exist
            if (!file) return false;


            // Use a stringstream to read the file into a string
            std::ostringstream contents;
            contents << file.rdbuf();
            file.close();

            _filedata = contents.str();
            return true;
        }


        // NOTE: for some reason i need to add whitespace at the end of the file
        // otherwise it doesnt pick up the last characters...
        bool tokenise_string(const std::string& _filedata, token_stream& _out)
        {

            // alphabet, number, symbol, whitespace
            // alphabet, number, symbol, whitespace
            // alphabet, number, symbol, whitespace
            // alphabet, number, symbol, whitespace
            bool match_table[4][4]
            {
                // alphabet
                {true, true, false, false},
                // number
                {true, true, false, false},
                // symbol
                {false, false, false, false},
                // whitespace
                {false, false, false, true},
            };
            auto get_toktype = [](char x)
                {
                    if (std::isalpha(x)) return token_alpha;
                    if (std::isalnum(x)) return token_number;
                    if (std::isspace(x)) return token_ws;
                    return token_symbol;
                };



            token Token;
            Token.type = token_ws;
            
            for (auto x : _filedata)
            {
                auto Type = get_toktype(x);

                if (match_table[Token.type][Type])
                {
                    Token._text += x;
                }
                else
                {
                    // make new token
                    if (not Token._text.empty())
                    {
                        // dont save whitespace
                        if (not std::isspace(Token._text[0]))
                        {
                            _out.push_back(Token);
                        }
                        Token._text.clear();
                    }

                    Token._text += x;
                    Token.type = Type;
                    Token.begin = Token.end;
                }

                Token.end.increment(x);
            }


            return true;
        }
        // process_file_data
        bool read_data_types(const token_stream& _tokens)
        {
            read_type_infos(_tokens);
            calc_type_sizes(_tokens);
            resolve_members();
            return true;
        }
        bool read_data_objects(const token_stream& _tokens)
        {
            // collect objects in namespace scope
            std::vector<object*> static_objects;
            collect_static_objects(static_objects);
            
            for (auto* obj : static_objects)
            {
                // printf("assigning %s\n", obj->_name.c_str());
                // an object should know its containing scope
                scope* _Containing_scope = obj->containing_scope;

                if (_Containing_scope == nullptr)
                {
                    __debugbreak();
                }
                scope* resolved_type = _Containing_scope->find_matching_type(obj->_type).get();
                obj->_resolved_type = resolved_type;

                uint64_t size = resolved_type->stack_size;

                if (size == 0)
                {
                    __debugbreak();
                    return false;
                }

                obj->_global_address = address_ptr_obj;
                obj->_local_address = 0;
                address_ptr_obj += size;

                printf("[%5u] '%-10s' '%s'\n", address_ptr_obj, obj->_type.c_str(), obj->_name.c_str());
            }

            return false;
        }
        bool read_data_functions(const token_stream& _tokens)
        {   
            std::list<function*> funcs;

            collect_all_funcs(funcs);


            for (auto* func : funcs)
            {
                resolve_return_type(func);
            }

            for (auto* func : funcs)
            {
                printf("working on function '%s'\n", func->_name.c_str());
                // process tokens into an ast
                read_func_infos(func);

                // calculate required info
                read_func_locals(func);
                calc_func_stack_size(func);


                // compile function
                compile_function_statements(func);

                // integrate function into program
                // todo: copy bytecode to program (in linking step)
                assign_func_adress(func);

                printf("-----------------\n");
            }
            
            for (auto* func : funcs)
            {
                func->_code._bytecode.clear();
                // compile function but this time all functions have an address
                compile_function_statements(func);
            }

            // there is no need to resolve invalid addresses.
            // each function was recompiled with all having valid addresses already
           

            // copy function data to program bytecode now...

            return false;
        }
        bool link_data_functions()
        {
            BYTECODE = initial_bytecode();

            // copy code from functions to bytecode
            // this has to be done in the same order as compilation
            // 
            std::list<function*> Funcs;
            collect_all_funcs(Funcs);

            // done!
            for (auto* fn : Funcs)
            {
                BYTECODE.insert(BYTECODE.end(), fn->_code._bytecode.begin(), fn->_code._bytecode.end());
            }

            return true;
        }

        bool print_program()
        {
            uint64_t counter = 0;
            std::string str = "output";
            
            str += ".compiled";

            std::ofstream OUT(str);
           for (auto& x : BYTECODE)
           {
               OUT << x << "\n";
           }
            return true;
        }
    private:
        
        // read_data_types
        bool read_type_infos(const token_stream& _tokens)
        {
            int Last = -1;
            int i;
            for (i = 0; i < _tokens.size() && i != Last; )
            {
                Last = i;

                // i have to do these here
                if (read_class_from_tokens(i, _tokens)) continue;
                else if (exit_scope_from_tokens(i, _tokens)) continue;
                else if (read_object_from_tokens(i, _tokens)) continue;
                else if (read_func_from_tokens(i, _tokens)) continue;
                else
                {
                    printf("invalid token found!\n");
                    printf(">\t'%s'\n",_tokens[i].text().c_str());
                    break;
                }
            }

            if (i != _tokens.size())
            {
                printf("failed to read type infos: read %i tokens\n", i);
            }

            return false;
        }
        bool calc_type_sizes(const token_stream& _tokens)
        {
            std::list<std::shared_ptr<scope>> TypeList;

            // order types by their dependancies
            collect_all_types(TypeList);


            int LastCount = -1;
            while(not TypeList.empty() && LastCount != TypeList.size())
            {
                LastCount = TypeList.size();

                for (auto& x : TypeList)
                {
                    if (calculate_type_size(x))
                    {
                        printf("calculated size of '%s' = %u\n", x->name.c_str(), x->stack_size);
                        remove_from_list(TypeList, x);
                        break;
                    }
                }
            }


            return TypeList.empty();
        }
        bool resolve_members()
        {
            std::list<std::shared_ptr<scope>> TypeList;
            collect_all_types(TypeList);

            // for each type
                // for each member
                // resolve member type 
            for (auto& x : TypeList)
            {
                for (auto& obj : x->locals)
                {
                    if (obj._storage == storage_layout)
                    {
                        obj._resolved_type = obj.containing_scope->find_matching_type(obj._type).get();
                    }
                }
            }
            return true;
        }
        

        // read_data_objects
        bool collect_static_objects(std::vector<object*>& out)
        {
            return collect_static_objects_impl(out, _root);
        }
        bool collect_static_objects_impl(std::vector<object*>& out, const std::shared_ptr<scope>& context)
        {
            for (auto& obj : context->locals)
            {
                if (obj._storage == storage_static)
                {
                    out.push_back(&obj);
                }
            }
            for (auto& x : context->children)
            {
                collect_static_objects_impl(out, x);
            }
            
            return true;
        }

        // read_data_functions
        bool resolve_return_type(function* func)
        {
            func->_resolved_return_type = ((v2::scope*)func->containing_scope)->find_matching_type(func->_return_type).get();
            return true;
        }
        bool read_func_infos(function* func)
        {
            printf("building code graph\n");

            // builds the ast for each function
            auto& tokens = func->_code._tokens;
            auto* context = (scope*)func->containing_scope;

            auto& locals = func->_locals;
            
            // remove { and }
            tokens.assign(tokens.begin() + 1, tokens.end() - 1);


            
            statement curr;
            // if current token isnt }
            // just read to ';'
            // then analyse the pattern of the tree for what to do next.

            // todo: read sequences of statements :
                // if
                // else
                // while
            // these start with a keyword, have an expression, 
            // then have a body of contents.
            // the function should hold a list of sequences, not statements.
            for (int i = 0; i < tokens.size(); ++i)
            {
                if (tokens[i].text() == ";")
                {
                    if (not curr.is_empty())
                    {
                        func->_code._intermediate.push_back(curr);
                        curr = {};
                    }
                }
                else
                {
                    curr.add(tokens[i].text());
                }
            }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // debug
            /* 
            for (auto& x : func->_code._intermediate)
             {
                 x.print_line();
             }
             */

            return false;
        }
        bool read_func_locals(function* func)
        {
            printf("assigning addresses to function locals\n");
            
            // records the locals defined within the function
            for (auto& Statement : func->_code._intermediate)
            {
///////////////////////////////////////////////////////////////////////////////////////////////////////////
                
///////////////////////////////////////////////////////////////////////////////////////////////////////////
                    expression* e = Statement.find([](expression* node)
                        {
                            // this and lhs are identifiers
                            bool b1 = is_identifier(node->operation);
                            bool b2 = node->lhs != nullptr && is_identifier(node->lhs->operation);

                            // lhs has no children
                            bool b3 = b2 && node->lhs->lhs == nullptr;
                            bool b4 = b2 && node->lhs->rhs == nullptr;
                            // there is no rhs
                            bool b5 = node->rhs == nullptr;

                            return b1 and b2 and b3 and b4 and b5;
                        });
                    if (e && e->lhs)
                    {
                        object obj;
                        obj._name = e->operation;
                        obj._type = e->lhs->operation;
                        obj._local_address = 0;
                        obj._global_address = 0;
                        obj._storage = storage_dynamic;
                        obj.containing_scope = (v2::scope*)func->containing_scope;

                        func->_locals.push_back(obj);
                    }
                
            }



            uint64_t LocalAddress = 0;
            scope* Scope = (scope*)func->containing_scope;
            
            for (auto& obj : func->_args)
            {
                obj._local_address = LocalAddress;
                obj._resolved_type = obj.containing_scope->find_matching_type(obj._type).get();
                
                printf("\t[%5u] [arg] %s %s\n",
                    obj._local_address,
                    obj._resolved_type->name.c_str(),
                    obj._name.c_str()
                );


                auto Type = Scope->find_matching_type(obj._type);
                LocalAddress += Type->stack_size;
            }
            for (auto& obj : func->_locals)
            {
                obj._local_address = LocalAddress;
                obj._resolved_type = obj.containing_scope->find_matching_type(obj._type).get();

                printf("\t[%5u] [local] %s %s\n",
                    obj._local_address,
                    obj._resolved_type->name.c_str(),
                    obj._name.c_str()
                );


                auto Type = Scope->find_matching_type(obj._type);
                LocalAddress += Type->stack_size;
            }

            return false;
        }
        bool calc_func_stack_size(function* func)
        {
            printf("calculating minimum function stack size: ");
            // counts the minimum amount of memory required for the function.
            // it can still push/pop values later during execution though.


            func->_stack_size = 0;

            for (auto& obj : func->_locals)
            {
                // calculate size
                uint64_t size = obj._resolved_type->stack_size;

                // update required stack space
                func->_stack_size += size;
            }

            printf("= %u\n", func->_stack_size);
            return true;
        }
        bool compile_function_statements(function* func) 
        {
            printf("compiling %u function statements\n",func->_code._intermediate.size());


            
            // create stack space for locals
            func->_code._bytecode.clear();
            func->_code._bytecode.push_back(opcode_push); 
            func->_code._bytecode.push_back(func->_stack_size + func->_work_size);           
            uint64_t funcsize_index = 1;
            
            // set the stack pointer                                  ;
            // caller saves state                                     ; [sp]
            // caller pushes args                                     ; [sp][args]
            // caller calls(func, retbuffer)                          ; [sp][args][ip][retbuf]
                // saves instruction pointer                          ; [sp][args][ip][retbuf]
                // saves return value ptr                             ; [sp][args][ip][retbuf]
            
            // goes to function                                      
                // push stacksize                                     ; [sp][args][ip][retbuf][STACK]
                // ... some work ...                                 
                // pop  stacksize                                     ; [sp][args][ip][retbuf]
                // returns AddressOfReturnValue                       ; [sp][args]
                    // copies AddressOfReturnValue into [retbuf]
            // goes to caller
            // caller pops args                                       ; [sp]
            // caller restores sp                                     ; 

            // todo: compile statements into bytecode...
            for (auto& x : func->_code._intermediate)
            {
                printf("compiling:\t\t");
                x.print_line();
                compile_function_statement(func, x);
            }

            func->_code._bytecode[funcsize_index] = func->_stack_size + func->_work_size;


            return false; 
        }
      
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
       
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        bool compile_function_statement(function* func, statement& code)
        {
            // this also needs to keep track of the size of the working memory.
            return code.compile_into(
                func, 
                (scope*)func->containing_scope);
        }

    private:
        bool read_class_from_tokens(int& i, const token_stream& _tokens)
        {
            if (_tokens[i].text() == "class")
            {
                // type name
                auto name = _tokens[++i].text();

                // error!
                if (_tokens[++i].text() != "{")
                {
                    __debugbreak();
                }

                // create type definition
                // enter type scope
                create_type(name);
                ++i;

                return true;
            }
            return false;
        }
        bool exit_scope_from_tokens(int& i, const token_stream& _tokens)
        {
            if (_tokens[i].text() == "}")
            {
                current_scope_ptr = current_scope_ptr->parent;
                ++i;
                return true;
            }
            return false;
        }       
        bool read_object_from_tokens(int& i, const token_stream& _tokens)
        {
            // ident ident ;
            // ident ident = ...
            if (_tokens.size() < i + 3) return false;


            // by default, an object uses the memory policy of its containing scope.


            std::string _typename = _tokens[i + 0].text();
            std::string _memberid = _tokens[i + 1].text();
            std::string _endtoken = _tokens[i + 2].text();
            
            if (not is_identifier(_typename)) return false;
            if (not is_identifier(_memberid)) return false;
            if (_endtoken != ";") return false; // objects cant be initialised in their definition.


            // member variable or global
            create_object(_memberid, _typename);
            i += 3;

            return true;
        }
        bool read_func_from_tokens(int& i, const token_stream& _tokens)
        {
            if (_tokens.size() < i + 3) return false;

            std::string _typename = _tokens[i + 0].text();
            std::string _function = _tokens[i + 1].text();
            std::string _endtoken = _tokens[i + 2].text();

            if (not is_identifier(_typename)) return false;
            if (not is_identifier(_function)) return false;
            if (_endtoken != "(") return false; // it has to be an object.

            i += 3;

            // read function args
            std::vector<object> args;
            read_func_args(i, _tokens, args);

            // read function code
            std::vector<token> code;
            read_func_code(i, _tokens, code);


            // create function
            create_function(_function, _typename, args, code);


            return true;
        }
private:
        bool read_func_args(int& i, const token_stream& _tokens, std::vector<object>& out)
        {
            int prevI = -1;

            // handle no args
            if (_tokens[i].text() == ")")
            {
                ++i;
                return true;
            }

            for (i; i < _tokens.size() - 3 && i != prevI; )
            {
                prevI = i;

                std::string type;
                std::string name;
                bool is_comma;
                bool is_paren;

                type = _tokens[i + 0].text();
                name = _tokens[i + 1].text();
                is_comma = _tokens[i + 2].text() == ",";
                is_paren = _tokens[i + 2].text() == ")";

                bool is_good = is_comma || is_paren;

                if (is_identifier(type) && is_identifier(name) && is_good)
                {
                    object obj;
                    obj._type = type;
                    obj._name = name;
                    obj._storage = storage_dynamic;
                    obj._local_address = 0;
                    obj._global_address = 0;
                    obj.containing_scope = current_scope().get();
                    
                    out.push_back(obj);

                    i += 3;
                }
            }

            bool is_paren;
            is_paren = _tokens[i - 1].text() == ")";

            if (is_paren)
            {
                return true;
            }

            return false;
        }
        bool read_func_code(int& i, const token_stream& _tokens, std::vector<token>& out)
        {
            uint64_t _depth = 1;
            int Start = i;

            while (_depth && i < _tokens.size())
            {
                ++i;
                if (_tokens[i].text() == "{") ++_depth;
                if (_tokens[i].text() == "}") --_depth;
            }

            int End = ++i;

            for (Start; Start != End; ++Start)
                out.push_back(_tokens[Start]);

            return true;
        }
    private:
        bool create_type(const std::string& name)
        {
            assert_name_is_unique_in_current_scope(name);


            auto Type = scope::type(name, current_scope()->children);
            Type->parent = current_scope_ptr;
            current_scope_ptr = Type;

            printf("creating type '%s'\n",name.c_str());
            return true;
        }
        bool create_object(const std::string& name, const std::string& type)
        {
            object obj;
            obj._name = name;
            obj._type = type;
            obj._storage = current_scope()->memory_policy;
            obj.containing_scope = current_scope().get();
            current_scope()->locals.push_back(obj);

            printf("creating object '%s' '%s'::'%s'\n", type.c_str(), current_scope()->name.c_str(), name.c_str());
            return true;
        }
        
        bool create_function(const std::string& name, const std::string& _return_type, const std::vector<object>& arg_infos, const std::vector<token>& function_code)
        {
            function func;
            func._name = name;
            func._args = arg_infos;
            func.containing_scope = current_scope().get();
            func._code._tokens = function_code;
            func._return_type = _return_type;
            func._global_address = 0;

            current_scope()->functions.push_back(func);

            // debug printing
            std::string Args;
            for (auto& x : arg_infos) Args += x._type + " " + x._name + ", ";

            if (Args.empty())
            {
                Args = "void";
            }
            else
            {
                Args.pop_back();
                Args.pop_back();
            }


            printf("creating function '%s' '%s'::'%s'(%s)\n", _return_type.c_str(), current_scope()->name.c_str(), name.c_str(), Args.c_str());
            return true;
        }
    private:
        bool assert_name_is_unique_in_current_scope(const std::string& _name)
        {
            for (auto& x : current_scope()->children)
            {
                if (x->name == _name) return false;
            }
            for (auto& x : current_scope()->locals)
            {
                if (x._name == _name) return false;
            }
            return true;
        }
    private:
        bool collect_all_types(std::list<std::shared_ptr<scope>>& out)
        {
            for (auto x : _root->children)
            {
                collect_types_impl(out, x);
            }
            return true;
        }
        bool collect_types_impl(std::list<std::shared_ptr<scope>>& out, std::shared_ptr<scope> curr)
        {
            out.push_back(curr);
            for (auto x : curr->children)
            {
                collect_types_impl(out, x);
            }
            return true;
        }
        bool calculate_type_size(std::shared_ptr<scope> out)
        {
            if (out->memory_policy != storage_layout)
                return false;

            for (auto& x : out->locals)
            {
                if (x._storage != storage_layout) continue;
                auto Type = out->find_matching_type(x._type);

                x._layout_address = out->stack_size;
                x._resolved_type = Type.get();

                out->stack_size += Type->stack_size;
                
                printf("[%5u] %s::%s resolved as %s%s.\n",
                    x._layout_address,
                    out->name.c_str(),
                    x._name.c_str(),
                    (x._resolved_type->alias_for != primitive_none ? "primitive " : ""),
                    x._resolved_type->name.c_str()
                );

                if (Type->stack_size == 0)
                {
                    out->stack_size = 0;
                    return false;
                }
            }
            return true;
        }  
        bool remove_from_list(std::list<std::shared_ptr<scope>>& out, std::shared_ptr<scope> x)
        {
            out.erase(std::find(out.begin(), out.end(), x));
            return true;
        }
    private:
        bool collect_all_funcs(std::list<function*>& out)
        {
            collect_funcs_impl(out, _root);
            return true;
        }
        bool collect_funcs_impl(std::list<function*>& out, std::shared_ptr<scope> curr)
        {
            for (auto& x : curr->functions)
            {
                out.push_back(&x);
            }
            for (auto& x : curr->children)
            {
                collect_funcs_impl(out, x);
            }
            return true;
        }
        bool assign_func_adress(function* func)
        {
            assert_function_has_size(func);

            // a function shares the same space as global objects.
            // this is what makes objects callable.
            // thus i need to use the global stack pointer to assign an address to all functions.
            // then i need to increment the stack pointer.
            // this only works because all objects have already been given an address
            // also because the functions are only given an address after their size is calculated.
            func->_global_address = address_ptr_fn + initial_bytecode().size();
            address_ptr_fn += func->_code._bytecode.size();
            return false;
        }
    private:
        bool assert_function_has_size(function* func)
        {
            return func->_code._bytecode.size() > 0;
        }
    };
}

inline void demo()
{
   v2::program prog;
   
   prog.define_intrinsic("float", primitive_float32);
   prog.define_intrinsic("int", primitive_int64);
   
   prog.parse_file("my_script.txt");
   
}



#if 0
class machine
{
    std::vector<uint64_t> bytecode;
    std::vector<uint64_t> memory;

    uint64_t sp=0;
    uint64_t ip=0;
private:
    template<typename T>
    T& fetch(uint64_t address);

    uint64_t make_global(uint64_t);
public:
    void execute()
    {
        opcode op = (opcode)bytecode[ip];
        uint64_t arg1 = bytecode[ip + 1];
        uint64_t arg2 = bytecode[ip + 2];
        uint64_t arg3 = bytecode[ip + 3];

        switch (op)
        {
        case opcode_none:
            return;


        case opcode_copy_gg:
            std::memcpy(memory.data() + arg1, memory.data() + arg2, arg3);
            break;
        case opcode_copy_gl:
            std::memcpy(memory.data() + arg1, memory.data() + make_global(arg2), arg3);
            break;
        case opcode_copy_lg:
            std::memcpy(memory.data() + make_global(arg1), memory.data() + arg2, arg3);
            break;
        case opcode_copy_ll:
            std::memcpy(memory.data() + make_global(arg1), memory.data() + make_global(arg2), arg3);
            break;
        case opcode_push:
            // save sp in separate stack
            // push new stack
            break;
        case opcode_pop:
            // pop sp from stack
            // restore stack
            break;
        case opcode_push_value_g: // push value onto current stack. doesnt change stack pointer. doesnt change existing addresses.
            break;
        case opcode_push_value_l:
            break;
        case opcode_push_value_c:
            break;
        case opcode_pop_value:
            break;
        case opcode_set_int_gg:
            memory[arg1] = memory[arg2];
            break;
        case opcode_set_int_gl:
            memory[arg1] = memory[make_global(arg2)];
            break;
        case opcode_set_int_gc:
            memory[arg1] = arg2;
            break;
        case opcode_set_int_lg:
            memory[make_global(arg1)] = memory[arg2];
            break;
        case opcode_set_int_ll:
            memory[make_global(arg1)] = memory[make_global(arg2)];
            break;
        case opcode_set_int_lc:
            memory[make_global(arg1)] = arg2;
            break;
        case opcode_add_int_ggg:
            memory[arg1] = memory[arg2] + memory[arg3];
            break;
        case opcode_add_int_ggl:
            memory[arg1] = memory[arg2] + memory[make_global(arg3)];
            break;
        case opcode_add_int_ggc:
            memory[arg1] = memory[arg2] + arg3;
            break;
        case opcode_add_int_glg:
            break;
        case opcode_add_int_gll:
            break;
        case opcode_add_int_glc:
            break;
        case opcode_add_int_gcg:
            break;
        case opcode_add_int_gcl:
            break;
        case opcode_add_int_lgg:
            break;
        case opcode_add_int_lgl:
            break;
        case opcode_add_int_lgc:
            break;
        case opcode_add_int_llg:
            break;
        case opcode_add_int_lll:
            break;
        case opcode_add_int_llc:
            break;
        case opcode_add_int_lcg:
            break;
        case opcode_add_int_lcl:
            break;
        case opcode_sub_int_ggg:
            break;
        case opcode_sub_int_ggl:
            break;
        case opcode_sub_int_ggc:
            break;
        case opcode_sub_int_glg:
            break;
        case opcode_sub_int_gll:
            break;
        case opcode_sub_int_glc:
            break;
        case opcode_sub_int_gcg:
            break;
        case opcode_sub_int_gcl:
            break;
        case opcode_sub_int_lgg:
            break;
        case opcode_sub_int_lgl:
            break;
        case opcode_sub_int_lgc:
            break;
        case opcode_sub_int_llg:
            break;
        case opcode_sub_int_lll:
            break;
        case opcode_sub_int_llc:
            break;
        case opcode_sub_int_lcg:
            break;
        case opcode_sub_int_lcl:
            break;
        case opcode_mul_int_ggg:
            break;
        case opcode_mul_int_ggl:
            break;
        case opcode_mul_int_ggc:
            break;
        case opcode_mul_int_glg:
            break;
        case opcode_mul_int_gll:
            break;
        case opcode_mul_int_glc:
            break;
        case opcode_mul_int_gcg:
            break;
        case opcode_mul_int_gcl:
            break;
        case opcode_mul_int_lgg:
            break;
        case opcode_mul_int_lgl:
            break;
        case opcode_mul_int_lgc:
            break;
        case opcode_mul_int_llg:
            break;
        case opcode_mul_int_lll:
            break;
        case opcode_mul_int_llc:
            break;
        case opcode_mul_int_lcg:
            break;
        case opcode_mul_int_lcl:
            break;
        case opcode_div_int_ggg:
            break;
        case opcode_div_int_ggl:
            break;
        case opcode_div_int_ggc:
            break;
        case opcode_div_int_glg:
            break;
        case opcode_div_int_gll:
            break;
        case opcode_div_int_glc:
            break;
        case opcode_div_int_gcg:
            break;
        case opcode_div_int_gcl:
            break;
        case opcode_div_int_lgg:
            break;
        case opcode_div_int_lgl:
            break;
        case opcode_div_int_lgc:
            break;
        case opcode_div_int_llg:
            break;
        case opcode_div_int_lll:
            break;
        case opcode_div_int_llc:
            break;
        case opcode_div_int_lcg:
            break;
        case opcode_div_int_lcl:
            break;
        case opcode_set_float_gg:
            break;
        case opcode_set_float_gl:
            break;
        case opcode_set_float_gc:
            break;
        case opcode_set_float_lg:
            break;
        case opcode_set_float_ll:
            break;
        case opcode_set_float_lc:
            break;
        case opcode_add_float_ggg:
            break;
        case opcode_add_float_ggl:
            break;
        case opcode_add_float_ggc:
            break;
        case opcode_add_float_glg:
            break;
        case opcode_add_float_gll:
            break;
        case opcode_add_float_glc:
            break;
        case opcode_add_float_gcg:
            break;
        case opcode_add_float_gcl:
            break;
        case opcode_add_float_lgg:
            break;
        case opcode_add_float_lgl:
            break;
        case opcode_add_float_lgc:
            break;
        case opcode_add_float_llg:
            break;
        case opcode_add_float_lll:
            break;
        case opcode_add_float_llc:
            break;
        case opcode_add_float_lcg:
            break;
        case opcode_add_float_lcl:
            break;
        case opcode_sub_float_ggg:
            break;
        case opcode_sub_float_ggl:
            break;
        case opcode_sub_float_ggc:
            break;
        case opcode_sub_float_glg:
            break;
        case opcode_sub_float_gll:
            break;
        case opcode_sub_float_glc:
            break;
        case opcode_sub_float_gcg:
            break;
        case opcode_sub_float_gcl:
            break;
        case opcode_sub_float_lgg:
            break;
        case opcode_sub_float_lgl:
            break;
        case opcode_sub_float_lgc:
            break;
        case opcode_sub_float_llg:
            break;
        case opcode_sub_float_lll:
            break;
        case opcode_sub_float_llc:
            break;
        case opcode_sub_float_lcg:
            break;
        case opcode_sub_float_lcl:
            break;
        case opcode_mul_float_ggg:
            break;
        case opcode_mul_float_ggl:
            break;
        case opcode_mul_float_ggc:
            break;
        case opcode_mul_float_glg:
            break;
        case opcode_mul_float_gll:
            break;
        case opcode_mul_float_glc:
            break;
        case opcode_mul_float_gcg:
            break;
        case opcode_mul_float_gcl:
            break;
        case opcode_mul_float_lgg:
            break;
        case opcode_mul_float_lgl:
            break;
        case opcode_mul_float_lgc:
            break;
        case opcode_mul_float_llg:
            break;
        case opcode_mul_float_lll:
            break;
        case opcode_mul_float_llc:
            break;
        case opcode_mul_float_lcg:
            break;
        case opcode_mul_float_lcl:
            break;
        case opcode_div_float_ggg:
            break;
        case opcode_div_float_ggl:
            break;
        case opcode_div_float_ggc:
            break;
        case opcode_div_float_glg:
            break;
        case opcode_div_float_gll:
            break;
        case opcode_div_float_glc:
            break;
        case opcode_div_float_gcg:
            break;
        case opcode_div_float_gcl:
            break;
        case opcode_div_float_lgg:
            break;
        case opcode_div_float_lgl:
            break;
        case opcode_div_float_lgc:
            break;
        case opcode_div_float_llg:
            break;
        case opcode_div_float_lll:
            break;
        case opcode_div_float_llc:
            break;
        case opcode_div_float_lcg:
            break;
        case opcode_div_float_lcl:
            break;
        case opcode_cmp_eq:
            break;
        case opcode_cmp_neq:
            break;
        case opcode_cmp_geq:
            break;
        case opcode_cmp_leq:
            break;
        case opcode_jmp:
            break;
        case opcode_jmp_if_0:
            break;
        case opcode_jmp_if_1:
            break;
        case opcode_call:
            break;
        case opcode_return:
            break;
        case opcode_return_g:
            break;
        case opcode_return_l:
            break;
        case opcode_return_c:
            break;
        case opcode_save:
            break;
        case opcode_load:
            break;
        case opcode_halt:
            break;
        case opcode_print_int:
            break;
        case opcode_print_flt:
            break;
        case opcode_count:
            break;
        default:
            break;
        }
    }
    
};
#endif 