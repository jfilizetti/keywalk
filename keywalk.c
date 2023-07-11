/*
 * This is in no way a finalized product but rather just a tool for evaluation.
 * However, the basic structure here is to fulfill that need and provide a baseline
 * for a real implmentation.
 */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int debug;

struct position {
	char x;
	char y;
} __attribute__((packed));

char keys[4][14] = {
	"`1234567890-=\0",
	"\0qwertyuiop[]\\", 
	"\0asdfghjkl;'\0\0",
	"\0zxcvbnm,./\0\0\0"
};
char keys_caps[4][14] = {
	"~!@#$%^&*()_+\0",
	"\0QWERTYUIOP{}|",
	"\0ASDFGHJKL:\"\0\0",
	"\0ZXCVBNM<>?\0\0\0"
};

void print_table()
{
	int x, y, c;
	for (y = 0; y < 4; y++) {
		for (x = 0; x < 14; x++) {
			c = keys[y][x];
			printf(" %c ",  c == '\0' ? ' ' : c);
		}
		printf("\n");
	}
	for (y = 0; y < 4; y++) {
		for (x = 0; x < 14; x++) {
			c = keys_caps[y][x];
			printf(" %c ",  c == '\0' ? ' ' : c);
		}
		printf("\n");
	}
}

int get_position(struct position *p, char c)
{
	for (p->y = 0; p->y < 4; p->y++) {
		for (p->x = 0; p->x < 14; p->x++) {
			if (c == keys[p->y][p->x] || c == keys_caps[p->y][p->x])
				return 1;
		}
	}
	return 0;
}

int check_word(char *word, char *buffer, size_t maxlen)
{
	struct position pos, last; 
	char *ptr;
	int i, x, y, valid, total, distance;
	size_t len;

	total = 0;
	distance = 0;
	valid = 0;
	memset(buffer, 0, maxlen);
	ptr = buffer;
	len = strlen(word);

	memset(&pos, 0, sizeof(pos));
	for (i = 0; i < len && valid < maxlen; i++) {
		/* For now if we ignore unknown characters */
		if (!get_position(&pos, word[i])) {
			if (debug)
				fprintf(stderr, "Invalid character (%x) for password: %s at %d\n", word[i], word, i);
			continue;
		}

		if (valid > 0) {
			x = abs(pos.x - last.x);
			y = abs(pos.y - last.y);
			distance = x + y;
			total += distance;
			*ptr++ = distance;
		}
		last = pos;
		valid++;
	}

	/* potential overflow */
	if (valid == maxlen)
		return -1;

	return ptr - buffer;
}

int count_values(char *distances, int dlen, int value)
{
	int i;
	int count = 0;

	for (i = 0; i < dlen; i++) {
		if (distances[i] == value)
			count++;
	}

	return count;
}

int find_sequences(char *distances, int dlen)
{
	int i, j, k, l;
	int match = 0;
	int seqlen, half;
	int sstart, mstart;
	int max = 1;
	int total = 0;

	/* Search for sequences from 2 to half the size */
	half = dlen / 2;
	for (seqlen = 2; seqlen <= half; seqlen++) {
		/* Start a sliding window starting at the start and working towards the end */
		for (sstart = 0; (sstart + seqlen) < (dlen - seqlen); sstart++) {
			match = 1;
			/* sliding window for the match */
			for (mstart = (sstart + seqlen); mstart < dlen; ) {
				/* match as much as we can */
				for (i = 0; distances[sstart + i] == distances[mstart + i] && i < seqlen && mstart + i < dlen; i++);
				
				/* if we reached seqlen it's a match */
				if (i == seqlen) {
					if (debug) {
					printf("(%d-%d) ", sstart, + sstart + (i-1));
						for (j = sstart; j < sstart + seqlen; j++) 
							printf("%d ", distances[j]);
						printf("  match (%d-%d)\n", mstart, mstart + (i-1));
					}
					match += 1;
					mstart += i;
				} else {
					mstart += i + 1;
				}
			}
			total = seqlen * match;
			if (total > max) {
				if (debug)
					printf("new max: total=%d max=%d seqlen=%d match=%d\n", total, max, seqlen, match);
				max = total;
			}
		}
	}


	return max;
}

void print_summary(char *word, char *distances, int dist_len, int seq_total, int zeros, int ones)
{
	char buffer[16384], *ptr;
	int i, blen, slen;
	float zeros_ratio, ones_ratio, seq_ratio;

	ptr = buffer;
	for (i = 0; i < dist_len; i++) {
		if (i)
			blen = sprintf(ptr, ",%d", distances[i]);
		else
			blen = sprintf(ptr, "%d", distances[i]);
		ptr += blen;
	}

	slen = strlen(word);
	zeros_ratio = ((float) zeros / (float) dist_len);
	ones_ratio = ((float) ones / (float) dist_len);
	seq_ratio = ((float) seq_total / (float) dist_len);
	printf("%s|%s|%d|%d|%.2f|%.2f|%.2f\n", word, buffer, slen, dist_len, zeros_ratio, ones_ratio, seq_ratio);
}

int main(int argc, char **argv)
{
	int i, x, y, total, distance;
	int fd, rc, slen, lseq;
	struct position pos, last;
	size_t len, remaining;
	char *buffer, *ptr, *word;
	char summary[4096];
	struct stat info;
	int good = 0, bad = 0;
	int ones, zeros;
	int c;

	while ((c = getopt(argc, argv, "d")) != -1) {
		switch (c) {
		case 'd':
			debug = 1;
			break;
		default:
			fprintf(stderr, "Invalid optioni: %c\n", c);
			exit(EXIT_FAILURE);
		}
	}

	if (argc - optind != 1)
		return EXIT_FAILURE;

	fd = open(argv[optind], O_RDONLY);
	if (fd == -1)
		return EXIT_FAILURE;

	rc = fstat(fd, &info);
	buffer = malloc(info.st_size);
	if (!buffer)
		return EXIT_FAILURE;
	memset(buffer, 0, info.st_size);

	remaining = info.st_size;
	ptr = buffer;
	while (remaining > 0) {
		len = read(fd, ptr, remaining);
		if (len == -1)
			return errno;
		ptr += len;
		remaining -= len;
	}
	close(fd);

	printf("word|distances|word-len|dist-len|zeros-ratio|ones-ratio|seq-ratio\n");
	ptr = buffer;
	while (*ptr) {
		word = strsep(&ptr, "\n");
		len = strlen(word);
		word[len] = '\0';
		if ((index(word, '|'))) {
			bad++;
			continue;
		}

		slen = check_word(word, summary, sizeof(summary));
		if (slen == -1) {
			bad++;
			continue;
		}

		good++;

		lseq = find_sequences(summary, slen);
		zeros = count_values(summary, slen, 0);
		ones = count_values(summary, slen, 1);
		print_summary(word, summary, slen, lseq, zeros, ones);
	}
}
