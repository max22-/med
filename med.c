#ifdef USE_C_STDLIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include <ctype.h>

static char buffer[10240] = {0};
static char buffer2[10240] = {0};
static int lines[1024];
static int bi = 0, bi2 = 0; /* buffer index, buffer2 index */

#define ITEMS(x) (sizeof(x) / sizeof(x[0]))
#define INC_PTR(dat, ptr) { if (ptr < ITEMS(dat) - 1) ptr++; else ERROR; }
#define DEC_PTR(ptr) { if (ptr > 0) ptr--; else ERROR; }

static char tib[256] = {0};
static int tptr;

enum { COMMAND, TEXT } state = COMMAND;

static int stack[256] = {0}, sptr = 0;

#ifdef USE_C_STDLIB
#define med_getchar getchar
#define med_putchar putchar
#define med_puts(x) printf("%s", x)
#define med_EOF EOF
#define med_printi(x) printf("%d", x)
#define med_printf(...) printf(__VA_ARGS__)
#define med_streq !strcmp
#define med_exit exit
#define med_fopen fopen
#define med_fwrite fwrite
#define med_fclose fclose
#define med_fgetc fgetc
#define med_quit exit
#endif

#define ERROR                                                                  \
  {                                                                            \
    med_putchar('?');                                                          \
    med_putchar('\n');                                                         \
    state = COMMAND;                                                           \
  }

static int med_input_token(char *buffer, int size)
{
  int ptr = 0;
  int c;
  do {
    c = med_getchar();
    if(c == med_EOF)
      exit(0);
    if(!isspace(c)) {
      buffer[ptr] = c;
      if(ptr < size - 1)
        ptr++;
      else {
        ERROR;
        return 0;
      }
    }
  } while (!isspace(c));
  buffer[ptr] = 0;
  return 1;
}

static void push(int n) {
  if (sptr >= ITEMS(stack)) {
    ERROR;
    return;
  }
  stack[sptr++] = n;
}

static int pop() {
  if (sptr > 0)
    return stack[--sptr];
  else {
    ERROR;
    med_exit(1);
  }
}

/*********** Commands ***********/

static void append() { state = TEXT; }

static void display() {
  int i;
  for (i = 0; i < ITEMS(lines); i++) {
    if (lines[i] != -1)
      med_printf("%3d  %s\n", i, &buffer[lines[i]]);
  }
}

static void print_stack() {
  int i;
  med_putchar('[');
  for (i = 0; i < sptr; i++) {
    med_printi(stack[i]);
    if (i != sptr - 1)
      med_puts(", ");
  }
  med_puts("]\n");
}

static void swap() {
  int a, b, t;
  if (sptr < 2) {
    ERROR;
    return;
  }
  a = pop();
  b = pop();
  t = lines[a];
  lines[a] = lines[b];
  lines[b] = t;
}

static void delete () {
  int l, i;
  if (sptr < 1) {
    ERROR;
    return;
  }
  l = pop();
  if (l < 0 || lines[l] == -1) {
    ERROR;
    return;
  }
  for (i = l; i < ITEMS(lines) - 1; i++)
    lines[i] = lines[i + 1];
  lines[ITEMS(lines) - 1] = -1;
}

static void pack()
{
  int i, j;
  char c;
  bi2 = 0;
  for(i = 0; lines[i] != -1; i++) {
    for(j = 0; (c = *(&buffer[lines[i]] + j)) != 0; j++) {
      buffer2[bi2] = c;
      INC_PTR(buffer2, bi2);
    }
    buffer2[bi2] = '\n';
    INC_PTR(buffer2, bi2);
  }
  if(bi2 > 1) {
    buffer2[bi2-1] = 0;
    DEC_PTR(bi2);
  }
}

static void unpack()
{
  int i;
  char c;
  int li = 0;
  bi = 0;
  for(i = 0; i < sizeof(buffer); i++)
    buffer[i] = 0;
  lines[0] = 0;
  for(i = 1; i < ITEMS(lines); i++)
    lines[i] = -1;
  for(i = 0; i < bi2; i++) {
    c = buffer2[i];
    if(c == '\n') {
      buffer[bi] = 0;
      INC_PTR(buffer, bi);
      INC_PTR(lines, li);
      lines[li] = bi;
    } else if (c != '\r')
      buffer[bi++] = c;
  }
}

static void save() {
  char file_path[256];
  FILE *f;
  med_puts("path: ");
  med_input_token(file_path, sizeof(file_path));
  pack();
  f = med_fopen(file_path, "w");
  if(f != NULL) {
    med_fwrite(buffer2, bi2, 1, f);
    fclose(f);
  } else ERROR;
}

static int open() {
  char file_path[256];
  FILE *f;
  int c;
  bi2 = 0;
  med_puts("path: ");
  med_input_token(file_path, sizeof(file_path));
  f = med_fopen(file_path, "r");
  if(f != NULL) {
    do {
      c = med_fgetc(f);
      if(c != med_EOF) {
        buffer2[bi2] = c;
        INC_PTR(buffer2, bi2);
      }
    } while(c != med_EOF);
  } else {
    ERROR;
    return 0;
  }
  unpack();
  return 1;
}

void new()
{
  int i;
  bi = 0;
  bi2 = 0;
  for(i = 0; i < sizeof(buffer); i++)
    buffer[i] = 0;
  lines[0] = 0;
  for(i = 1; i < ITEMS(lines); i++)
    lines[i] = -1;
}

void quit()
{
  med_quit(0);
}

/*********** End of commands ***********/

/*[[[cog
import cog

commands = {
  "a": "append",
  "%": "display",
  "stack": "print_stack",
  "swp": "swap",
  "d": "delete",
  "pack": "pack",
  "unpack": "unpack",
  "s": "save",
  "o": "open",
  "n": "new",
  "q": "quit"
  
}

cog.outl("int exec_cmd(char *cmd) {");
for (k, v) in commands.items():
  cog.out(f"""
  if (med_streq(cmd, "{k}")) {{
    {v}();
    return 1;
  }}""", trimblanklines=True)
cog.out("""
  ERROR;
  return 0;
}""", trimblanklines=True)

]]]*/
int exec_cmd(char *cmd) {
  if (med_streq(cmd, "a")) {
    append();
    return 1;
  }
  if (med_streq(cmd, "%")) {
    display();
    return 1;
  }
  if (med_streq(cmd, "stack")) {
    print_stack();
    return 1;
  }
  if (med_streq(cmd, "swp")) {
    swap();
    return 1;
  }
  if (med_streq(cmd, "d")) {
    delete();
    return 1;
  }
  if (med_streq(cmd, "pack")) {
    pack();
    return 1;
  }
  if (med_streq(cmd, "unpack")) {
    unpack();
    return 1;
  }
  if (med_streq(cmd, "s")) {
    save();
    return 1;
  }
  if (med_streq(cmd, "o")) {
    open();
    return 1;
  }
  if (med_streq(cmd, "n")) {
    new();
    return 1;
  }
  if (med_streq(cmd, "q")) {
    quit();
    return 1;
  }
  ERROR;
  return 0;
}
/*[[[end]]]*/

static void parse() {
  char c = tib[0];
  int i, number_flag = 1, n = 0;
  if (c == '-' || isdigit(c)) {
    for (i = 1; i < tptr - 1; i++) {
      if (!isdigit(tib[i])) {
        number_flag = 0;
        break;
      }
    }
  } else
    number_flag = 0;
  if (number_flag) {
    for (i = 0; i < tptr; i++)
      if (isdigit(tib[i]))
        n = n * 10 + tib[i] - '0';
    if (tib[0] == '-')
      n *= -1;
    push(n);
  } else
    exec_cmd(tib);
}

int main(int argc, char *argv[]) {
  int i, c, quit = 0;
  int li = 0;

  lines[0] = 0; /* we create a first line starting a 0 in the buffer */
  for (i = 1; i < ITEMS(lines); i++)
    lines[i] = -1;

  while (!quit) {
    c = med_getchar();
    if (c == med_EOF)
      quit = 1;

    switch (state) {
    case COMMAND:
      if (!isspace(c)) {
        tib[tptr] = c;
        INC_PTR(tib, tptr);
      } else {
        tib[tptr] = 0;
        INC_PTR(tib, tptr);
        if (tptr > 1)
          parse();
        tptr = 0;
        if (c == '\n')
          print_stack();
      }
      break;

    case TEXT:
      if (c == '\n') {
        buffer[bi] = 0;
        INC_PTR(buffer, bi);
        if (med_streq(&buffer[lines[li]], ".")) {
          lines[li] = bi; /* we skip this line containing only a dot */
          state = COMMAND;
        } else {
          INC_PTR(lines, li);
          lines[li] = bi;
        }
      } else if (c != '\r')
        buffer[bi++] = c;
      break;
    }
  }

  return 0;
}
