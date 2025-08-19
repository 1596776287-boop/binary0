#include <cage.h>

int main(int argc, char* argv[]){
    if(argc < 2) {
        std::cerr << "Usage: encrypt <dbfile>\n";
        return 1;
    }
    std::string dbpath = argv[1];
    std::filesystem::path path(dbpath);
    if(path.extension() == ".bin")
    encryptenc(dbpath, "060305");
std::cout<< "Database encrypted successfully.\n";
    return 0;
}