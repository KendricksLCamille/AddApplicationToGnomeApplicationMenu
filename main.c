/*
 * Purpose: Automatically generate a desktop file once passed for freedesktop compliant DEs.
 * Sources:
 *      https://unix.stackexchange.com/a/103222 -> Read further down for more info
 *      https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html
 * */


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>

const int start = 2;
const char* STR = "[Desktop Entry]\n"
                  "Type=Application\n"
                  "Encoding=UTF-8\n"
                  "Terminal=false\n";
const char* help_str = "\"Name of Files\" -n \"Application Name\" -c \"Comments\" -i \"Path to Icon\" -e \"Path to Executable\" -cat \"TAG_1;TAG_2;TAG_3;...\"";

FILE * fp;
char* name;
char* comment;
char* icon_path;
char* executable_path;
char* categories;


#define DO(variable_name, initial, exclusive_end, increment_value, body) for(int variable_name = initial; variable_name < exclusive_end; variable_name = variable_name + increment_value) body
#define DO_TILL(nameOfVariable, start, max, body) DO(nameOfVariable, start, max,1, body)


void print_help(char* application){

    printf("Error Occurred. Expected format: %s %s\n", application, help_str);
}
char* get_value(int count, char** args, char* key){
    DO(i, start, count, 2, {
        char* str = args[i];

        if(strcmp(str, key) == 0){
            if(i + 1 < count){
                return args[i + 1];
            }

            printf("Value missing for %s {value}", key);
            exit(1);
        }
    })
    printf("%s doesn't exist here", key);
    return NULL;
}
bool help_requested(char** args, int argCount){
    const char data[2][7] = { "-h", "--help" };

    //skipping 0 since it is the url to the application

    DO_TILL(i, 0, argCount, {

        assert(i < argCount);
        char* str = args[i];

        DO_TILL(j, 0, 2, {
            if(strcmp(str, data[j]) == 0){
                print_help(args[0]);
                return true;
            }
        })

    })

    return false;
}
char* concat_MALLOC(const char* begin, char* end){
    unsigned long s1len = strlen(begin), s2len = strlen(end);
    unsigned long len = s1len + s2len + 1;
    char* str = malloc(len);
    strcat(str, begin);
    strcat(str + s1len, end);
    str[len - 1] = '\0';
    return str;
}

int main(int count, char** args) {

    char* userPath = getenv("HOME");

    if(userPath == NULL){
        perror("Couldn't obtain user directory");
        exit(errno);
    }

    //Purposely choose to leave dangling pointers out of laziness
    userPath = concat_MALLOC(userPath, "/");
    userPath = concat_MALLOC(userPath, ".local/share/applications/");

    if(help_requested(args, count)){
        exit(0);
    }

#define SET_IF_VALID_ELSE_EXIT_IF_NULL(var_name, input) var_name = input; if(var_name == NULL) { exit(1); }
    SET_IF_VALID_ELSE_EXIT_IF_NULL(name, get_value(count, args, "-n"))
    SET_IF_VALID_ELSE_EXIT_IF_NULL(comment, get_value(count, args, "-c"))
    SET_IF_VALID_ELSE_EXIT_IF_NULL(icon_path, get_value(count, args, "-i"))
    SET_IF_VALID_ELSE_EXIT_IF_NULL(executable_path, get_value(count, args, "-e"))
    SET_IF_VALID_ELSE_EXIT_IF_NULL(categories, get_value(count, args, "-cat"))

    char* path = concat_MALLOC(userPath, args[start - 1]);
    char* path2 = concat_MALLOC(path, ".desktop");
    free(path);

    fp = fopen (path2, "w+");
    if(fp == NULL) {
        perror("Can't access path");
        exit(errno);
    }

    fprintf(fp, "%s", STR);
#define ASSIST_IN_FILE_PRINT(key, value) {char* _key = #key; fprintf(fp,"%s=%s\n",_key, value);}
    ASSIST_IN_FILE_PRINT(Name, name)
    ASSIST_IN_FILE_PRINT(Comment, comment)
    ASSIST_IN_FILE_PRINT(Icon, icon_path)
    ASSIST_IN_FILE_PRINT(Exec, executable_path)
    ASSIST_IN_FILE_PRINT(Categories, categories)
    fclose(fp);

#undef ASSIST_IN_FILE_PRINT
#undef SET_IF_VALID_ELSE_EXIT_IF_NULL
    return 0;
}
