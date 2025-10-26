#include <stdio.h>
#include "../../include/colors.h"
#include "../../include/test.h"

#define EXPECTED_RESULT 42
#define FAIL_RESULT     1

typedef struct s_testStats
{
	int total;
	int passed;
} t_testStats;

static void printStatus(const char *msg, int success)
{
	printf("%s[%s]%s %s\n", success ? GREEN : RED, success ? "PASS" : "FAIL", RESET, msg);
}

static void runTest(const char *desc, int (*func)(void), int expected, t_testStats *stats)
{
	int result;

	stats->total++;
	result = func();
	if (result == expected)
	{
		stats->passed++;
		printStatus(desc, 1);
	}
	else
		printStatus(desc, 0);
}

static void runTests(t_testStats *stats)
{
	// Add more tests here as needed
	runTest("testReturn() should return 42", test, EXPECTED_RESULT, stats);
	runTest("testFail() should return 1", test2, FAIL_RESULT, stats);
}

int main(void)
{
	t_testStats stats = {0, 0};

	printf(CYAN "Starting tests...\n" RESET);

	runTests(&stats);

	printf(CYAN "\nSummary:\n" RESET);
	printf("Tests run: %d\n", stats.total);
	printf("Tests passed: %d\n", stats.passed);
	printf("Success rate: %.2f%%\n", stats.total ? (stats.passed * 100.0 / stats.total) : 0.0);

	if (stats.passed == stats.total)
		printf(GREEN "ALL TESTS PASSED\n" RESET);
	else
		printf(RED "SOME TESTS FAILED\n" RESET);

	return 0;
}
