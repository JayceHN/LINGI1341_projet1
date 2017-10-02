#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

/*
*  @pre : nbr, le poiteur vers la valeur à récupérer
*  @post : return la valeur, convertie, lue de l'hexadécimal vers un long double
           return 0 si strlen(nbr) < 3
*/
long double convert(char *nbr)
{
  // "OX\0"
  if (strlen(nbr) < 3){
    return strtold(nbr, NULL);
  }

  /* on récupère les deux premières valeur de la chaine de caractères, donc '0'
   * et 'X' si c'est une valeur hexadécimale.
  */
  char a = *(nbr+0);
  char b = *(nbr+1);

  if(a == '0' && b == 'x'){
    char *value = (char *) malloc(sizeof(char) *(strlen(nbr)-2));
    if(!value) return strtold(nbr, NULL);
    // on ne sélectionne pas le 0X
    /* strncpy()  function  is similar to strcpy,
    except that at most n bytes of src are copied. */
    strncpy(value, nbr+2, strlen(nbr) - 2);
    /*
    If  endptr is not NULL, strtol() stores the address of the first invalid
    character in *endptr.  If there were no digits at all, strtol() stores the
    original value of nptr in *endptr (and returns 0).  In  particular,  if
    *nptr is not '\0' but **endptr is '\0' on return, the entire string is valid.

    The strtoll() function works just like the strtol() function but returns a
    long long integer value.
    */
    return strtoll(value, NULL, 16);
  }

  return strtold(nbr, NULL);

}

/*
calc [operations] [precision]
  -a ADD
  Ajoute ADD à la valeur courante
  -s SUB
  Soustrait SUB à la valeur courante
  -m MUL
  Multiplie la valeur courante par MUL
  -d DIV
  Divise la valeur courante par DIV
  -I
  Incrémente la valeur courante de 1
  -D
  Décrémente la valeur courante de 1
*/

int main(int argc, char *argv[])
{
  /* initialisation */
  int operation = 0;
  int precision = 0;
  long double nbr = 0;
  long double res = 0;

  /* use getopt to retrieve options :
        -a ADD
        -s SUB
        -m MUL
        -d DIV
        -I INCR
        -D DECR

    int getopt(int argc, char * const argv[],const char *optstring);
    If an option was successfully found, then getopt() returns the option
      character.  If all command-line options have been parsed, then
      getopt() returns -1.

      extern char *optarg;
      getopt() places a pointer to the following text in the same argv-element,
      or the text of the following argv-element, in optarg.
  */
  while((operation = getopt(argc, argv, ":a:s:m:d:ID")) != -1) {
    switch (operation){
      case 'a':
        res = convert(optarg); // char => long
        if(res == 0){
          exit(EXIT_FAILURE);
        }
        //ADD
        nbr += res;
        break;

      case 's':
        res = convert(optarg);
        if(res == 0){
          exit(EXIT_FAILURE);
        }
        //SUB
        nbr -= res;
        break;

      case 'm' :
        res = convert(optarg);
        if(res == 0){
          exit(EXIT_FAILURE);
        }
        //MUL
        nbr *= res;
        break;

      case 'd' :
        res = convert(optarg);
        if(res == 0){
          exit(EXIT_FAILURE);
        }
        //DIV
        nbr /= res;
        break;

      case 'I' : nbr++; //INCR
        break;
      case 'D' : nbr--; //DECR
        break;

      default: fprintf(stderr, "Error\n");
      exit(EXIT_FAILURE);

    }
  }

  /* extern int optind, The variable optind is the index of the next
    element of the argv[]
   retrieve the precision
    */
  if(optind < argc)
    precision = convert(argv[optind]);

  if(precision > 0){
    printf("%0.*Lf\n", precision, nbr);
    return EXIT_SUCCESS;
  }
  fprintf(stdout, "%lld\n", (long long) nbr);
  return EXIT_SUCCESS;

}
