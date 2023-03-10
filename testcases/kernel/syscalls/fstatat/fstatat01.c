/*
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) International Business Machines  Corp., 2006
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * DESCRIPTION
 *	This test case will verify basic function of fstatat64/newfstatat
 *	added by kernel 2.6.16 or up.
 *
 * Author
 *	Yi Yang <yyangcdl@cn.ibm.com>
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "config.h"
#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

#define TEST_CASES 6
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

void setup();
void cleanup();

char *TCID = "fstatat01";
int TST_TOTAL = TEST_CASES;

static const char pathname[] = "fstatattestdir",
		  testfile[] = "fstatattestfile.txt",
		  testfile2[] = "fstatattestdir/fstatattestfile.txt";
static char *testfile3;

static int fds[TEST_CASES];
static const char *filenames[TEST_CASES];
static const int expected_errno[] = { 0, 0, ENOTDIR, EBADF, EINVAL, 0 };
static const int flags[] = { 0, 0, 0, 0, 9999, 0 };

#if !defined(HAVE_FSTATAT)
#if (__NR_fstatat64 > 0)
int fstatat(int dirfd, const char *filename, struct stat64 *statbuf, int flags)
{
	return tst_syscall(__NR_fstatat64, dirfd, filename, statbuf, flags);
}
#elif (__NR_newfstatat > 0)
int fstatat(int dirfd, const char *filename, struct stat *statbuf, int flags)
{
	return tst_syscall(__NR_newfstatat, dirfd, filename, statbuf, flags);
}
#else
int fstatat(int dirfd, const char *filename, struct stat *statbuf, int flags)
{
	return tst_syscall(__NR_fstatat, dirfd, filename, statbuf, flags);
}
#endif
#endif

int main(int ac, char **av)
{
	int lc, i;
#if !defined(HAVE_FSTATAT) && (__NR_fstatat64 > 0)
	static struct stat64 statbuf;
#else
	static struct stat statbuf;
#endif

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(fstatat(fds[i], filenames[i], &statbuf, flags[i]));

			if (TEST_ERRNO == expected_errno[i]) {
				tst_resm(TPASS | TTERRNO,
					 "fstatat failed as expected");
			} else
				tst_resm(TFAIL | TTERRNO, "fstatat failed");
		}

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	char *abs_path = tst_get_tmpdir();

	SAFE_ASPRINTF(cleanup, &testfile3, "%s/fstatattestfile3.txt", abs_path);
	free(abs_path);

	SAFE_MKDIR(cleanup, pathname, 0700);

	fds[0] = SAFE_OPEN(cleanup, pathname, O_DIRECTORY);
	fds[1] = fds[4] = fds[0];

	SAFE_FILE_PRINTF(cleanup, testfile, testfile);
	SAFE_FILE_PRINTF(cleanup, testfile2, testfile2);

	fds[2] =  SAFE_OPEN(cleanup, testfile3, O_CREAT | O_RDWR, 0600);

	fds[3] = 100;
	fds[5] = AT_FDCWD;

	filenames[0] = filenames[2] = filenames[3] = filenames[4] =
	    filenames[5] = testfile;
	filenames[1] = testfile3;

	TEST_PAUSE;
}

void cleanup(void)
{
	if (fds[0] > 0)
		close(fds[0]);
	if (fds[2] > 0)
		close(fds[2]);

	free(testfile3);
	tst_rmdir();
}
