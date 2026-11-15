/*
 * IFJ + IAL projekt 2026/2027
 * Adam Havier (xhaviea00)
 */
#include "codegen.h"
#include <stdio.h>

int main(void)
{
    const CodegenFrame gf = CODEGEN_GF;
    const CodegenFrame lf = CODEGEN_LF;
    const CodegenFrame tf = CODEGEN_TF;
    const unsigned char text[] = "A B#\\\n";
    puts(".IFJcode26");

    printf("WRITE ");
    codegen_string(text, sizeof(text) - 1);
    putchar('\n');
    printf("WRITE ");
    codegen_int(1234567890123LL);
    puts("\nWRITE string@\\010");
    printf("WRITE ");
    codegen_float(2.5);
    puts("\nWRITE string@\\010");

    printf("WRITE ");
    codegen_bool(true);
    puts("\nWRITE string@\\010");
    printf("WRITE ");
    codegen_bool(false);
    puts("\nWRITE string@\\010");

    codegen_defvar(gf, "x");
    puts("CREATEFRAME");
    puts("PUSHFRAME");
    codegen_defvar(lf, "y");
    codegen_move_int(lf, "y", 42);
    codegen_move_var(gf, "x", lf, "y");
    codegen_add_int(lf, "y", gf, "x", 8);

    codegen_jumpifeq_int("vypis_vysledok", lf, "y", 50);
    puts("WRITE int@999");
    codegen_label("vypis_vysledok");
    codegen_write_var(lf, "y");
    puts("WRITE string@\\010");

    codegen_move_int(lf, "y", 0);
    codegen_label("zaciatok_cyklu");
    codegen_jumpifeq_int("koniec_cyklu", lf, "y", 3);
    codegen_add_int(lf, "y", lf, "y", 1);
    codegen_jump("zaciatok_cyklu");
    codegen_label("koniec_cyklu");
    codegen_write_var(lf, "y");
    puts("WRITE string@\\010");

    puts("CREATEFRAME");
    codegen_defvar(tf, "cislo");
    codegen_move_int(tf, "cislo", 42);
    codegen_call("vypis_cislo");
    codegen_write_var(tf, "cislo");
    puts("WRITE string@\\010");
    puts("WRITE int@8");
    puts("WRITE string@\\010");
    puts("POPFRAME");
    puts("EXIT int@0");

    codegen_label("vypis_cislo");
    puts("PUSHFRAME");
    codegen_add_int(lf, "cislo", lf, "cislo", 1);
    puts("POPFRAME");
    codegen_return();

    return 0;
}