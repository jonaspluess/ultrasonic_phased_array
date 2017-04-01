/*******************************************************************************
 *
 * Copyright (C) 2016, Mijnssen Raphael
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
#include <math.h>

#define PROGRAMNAME "aperture_test"
#define VERSION "0.0"
#define COPYRIGHT "Copyright (C) 2016, Mijnssen Raphael"
#define LICENCE "Licence GPLv2+: GNU GPL version 2 or later <http://gnu.org/licences/gpl.html>.\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law."

#define APERTURE_RECT 0
#define APERTURE_COS 1
#define APERTURE_COS_2 2
#define APERTURE_GAUSS 3

#define STDEV_GAUSS 0.5

#define PWM_DUTY_MAX_P 262
#define PWM_DUTY_ZERO_P 525

static void usage(void);
static void version(void);
static float get_aperture(int aperture, int n, int max_n);
static int get_pwm_value(float aperture_relative);


int main(int argc, char **argv)
{
	int c, aperture, n, max_n;
	float d_lambda_ratio;

	while((c = getopt(argc, argv, "a:n:d:v")) != -1){
		switch(c){
			case 'a':
				aperture = (atoi(optarg));
				break;
			case 'n':
				max_n = (atoi(optarg));
				break;
			case 'd':
				d_lambda_ratio = (float)(atof(optarg));
				break;
			case 'v':
				version();
				exit(EXIT_SUCCESS);
			default:
				usage();
				exit(EXIT_FAILURE);
		}
	}

	if(argc < 3){
		fprintf(stderr, "Error: Not enough arguments.\n");
		usage();
		return EXIT_FAILURE;
	}

	printf("====================================================\n");
	printf("This program calculates weighting factors for 1xN-\n");
	printf("Arrays in order to achieve a desired aperture:\n");
	printf("	Rectangular aperture = 0\n");
	printf("	Cosine aperture = 1\n");
	printf("	Squared cosine aperture = 2\n");
	printf("	Gaussian aperture = 3\n");
	printf("====================================================\n");
	printf("Aperture is:\t%i\n", aperture);
	printf("Number of elements is:\t%i\n", max_n);
	printf("d/lambda ratio is:\t%3.2e\n", d_lambda_ratio);

	printf("====================================================\n");
	printf("N \tRealtive-Value \tPWM-Value\n");
	printf("====================================================\n");
	for(n = 1; n <= max_n; n++){
		printf("%i\t%3.2e\t%i\n", n, get_aperture(aperture,n,max_n),
		get_pwm_value(get_aperture(aperture,n,max_n)));
	}

	return EXIT_SUCCESS;
}

static void usage(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "   %s [options]\n", PROGRAMNAME);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -v - Show the version of this program.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Arguments:\n");
	fprintf(stderr, "  -a [aperture] - set the aperture.\n");
	fprintf(stderr, "	Rectangular aperture = 0\n");
	fprintf(stderr, "	Cosine aperture = 1\n");
	fprintf(stderr, "	Squared cosine aperture = 2\n");
	fprintf(stderr, "	Gaussian aperture = 3\n");
	fprintf(stderr, "  -n [number of elements] - set number of transmitters.\n");
	fprintf(stderr, "  -d [d to lambda ratio] - set ratio between element-\n");
	fprintf(stderr, "   spacing and wavelength (d/lambda)");
	fprintf(stderr, "\n");
}

static void version(void)
{
	printf("%s %s, compiled on %s, %s.\n", PROGRAMNAME, VERSION, __DATE__, __TIME__);
	printf("%s\n", COPYRIGHT);
	printf("%s\n", LICENCE);
}

static float get_aperture(int aperture, int n, int max_n)
{
	float result_n;
	float result_max;

	int index_at_max = (max_n+1)/2;

	switch(aperture){
		case APERTURE_RECT:
			return 1.0;
			break;
		case APERTURE_COS:
			result_n = sin(M_PI*n/((max_n+2)-1));
			result_max = sin(M_PI*index_at_max/((max_n+2)-1));
			return result_n*1.0/result_max;
			break;
		case APERTURE_COS_2:
			result_n = powf(sin(M_PI*n/((max_n+2)-1)), 2.0);
			result_max = powf(sin(M_PI*index_at_max/((max_n+2)-1)), 2.0);
			return result_n*1.0/result_max;
			break;
		case APERTURE_GAUSS:
			result_n = exp(-0.5*powf(((n-1)-(max_n-1)/2.0)/(STDEV_GAUSS*(max_n-1)/2.0), 2.0));
			result_max = exp(-0.5*powf(((index_at_max-1)-(max_n-1)/2.0)/(STDEV_GAUSS*(max_n-1)/2.0), 2.0));
			return result_n*1.0/result_max;
			break;
		default:
			break;
	}

	return 0;
}

static int get_pwm_value(float aperture_relative)
{
	//todo: PWM_DUTY_ZERO_P and PWM_DUTY_MAX_P - #defines in transmitter.c
	return PWM_DUTY_ZERO_P - (int) round(aperture_relative*(PWM_DUTY_ZERO_P-PWM_DUTY_MAX_P));
}

