#pragma once
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unordered_set>
#define POS 1
#define NEG 0
using namespace std;
typedef unsigned int VAR;
static VAR num_var;
typedef string CUBE;
typedef unordered_set<CUBE> SOP;
istream & operator >> (istream & is, SOP & sop) {
    is >> num_var; CUBE cube; while(is>>cube) sop.insert(cube);
    return is;
}
ostream & operator << (ostream & os, const SOP & sop) {
    os << num_var << endl; for (auto cube : sop) os << cube << endl;
    return os;
}
auto operator + (const SOP & x, const SOP & y) {
    SOP ret;
    for (auto cube : x) ret.insert(move(cube));
    for (auto cube : y) ret.insert(move(cube));
    return ret;
}
auto operator * (const SOP & x, VAR var) {
    SOP ret;
    VAR _var = var>>1; auto ch = var&0b01?'1':'0';
    for (auto cube : x) {
        auto _cube = string(cube); _cube[_var] = ch;
        if (cube[_var]==ch) ret.insert(_cube);
        if (cube[_var]=='-')ret.insert(_cube);
    }
    return ret;
}
auto to_cube (VAR var) {
    CUBE cube="";
    VAR _var = var>>1; auto ch = var&0b01?'1':'0';
    for(VAR pos=0;pos<num_var;pos++)
        cube+=(pos==_var? ch:'-');
    return cube;
}
auto to_sop (const CUBE & cube) {
    SOP sop; sop.insert(move(cube)); return sop;
}
auto de_morgen(const CUBE & cube) {
    SOP ret;
    for(VAR pos=0;pos<num_var;pos++) if (cube[pos]!='-')
        ret.insert(move(to_cube((pos<<1)+(cube[pos]=='0'))));
    return ret;
}
auto cube_1() {
    CUBE cube = ""; for(VAR i=0;i<num_var;i++) cube+="-";
    return cube;
}
auto is_1(const SOP & sop) {
    for (auto cube : sop) if (cube==cube_1()) return true;
    return false;
}
auto most_binate_var(const SOP & sop){
    auto eval = [=](VAR var) {
        VAR cnt[2] = {0,0};
        for (auto c:sop) if(c[var]!='-') cnt[c[var]=='1']++;
        return make_pair((int)min(cnt[0], cnt[1]), (int)(cnt[0]+cnt[1]));
    };
    VAR best_var; auto best_score = make_pair(-1, 0);
    for (VAR i=0;i<num_var;i++) {
        auto score = eval(i);
        if (score.second == 0) continue;
        if (best_score < score) {
            best_score = score;
            best_var = i;
        }
    }
    return best_var;
}
auto shannon_decomposition(const SOP & sop, VAR var) {
    SOP neg_sop; SOP pos_sop;
    for (auto cube : sop) {
        auto _cube = string(cube); _cube[var]='-';
        if (cube[var]!='1') neg_sop.insert(string(_cube));
        if (cube[var]!='0') pos_sop.insert(string(_cube));
    }
    return tuple<SOP, SOP>(neg_sop, pos_sop);
}
auto complement(const SOP & sop) {
    if (sop.size()==0) return to_sop(cube_1());
    if (is_1(sop))     return SOP();
    if (sop.size()==1) return de_morgen(*sop.begin());
    VAR var = most_binate_var(sop);
    auto sops = shannon_decomposition(sop, var);
    return complement(get<POS>(sops)) * (var*2+POS)
         + complement(get<NEG>(sops)) * (var*2+NEG);
}