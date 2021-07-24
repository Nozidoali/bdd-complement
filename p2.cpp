/**
 * author: Wang Hanyu
 * date: 2021-07-24
 * description: a c++11 standard implementation of sop to bdd.
 */

#include "p2.h"

static const char BOOLEAN_CHAR[4] = "01-";

BOOLEAN char_to_boolean(char ch) {
    unsigned int i;
    for (i=0; i<3; i++)
        if (BOOLEAN_CHAR[i] == ch) break;
    return (BOOLEAN)i;
}

istream & operator >> (istream & is, ROBDD & bdd) {
    is >> bdd.num_var;
    string str;
    while (is>>str) {
        CUBE cube;
        for ( auto ch : str ) {
            cube.push_back(char_to_boolean(ch));
        }
        assert(cube.size() == bdd.num_var);
        bdd.data.push_back(cube);
    }
    return is;
}

auto sop_is_tautology ( const SOP & sop, 
                        const VAR_LIST & var_list, 
                        const ENTRY_LIST & entry_list ) {
    /**
     * sop_is_tautology: return if the sop left is constant
     * @param sop: the original large sop
     * @param var_list: the variable left
     * @param entry_list: the entries left in the sop
     */
    for (auto entry : entry_list) {
        bool flag1 = true; // flag 1: contains all don't cares
        for (auto var : var_list)
            if (sop[entry][var] != DONTCARE) flag1 = false;
        if (flag1) return true;
    }
    return false;
}

auto cube_to_string ( const CUBE & cube, const VAR_LIST & var_list ) {
    string cube_str = "";
    for (auto var : var_list) {
        auto val = cube[var];
        cube_str += BOOLEAN_CHAR[val];
    }
    return cube_str;
}

ostream & operator << (ostream & os, const CUBE & cube) {
    for (auto val : cube)
        os << BOOLEAN_CHAR[val];
    os << endl;
    return os;
}

ostream & operator << (ostream & os, const SOP & sop) {
    for (auto cube : sop)
        os << cube;
    return os;
}

auto unique_entries   ( const SOP & sop, 
                        const VAR_LIST & var_list, 
                        const ENTRY_LIST & entry_list ) {
    ENTRY_LIST unique_entry_list;
    unordered_set<string> cube_list;
    for (auto entry : entry_list) {
        auto cube_str = cube_to_string(sop[entry], var_list);
        if (cube_list.find(cube_str) == cube_list.end()) {
            unique_entry_list.push_back(entry);
            cube_list.insert(cube_str);
        }
    }
    return unique_entry_list;
}
auto find_pivot       ( const SOP & sop, 
                        const VAR_LIST & var_list, 
                        const ENTRY_LIST & entry_list ) {
    /**
     * find_pivot: find the best var that can reduce the entropy
     * @param sop: the original large sop
     * @param var_list: the variable left
     * @param entry_list: the entries left in the sop
     */
    auto entropy_decend = [=] (VAR var) {
        double pos_cube = 0.1; // avoid log 0
        double neg_cube = 0.1; // avoid log 0
        double tot_cube = (double)entry_list.size();
        unordered_set<string> hash_table; // hashing
        for (auto entry : entry_list)
            hash_table.insert(cube_to_string(sop[entry], var_list));
        VAR pivot_pos = 0;
        for (;var_list[pivot_pos]!=var;pivot_pos++){}
        for (auto cube : hash_table) {
            if (cube[pivot_pos] != '0') pos_cube+=1.0;
            if (cube[pivot_pos] != '1') neg_cube+=1.0;
        }
        return tuple<double, double>( pos_cube / (pos_cube+neg_cube),
                                        tot_cube*log(tot_cube) 
                                        - pos_cube*log(pos_cube)
                                        - neg_cube*log(neg_cube));
    };
    assert(var_list.size() != 0 && entry_list.size() != 0);
    VAR best_pivot = var_list.size();
    double best_score = 0.0;
    double prediction = 0.0;
    for (auto pivot : var_list) {
        auto result = entropy_decend(pivot);
        if (best_pivot == var_list.size() || get<1>(result) > best_score) {
            prediction = get<0>(result);
            best_score = get<1>(result);
            best_pivot = pivot;
        }
    }
    assert(best_score != -1.0);
    return tuple<VAR, double>(best_pivot, prediction);
}

auto split_with_pivot ( VAR var,
                        const SOP & sop, 
                        const VAR_LIST & var_list, 
                        const ENTRY_LIST & entry_list ) {
    /**
     * split_with_pivot: shannon decomposition w.r.t pivot
     * @param sop: the original large sop
     * @param var_list: the variable left
     * @param entry_list: the entries left in the sop
     */
    assert(find(var_list.begin(), var_list.end(), var) != var_list.end());
    ENTRY_LIST pos_entry_list;
    ENTRY_LIST neg_entry_list;
    for (auto entry : entry_list) {
        if (sop[entry][var] != FALSE) pos_entry_list.push_back(entry);
        if (sop[entry][var] != TRUE)  neg_entry_list.push_back(entry);
    }
    return tuple<ENTRY_LIST, ENTRY_LIST>(pos_entry_list, neg_entry_list);
}

auto get_new_var_list (VAR pivot, const VAR_LIST & var_list ) {
    VAR_LIST new_var_list;
    for (auto var : var_list)
        if (var != pivot) new_var_list.push_back(var);
    return new_var_list;
}

void sop_to_bdd       ( unique_ptr<ROBDD::BNode> & root, 
                        const SOP & sop, 
                        const VAR_LIST & var_list, 
                        const ENTRY_LIST & entry_list ) {
    /**
     * sop_to_bdd: convert the sop to bdd
     * @param root: the current node in the binary tree
     * @param sop: the original large sop
     * @param var_list: the variable left
     * @param entry_list: the entries left in the sop
     */
    assert(root == nullptr);
    if (entry_list.size() == 0) {
        root = make_unique<ROBDD::BNode>(0); // const 0
        return;
    }
    if (sop_is_tautology(sop, var_list, entry_list)) {
        root = make_unique<ROBDD::BNode>(1); // const 1
        return; 
    }
    auto result = find_pivot(sop, var_list, entry_list);
    auto pivot = get<0>(result);
    root = make_unique<ROBDD::BNode>(get<1>(result));
    root->var = pivot;
    auto splited_sop = split_with_pivot(pivot, sop, var_list, entry_list);
    auto new_var_list = get_new_var_list(pivot, var_list);
    auto pos_entry_list = unique_entries(sop, new_var_list, get<0>(splited_sop));
    auto neg_entry_list = unique_entries(sop, new_var_list, get<1>(splited_sop));
    sop_to_bdd(root->l0, sop, new_var_list, neg_entry_list);
    sop_to_bdd(root->l1, sop, new_var_list, pos_entry_list);
}

auto get_entry_list (const ROBDD & bdd) {
    ENTRY_LIST entry_list;
    for (auto i=0;i<bdd.data.size();++i)
        entry_list.push_back(i);
    return entry_list;
}

auto get_var_list (const ROBDD & bdd) {
    VAR_LIST var_list;
    for (auto i=0;i<bdd.num_var;++i)
        var_list.push_back(i);
    return var_list;
}

auto bdd_to_sop_rec (ROBDD & bdd, const unique_ptr<ROBDD::BNode> & root, CUBE & cube) {
    if (root->val == 0) bdd.data.push_back(CUBE(cube));
    if (root->l0 == nullptr && root->l1 == nullptr) {
        if (root->val == 0.0) bdd.data.push_back(CUBE(cube));            
        return;
    }
    assert(root->l0 && root->l1);
    auto pivot = root->var;
    cube[pivot] = FALSE; bdd_to_sop_rec(bdd, root->l0, cube); // recursion left
    cube[pivot] = TRUE;  bdd_to_sop_rec(bdd, root->l1, cube); // recursion right
    cube[pivot] = DONTCARE;
    return;
}

auto bdd_to_sop (ROBDD & bdd) {
    CUBE cube;
    for (auto i=0; i<bdd.num_var; ++i) cube.push_back(DONTCARE);
    assert(cube.size() == bdd.num_var);
    bdd_to_sop_rec(bdd, bdd.root, cube);
}

void complement(ROBDD & bdd) {
    sop_to_bdd(bdd.root, bdd.data, get_var_list(bdd), get_entry_list(bdd));
    for (auto & cube : bdd.data) cube.clear();
    bdd.data.clear();
    bdd_to_sop(bdd);
}

ostream & operator << (ostream & os, const ROBDD & bdd) {
    unordered_set<string> ans; // hashing
    os << bdd.num_var << endl;
    auto var_list = get_var_list(bdd);
    for (auto cube : bdd.data)
        ans.insert(cube_to_string(cube, var_list));
    for (auto cube : ans)
        os << cube << endl;
    return os;
}