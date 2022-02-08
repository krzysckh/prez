#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
	int padding;
	char *image_handler;
	char *markup_handler;
	char *title_handler;
	char *color_handler;
} Configuration;

typedef struct {
	int x;
	int y;
} winsz;

Configuration read_conf(FILE *conf_f) {
	Configuration ret;

	char *conf_c,
	     *conf_c_ptr,
	     *line,
	     *token;
	int c,
	    i = 0,
	    sz,
	    lines = 0,
	    curr_line_n = 0,
	    line_len;

	ret.padding = 5;
	ret.image_handler = NULL;
	ret.markup_handler = "prez";
	ret.title_handler = "prez";
	ret.color_handler = "prez";

	if (conf_f != NULL) {
		fseek(conf_f, 0L, SEEK_END);
		sz = ftell(conf_f);
		fseek(conf_f, 0L, SEEK_SET);

		conf_c = malloc(sizeof(char) * sz);
		conf_c_ptr = conf_c;

		while ((c = fgetc(conf_f)) != EOF) {
			if (c == '\n') lines ++;
			conf_c[i] = c;
			i ++;
		}

		for (curr_line_n = 0; curr_line_n < lines; curr_line_n ++) {
			i = 0;
			line_len = 0;

			while (conf_c[i] != '\n') {
				line_len ++;
				i ++;
			}
			
			if (line_len > 0) {
				line = malloc(sizeof(char) * (line_len + 1));
				strncpy(line, conf_c, line_len);

				token = strtok(line, "\t");

				if (strcmp(token, "padding") == 0) {
					ret.padding = atoi(strtok(NULL, "\n"));
				} else if (strcmp(token, "image_handler") == 0) {
					ret.image_handler = strtok(NULL, "\n");
				} else if (strcmp(token, "markup_handler") == 0) {
					ret.markup_handler = strtok(NULL, "\n");
				} else if (strcmp(token, "title_handler") == 0) {
					ret.title_handler = strtok(NULL, "\n");
				} else if (strcmp(token, "color_handler") == 0) {
					ret.color_handler = strtok(NULL, "\n");
				} else {
					printf("prez: config syntax error!\n %d | %s\n\nunrecognised token %s\n",
						curr_line_n+1, line, token);
				}

				free(line);
			}
			conf_c += line_len + 1;

		}
	} else {
		fprintf(stderr, "prez: warning: could not read configuration file\n");
		return ret;
	}

	free(conf_c_ptr);
	return ret;
}

void cls() {
	printf("\033c");
}

void goto_xy(int x, int y) {
	printf("\033[%d;%dH", y, x);
}

winsz getwinsz() {
	winsz ret;

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	ret.x = w.ws_col;
	ret.y = w.ws_row;
	return ret;
}

int quiet_getch() {
	/* this function is taken from
	 * nocurses.h librarys getche() function
	 * written by Rahul M. Juliato
	 * under the GPL-2.0 license
	 */

	struct termios oldattr, newattr;
	tcgetattr(STDIN_FILENO, &oldattr);

	newattr = oldattr;
	newattr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newattr);

	int ch = getc(stdin);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);

	return ch;
}

int main (int argc, char *argv[]) {
	FILE *in_real,
	     *in = tmpfile();

	char *cfg = NULL,
	     *text;

	int opt,
	    c,
	    i,
	    sz;

	while ((opt = getopt(argc, argv, "hc:")) != -1) {
		switch (opt) {
			case 'h':
				printf("usage: prez [-c config] file.prez\n");
				break;
			case 'c':
				cfg = optarg;
				break;
		}
	}

	if (cfg == NULL) {
		cfg = malloc(sizeof(char) * (strlen(getenv("HOME")) + 1) \
			       	+ (strlen("/.config/prez/prez.cfg")));
		sprintf(cfg, "%s/.config/prez/prez.cfg", getenv("HOME"));
	}

	if (argv[optind] != NULL) {
		in_real = fopen(argv[optind], "rb+");
	} else {
		in_real = stdin;
	}

	while ((c = fgetc(in_real)) != EOF) {
		fputc(c, in);
	}

	fseek(in, 0L, SEEK_END);
	sz = ftell(in);
	fseek(in, 0L, SEEK_SET);

	text = malloc(sizeof(char) * sz);

	for (i = 0; i < sz; i++) {
		text[i] = fgetc(in);
	}

	Configuration conf = read_conf(fopen(cfg, "rb+"));

	free(text);
	fclose(in);
	fclose(in_real);
	return 0;
}
