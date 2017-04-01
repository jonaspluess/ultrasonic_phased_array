/*******************************************************************************
 *
 * Copyright (C) 2016, Pluess Jonas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 ******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>


#define PROGRAMNAME "sampling_calculator"
#define VERSION "0.0"
#define COPYRIGHT "Copyright (C) 2016, Pluess Jonas"
#define LICENCE "Licence GPLv2+: GNU GPL version 2 or later <http://gnu.org/licences/gpl.html>.\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law."


static void usage(void);
static void version(void);


int main(int argc, char **argv)
{
	int c, n, max_n;
	float lower_freq, upper_freq, bandwidth;

	while((c = getopt(argc, argv, "u:l:hv")) != -1){
		switch(c){
			case 'u':
				upper_freq = (float)(atof(optarg));
				break;
			case 'l':
				lower_freq = (float)(atof(optarg));
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				version();
				exit(EXIT_SUCCESS);
			case '?':
				if(optopt == '-'){
					fprintf(stderr, "This program does not support longopts.\n");
				}
			default:
				usage();
				exit(EXIT_FAILURE);
		}
	}

	if(argc < 3){
		fprintf(stderr, "Error: Not enough arguments. You have to specify the upper and the lower frequency.\n");
		usage();
		return EXIT_FAILURE;
	}else if(upper_freq < lower_freq){
		fprintf(stderr, "Error: The upper frequency has to be bigger than the lower frequency.\n");
		usage();
		return EXIT_FAILURE;
	}

	bandwidth = upper_freq - lower_freq;
	max_n = upper_freq / bandwidth;

	printf("====================================================\n");
	printf("This program calculates all possible sampling rates.\n");
	printf("Lower Frequency is:\t%e\n", lower_freq);
	printf("Upper Frequency is:\t%e\n", upper_freq);
	printf("Bandwidth is:\t\t%e\n", bandwidth);
	printf("====================================================\n");
	printf("N \tMinimal \tMaximal \tOptimal\n");
	printf("====================================================\n");

	for(n = 1; n <= max_n; n++){
		if(n == 1){
			printf("%i\t%e\tInfinity\t%e\n", n, 2.0 * upper_freq / n, 2.0 * (upper_freq + lower_freq) / (2.0 * n - 1.0));
		}else{
			printf("%i\t%e\t%e\t%e\n", n, 2.0 * upper_freq / n, 2.0 * lower_freq / (n - 1.0), 2.0 * (upper_freq + lower_freq) / (2.0 * n - 1.0));
		}
	}

	return EXIT_SUCCESS;
}

static void usage(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "   %s [options] DEVICE\n", PROGRAMNAME);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h - Show this help text.\n");
	fprintf(stderr, "  -v - Show the version of this program.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Arguments:\n");
	fprintf(stderr, "  -l [lower_frequency] - set the lower frequency.\n");
	fprintf(stderr, "  -u [upper_frequency] - set the upper frequency.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Formulas:\n");
	fprintf(stderr, "                                                    \n");
	fprintf(stderr, "                 upper_freq            upper_freq   \n");
	fprintf(stderr, "  max_N = ------------------------- = ------------  \n");
	fprintf(stderr, "           upper_freq - lower_freq     bandwidth    \n");
	fprintf(stderr, "                                                    \n");
	fprintf(stderr, "                                                    \n");
	fprintf(stderr, "              2 * upper_freq                        \n");
	fprintf(stderr, "  min_freq = ----------------                       \n");
	fprintf(stderr, "                     n                              \n");
	fprintf(stderr, "                                                    \n");
	fprintf(stderr, "                                                    \n");
	fprintf(stderr, "              2 * lower_freq                        \n");
	fprintf(stderr, "  max_freq = ----------------                       \n");
	fprintf(stderr, "                  n - 1                             \n");
	fprintf(stderr, "                                                    \n");
	fprintf(stderr, "                                                    \n");
	fprintf(stderr, "              2 * (upper_freq + lower_freq)         \n");
	fprintf(stderr, "  opt_freq = ------------------------------         \n");
	fprintf(stderr, "                      2 * n - 1                     \n");
	fprintf(stderr, "                                                    \n");
}

static void version(void)
{
	printf("%s %s, compiled on %s, %s.\n", PROGRAMNAME, VERSION, __DATE__, __TIME__);
	printf("%s\n", COPYRIGHT);
	printf("%s\n", LICENCE);
}
