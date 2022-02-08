#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int padding;
	char *image_handler;
	char *markup_handler;
	char *title_handler;
	char *color_handler;
} Configuration;

Configuration read_conf(FILE *conf_f) {
	Configuration ret;

	char *conf_c,
	     *conf_c_ptr,
	     *line,
	     *token;
	int c,
	    i = 0,
	    j = 0,
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
	}

	free(conf_c_ptr);
	return ret;
}

int main (int argc, char *argv[]) {
	Configuration conf = read_conf(fopen("prez.cfg", "rb+"));

	return 0;
}
