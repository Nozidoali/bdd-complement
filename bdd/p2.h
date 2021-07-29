#pragma once

#include <cstring>
#include <iostream>
#include <array>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <cmath>
#include <algorithm>
#include <unordered_set>

using namespace std;
using std::make_unique;
using std::unique_ptr;

enum BOOLEAN {
    TRUE, FALSE, DONTCARE
};
typedef vector<BOOLEAN> CUBE;
typedef vector<CUBE> SOP;

typedef unsigned int VAR;
typedef vector<VAR> VAR_LIST;

typedef unsigned int ENTRY;
typedef vector<ENTRY> ENTRY_LIST;

struct ROBDD {
    struct BNode {
        double val;
        VAR var;
        unique_ptr<BNode> l0;
        unique_ptr<BNode> l1;
        BNode(double _val, BNode * _l0=nullptr, BNode * _l1=nullptr): l0(_l0), l1(_l1), val(_val){}
    };
    unique_ptr<BNode> root;
    int num_var;
    
    SOP data;
    ROBDD () {root=nullptr;}
};

istream & operator >> (istream & is, ROBDD & bdd);
ostream & operator << (ostream & os, const ROBDD & bdd);
void complement(ROBDD & bdd);