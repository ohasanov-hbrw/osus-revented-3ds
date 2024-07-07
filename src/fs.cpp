#include "fs.hpp"
#include "globals.hpp"
#include <stdio.h> 
#include <dirent.h> 
#include <time_util.hpp>

namespace fs = std::filesystem;

std::vector<std::string> ls(char* extension) {
    
  
    // opendir() returns a pointer of DIR type.  
    

    
    for (int i = 0; i < Global.Path.size(); i++) {
        if (Global.Path[i] == '\\') {
            Global.Path[i] = '/';
        }
    }

    
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    



    std::vector<std::string> text;
    text.clear();
    /*for (const auto & entry : fs::directory_iterator(Global.Path)){
        std::string filename = entry.path().filename().string();
        fs::directory_entry isDirectory(entry.path());
        if(filename[0] != '.'){
            if(isDirectory.is_directory()){
                filename.push_back('/');
                text.push_back(filename);
            }
            else if(entry.path().extension() == extension){
                text.push_back(filename);
            }
            
        }
    }
    sort(text.begin(), text.end());*/
    struct dirent *de;  // Pointer for directory entry 

    DIR *dr = opendir(Global.Path.c_str()); 
  
    if (dr == NULL){ // opendir returns NULL if couldn't open directory { 
        printf("Could not open current directory" ); 
    }
    else{
        while ((de = readdir(dr)) != NULL) {
            //printf("%s %d\n", de->d_name, de->d_type); 
            //SleepInMs(50);
            std::string filename = de->d_name;
            if(filename[0] != '.'){
                if(de->d_type == DT_DIR){
                    filename.push_back('/');
                    text.push_back(filename);
                }
                else if(IsFileExtension(filename.c_str(), extension)){
                    text.push_back(filename);
                }
            }
        }
  
        closedir(dr);
    }

    return text;
}

void create_dir(const std::string& path) {
    fs::create_directory(path);
}

std::string get_without_ext(const std::string& path) {
    fs::path path_p(Global.selectedPath);
    return path_p.stem().string();
}

int on_extract_entry(const char *filename, void *arg) {
    int i = 0;
    int n = *(int *)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
}
