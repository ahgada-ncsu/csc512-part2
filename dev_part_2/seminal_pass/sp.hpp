#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include <map>
#include <string>
#include <set>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

typedef struct {
    int id;
    string name;
} param;

typedef struct {
    string name;
} variable;

typedef struct {
    int line_num;
    string name;
    vector<param> args;
} func_map;

typedef struct {
    int line_num;
    vector<variable> vars;
    string scope;
} line_map;

typedef struct {
    int gets_at_line;
    string type;
    string code;
    line_map vars;
} get_list;

typedef struct {
    string name;
    string scope;
    int defined_at_line;
    vector<get_list> gets_value_infos;
} var_map;

typedef struct {
    string name;
    vector<param> args;
    string scope;
    int line;
} func_call_map;

vector<line_map> variables_per_line;    // Variables defined at each line
vector<func_map> functions;             // Functions and their arguments
vector<var_map> variable_infos;              // Variables and their gets
vector<func_call_map> function_calls;   // Function calls and their arguments

vector<string> input_functions = {"scanf", "fread", "fopen", "getc"};