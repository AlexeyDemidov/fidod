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

FIDO_USER=	fido
FIDO_GROUP=	fido

MAN8=	fidod.8

pre-install:
	${SETENV} ${MAKE_ENV} perl ${SCRIPTDIR}/createuser
	${INSTALL} -d -o ${FIDO_USER} -g ${FIDO_GROUP} -m 751 /var/spool/fido
	${INSTALL} -d -o ${FIDO_USER} -g ${FIDO_GROUP} -m 751 /var/spool/fido/inb
	${INSTALL} -d -o ${FIDO_USER} -g ${FIDO_GROUP} -m 751 /var/spool/fido/inb.prot
	${INSTALL} -d -o ${FIDO_USER} -g ${FIDO_GROUP} -m 751 /var/log/fido

post-install:
	${INSTALL_SCRIPT} ${FILESDIR}/fidod.sh ${PREFIX}/etc/rc.d/fidod.sh.sample
	${INSTALL_DATA} ${WRKSRC}/conf/fidod.conf ${PREFIX}/etc/fidod.conf.sample
.if !defined(NOPORTDOCS)
	${MKDIR} ${PREFIX}/share/doc/fidod
.for file in NEWS README TODO
	${INSTALL_DATA} ${WRKSRC}/${file} ${PREFIX}/share/doc/fidod
.endfor
.endif
	if [ ! -f ${PREFIX}/info/dir ]; then \
		${SED} -ne '1,/Menu:/p' /usr/share/info/dir > ${PREFIX}/info/dir; \
	fi
.for info in fidod.info
	install-info ${PREFIX}/info/${info} ${PREFIX}/info/dir
.endfor

.include <bsd.port.mk>
