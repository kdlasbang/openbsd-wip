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

#define PAGE_SIZE 4096

#include <err.h>
#include <fuse.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <machine/vmmvar.h>

#include "vmd.h"

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
int virtiofsclient_send_fuse_msg(struct vm_fsop *);

static uint64_t curseq;

int
virtiofsclient_send_fuse_msg(struct vm_fsop *buf)
{
	int fd, ret;
	uint64_t seq;

	seq = buf->seq;

	fd = open("/dev/vmm", O_RDWR);
	if (fd == -1)
		err(1, "open /dev/vmm");

	printf("%s: calling FSOP_PUT ioctl for opcode %d seq %lld\n", __func__,
	    buf->opcode, buf->seq);

	ret = ioctl(fd, VMM_IOC_FSOP_PUT, buf);
	if (ret)
		err(1, "ioctl");

	printf("%s: after FSOP_PUT ioctl for seq %lld, new seq=%lld\n", __func__, seq, buf->seq);

	close(fd);

	return ret;
}

int
virtiofsclient_getattr(const char *path, struct stat *statbuf)
{
	struct vm_fsop_getattr ga, *retbuf;
	struct vm_fsop op;
	int ret;

	printf("%s: called\n", __func__);

	op.opcode = VMMFSOP_GETATTR;
	op.seq = ++curseq;
	strlcpy((char *)&ga.name, path, 256);
	memcpy(&op.payload, &ga, sizeof(ga));

	ret = virtiofsclient_send_fuse_msg(&op);
	if (!ret) {
		retbuf = (struct vm_fsop_getattr *)&op.payload;

		memcpy(statbuf, &retbuf->statbuf, sizeof(*statbuf));
		return retbuf->err;
	}

	return ret;
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




/*Doesn't work properly*/
int
virtiofsclient_mknod(const char *path, mode_t mode, dev_t rdev)
{
    struct vm_fsop_mknod mkn, *retbuf;
    struct vm_fsop op;
    int ret;

    printf("%s: called\n", __func__);

    op.opcode = VMMFSOP_MKNOD;
    op.seq = ++curseq;
    strlcpy((char *)&mkn.name, path, 256);
    mkn.mode = mode;
    mkn.dev = rdev;
    memcpy(&op.payload, &mkn, sizeof(mkn));

    ret = virtiofsclient_send_fuse_msg(&op);
    if (!ret) {
        retbuf = (struct vm_fsop_mknod *)&op.payload;
        return retbuf->err;
    }

    return ret;

}

int
virtiofsclient_mkdir(const char *path, mode_t mode)
{
	struct vm_fsop_mkdir md, *retbuf;
	struct vm_fsop op;
	int ret;

	printf("%s: called\n", __func__);

	op.opcode = VMMFSOP_MKDIR;
	op.seq = ++curseq;
	strlcpy((char *)&md.name, path, 256);
	md.mode = mode;
	memcpy(&op.payload, &md, sizeof(md));

	ret = virtiofsclient_send_fuse_msg(&op);
	if (!ret) {
		retbuf = (struct vm_fsop_mkdir *)&op.payload;

		return retbuf->err;
	}

	return ret;
}

int
virtiofsclient_unlink(const char *path)
{
	warnx("unimplemented function %s for path %s", __func__, path);

	return -EIO;
}


/*Work properly*/
int
virtiofsclient_rmdir(const char *path)
{
    struct vm_fsop_rmdir rmd, *retbuf;
    struct vm_fsop op;
    int ret;

    printf("%s: called\n", __func__);

    op.opcode = VMMFSOP_RMDIR;
    op.seq = ++curseq;
    strlcpy((char *)&rmd.name, path, 256);
    memcpy(&op.payload, &rmd, sizeof(rmd));

    ret = virtiofsclient_send_fuse_msg(&op);
    if (!ret) {
        retbuf = (struct vm_fsop_rmdir *)&op.payload;
        return retbuf->err;
    }

    return ret; 
}

int
virtiofsclient_symlink(const char *from, const char *to)
{
	warnx("unimplemented function %s for from=%s to=%s", __func__,
	    from, to);

	return -EIO;
}



/*Work properly*/
int
virtiofsclient_rename(const char *from, const char *to)
{
    struct vm_fsop_rename rn, *retbuf;
    struct vm_fsop op;
    int ret;

    printf("%s: called\n", __func__);

    op.opcode = VMMFSOP_RENAME;
    op.seq = ++curseq;
    strlcpy((char *)&rn.old_path, from, 256);
    strlcpy((char *)&rn.new_path, to, 256);
    memcpy(&op.payload, &rn, sizeof(rn));

    ret = virtiofsclient_send_fuse_msg(&op);
    if (!ret) {
        retbuf = (struct vm_fsop_rename *)&op.payload;
        return retbuf->err;
    }

    return ret;

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



/*Doesn't work properly*/
int
virtiofsclient_truncate(const char *path, off_t size)
{
    struct vm_fsop_truncate tru, *retbuf;
    struct vm_fsop op;
    int ret;

    printf("%s: called\n", __func__);

    op.opcode = VMMFSOP_TRUNCATE;
    op.seq = ++curseq;
    tru.size = size;
    strlcpy((char *)&tru.name, path, 256);
    memcpy(&op.payload, &tru, sizeof(tru));

    ret = virtiofsclient_send_fuse_msg(&op);
    if (!ret) {
        retbuf = (struct vm_fsop_truncate *)&op.payload;
        return retbuf->err;
    }

    return ret;
}

int
virtiofsclient_utime(const char *file, struct utimbuf *timep)
{
	warnx("unimplemented function %s for file %s", __func__, file);

	return -EIO;
}


/*Doesn't work properly*/
int
virtiofsclient_open(const char *path, struct fuse_file_info *fi)
{
    struct vm_fsop_open on, *retbuf;
    struct vm_fsop op;
    int ret;

    printf("%s: called\n", __func__);
    printf("virtiofsclient_open() fi->falgs=%d\n", fi->flags);
	
    op.opcode = VMMFSOP_OPEN;
    op.seq = ++curseq;
    strlcpy((char *)&on.name, path, 256);
    on.fi = *fi;
    printf("virtiofsclient_open() on->fi.falgs=%d\n", on.fi.flags);
    memcpy(&op.payload, &on, sizeof(on));

    ret = virtiofsclient_send_fuse_msg(&op);
    if (!ret) {
        retbuf = (struct vm_fsop_open *)&op.payload;

        memcpy(fi, &retbuf->fi, sizeof(*fi));
        return retbuf->err;
    }

    return ret;

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
	struct vm_fsop_statfs ga, *retbuf;
	struct vm_fsop op;
	int ret;

	printf("%s: called\n", __func__);

	op.opcode = VMMFSOP_STATFS;
	op.seq = ++curseq;
	strlcpy((char *)&ga.name, path, 256);
	memcpy(&op.payload, &ga, sizeof(ga));

	ret = virtiofsclient_send_fuse_msg(&op);
	if (!ret) {
		retbuf = (struct vm_fsop_statfs *)&op.payload;

		memcpy(buf, &retbuf->statvfs, sizeof(*buf));
		printf("%s: f_bsize = %ld\n", __func__, retbuf->statvfs.f_bsize);
		printf("%s: f_blocks = %lld\n", __func__, retbuf->statvfs.f_blocks);
		printf("%s: f_bavail = %lld\n", __func__, retbuf->statvfs.f_bavail);
		return retbuf->err;
	}

	return ret;
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
	struct vm_fsop_opendir od, *retbuf;
        struct vm_fsop op;
        int ret;

        printf("%s: called\n", __func__);

        op.opcode = VMMFSOP_OPENDIR;
        op.seq = ++curseq;
        strlcpy((char *)&od.name, path, 256);
        memcpy(&op.payload, &od, sizeof(od));

        ret = virtiofsclient_send_fuse_msg(&op);
        if (!ret) {
                retbuf = (struct vm_fsop_opendir *)&op.payload;
                memcpy(fi, &retbuf->fi, sizeof(*fi));
                return retbuf->err;
        }

        return ret;
//	warnx("unimplemented function %s for path %s", __func__, path);

//	return -EIO;
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



/*Doesn't work properly*/
int
virtiofsclient_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    struct vm_fsop_create ca, *retbuf;
    struct vm_fsop op;
    int ret;

    printf("%s: called\n", __func__);

    op.opcode = VMMFSOP_CREATE;
    op.seq = ++curseq;
    strlcpy((char *)&ca.name, path, 256);
    ca.mode = mode;
    memcpy(&op.payload, &ca, sizeof(ca));

    ret = virtiofsclient_send_fuse_msg(&op);
    if (!ret) {
        retbuf = (struct vm_fsop_create *)&op.payload;

        memcpy(fi, &retbuf->fi, sizeof(*fi));

        return retbuf->err;
    }

    return ret;

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
