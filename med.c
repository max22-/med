#ifdef USE_C_STDLIB
#include <stdio.h>
#include <string.h>
#endif
#include <ctype.h>

static char buffer[1024] = {0};
static int lines[256];
static int bi = 0, li = 0; /* buffer index, line index */
#define INC_BI                                                                 \
  {                                                                            \
    if (bi < sizeof(buffer) - 1)                                               \
      bi++;                                                                    \
    else                                                                       \
      ERROR;                                                                   \
  }
#define INC_LI                                                                 \
  {                                                                            \
    if (li < sizeof(lines) / sizeof(lines[0]) - 1)                             \
      li++;                                                                    \
    else                                                                       \
      ERROR;                                                                   \
  }

static char tib[256] = {0};
static int tptr;
#define INC_TPTR                                                               \
  {                                                                            \
    if (tptr < sizeof(tib) - 1)                                                \
      tptr++;                                                                  \
    else                                                                       \
      ERROR;                                                                   \
  }

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
#endif

#define ERROR                                                                  \
  {                                                                            \
    med_putchar('?');                                                          \
    med_putchar('\n');                                                         \
    state = COMMAND;                                                           \
  }

/************** Commands ****************/

void append() { state = TEXT; }

void display() {
  int i;
  for (i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
    if (lines[i] != -1)
      med_printf("%02d %s\n", i, &buffer[lines[i]]);
  }
}

void print_stack() {
  int i;
  med_putchar('[');
  for (i = 0; i < sptr; i++) {
    med_printi(stack[i]);
    if (i != sptr - 1)
      med_puts(", ");
  }
  med_puts("]\n");
}

void push(int n) {
  if (sptr >= sizeof(stack) / sizeof(stack[0])) {
    ERROR;
    return;
  }
  stack[sptr++] = n;
}

int pop() {
  if (sptr > 0)
    return stack[--sptr];
  else {
    ERROR;
  }
}

/*[[[cog
import cog

commands = {
  "a": "append",
  "%": "display",
  "stack": "print_stack"
}

cog.outl("int exec_cmd(char *cmd) {");
for (k, v) in commands.items():
  cog.out(f"""
  if(med_streq(cmd, "{k}")) {{
    {v}();
    return 1;
  }}""", trimblanklines=True)
cog.out("""
  ERROR;
  return 0;
}""", trimblanklines=True)

]]]*/
int exec_cmd(char *cmd) {
  if(med_streq(cmd, "a")) {
    append();
    return 1;
  }
  if(med_streq(cmd, "%")) {
    display();
    return 1;
  }
  if(med_streq(cmd, "stack")) {
    print_stack();
    return 1;
  }
  ERROR;
  return 0;
}
/*[[[end]]]*/

void parse() {
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

  lines[0] = 0; /* we create a first line starting a 0 in the buffer */
  for (i = 1; i < sizeof(lines) / sizeof(lines[0]); i++)
    lines[i] = -1;

  while (!quit) {
    c = med_getchar();
    if (c == med_EOF)
      quit = 1;

    switch (state) {
    case COMMAND:
      if (c != '\n' && c != '\r') {
        tib[tptr] = c;
        INC_TPTR;
      }

      if (c == '\n') {
        tib[tptr] = 0;
        INC_TPTR;
        if (tptr > 1)
          parse();
        tptr = 0;
      }
      break;

    case TEXT:
      printf("c='%c'\n", c);
      if (c == '\n') {
        buffer[bi] = 0;
        INC_BI
        printf("line = \"%s\"\n", &buffer[lines[li]]);
        if(med_streq(&buffer[lines[li]], ".")) {
          state = COMMAND;
        } else {
          INC_LI
          lines[li] = bi;
        }
      }
      else
        buffer[bi++] = c;
      break;
    }
  }

  return 0;
}
