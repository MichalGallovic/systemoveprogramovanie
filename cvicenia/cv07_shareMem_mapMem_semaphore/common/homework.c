#include "homework.h"
#include <stdlib.h>
#include "utility.h"
#include <time.h>
#include <limits.h>

void PrintHomework(Homework * homework)
{
	print("%d + %d = %d\n", homework->augend, homework->addend, homework->sum);
}

void GenerateRandomHomework(Homework * homework)
{
	srand(time(NULL));
	homework->augend = rand() % 10;
	homework->addend = rand() % 10;
	homework->sum = INT_MIN; //namiesto vymazania tejto polozky
}

void DoHomework(Homework * homework)
{
	homework->sum = homework->augend + homework->addend;
}
