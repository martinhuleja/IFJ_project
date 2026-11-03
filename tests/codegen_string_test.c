/*
 * IFJ + IAL projekt 2026/2027
 * Adam Havier (xhaviea00)
 *
 * AI-generated code (ChatGPT).
 * Prompt: Create a small driver that emits IFJcode26 using codegen_string
 * for decoded text containing a space, hash, backslash and newline.
 */
#include "codegen.h"
#include <stdio.h>

int main(void)
{
    const unsigned char text[] = "A B#\\\n";

    printf(".IFJcode26\n");
    printf("WRITE ");
    codegen_string(text, sizeof(text) - 1);
    printf("\n");

    printf("WRITE ");
    codegen_int(1234567890123LL);
    printf("\nWRITE string@\\010\n");

    printf("WRITE ");
    codegen_float(2.5);
    printf("\nWRITE string@\\010\n");

    printf("WRITE ");
    codegen_bool(true);
    printf("\nWRITE string@\\010\n");

    printf("WRITE ");
    codegen_bool(false);
    printf("\nWRITE string@\\010\n");

    printf("EXIT int@0\n");

    return 0;
}