/// 
/// This program is an algorithm that will determine a location for an agent to move to based on the agents around them.
/// These agents prefer either putting a { at the end of a line, or on a new line on its own.
/// The results from the program will either be printed to the screen in a regular iterative method, or it will
/// 	be printed using the curses mode that was shown in a previous homework. 
/// The way to tell how it will be printed is based on the number of cycles wanted. If there is no cycles input,
/// 	then it defaults to infinite mode. If it is specified, then it will run that many times.
/// The user can pass in the data that is required, or it can leave it to the default values made by the program.
///
/// @author Giovanni Coppola (gac6151@rit.edu)
///

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> // uses usleep as the sleep function when needed
#include <ncurses.h> // uses the curses mode

// Default values for if the user chooses to not enter the data for the given variable
#define DEFAULT_MIN_TIME 900000
#define DEFAULT_MIN_CYCLES -1
#define DEFAULT_DIMENSION 15
#define DEFAULT_STRENGTH 50
#define DEFAULT_VACANCIES 20
#define DEFAULT_ENDLINES 60

// Function Prototypes:
void FillTheGrid(int vacancies, int endlines, int size, char grid[]);
void ConvertToMatrix(int size, char grid[], char matrix[][size]);
double HappinessMeasure(int row, int col, int size, char matrix[][size], char test);
int MoveTheAgent(int row, int col, int size, char matrix[][size]);
void ConvertToGrid(int size, char grid[], char matrix[][size]);
int OpenPreviousVacancies(int size, char matrix[][size]);
double CityHappiness(int size, char matrix[][size]);
void CreateNextCycle(int size, char matrix[][size], double strength);
void PrintCity(int size, char matrix[][size], int cycle, int moves, double happiness, int strength, int vacancy, int endline);
void PrintCityUsingCursors(int size, char matrix[][size], int cycle, int moves, double happiness, int strength, int vacancy, int endline);
void PrintHelp();
void PrintUsage();

int main(int argc, char* argv[]) {
	// Integer for the opt argument truth value
	int opt;

	// Integer values that will determine how the algorithm operates and changes
	// 	sleep time will be a long int since it is in microseconds
	long int delayTime = DEFAULT_MIN_TIME;
	int numOfCycles = DEFAULT_MIN_CYCLES;
	int dimensionOfGrid = DEFAULT_DIMENSION;
	int totalSizeOfGrid = dimensionOfGrid*dimensionOfGrid;
	int strengthOfPreference = DEFAULT_STRENGTH;
	int vacancy = DEFAULT_VACANCIES;
	int endlineAgents = DEFAULT_ENDLINES;
	int tempValue;

	// Set the seed value for the srandom function
	srandom( 41 );

	// While loop to go through each of the command line arguments
	while ( (opt = getopt(argc,argv,"ht:c:d:s:v:e:") ) != -1) {
		switch(opt) {
		case 'h':	
			// Display the help message for the user
			PrintHelp();
			return(EXIT_FAILURE);
			break;
		case 't':
			// Set the current time delay, if there is a valid time delay passed in
			tempValue = (long int)strtol(optarg, NULL, 10);
			if (tempValue > 0) {
			       delayTime = tempValue;
			}
			break;
		case 'c':
			// Set the current number of cycles, if there is a valid one passed in
			tempValue = (int)strtol(optarg, NULL, 10);
			if (tempValue >= 0) {
				numOfCycles = tempValue;
			} else {
				fprintf(stderr, "count (%d) must be a non-negative integer.", tempValue);
				PrintUsage();
				return(EXIT_FAILURE);
			}
			break;
		case 'd':
			// Set the current dimensions for the grid of the city so that it can be a matrix
			tempValue = (int)strtol(optarg, NULL, 10);
			if (tempValue >=5 && tempValue <= 39) {
				dimensionOfGrid = tempValue;
			} else {
				fprintf(stderr, "dimension (%d) must be a value in [5...39]\n", tempValue);
				PrintUsage();
				return(EXIT_FAILURE);
			}
			break;
		case 's':
			// Set the current stength of preference percentage
			tempValue = (int)strtol(optarg, NULL, 10);
			if (tempValue >= 1 && tempValue <= 99) {
				strengthOfPreference = tempValue;
			} else {
				fprintf(stderr, "preference strength (%d) must be a value in [1...99]", tempValue);
				PrintUsage();
				return(EXIT_FAILURE);
			}
			break;
		case 'v':
			// Set the current vacancy percentage
			tempValue = (int)strtol(optarg, NULL, 10);
			if (tempValue >= 1 && tempValue <= 99) {
				vacancy = tempValue;
			} else {
				fprintf(stderr, "vacancy (%d) must be a value in [1...99]", tempValue);
				PrintUsage();
				return(EXIT_FAILURE);
			}
		case 'e':
			// Set the current endline percentage
			tempValue = (int)strtol(optarg, NULL, 10);
			if (tempValue >=1 && tempValue <= 99) {
				endlineAgents = tempValue;
			} else {
				fprintf(stderr, "endline proportion (%d) must be a value in [1...99]", tempValue);
				PrintUsage();
				return(EXIT_FAILURE);
			}
			break;
		case ':':
			// If the command line is missing an argument, print the usage message and return with failure
			PrintUsage();
			return(EXIT_FAILURE);
			break;
		case '?':
		default:
			// Display the usage message and exit the code with failure
			PrintUsage();
			return(EXIT_FAILURE);
			break;
		}
	}

	// Find the new total size of the grid
	totalSizeOfGrid = dimensionOfGrid*dimensionOfGrid;

	// Create the grid that will be populated and used in the algorithm
	char city[totalSizeOfGrid];
	// Now, create the matrix that the 1d grid will be converted to
	char matrixCity[dimensionOfGrid][dimensionOfGrid];

	// Create the variables that will be used for the number of moves in the cycle and the happiness of the city
	int moves = 0;
	double happinessOfCity;

	// Call the function to fill the grid
	FillTheGrid(vacancy, endlineAgents, dimensionOfGrid, city);
	// Call the function to convert the grid to a matrix
	ConvertToMatrix(dimensionOfGrid, city, matrixCity);

	// Determine if the user wanted to use curses mode, or regular print mode
	if (numOfCycles != -1) {
		// Use regular printing mode
		for (int index = 0; index <= numOfCycles; index++) {
			// Set the number of moves and the current average happiness of the city
			moves = OpenPreviousVacancies(dimensionOfGrid, matrixCity);
			happinessOfCity = CityHappiness(dimensionOfGrid, matrixCity);

			// Print the city and create the next cycle for the algorithm
			PrintCity(dimensionOfGrid, matrixCity, index, moves, happinessOfCity, strengthOfPreference, vacancy, endlineAgents);
			
			// Create the next cycle of the grid
			CreateNextCycle(dimensionOfGrid, matrixCity, (strengthOfPreference/100.0));
		}
	} else {
		int cycleNumber = 0;
		initscr();
		while (1) {
			// Set the number of moves and the current average happiness of the city
			moves = OpenPreviousVacancies(dimensionOfGrid, matrixCity);
			happinessOfCity = CityHappiness(dimensionOfGrid, matrixCity);
			
			// Print the city and create the next cycle for the algorithem
			PrintCityUsingCursors(dimensionOfGrid, matrixCity, cycleNumber, moves, happinessOfCity, strengthOfPreference, vacancy, endlineAgents);

			// Delay the screen by the given (or default) delay time
			usleep(delayTime);

			// Create the next cycle of the grid
			CreateNextCycle(dimensionOfGrid, matrixCity, (strengthOfPreference/100.0));
			
			// Increment the number of cycles since one cycle passed
			cycleNumber++;
		}
		endwin();
	}

 	return 0;
}

/**
 * This function will fill the grid with a calculate amount of vacancies, end 
 * 	line characters, and new line characters. The grid will then be randomly
 * 	shuffled using the modern shuffle algorithm
 *
 * @param vacancies - the percentage of vacancies for the current grid
 * @param endLine - the percentage of end line userse
 * @param size - the size of one side in a 2-d array
 * @param grid - the 1d array 
 */ 
void FillTheGrid(int vacancies, int endlines, int size, char grid[]) {
	// Find the size of the 2d array in a 1d array form
	int sizeOfArray = size*size;

	// Create the long int that will be used for the random int in the shuffle algorithm
	long int randomNum;
	// Create a char that will be used as a temp when swapping two values
	char temp;

	// Convert the given percentages to decimals
	double vacancyDecimal = vacancies/100.0;
	double endlineDecimal = endlines/100.0;

	// Find the amount of the vacancies, end lines, and new lines
	int vacancyAmount = vacancyDecimal * sizeOfArray;
	int endLineAmount = (sizeOfArray - vacancyAmount) * endlineDecimal;
	int newLineAmount = (sizeOfArray - vacancyAmount) - endLineAmount;

	// Fill the grid with the proper amount of new lines, end lines, and vacancies
	for (int index = 0; index < sizeOfArray; index++) {
		if (endLineAmount > 0) {
			grid[index] = 'e';
			endLineAmount--;
		} else if (newLineAmount > 0) {
			grid[index] = 'n';
			newLineAmount--;
		} else {
			grid[index] = '.';
		}
	}

	// Use Fisher-Yates shuffle algorithm
	for (int i = 0; i < (sizeOfArray-2); i++) {
		// Create the first random integer and swap two values
		randomNum = (rand() % (sizeOfArray));
		temp = grid[randomNum];
		grid[randomNum] = grid[i];
		grid[i] = temp;
	}
}

/**
 * This function will read the neighbors of the current location on the grid.
 * 	It will calculate the ratio of the users who code like them to the other
 * 	non-vacant neighbors
 * 
 * @param row - the current row position in the grid
 * @param col - the current col position in the grid
 * @param size - the size of one side of the matrix
 * @param matrix - the 2d array of the city
 * @param test - this will be the character that is used to compare the agent with to find the happiness
 * @return - the ratio of the users who code like them, to the non-vacant ones who don't
 */
double HappinessMeasure(int row, int col, int size, char matrix[][size], char test) {
	// Create two integers for the counts
	int totalNeighbors = 0;
	int sameNeighbors = 0;

	// Search the top row of the neighbors of the agent for a similar agent to them
	if (row > 0) {
		if (col > 0 && matrix[row-1][col-1] != '.') {
			totalNeighbors++;
			if (matrix[row-1][col-1] == test) {
				sameNeighbors++;
			}
		}
		if (matrix[row-1][col] != '.') {
			totalNeighbors++;
			if (matrix[row-1][col] == test) {
				sameNeighbors++;
			}
		}
		if (col < (size-1) && matrix[row-1][col+1] != '.') {
			totalNeighbors++;
			if (matrix[row-1][col+1] == test) {
				sameNeighbors++;
			}
		}
	}

	// Search on the same row of the agent to look for a similar agent
	if (col > 0 && matrix[row][col-1] != '.') {
		totalNeighbors++;
		if (matrix[row][col-1] == test) {
			sameNeighbors++;
		}
	}

	if (col < (size-1) && matrix[row][col+1] != '.') {
		totalNeighbors++;
		if (matrix[row][col+1] == test) {
			sameNeighbors++;
		}
	}

	// Search on the bottom row the neighbors of the agent to look for a similar agent
	if (row < (size-1)) {
		if (col > 0 && matrix[row+1][col-1] != '.') {
			totalNeighbors++;
			if (matrix[row+1][col-1] == test) {
				sameNeighbors++;
			}
		}
		if (matrix[row+1][col] != '.') {
			totalNeighbors++;
			if (matrix[row+1][col] == test) {
				sameNeighbors++;
			}
		}
		if (col < (size-1) && matrix[row+1][col+1] != '.') {
			totalNeighbors++;
			if (matrix[row+1][col+1] == test) {
				sameNeighbors++;
			}
		}
	}

	// If there are no endlines or newlines, then the user will be 100% happy
	if (totalNeighbors == 0) {
		return 1.0;
	}
	// Otherwise, just return the ratio of the same neighbors to the total neighbors
	return (double)sameNeighbors/(double)totalNeighbors;
}

/**
 * Function to convert the 1d array to a matrix
 *
 * @param size - the size of one side of the matrix
 * @param grid - the grid that will be converted
 * @param matrix - the matrix that the grid will be converted into
 */
void ConvertToMatrix(int size, char grid[], char matrix[][size]) {
	// Create variables for the index of the 1d array as it is being searched
	int index = 0;
	// Create the temp matrix that will be copied in the actual matrix
	char tempMatrix[size][size];
	// Loop through and assign the variable to the temp matrix
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			tempMatrix[row][col] = grid[index++];
		}
	}
	// Copy the temp matrix over to the actual matrix
	memcpy(matrix, tempMatrix, sizeof(tempMatrix));
}

/**
 * Function that will move the current agent to a vacant sqaure, and make the square that he was just at, unusable
 *
 * @param row - the current row value of the matrix
 * @param col - the current col value of the matrix
 * @param size - the size of one side of the matrix
 * @param matrix - the 2d array of the city
 */
int MoveTheAgent(int row, int col, int size, char matrix[][size]) {
	char tempChar = matrix[row][col];
	char vacant;
	// Search the entire grid for a valid move location
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			vacant = matrix[i][j];
			// If the current spot is vacant, move the agent and set the spot so that another can't use it
			if (vacant == '.') {
				matrix[i][j] = tempChar;
				matrix[row][col] = '*';
				return 0;
			} 
		}
	}
	return -1;
}

/**
 * Function that will convert the matrix to a grid
 *
 * @param size - the size of one side of the matrix
 * @param grid - the 1d grid of the city
 * @param matrix - the 2d grid of the city
 */
void ConvertToGrid(int size, char grid[], char matrix[][size]) {
	int index = 0;
	char tempGrid[size];

	// Lop through each element in the matrix and put each value in a 1d grid
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			tempGrid[index] = matrix[row][col];
			index++;
		}
	}

	memcpy(grid, tempGrid, sizeof(tempGrid));
}

/**
 * Function that will open up the vacancies that were previously filled from the last cycle
 *
 * @param size - the size of one side of the matrix
 * @param matrix - the 2d grid of the city
 * @return - the total number of moves that were made during the last cycle
 */
int OpenPreviousVacancies(int size, char matrix[][size]) {
	int count = 0;
	
	// Loop through each element of the matrix and open the vacancies. Return a count for each vacancy opened this way
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			if (matrix[row][col] == '*') {
				matrix[row][col] = '.';
				count++;			
			}
		}
	}	

	return count;
}

/**
 * Function that will find the average happiness of the residents of the city
 *
 * @param size - the size of one side of the matrix
 * @param matrix - the 2d grid of the city
 * @return - the average happiness of each agent in the city (as a decimal)
 */
double CityHappiness(int size, char matrix[][size]) {
	double happiness = 0.0;
	int totalAgents = 0;

	// Loop through and find the total happiness of each non-vacant agent
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			if (matrix[row][col] == 'e' || matrix[row][col] == 'n') {
				double happy = HappinessMeasure(row, col, size, matrix, matrix[row][col]);
				happiness += happy;
				totalAgents++;
			}
		}
	}

	// Return the average happiness of each agent
	return happiness/totalAgents;
}

/**
 * Function that will create the next cycle by moving each of the agents, if needed
 *
 * @param size - the size of one size of the matrix
 * @param matrix - the 2d grid of the city
 * @param strength - this is the strength of preference that will be used to tell if a neighbor is happy or not
 */
void CreateNextCycle(int size, char matrix[][size],  double strength) {
	// Loop through and move the current agent to generate the next cycle of the program
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			if (matrix[row][col] == 'e' || matrix[row][col] == 'n') {
				//printf("Happiness: %f\n", HappinessMeasure(row, col, size, matrix, matrix[row][col]));
				if (HappinessMeasure(row, col, size, matrix, matrix[row][col]) < strength) {
					MoveTheAgent(row, col, size, matrix);
				}
			}
		}
	}
}

/**
 * Function to print the city data to the regular output
 *
 * @param size - the size of one side of the matrix
 * @param matrix - the 2d grid of the city
 * @param cycle - the current cycle that the program is on
 * @param moves - the number of moves that were done in the previous cycle
 * @param happiness - the average happiness of the city
 * @param strength - the strength of preference percentage
 * @param vacancy - the vacancy percentage
 * @param endline - the endline user percentage
 */
void PrintCity(int size, char matrix[][size], int cycle, int moves, double happiness, int strength, int vacancy, int endline){
	// Loop through and print the matrix 
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			printf("%c", matrix[row][col]);
		}
		printf("\n");
	}

	// Print each one of the other values for the matrix, and then tell the user how they can quit
	printf("cycle: %d\n", cycle);
	printf("moves this cycle: %d\n", moves);
	printf("teams' \"happiness\": %.4f\n", happiness);
	printf("dim: %d, %%strength of preference: %d%%, %%vacancy: %d%%, %%end: %d%%\n", size, strength, vacancy, endline);
}

/**
 * Function that will print the city in the same format as the PrintCity function, but this one will use the infinite
 * 	curses mode
 * @param size - the size of one side of the matrix
 * @param matrix - the 2d grid of the city
 * @param cycle - the current cycle that the program is on
 * @param moves - the number of moves made in the last cycle
 * @param happiness - the average happiness of the agents in the city
 * @param strength - the strength of preference percentage
 * @param vacancy - the vacancy percentage
 * @param endline - the endline user percentage
 */
void PrintCityUsingCursors(int size, char matrix[][size], int cycle, int moves, double happiness, int strength, int vacancy, int endline){ 
	move(0, 0);
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			printw("%c", matrix[row][col]);
		}
		printw("\n");
	}
	refresh();
	move(size, 0);
	printw("cycle: %d\n", cycle);
	printw("moves this cycle: %d\n", moves);
	printw("teams' \"happiness\": %.4f\n", happiness);
	printw("dim: %d, %%strength of preference:  %d%%, %%vacancy:  %d%%, %%end:  %d%%\n", size, strength, vacancy, endline);
	printw("Use Control-C to quit.\n");
	refresh();
}

/**
 * Function to print how the help should be displayed to stderr
 */
void PrintHelp() {
	fprintf(stderr,
		"usage:\n"
		"bracetopia [-h] [-t N] [-c N] [-d dim] [-s %%str] [-v %%vac] [-e %%end]\n"
		"Option      Default   Example   Description\n"
		"'-h'        NA        -h        print this usage message.\n"
		"'-t N'      900000    -t 5000   microseconds cycle delay.\n"
		"'-c N'      NA        -c4       count cycle maximum value.\n"
		"'-d dim'    15        -d 7      width and height dimension.\n"
		"'-s %%str'  50        -s 30     strength of preference.\n"
		"'-v %%vac'  20        -v30      percent vacancies.\n"
		"'-e %%endl' 60        -e75      percent Endline braces. Others want Newline.\n"
	       );
}

/**
 * Function to print the usage message to stderr
 */
void PrintUsage() {
	fprintf(stderr,
		"usage: \n"
		"bracetopia [-h] [-t N] [-c N] [-d dim] [-s %%str] [-v%%vac] [-e %%end]\n"
	       );
}
