# $OpenBSD$

.if ${MACHINE} == "amd64"

PROG=		virtiofsclientd
SRCS=		virtiofsclientd.c

CFLAGS+=	-Wall -I${.CURDIR}
CFLAGS+=	-Wstrict-prototypes -Wmissing-prototypes
CFLAGS+=	-Wmissing-declarations
CFLAGS+=	-Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+=	-Wsign-compare

LDADD+=		-lfuse
DPADD+=		${LIBFUSE}

.else

NOPROG= yes

.endif

# MAN=		virtiofsclientd.8
MAN=

.include <bsd.prog.mk>
