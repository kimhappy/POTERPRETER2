//-std=c++2a
#include <bits/stdc++.h>

#define ef else if
#define cf if constexpr

class Tree;
class UFunc;
class Val;

using F64   = double;
using DFunc = std::function< Val(std::vector< Val >&) >;

class UFunc {
private:
    std::shared_ptr< Tree > decl;
    std::vector< std::shared_ptr< Tree > > contents;

public:
    UFunc() {}
    UFunc(const std::shared_ptr< Tree >& decl, const std::vector< std::shared_ptr< Tree > >&& contents)
    : decl(decl), contents(contents) {}
    UFunc(const std::shared_ptr< Tree >& decl, const std::vector< std::shared_ptr< Tree > >&  contents)
    : decl(decl), contents(contents) {}

    UFunc copy();
    Val capture();
    Val run(const std::vector< Val >& para);
    std::string to_s();
};

class Val {
private:
    std::variant< std::monostate, UFunc, DFunc, F64 > val;

public:
    enum class Type{ NONE, UFUNC, DFUNC, NUM };

    Val() {}
    Val(auto val): val(val) {}

    Val copy();
    Type type();
    std::string to_s();
    std::string to_c();
    template< typename T >
    T& get();
};

struct Tree {
    enum class Expr {
        now_val,
        var,
        decl_func_start,
        decl_func_end,
        lambda,
        to_capture,
        decl_var,
        assign,
        now_capture,
        function_call
    } expr;
    Val val;
    std::string atom;
    std::shared_ptr< Tree > head = nullptr;
    std::vector< std::shared_ptr< Tree > > tail;

    Tree() {}
    Tree(std::string s);
    Tree(Expr expr, Val val, std::string& atom, std::shared_ptr< Tree > head, std::vector< std::shared_ptr< Tree > >& tail)
    : expr(expr), val(val), atom(atom), head(head), tail(tail) {}

    Tree copy();
    Val eval();
    void capture(bool flag);
    std::string to_s();
};

std::vector< std::map< std::string, Val > > var_stack;
bool lc = false;

namespace Helper {
    std::shared_ptr< Tree > read_line(std::vector< std::shared_ptr< Tree > >& container, int  line);
    Val eval_line(std::vector< std::shared_ptr< Tree > >& container, int& line);

    DFunc changer1(auto func) {
        return [func](std::vector< Val >& para) -> Val {
            Val v = para[ 0 ];
            F64 n = v.get< F64 >();
            return (F64)func(n);
        };
    }

    DFunc changer2(auto func) {
        return [func](std::vector< Val >& para) -> Val {
            Val v0 = para[ 0 ], v1 = para[ 1 ];
            F64 n0 = v0.get< F64 >(), n1 = v1.get< F64 >();
            return (F64)func(n0, n1);
        };
    }

    Val func_if(std::vector< std::shared_ptr< Tree > >& para) {
        return para[ 1 + (para[ 0 ]->eval().get< F64 >() == 0) ]->eval();
    }

    Val func_and(std::vector< std::shared_ptr< Tree > >& para) {
        return para[ 0 ]->eval().get< F64 >() && para[ 1 ]->eval().get< F64 >();
    }

    Val func_or(std::vector< std::shared_ptr< Tree > >& para) {
        return para[ 0 ]->eval().get< F64 >() || para[ 1 ]->eval().get< F64 >();
    }

    Val print_num (std::vector< Val >& para) {
        std::cout << para[ 0 ].to_s();
        lc = true;
        return 1;
    }

    Val print_char(std::vector< Val >& para) {
        std::cout << para[ 0 ].to_c();
        lc = true;
        return 1;
    }
}

Tree::Tree(std::string s) {
    if(s[ 0 ] == '#') {
        expr = Expr::decl_func_start;
        head = std::make_shared< Tree >(s.substr(5, s.size() - 5));
        return;
    }
    ef(s == "end") {
        expr = Expr::decl_func_end;
        return;
    }
    ef(s[ 0 ] == '[' && s.back() == ']') {
        expr = Expr::to_capture;
        head = std::make_shared< Tree >(s.substr(1, s.size() - 2));
        return;
    }

    for(int i = 0, t1 = 0; i < s.size(); ++i) {
        if((t1 += (s[ i ] == '(') - (s[ i ] == ')')) == 0 && s[ i ] == '=') {
            if(s[ 0 ] == '`') {
                expr = Expr::decl_var;
                atom = s.substr(1, i - 2);
            } else {
                expr = Expr::assign;
                atom = s.substr(0, i - 1);
            }

            head = std::make_shared< Tree >(s.substr(i + 2, s.size() - i - 2));
            return;
        }
    }

    if(s.back() != ')') {
        if(s[ 0 ] == '!') {
            expr = Expr::now_capture;
            head = std::make_shared< Tree >(s.substr(1, s.size() - 1));
        }
        ef(std::isdigit(s[ 0 ]) || s[ 0 ] == '-') {
            expr = Expr::now_val;
            val  = std::stod(s);
        } else {
            expr = Expr::var;
            atom = s;
        }

        return;
    }

    if(s[ s.size() - 2 ] == '(') {
        if(s.substr(0, s.size() - 2) == "lambda")
            expr = Expr::lambda;
        else {
            expr = Expr::function_call;
            head = std::make_shared< Tree >(s.substr(0, s.size() - 2));
        }
        return;
    }

    for(int i = s.size() - 1, t1 = 0, t2 = s.size() - 1; i >= 0; --i) {
        t1 += (s[ i ] == '(') - (s[ i ] == ')');

        if(t1 == -1 && s[ i - 2 ] == ',') {
            tail.emplace_back(new Tree(s.substr(i, t2 - i)));
            i = t2 = i - 2;
        }
        ef(t1 == -1 && s[ i - 1 ] == '(') {
            tail.emplace_back(new Tree(s.substr(i, t2 - i)));
            std::reverse(tail.begin(), tail.end());

            if(s = s.substr(0, i - 1); s == "lambda")
                expr = Expr::lambda;
            ef((head = std::make_shared< Tree >(s))->expr == Expr::lambda) {
                expr = Expr::now_val;
                val  = UFunc(head, tail);
            } else
                expr = Expr::function_call;
            return;
        }
    }
}

Tree Tree::copy() {
    std::vector< std::shared_ptr< Tree > > ntail(tail.size());

    for(int i = 0; i < tail.size(); ++i)
        ntail[ i ] = std::make_shared< Tree >(tail[ i ]->copy());

    if(head == nullptr)
        return {
            .expr = expr,
            .val  = val.copy(),
            .atom = atom,
            .head = nullptr,
            .tail = ntail
        };

    return {
        .expr = expr,
        .val  = val.copy(),
        .atom = atom,
        .head = std::make_shared< Tree >(head->copy()),
        .tail = ntail
    };
}

Val Tree::eval() {
    switch(expr) {
        case Expr::now_val: return val;
        case Expr::var: {
            for(auto i = var_stack.rbegin(); i != var_stack.rend(); ++i)
                if(auto fi = i->find(atom); fi != i->end())
                    return fi->second;
            std::cout << '\'' << atom << "\' not found!\n";
            std::exit(0);
        }
        case Expr::decl_var: return var_stack.back()[ atom ] = head->eval();
        case Expr::assign: {
            for(auto i = var_stack.rbegin(); i != var_stack.rend(); ++i)
                if(auto fi = i->find(atom); fi != i->end())
                    return fi->second = head->eval();
            std::cout << '\'' << atom << "\' not found!\n";
            std::exit(0);
        }
        case Expr::now_capture: return head->eval().get< UFunc >().capture();
        case Expr::function_call: {
            if(head->expr == Expr::var && head->atom == "if")
                return Helper::func_if(tail);
            ef(head->expr == Expr::var && head->atom == "and")
                return Helper::func_and(tail);
            ef(head->expr == Expr::var && head->atom == "or")
                return Helper::func_or(tail);
            else {
                std::vector< Val > v(tail.size());
                auto i1 = v.begin();
                for(auto i2 = tail.begin(); i2 != tail.end(); *(i1++) = (*i2++)->eval());
                if(auto t1 = head->eval(); t1.type() == Val::Type::UFUNC)
                    return head->eval().get< UFunc >().run(v);
                return head->eval().get< DFunc >()(v);
            }
        }
    }
}

void Tree::capture(bool flag) {
    switch(expr) {
        case Expr::var: {
            if(flag) {
                val  = eval();
                expr = Expr::now_val;
            }

            return;
        }
        case Expr::to_capture: {
            if(!flag) {
                head->capture(true);
                expr = head->expr;
                val  = head->val;
                atom = head->atom;
                tail = head->tail;
                head = head->head;
            }

            return;
        }
        case Expr::function_call: {
            if(head->expr != Expr::var || (head->atom != "if" && head->atom != "or" && head->atom != "and"))
                head->capture(flag);
            for(auto& i: tail) i->capture(flag);
            return;
        }
        default: {
            if(expr == Expr::now_capture || expr == Expr::decl_var || expr == Expr::assign) {
                head->capture(flag);
                return;
            }
        }
    }
}

std::string Tree::to_s() {
    switch(expr) {
        case Expr::decl_func_start: return std::string("decl_function_start(") + head->to_s() + ')';
        case Expr::decl_func_end:   return "end";
        case Expr::to_capture:      return std::string("to_capture(") + head->to_s() + ')';
        case Expr::decl_var:        return std::string("decl_var(") + atom + ", " + head->to_s() + ')';
        case Expr::assign:          return std::string("assign(") + atom + ", " + head->to_s() + ')';
        case Expr::now_capture:     return std::string("now_capture(") + head->to_s() + ')';
        case Expr::var:             return atom;
        case Expr::now_val:         return val.to_s();
        case Expr::lambda: {
            std::string s = "lambda(";
            for(auto& i: tail) s += i->to_s() + ", ";
            if(!tail.empty()) s = s.substr(0, s.size() - 2);
            return s + ')';
        }
        case Expr::function_call: {
            std::string s = head->to_s() + "(";
            for(auto& i: tail) s += i->to_s() + ", ";
            if(!tail.empty()) s = s.substr(0, s.size() - 2);
            return s + ')';
        }
    }
}

    std::shared_ptr< Tree > decl;
    std::vector< std::shared_ptr< Tree > > contents;

//todo: impl
UFunc UFunc::copy() {
    std::vector< std::shared_ptr< Tree > > ncontents(contents.size());

    for(int i = 0; i < contents.size(); ++i)
        ncontents[ i ] = std::make_shared< Tree >(contents[ i ]->copy());

    return {
        .decl     = std::make_shared< Tree >(decl->copy()),
        .contents = ncontents
    };
}

// todo: 최적화
Val UFunc::capture() {
    UFunc ret = this->copy();
    for(auto& i: ret.contents) i->capture(false);
    return ret;
}

Val UFunc::run(const std::vector< Val >& para) {
    //std::cout << to_s() << '\n';
    var_stack.resize(var_stack.size() + 1);
    for(int i = 0; i < para.size(); ++i)
        var_stack.back()[ decl->tail[ i ]->atom ] = para[ i ];
    var_stack.back()[ "NYA" ] = this->copy();
    for(int i = 0; i < contents.size() - 1; Helper::eval_line(contents, i));
    Val ret = contents[ contents.size() - 1 ]->eval();
    var_stack.pop_back();
    return ret;
}

std::string UFunc::to_s() {
    std::string s = "{ decl: " + decl->to_s() + ", contents: { ";
    for(auto& i: contents) s += i->to_s() + ", ";
    return s.substr(0, s.size() - 2) + " }";
}

Val Val::copy() {
    return (type() == Type::UFUNC) ? get< UFunc >().copy() : val;
}

Val::Type Val::type() {
    return static_cast< Type >(val.index());
}

std::string Val::to_s() {
    switch(type()) {
        case Type::NONE:  return "None";
        case Type::UFUNC: return get< UFunc >().to_s();
        case Type::DFUNC: return "DFunc";
        case Type::NUM:  return std::to_string(get< F64 >());
    }
}

std::string Val::to_c() {
    switch(type()) {
        case Type::NONE:  return "None";
        case Type::UFUNC: return get< UFunc >().to_s();
        case Type::DFUNC: return "DFunc";
        case Type::NUM:   return std::string(1, get< F64 >());
    }
}

template< typename T >
T& Val::get() {
    return std::get< T >(val);
}

std::shared_ptr< Tree > Helper::read_line(std::vector< std::shared_ptr< Tree > >& container, int line) {
    while(container.size() <= line) {
        std::string s;
        if(lc) std::cout << '\n';
        lc = false;
        std::getline(std::cin, s);
        if(s == "EXIT") std::exit(0);
        s.erase(0, s.find_first_not_of(' '));
        if((s = s.substr(0, s.find("//"))).empty()) continue;
        container.emplace_back(new Tree(s));
    }

    return container[ line ];
}

Val Helper::eval_line(std::vector< std::shared_ptr< Tree > >& container, int& line) {
    int bi = line + 1;
    std::shared_ptr< Tree > tr1 = Helper::read_line(container, line);

    if(tr1->expr == Tree::Expr::decl_func_start) {
        for(int t1 = 1; ; ) {
            std::shared_ptr< Tree > tr2 = read_line(container, ++line);
            t1 += (tr2->expr == Tree::Expr::decl_func_start)
                - (tr2->expr == Tree::Expr::decl_func_end);
            if(t1 == 0) break;
        }

        return var_stack.back()[ tr1->head->head->atom ] = UFunc(tr1->head, std::vector(container.begin() + bi, container.begin() + line++));
    }

    ++line;
    return tr1->eval();
}

int main() {
    std::vector< std::shared_ptr< Tree > > codes;

    var_stack.push_back(std::map< std::string, Val > {
        { "print_num"    , DFunc(Helper::print_num ) },
        { "print_char"   , DFunc(Helper::print_char) },
        { "not"          , Helper::changer1(std::logical_not()) },
        { "add"          , Helper::changer2(std::plus()) },
        { "sub"          , Helper::changer2(std::minus()) },
        { "mul"          , Helper::changer2(std::multiplies()) },
        { "div"          , Helper::changer2(std::divides()) },
        { "pow"          , Helper::changer2((F64(*)(F64, F64))(std::pow)) },
        { "greater"      , Helper::changer2(std::greater()) },
        { "less"         , Helper::changer2(std::less()) },
        { "greater_equal", Helper::changer2(std::greater_equal()) },
        { "less_equal"   , Helper::changer2(std::less_equal()) },
        { "same"         , Helper::changer2(std::equal_to()) },
        { "diff"         , Helper::changer2(std::not_equal_to()) },
        { "sin"          , Helper::changer1((F64(*)(F64))(std::sin)) },
        { "cos"          , Helper::changer1((F64(*)(F64))(std::cos)) },
        { "tan"          , Helper::changer1((F64(*)(F64))(std::tan)) },
        { "asin"         , Helper::changer1((F64(*)(F64))(std::asin)) },
        { "acos"         , Helper::changer1((F64(*)(F64))(std::acos)) },
        { "atan"         , Helper::changer1((F64(*)(F64))(std::atan)) },
        { "sinh"         , Helper::changer1((F64(*)(F64))(std::sinh)) },
        { "cosh"         , Helper::changer1((F64(*)(F64))(std::cosh)) },
        { "tanh"         , Helper::changer1((F64(*)(F64))(std::tanh)) },
        { "asinh"        , Helper::changer1((F64(*)(F64))(std::asinh)) },
        { "acosh"        , Helper::changer1((F64(*)(F64))(std::acosh)) },
        { "atanh"        , Helper::changer1((F64(*)(F64))(std::atanh)) },
        { "log"          , Helper::changer1((F64(*)(F64))(std::log)) },
        { "ceil"         , Helper::changer1((F64(*)(F64))(std::ceil)) },
        { "floor"        , Helper::changer1((F64(*)(F64))(std::round)) },
        { "round"        , Helper::changer1((F64(*)(F64))(std::floor)) }
    });
    codes.reserve(100000);
    std::cout << "POTERPRETER READY!\n";
    for(int i = 0; ; Helper::eval_line(codes, i));
    return 0;
}
