/* $OpenBSD$ */
/*
 * Copyright (c) 2021 Mike Larkin <mlarkin@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <fuse.h>
#include <string.h>

#include <sys/errno.h>
#include <sys/stat.h>

int virtiofsclient_getattr(const char *, struct stat *);
void *virtiofsclient_init(struct fuse_conn_info *);
int virtiofsclient_readlink(const char *, char *, size_t);
int virtiofsclient_mknod(const char *, mode_t, dev_t);
int virtiofsclient_mkdir(const char *, mode_t);
int virtiofsclient_unlink(const char *);
int virtiofsclient_rmdir(const char *);
int virtiofsclient_symlink(const char *, const char *);
int virtiofsclient_rename(const char *, const char *);
int virtiofsclient_link(const char *, const char *);
int virtiofsclient_chmod(const char *, mode_t);
int virtiofsclient_chown(const char *, uid_t, gid_t);
int virtiofsclient_truncate(const char *, off_t);
int virtiofsclient_utime(const char *, struct utimbuf *);
int virtiofsclient_open(const char *, struct fuse_file_info *);
int virtiofsclient_read(const char *, char *, size_t, off_t,
    struct fuse_file_info *);
int virtiofsclient_write(const char *, const char *, size_t, off_t,
    struct fuse_file_info *);
int virtiofsclient_statfs(const char *, struct statvfs *);
int virtiofsclient_flush(const char *, struct fuse_file_info *);
int virtiofsclient_release(const char *, struct fuse_file_info *);
int virtiofsclient_fsync(const char *, int, struct fuse_file_info *);
int virtiofsclient_opendir(const char *, struct fuse_file_info *);
int virtiofsclient_readdir(const char *, void *, fuse_fill_dir_t, off_t,
    struct fuse_file_info *);
int virtiofsclient_releasedir(const char *, struct fuse_file_info *);
int virtiofsclient_fsyncdir(const char *, int, struct fuse_file_info *);
void virtiofsclient_destroy(void *);
int virtiofsclient_access(const char *, int);
int virtiofsclient_create(const char *, mode_t, struct fuse_file_info *);
int virtiofsclient_ftruncate(const char *, off_t, struct fuse_file_info *);
int virtiofsclient_fgetattr(const char *, struct stat *,
    struct fuse_file_info *);
int virtiofsclient_utimens(const char *, const struct timespec *);

int
virtiofsclient_getattr(const char *path, struct stat *statbuf)
{
	warnx("unimplemented function %s for path %s", __func__, path);

/* XXX testing */
	memset(statbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		statbuf->st_mode = S_IFDIR | 0755;
		statbuf->st_nlink = 2;
		return 0;
	}
/* XXX */

	return -EIO;
}

void *
virtiofsclient_init(struct fuse_conn_info *conn)
{
	return NULL;
}

int
virtiofsclient_readlink(const char *path, char *buf, size_t size)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_mknod(const char *path, mode_t mode, dev_t rdev)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_mkdir(const char *path, mode_t mode)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_unlink(const char *path)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_rmdir(const char *path)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_symlink(const char *from, const char *to)
{
	warnx("unimplemented function %s for from=%s to=%s", __func__,
	    from, to);

	return -EIO;
}

int
virtiofsclient_rename(const char *from, const char *to)
{
	warnx("unimplemented function %s for from=%s to=%s", __func__,
	    from, to);

	return -EIO;
}

int
virtiofsclient_link(const char *from, const char *to)
{
	warnx("unimplemented function %s for from=%s to=%s", __func__,
	    from, to);

	return -EIO;
}

int
virtiofsclient_chmod(const char *path, mode_t mode)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_chown(const char *path, uid_t uid, gid_t gid)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_truncate(const char *path, off_t size)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_utime(const char *file, struct utimbuf *timep)
{
	warnx("unimplemented function %s for file %s", __func__, file);

	return -EIO;
}

int
virtiofsclient_open(const char *path, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_read(const char *path, char *buf, size_t size,
    off_t offset, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_write(const char *path, const char *buf, size_t size,
    off_t offset, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_statfs(const char *path, struct statvfs *buf)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_flush(const char *path, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_release(const char *path, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_opendir(const char *path, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_readdir(const char *path, void *buf, fuse_fill_dir_t fillfn,
    off_t offset, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_releasedir(const char *path, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

void
virtiofsclient_destroy(void *cookie)
{
	warnx("unimplemented function %s", __func__);
}

int
virtiofsclient_access(const char *path, int amode)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_ftruncate(const char *path, off_t offset,
    struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_fgetattr(const char *path, struct stat *statbuf,
    struct fuse_file_info *fi)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

int
virtiofsclient_utimens(const char *path, const struct timespec *times)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}

const struct fuse_operations virtiofsclient_op = {
	.getattr	= virtiofsclient_getattr,
	.readlink	= virtiofsclient_readlink,
	.mknod		= virtiofsclient_mknod,
	.mkdir		= virtiofsclient_mkdir,
	.unlink		= virtiofsclient_unlink,
	.rmdir		= virtiofsclient_rmdir,
	.symlink	= virtiofsclient_symlink,
	.rename		= virtiofsclient_rename,
	.link		= virtiofsclient_link,
	.chmod		= virtiofsclient_chmod,
	.chown		= virtiofsclient_chown,
	.truncate	= virtiofsclient_truncate,
	.utime		= virtiofsclient_utime,
	.open		= virtiofsclient_open,
	.read		= virtiofsclient_read,
	.write		= virtiofsclient_write,
	.statfs		= virtiofsclient_statfs,
	.flush		= virtiofsclient_flush,
	.release	= virtiofsclient_release,
	.fsync		= virtiofsclient_fsync,
	.opendir	= virtiofsclient_opendir,
	.readdir	= virtiofsclient_readdir,
	.releasedir	= virtiofsclient_releasedir,
	.fsyncdir	= virtiofsclient_fsyncdir,
	.init		= virtiofsclient_init,
	.destroy	= virtiofsclient_destroy,
	.access		= virtiofsclient_access,
	.create		= virtiofsclient_create,
	.ftruncate	= virtiofsclient_ftruncate,
	.fgetattr	= virtiofsclient_fgetattr,
	.utimens	= virtiofsclient_utimens,
};

int
main(int argc, char **argv)
{
	return fuse_main(argc, argv, &virtiofsclient_op, NULL);
}
