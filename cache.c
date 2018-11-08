#define _GNU_SOURCE

#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>


#define CACHE_LINE_SIZE  64
#define TOTAL_ITERATIONS 100000000
#define CACHE_SHIFT 6
#define MEDIAN 3

char *heap_ptr = NULL;
int *cache_map = NULL;
long long global_sum = 0;
int heap_size = 1;   /* heap size in KB */
size_t size_in_bytes = 1024;
size_t num_cache_lines = 1024 / CACHE_LINE_SIZE;

static void access_cache_line(int x)
{
	global_sum += heap_ptr[x << CACHE_SHIFT];
}

static unsigned long long rdtsc()
{
	unsigned hi, lo;

	asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
	return (((unsigned long long)hi) << 32) | lo;
}

static unsigned long long run_workload()
{
	unsigned long long time;
	long long i, j, num_iterations;


	/* bring everything to the cache */
	for (j = 0; j < num_cache_lines; j++) {
		heap_ptr[j << CACHE_SHIFT] = rand() % 256; 
	}

	num_iterations = TOTAL_ITERATIONS / num_cache_lines;

	time = rdtsc();

	for (i = 0; i < num_iterations; i++) {
		for (j = 0; j < num_cache_lines; j++) {
			access_cache_line(cache_map[j]);
		}
	}

	return rdtsc() - time;
}

int main(int argc, const char *argv[])
{
	unsigned long long median[MEDIAN];
	unsigned long long time;
	long long i, j, k;
	size_t cache_map_size, cache_size, total_size;
	char *cache_temp;
	int *cache_ptr;
	int idx, idx1;
	cpu_set_t cpu;

	CPU_ZERO(&cpu);
	CPU_SET(0, &cpu);
	if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpu) == -1) {
		printf("Unable to set CPU affinity!\n");
		return 0;
	}

	if (argc == 2) {
		heap_size = atoi(argv[1]);
		if (heap_size <= 0) {
			printf("unsupported heap size %d\n", heap_size);
			return 0;
		}
		size_in_bytes = ((size_t)heap_size) << 10;
		num_cache_lines = size_in_bytes / CACHE_LINE_SIZE;
	}

	assert(size_in_bytes > 0);
	assert(num_cache_lines > 0);

	heap_ptr = (char*)mmap(NULL, size_in_bytes, PROT_READ|PROT_WRITE,
	                       MAP_PRIVATE|MAP_ANON, -1, 0);

	if (heap_ptr == MAP_FAILED) {
		printf("not able to allocate memory %zd bytes\n", size_in_bytes);
		return 0;
	}

	cache_map_size = num_cache_lines * sizeof(int);

	cache_ptr = (int*)malloc(cache_map_size + CACHE_LINE_SIZE);

	if (cache_ptr == NULL) {
		printf("not able to allocate memory2");
		return 0;
	}
	cache_map = 
		(int*)(((unsigned long)cache_ptr + CACHE_LINE_SIZE) & ~(CACHE_LINE_SIZE-1));

	cache_temp = (char*)malloc(num_cache_lines);

	if (cache_temp == NULL) {
		printf("not able to allocate memory3");
		return 0;
	}

	memset(cache_temp, 0, num_cache_lines);

	/* add randomness to reduce the effect of hardware cache prefetch */
	for (i = 0; i < num_cache_lines; i++) {
		idx = rand() % num_cache_lines;
		if (cache_temp[idx] == 1) {
			for (j = 1; j < num_cache_lines; j++) {
				idx1 = (idx + j) % num_cache_lines;
				if (cache_temp[idx1] == 0) {
					idx = idx1;
					break;
				}
			}
		}
		assert(idx < num_cache_lines);
		assert(cache_temp[idx] == 0);
		cache_temp[idx] = 1;
		cache_map[i] = idx;
	}

	free(cache_temp);

	for (i = 0; i < MEDIAN; i++) {
		time = run_workload();
		for (j = 0; j < i; j++) {
			if (median[j] > time) {
				for (k = i-1; k >= j; k--) {
					median[k+1] = median[k];
				}
				median[j] = time;
				goto out;
			}
		}
		median[i] = time;
out:
		time = 0;
	}

	free(cache_ptr);
	munmap(heap_ptr, size_in_bytes);


	cache_size = num_cache_lines * CACHE_LINE_SIZE;
	total_size = cache_size + cache_map_size;

	printf("Cache SZ:%zd(KB) time:%lld(ms) num-random-loads:%lld\n",
		total_size >> 10, median[((MEDIAN+1)/2) - 1]/2530000, (long long)MEDIAN * TOTAL_ITERATIONS);

	return global_sum;
}
