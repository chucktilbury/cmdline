/*
 * Simple program to demonstrate usage.
 *
 * A person thinking of using this library should play around with it to get a
 * feel for how it works. It's a simple library but there are many
 * permutations of configuration vs. correct and incorrect input.
 *
 * Build string:
 * gcc -g -Wall -Wextra main.c cmdline.c
 *
 * Example use:
 * ./a.out -v10 -isomething
 */
#include <stdio.h>
#include "cmdline.h"

int main(int argc, char** argv)
{
    initCmdLine(CL_NO_FL, "This is a demonstration command line scenario.\n");
    addStrParam("-i", "incl", "set the include path", NULL, CL_REQD);
    addStrParam("-o", "outp", "create the output path", "./", CL_NONE);
    addNumParam("-v", "verbo", "set the level of verbosity", 0, CL_NONE);
    addCBwoParam("-h", NULL, "print this helpful information", showUseCmdLine, CL_NONE);
    parseCmdLine(argc, argv);

    printf("-i = %s\n", getStrParam("incl"));
    printf("-o = %s\n", getStrParam("outp"));
    printf("-v = %d\n", getNumParam("verbo"));

    resetCLFileList();
    for(const char* str = iterateCLFileList(); str != NULL; str = iterateCLFileList())
        printf("    file = %s\n", str);

    destroyCmdLine();
    return 0;
}
