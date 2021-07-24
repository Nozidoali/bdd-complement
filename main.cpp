#include "p2.h"

using namespace std;

int main(int argc, char ** argv) {
    ifstream fin(argv[1]);
    ROBDD bdd;
    fin >> bdd;
    complement(bdd);
    cout << bdd;
    return 0;
}