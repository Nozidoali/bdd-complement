#include "p2.h"

int main (int argc, char ** argv) {
    ifstream fin(argv[1]);
    SOP sop; fin >> sop;
    auto completed_sop = complement(sop);
    cout << completed_sop;
    return 0;    
}