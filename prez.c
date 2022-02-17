#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "chars.h"

typedef struct {
	bool in_paragraph;
	bool bold;
	bool cursive;
	bool header;
	bool link;
	bool image;
} CharInfo;

typedef struct {
	int padding;
	int header_height;
	char *image_handler;
	char *header_handler;
} Configuration;

typedef struct {
	int x;
	int y;
} Position;

typedef struct {
	int x;
	int y;
} winsz;

/* reads configuration from conf_f
 * will set defaults if conf_f == NULL
 */
Configuration read_conf(FILE *conf_f) {
	Configuration ret;

	char *conf_c,
	     *conf_c_ptr,
	     *line,
	     *token,
	     *tmp;
	int c,
	    i = 0,
	    sz,
	    lines = 0,
	    curr_line_n = 0,
	    line_len;

	ret.padding = 5;
	ret.header_height = 8;
	ret.image_handler = NULL;
	ret.header_handler = "prez";

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
				} else if (strcmp(token, "header_height") == 0) {
					ret.header_height = atoi(strtok(NULL, "\n"));
				} else if (strcmp(token, "image_handler") == 0) {
					ret.image_handler = NULL;
					tmp = NULL;
					tmp = strtok(NULL, "\n");
					ret.image_handler = malloc(sizeof(char) * (strlen(tmp) + 1));
					strcpy(ret.image_handler, tmp);
				} else if (strcmp(token, "header_handler") == 0) {
					ret.header_handler = NULL;
					tmp = NULL;
					tmp = strtok(NULL, "\n");
					ret.header_handler = malloc(sizeof(char) * (strlen(tmp) + 1));
					strcpy(ret.header_handler, tmp);
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

/* clears screen */
void cls() {
	printf("\033c");
}

/* set cursor to x, y 
 * should be used as a helper function
 */
void goto_xy(int x, int y) {
	printf("\033[%d;%dH", y, x);
}

/* gets window size
 * should be used as a helper
 * function
 */
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

Position header_write_ch(Position cur_pos, Configuration cnf, char ch) {
	Position ret = {cur_pos.x, cur_pos.y};
	int i, x = cur_pos.x, y = cur_pos.y;

	if (isspace(ch)) {
		ret.x += 8;
		return ret;
	}

	if (strcmp(cnf.header_handler, "prez") == 0) {
		int pos_in_chars = 0;
		if (ch < 65) {
			pos_in_chars = 0;
		} else if (ch > 96 && ch < 122) {
			pos_in_chars = ch - 65 - 32;
		} else if (ch > 40 && ch < 91) {
			pos_in_chars = ch - 65;
			/* ascii(7) */
		}

		for (i = 0; i < 8; i++) {
			goto_xy(cur_pos.x, y);
			printf("%s", big_chars[pos_in_chars][i]);
			y ++;
		}


		ret.x += 8;
	} else {
		bool _run = true;
		FILE *handler_f;
		int c,
		    max_x = x;
		char *command = malloc(sizeof(char) * (strlen(cnf.header_handler) + 3));
		/*                                                                 ~~~ */
		/*   strlen(" a"), where 'a' is a random letter; + space for NULL byte */

		snprintf(command, strlen(cnf.header_handler) + 3,"%s %c", cnf.header_handler, ch);

		handler_f = popen(command, "r");

		if (handler_f == NULL) {
			cls();
			goto_xy(0,0);
			printf("Error running %s. Exiting...\n", command);
			exit(1);
		}

		i = 0;
		while (_run) {
			c = fgetc(handler_f);
			switch (c) {
				case '\n':
					y ++;
					max_x = (x > max_x) ? x : max_x;
					x = cur_pos.x;
					break;
				case EOF:
					_run = false;
					break;
				default:
					goto_xy(x, y);
					fputc(c, stdout);
					x ++;
					break;
			}
		}

		ret.x = max_x;

		pclose(handler_f);
		free(command);
	}

	return ret;
}

Position write_ch(Position cur_pos, Configuration cnf, CharInfo cinfo, char ch) {
	Position ret = {cur_pos.x, cur_pos.y};
	if (cur_pos.x > (getwinsz().x - cnf.padding)) {
		ret.x = cnf.padding;
		ret.y ++;
	}

	goto_xy(ret.x, ret.y);
	ret.x ++;

	if (cinfo.header) {
		ret = header_write_ch(ret, cnf, ch);
	} else {
		fputc(ch, stdout);
	}

	return ret;
}

Position render_img(Position cur_pos, Configuration cnf, char *path) {
	Position ret = {cur_pos.x, cur_pos.y};

	int c;
	FILE *pipe;
	char *tmp = malloc(sizeof(char) * 1024),
	     *cmd = malloc(sizeof(char) * (strlen(path) + strlen(cnf.image_handler) + 2));
	/* 2 because space between the two args and \0 at the end */

	sprintf(cmd, "%s %s", cnf.image_handler, path);

	if (strcmp(cnf.image_handler, "prez") == 0) {
		printf("[IMAGE: %s]", path);
		ret.x = cnf.padding;
		ret.y ++;
	} else {
		ret.x = cnf.padding;
		goto_xy(ret.x, ret.y);
		pipe = popen(cmd, "r");

		while (true) {
			c = fgetc(pipe);
			if (c == EOF) break;
			/* end at EOF */

			fputc(c, stdout);
			/* else, print ch */
			ret.y = (c == '\n') ? ret.y + 1 : ret.y;
			if (c == '\n') goto_xy(ret.x, ret.y);
			/* add padding on newline */
		}

		pclose(pipe);
	}

	free(tmp);
	free(cmd);
	return ret;
}

int main (int argc, char *argv[]) {
	FILE *in_real,
	     *in = tmpfile();

	char *cfg = NULL,
	     *text,
	     *_img_fname;

	int opt,
	    c,
	    i,
	    j,
	    sz,
	    _img_ln_len;

	bool _run = true,
	     quit = false,
	     _found_sld;


	CharInfo chr = {false, false, false, false, false, false};

	Position pos = {0, 0};

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
		printf("usage: prez [-c config] file.prez\n");
		exit(1);
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

	cls();
	printf("\033[?25l"); /* hide the cursor */

	Configuration conf = read_conf(fopen(cfg, "rb+"));
	i = 0;

	pos.x += conf.padding;
	pos.y += conf.padding;
	while (i < sz) {
		if (text[i] == '~') {
			i ++;
			switch (text[i]) {
				case 'i':
					j = i;
					_img_ln_len = 0;
					_img_fname = NULL;

					while (!(text[j] == '~' && text[j+1] == 'I') && (j < sz)) {
						_img_ln_len ++;
						j ++;
					}

					_img_fname = malloc(sizeof(char) * (_img_ln_len + 1));
					for (j = 0; j < _img_ln_len + 1; j++) _img_fname[j] = '\0';

					for (j = 0; j < _img_ln_len-1; j++) {
						i ++;
						_img_fname[j] = text[i];
					}
					i += 2;

					pos = render_img(pos, conf, _img_fname);


					free(_img_fname);
					break;
				case 'p':
					if (chr.in_paragraph) {
						/* starting a paragraph in paragraph?
						 * weird
						 */
						break;
					}

					pos.x += (conf.padding) / 2;
					chr.in_paragraph = true;
					
					break;
				case 'P':
					if (chr.in_paragraph != true) {
						/* something wring with syntax
						 * ending paragraph without starting it
						 */
						break;
					}

					chr.in_paragraph = false;
					pos.x = conf.padding;
					pos.y ++;

					break;
				case 'h':
					chr.header = true;
					break;
				case 'H':
					chr.header = false;
					pos.y += conf.header_height;
					break;
				case 'b':
					chr.bold = true;
					printf("\033[1m");
					break;
				case 'B':
					chr.bold = false;
					printf("\033[22m");
					break;
				case 'c':
					chr.cursive = true;
					printf("\033[3m");
					break;
				case 'C':
					chr.cursive = false;
					printf("\033[23m");
					break;
				case '-':
					_run = true;
					_found_sld = false;
					while (_run) {
						switch (quiet_getch()) {
							case ' ':
							case 'l':
								pos.x = conf.padding;
								pos.y = conf.padding;

								cls();
								_run = false;
								break;
							case 'h':
								for (j = 0; j < 2; j ++) {
									_found_sld = false;
									i -= 2;
									while (_found_sld == false && i >= 0) {
										i --;
										if (text[i] == '~' && text[i+1] == '-') {
											_found_sld = true;
										}
									}
								}

								pos.x = conf.padding;
								pos.y = conf.padding;

								/*i ++;*/

								cls();

								_run = false;

								break;
							case 'q':
								i = sz;
								_run = false;
								quit = true;

								break;
						}
					}
					break;
				default:
					cls();
					printf("something is wrong with your syntax near ~%c\n", text[i]);
					break;
			}
		} else if (text[i] == '\n') {
			pos.x = conf.padding;
			pos.y ++;
		} else {
			pos = write_ch(pos, conf, chr, text[i]);
		}

		i ++;
	}

	while (true && quit == false) {
		if (quiet_getch() == 'q') {
			break;
		}
	}


	cls();
	goto_xy(0, 0);
	printf("\033[?25h");
	free(text);
	fclose(in);
	fclose(in_real);
	return 0;
}
