# New ports collection makefile for:    fidod
# Date created:                         4 September 1999
# Whom:                                 Alex L. Demidov
#
# $FreeBSD$

PORTNAME=	fidod
PORTVERSION=	0.2.1
CATEGORIES= 	mail
MASTER_SITES=	http://home.vinf.ru/~alexd/files/

MAINTAINER=	alexd@vinf.ru

GNU_CONFIGURE=  yes
USE_GMAKE=	yes
USE_AUTOCONF=   yes

CONFIGURE_ARGS += --with-etcdir=${PREFIX}/etc/
CONFIGURE_ARGS += --with-ifmaildir=${PREFIX}/libexec/ifmail/

MAN8=	fidod.8

post-install:
	@ ${INSTALL_DATA} ${WRKSRC}/conf/fidod.conf ${PREFIX}/etc/fidod.conf.sample
.if !defined(NOPORTDOCS)
	@ ${MKDIR} ${PREFIX}/share/doc/fidod
.for file in NEWS README TODO
	@ ${INSTALL_DATA} ${WRKSRC}/${file} ${PREFIX}/share/doc/fidod
.endfor
.endif
	if [ ! -f ${PREFIX}/info/dir ]; then \
		${SED} -ne '1,/Menu:/p' /usr/share/info/dir > ${PREFIX}/info/dir; \
	fi
.for info in fidod.info
	install-info ${PREFIX}/info/${info} ${PREFIX}/info/dir
.endfor

.include <bsd.port.mk>
