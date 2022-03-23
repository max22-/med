#ifdef USE_C_STDLIB
#include <stdio.h>
#endif
#include <ctype.h>

static char buffer[1024];
static int lines[256];
static int bi = 0, li = 0;  /* buffer index, line index */
#define INC_BI { if(bi < sizeof(buffer) - 1) bi++; else ERROR; }
#define INC_LI { if(li < sizeof(lines) / sizeof(lines[0]) - 1) li++; else ERROR; }

enum {COMMAND, NUMBER, TEXT} state = COMMAND;

static int stack[256] = {0}, sptr = 0;

#ifdef USE_C_STDLIB
#define med_getchar getchar
#define med_putchar putchar
#define med_puts(x) printf("%s", x)
#define med_EOF EOF
#define med_printi(x) printf("%d", x)
#define med_printf(...) printf(__VA_ARGS__)
#endif

#define ERROR { med_putchar('?'); med_putchar('\n'); state = COMMAND; }

void display()
{
  int i;
  for(i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
    if(lines[i] != -1)
      med_printf("%02d %s\n", i, &buffer[lines[i]]);
  }
}

void push_digit(unsigned char d)
{
  if(sptr >= sizeof(stack) / sizeof(stack[0])) {
    ERROR;
    return;
  }
  stack[sptr] = stack[sptr] * 10 + d - '0';
}

void print_stack()
{
  int i;
  med_putchar('[');
  for(i = 0; i < sptr; i++) {
    med_printi(stack[i]);
    if(i != sptr - 1)
      med_puts(", ");
  }
  med_puts("]\n");
}

int main(int argc, char *argv[])
{
  int i, c, quit = 0;

  lines[0] = 0;  /* we create a first line starting a 0 in the buffer */
  for(i = 1; i < sizeof(lines) / sizeof(lines[0]); i++)
    lines[i] = -1;
  
  while(!quit) {
    
    c = med_getchar();
    if(c == med_EOF)
      quit = 1;
    
    switch(state) {
    case COMMAND:
      if(isdigit(c)) {
	push_digit(c);
	state = NUMBER;
	break;
      }
      switch(c) {
      case 'a':
	state = TEXT;
	med_getchar();
	break;
      case '%':
	display();
	break;
      case '\n':
	print_stack();
	break;
      default:
	printf("?\n");
	break;
      }
      break;
      
    case NUMBER:
      if(isdigit(c))
	push_digit(c);
      else if(isspace(c)) {
	sptr++;
	print_stack();
	state = COMMAND;
      }
      else {
	stack[sptr] = 0;
	ERROR;
      }
      break;
      
    case TEXT:
      if(c == '\n') {
	buffer[bi] = 0;
	INC_BI
	lines[li] = bi;
	INC_LI
      } else if(c == '.')
	state = COMMAND;
      else	
	buffer[bi++] = c;
      break;
    }
    
  }
  
  return 0;
}
