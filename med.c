#ifdef USE_C_STDLIB
#include <stdio.h>
#endif

char buffer[1024];
int lines[256];
int bi, li;  /* buffer index, line index */
#define INC_BI { if(bi < sizeof(buffer) - 1) bi++; else { med_putchar('?'); state = COMMAND; } }
#define INC_LI { if(li < sizeof(lines) / sizeof(lines[0]) - 1) li++; else { med_putchar('?'); state = COMMAND; } }

enum editor_state {COMMAND, TEXT};

#ifdef USE_C_STDLIB
#define med_getchar getchar
#define med_putchar putchar
#define med_EOF EOF
#define med_printf(...) printf(__VA_ARGS__)
#endif

void med_display()
{
  int i;
  for(i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
    if(lines[i] != -1)
      med_printf("%02d %s\n", i, &buffer[lines[i]]);
  }
}

int main(int argc, char *argv[])
{
  enum editor_state state = COMMAND;
  int i, c, quit = 0;

  bi = 0;
  li = 0;
  lines[0] = 0;  /* we create a first line starting a 0 in the buffer */
  for(i = 1; i < sizeof(lines) / sizeof(lines[0]); i++) {
    lines[i] = -1;
  }
  
  while(!quit) {
    c = med_getchar();
    if(c == med_EOF)
      quit = 1;
    switch(state) {
    case COMMAND:
      switch(c) {
      case 'a':
	state = TEXT;
	med_getchar();
	break;
      case '%':
	med_display();
	break;
      case '\n':
	break;
      default:
	printf("?\n");
	break;
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
