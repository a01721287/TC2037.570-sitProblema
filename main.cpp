// =================================================================
// File: main.cpp
// Author: Alejandro Hernández A01721287
// Description: This file highlights the input files code in an html file.
//              To compile: g++ main.cpp -lpthread -o app
//              To run: ./app input_files/*.cpp
//
// Copyright (c) 2022 by Tecnologico de Monterrey.
// All Rights Reserved. May be reproduced for any non-commercial
// purpose.
// =================================================================

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include "utils.h"
#include <fstream>
#include <string>
#include <regex>

using namespace std;

const int THREADS = 8;

//Establecer las palabras reservadas como strings, luego estas se interpretaran como regex
const string reserved = "(void|auto|break|case|const|continue|default|do|else|enum|extern|for|goto|if|inline|register|restrict|return|sizeof|static|struct|switch|typedef|union|void|volatile|while|namespace)( |\\(|\\{)";
const string data_type = "(int|string|double|float|char|bool|long|short|signed|unsigned)( |\\*)";
const string pragma = "(#include \\&lt;([^;]*)\\&gt;)|(#include \"(.*?)\")";
const string digit = "\\d";
const string operators ="(\\+|\\-|\\*|\\%|\\=|&lt;&lt;|&gt;&gt;|\\(|\\))";
const string comment ="(\\/\\/)(.+?)(?=[\n\r]|\\*\\))";

// =========================== SEQUENTIAL ==========================
void resaltado_sec(int argc, char* argv[]){
    //cambiar la lista de argumentos dados de char a string
    vector<string> filenames(argv, argv+argc);
    //obtener los nombres de los archivos al quitar el path que venía antes y también el tipo .cpp
    for (int i = 0; i < argc; i++){
        string tmp = filenames[i];
        tmp = tmp.replace(0,filenames[i].find('/')+1,"");
        tmp = tmp.replace((tmp.size()-4),4,"");
        filenames[i] = tmp;
    }

    //Para cada path dado de los argumentos, resaltar el texto
    for (int i = 0; i < argc; i++){
        //Lectura de archivo
        fstream arch;
        arch.open(argv[i], ios::in);
        string code = "";
        string line;
        do{
            getline(arch,line);
            code.append(line);
        } while (!arch.eof());
        arch.close();

        //Preparar el archivo a leer cambiando los <> para no confundir a HTML
        code = regex_replace(code, regex("<"), "&lt;");
        code = regex_replace(code, regex(">"), "&gt;");

        //Reemplazar con regex para darles clases css a las palabras reservadas
        code = regex_replace(code, regex(operators), "<text class=\"op\">$&</text>");
        code = regex_replace(code, regex(reserved), "<text class=\"reserved\">$&</text>");
        code = regex_replace(code, regex(digit), "<text class=\"number\">$&</text>");
        code = regex_replace(code, regex(data_type), "<text class=\"data_type\">$&</text>");
        code = regex_replace(code, regex(comment), "<text class=\"comment\">$&</text>");
        code = regex_replace(code, regex(pragma), "<text class=\"pragma\">$&</text>");

        //Crear el archivo html resaltado.
        fstream arch2;
        arch2.open("output_files_seq/" + filenames[i] + ".html", ios::out);
        string boilerplate = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n\t<meta charset=\"UTF-8\">\n\t<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<title>"+ filenames[i] +"</title>\n<link rel=\"stylesheet\" href=\"../styles.css\">\n</head>\n<body>\n";
        arch2<< boilerplate<<"<pre class=\"text\">"<<code << "</pre>"<< "\n</body>\n</html>";
        arch2.close();
    }
}
// =========================== SEQUENTIAL ==========================

// =========================== CONCURRENT ==========================
typedef struct {
    int start, end;
    char **argv;
} Block;

void* task(void *argv){
    Block *block;
    block = (Block*) argv;
    //cambiar la lista de argumentos dados de char a string
    vector<string> filenames(block->argv, block->argv+block->end);
    for (int i = block->start; i < block->end; i++) {
        //obtener los nombres de los archivos al quitar el path que venía antes y también el tipo .cpp
        string tmp = filenames[i];
        tmp = tmp.replace(0,filenames[i].find('/')+1,"");
        tmp = tmp.replace((tmp.size()-4),4,"");
        filenames[i] = tmp;
    }

    //Para cada path dado de los argumentos, resaltar el texto
    for (int i = block->start; i < block->end; i++) {
        //Lectura de archivo
        fstream arch;
        arch.open(block->argv[i], ios::in);
        string code = "";
        string line;
        do{
            getline(arch,line);
            code.append(line);
        } while (!arch.eof());
        arch.close();

        //Preparar el archivo a leer cambiando los <> para no confundir a HTML
        code = regex_replace(code, regex("<"), "&lt;");
        code = regex_replace(code, regex(">"), "&gt;");

        //Reemplazar con regex para darles clases css a las palabras reservadas
        code = regex_replace(code, regex(operators), "<text class=\"op\">$&</text>");
        code = regex_replace(code, regex(reserved), "<text class=\"reserved\">$&</text>");
        code = regex_replace(code, regex(digit), "<text class=\"number\">$&</text>");
        code = regex_replace(code, regex(data_type), "<text class=\"data_type\">$&</text>");
        code = regex_replace(code, regex(comment), "<text class=\"comment\">$&</text>");
        code = regex_replace(code, regex(pragma), "<text class=\"pragma\">$&</text>");

        //Crear el archivo html resaltado.
        fstream arch2;
        arch2.open("output_files_conc/" + filenames[i] + ".html", ios::out);
        string boilerplate = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n\t<meta charset=\"UTF-8\">\n\t<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<title>"+ filenames[i] +"</title>\n<link rel=\"stylesheet\" href=\"../styles.css\">\n</head>\n<body>\n";
        arch2<< boilerplate<<"<pre class=\"text\">"<<code << "</pre>"<< "\n</body>\n</html>";
        arch2.close();
    }
    pthread_exit(0);
}
// =================================================================

//correr con input_files/*.cpp como parámetro
int main(int argc, char* argv[]){
    //eliminar el ejecutable de los argumentos
    argc--;argv++;
    double sequential, concurrent;
    int blockSize;
    //inicialización de threads y constructor "Block"
    pthread_t tids[THREADS];
    Block blocks[THREADS];  

    //==========Secuencial=========
    sequential = 0;
    start_timer();
    resaltado_sec(argc, argv);
    sequential += stop_timer();
    cout << "sequential average time = " << setprecision(5) << (sequential / N) << " ms" << endl;

     // ============Concurrente===============
    blockSize = argc/THREADS;
    //asignación de archivos y límites a "blocks" en base del tamaño de threads
    for (int i = 0; i < THREADS; i++) {
        blocks[i].start = i * blockSize;
        blocks[i].end = (i != (THREADS - 1))? (i + 1) * blockSize : argc;
        blocks[i].argv = argv;
    }

    concurrent = 0;
    start_timer();
    //Creación de threads
    for(int j = 0; j < THREADS; j++){
        pthread_create(&tids[j], NULL, task, &blocks[j]);
    }

    //Unión de threads
    for(int j = 0; j < THREADS; j++){
        pthread_join(tids[j], NULL);
    }
    concurrent += stop_timer();

    cout << "concurrent average time = " << setprecision(5) << (concurrent / N) << " ms" << endl;
    cout << "speed up = " << setprecision(2) << (sequential / concurrent) << endl;

    return 0;
}